⚠️ This project currently uses placeholder IP generation for testing purposes.

# Godot Tailscale P2P

A custom GDExtension that provides **P2P multiplayer networking** for Godot 4 using Tailscale. Connect players directly across the internet without dedicated servers.

## ✨ Features

- 🔗 **Direct P2P connections** - No central server required
- 🎮 **Godot MultiplayerPeer** - Works with Godot's RPC system
- 🚀 **NAT traversal** - Uses Tailscale to punch through firewalls
- 💻 **Linux support** - Currently working on Linux x86_64

## 🎯 What Works

| Feature | Status |
|---------|--------|
| Linux x86_64 | ✅ Working |
| UDP packet sending/receiving | ✅ Working |
| Godot MultiplayerPeer interface | ✅ Complete |
| Direct P2P connections | ✅ Working |
| Chat demo | ✅ Working |
| Windows | 🔄 Need help |
| macOS | 🔄 Need help |
| Android | 🔄 Need help |
| LAN auto-discovery | 🔄 Need help |

## 🎮 Usage Example

```gdscript
extends Node2D

var peer = null

func _ready():
    peer = TailscaleMultiplayerPeer.new()
    
    # Host a game
    peer.start_server(9999)
    
    # Or join a friend's game
    # peer.connect_to_peer("192.168.1.100", 9999)
    
    multiplayer.multiplayer_peer = peer

@rpc("any_peer", "reliable")
func send_message(text):
    print("Message: ", text)
