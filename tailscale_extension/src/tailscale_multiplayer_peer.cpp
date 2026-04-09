#include "tailscale_multiplayer_peer.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

// System includes for UDP sockets
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <random>

namespace godot {

void TailscaleMultiplayerPeer::_bind_methods() {
    ClassDB::bind_method(D_METHOD("start_server", "port"), &TailscaleMultiplayerPeer::start_server, DEFVAL(9999));
    ClassDB::bind_method(D_METHOD("connect_to_peer", "address", "port"), &TailscaleMultiplayerPeer::connect_to_peer, DEFVAL(9999));
    ClassDB::bind_method(D_METHOD("get_tailscale_ip"), &TailscaleMultiplayerPeer::get_tailscale_ip);
    ClassDB::bind_method(D_METHOD("is_connected"), &TailscaleMultiplayerPeer::is_connected);
}

TailscaleMultiplayerPeer::TailscaleMultiplayerPeer() {
    // Generate random unique ID
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 9999);
    unique_id = dis(gen);
    
    is_server_mode = false;
    target_peer = 1;
    transfer_mode = MultiplayerPeer::TRANSFER_MODE_RELIABLE;
    transfer_channel = 0;
    connected = false;
    current_packet_peer = 0;
    udp_socket = -1;
    udp_port = 0;
    connected_port = 0;
    
    // Generate a fake Tailscale IP
    tailscale_ip = "100.64." + String::num_int64(dis(gen) % 256) + "." + String::num_int64(dis(gen) % 256);
}

TailscaleMultiplayerPeer::~TailscaleMultiplayerPeer() {
    _close();
}

void TailscaleMultiplayerPeer::_poll() {
    if (udp_socket < 0 || !connected) {
        return;
    }
    
    // Set socket to non-blocking for polling
    int flags = fcntl(udp_socket, F_GETFL, 0);
    fcntl(udp_socket, F_SETFL, flags | O_NONBLOCK);
    
    uint8_t buffer[1400];
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);
    
    while (true) {
        int len = recvfrom(udp_socket, buffer, sizeof(buffer), 0, 
                          (struct sockaddr*)&from_addr, &from_len);
        if (len <= 0) break;
        
        // Create packet and add to queue
        PackedByteArray packet;
        packet.resize(len);
        memcpy(packet.ptrw(), buffer, len);
        packet_queue.push_back(packet);
        
        // Store who sent this packet (for _get_packet_peer)
        current_packet_peer = 1; // Simplified: peer ID 1
        
        UtilityFunctions::print("Received ", len, " bytes from peer");
    }
}

void TailscaleMultiplayerPeer::_close() {
    if (udp_socket >= 0) {
        ::close(udp_socket);
        udp_socket = -1;
    }
    connected = false;
    is_server_mode = false;
    packet_queue.clear();
    UtilityFunctions::print("Tailscale connection closed");
}

bool TailscaleMultiplayerPeer::_is_server() const {
    return is_server_mode;
}

int TailscaleMultiplayerPeer::_get_unique_id() const {
    return unique_id;
}

MultiplayerPeer::ConnectionStatus TailscaleMultiplayerPeer::_get_connection_status() const {
    if (connected && udp_socket >= 0) {
        return MultiplayerPeer::CONNECTION_CONNECTED;
    }
    return MultiplayerPeer::CONNECTION_DISCONNECTED;
}

void TailscaleMultiplayerPeer::_set_target_peer(int p_peer) {
    target_peer = p_peer;
}

int TailscaleMultiplayerPeer::_get_packet_peer() const {
    return current_packet_peer;
}

