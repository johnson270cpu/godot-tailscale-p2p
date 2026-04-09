#ifndef TAILSCALE_MULTIPLAYER_PEER_H
#define TAILSCALE_MULTIPLAYER_PEER_H

#include <godot_cpp/classes/multiplayer_peer_extension.hpp>
#include <godot_cpp/classes/multiplayer_peer.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

class TailscaleMultiplayerPeer : public MultiplayerPeerExtension {
    GDCLASS(TailscaleMultiplayerPeer, MultiplayerPeerExtension);

private:
    int unique_id;
    bool is_server_mode;
    int target_peer;
    Vector<PackedByteArray> packet_queue;
    MultiplayerPeer::TransferMode transfer_mode;
    int transfer_channel;
    bool connected;
    String tailscale_ip;
    int current_packet_peer;
    
    // Internal UDP socket (simulated Tailscale)
    int udp_socket;
    int udp_port;
    String connected_address;
    int connected_port;

protected:
    static void _bind_methods();

public:
    TailscaleMultiplayerPeer();
    ~TailscaleMultiplayerPeer();

    // Godot multiplayer peer interface
    virtual void _poll() override;
    virtual void _close() override;
    virtual bool _is_server() const override;
    virtual int _get_unique_id() const override;
    virtual ConnectionStatus _get_connection_status() const override;
    virtual void _set_target_peer(int p_peer) override;
    virtual int _get_packet_peer() const override;
    virtual Error _put_packet(const uint8_t* p_buffer, int p_buffer_size) override;
    virtual Error _get_packet(const uint8_t** r_buffer, int32_t* r_buffer_size) override;
    virtual int _get_available_packet_count() const override;
    virtual int _get_max_packet_size() const override;
    virtual void _disconnect_peer(int p_peer, bool p_force) override;
    virtual TransferMode _get_transfer_mode() const override;
    virtual void _set_transfer_mode(TransferMode p_mode) override;
    virtual int _get_transfer_channel() const override;
    virtual void _set_transfer_channel(int p_channel) override;
    virtual int _get_packet_channel() const override;
    virtual TransferMode _get_packet_mode() const override;

    // Custom methods exposed to GDScript
    void start_server(int p_port = 9999);
    void connect_to_peer(const String& p_address, int p_port = 9999);
    String get_tailscale_ip() const { return tailscale_ip; }
    bool is_connected() const { return connected; }
};

} // namespace godot

#endif