Error TailscaleMultiplayerPeer::_put_packet(const uint8_t* p_buffer, int p_buffer_size) {
    if (udp_socket < 0 || !connected) {
        UtilityFunctions::print("Cannot send: socket not ready");
        return ERR_UNCONFIGURED;
    }
    
    if (is_server_mode) {
        // Server mode: need to know which client to send to
        // For now, we'll broadcast or use stored client address
        if (connected_address.is_empty()) {
            UtilityFunctions::print("No client connected");
            return ERR_UNAVAILABLE;
        }
        
        struct sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        client_addr.sin_family = AF_INET;
        client_addr.sin_port = htons(connected_port);
        inet_pton(AF_INET, connected_address.utf8().get_data(), &client_addr.sin_addr);
        
        int sent = sendto(udp_socket, p_buffer, p_buffer_size, 0,
                         (struct sockaddr*)&client_addr, sizeof(client_addr));
        
        if (sent != p_buffer_size) {
            return ERR_UNAVAILABLE;
        }
    } else {
        // Client mode: send to the server we connected to
        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(connected_port);
        inet_pton(AF_INET, connected_address.utf8().get_data(), &server_addr.sin_addr);
        
        int sent = sendto(udp_socket, p_buffer, p_buffer_size, 0,
                         (struct sockaddr*)&server_addr, sizeof(server_addr));
        
        if (sent != p_buffer_size) {
            return ERR_UNAVAILABLE;
        }
    }
    
    UtilityFunctions::print("Sent ", p_buffer_size, " bytes");
    return OK;
}

Error TailscaleMultiplayerPeer::_get_packet(const uint8_t** r_buffer, int32_t* r_buffer_size) {
    if (packet_queue.is_empty()) {
        return ERR_UNAVAILABLE;
    }
    
    PackedByteArray packet = packet_queue[0];
    packet_queue.remove_at(0);
    
    // Store packet data in a static vector that will persist
    static PackedByteArray static_packet;
    static_packet = packet;
    *r_buffer = static_packet.ptr();
    *r_buffer_size = static_packet.size();
    
    return OK;
}

int TailscaleMultiplayerPeer::_get_available_packet_count() const {
    return packet_queue.size();
}

int TailscaleMultiplayerPeer::_get_max_packet_size() const {
    return 1400;
}

void TailscaleMultiplayerPeer::_disconnect_peer(int p_peer, bool p_force) {
    connected = false;
    if (udp_socket >= 0) {
        ::close(udp_socket);
        udp_socket = -1;
    }
}

MultiplayerPeer::TransferMode TailscaleMultiplayerPeer::_get_transfer_mode() const {
    return transfer_mode;
}

void TailscaleMultiplayerPeer::_set_transfer_mode(TransferMode p_mode) {
    transfer_mode = p_mode;
}

int TailscaleMultiplayerPeer::_get_transfer_channel() const {
    return transfer_channel;
}

void TailscaleMultiplayerPeer::_set_transfer_channel(int p_channel) {
    transfer_channel = p_channel;
}

int TailscaleMultiplayerPeer::_get_packet_channel() const {
    return transfer_channel;
}

MultiplayerPeer::TransferMode TailscaleMultiplayerPeer::_get_packet_mode() const {
    return transfer_mode;
}

void TailscaleMultiplayerPeer::start_server(int p_port) {
    is_server_mode = true;
    udp_port = p_port;
    
    // Create UDP socket
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket < 0) {
        UtilityFunctions::print("Failed to create socket");
        return;
    }
    
    // Bind to port
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(p_port);
    
    if (bind(udp_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        UtilityFunctions::print("Failed to bind to port ", p_port);
        ::close(udp_socket);
        udp_socket = -1;
        return;
    }
    
    connected = true;
    UtilityFunctions::print("=== Tailscale Server Started ===");
    UtilityFunctions::print("Virtual IP: ", tailscale_ip);
    UtilityFunctions::print("Port: ", p_port);
    UtilityFunctions::print("Share this IP with friends to connect!");
}

void TailscaleMultiplayerPeer::connect_to_peer(const String& p_address, int p_port) {
    is_server_mode = false;
    connected_address = p_address;
    connected_port = p_port;
    
    // Create UDP socket
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket < 0) {
        UtilityFunctions::print("Failed to create socket");
        return;
    }
    
    // Bind to random port for client
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(0); // Let OS assign random port
    
    if (bind(udp_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        UtilityFunctions::print("Failed to bind socket");
        ::close(udp_socket);
        udp_socket = -1;
        return;
    }
    
    connected = true;
    UtilityFunctions::print("=== Connected to Server ===");
    UtilityFunctions::print("Server at: ", p_address, ":", p_port);
    UtilityFunctions::print("Your virtual IP: ", tailscale_ip);
}

} // namespace godot
