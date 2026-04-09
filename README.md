# ⚠️ EXPERIMENTAL PROJECT

This project is currently **experimental** and only works on **Linux x86_64**. Use for testing and development only.

---

# Godot Tailscale P2P

A custom GDExtension that provides **P2P multiplayer networking** for Godot 4 using Tailscale. Connect players directly across the internet without dedicated servers.

## ✨ Features

- 🔗 **Direct P2P Connections** - No central server required, players connect directly
- 🎮 **Godot MultiplayerPeer Integration** - Seamless integration with Godot's RPC system
- 🚀 **NAT Traversal** - Uses Tailscale's mesh networking to punch through firewalls
- 🔒 **Secure** - Leverages Tailscale's encryption and security
- 🚧 **Experimental** - Still in active development, APIs may change

## 🎯 Platform Status

| Platform | Status | Help Needed |
|----------|--------|-------------|
| Linux x86_64 | ✅ **Experimental** | Bug reports, optimization |
| Windows | ❌ **Not Started** | Full implementation needed |
| macOS | ❌ **Not Started** | Full implementation needed |
| Android | ❌ **Not Started** | Full implementation needed |

## 🎮 Usage Example

```gdscript
extends Node2D

var peer = null

func _ready():
    peer = TailscaleMultiplayerPeer.new()
    
    # Host a game server
    peer.start_server(9999)
    
    # Or join a friend's game
    # peer.connect_to_peer("100.x.x.x", 9999)
    
    multiplayer.multiplayer_peer = peer

@rpc("any_peer", "reliable")
func send_message(text: String):
    print("Message received: ", text)
```

## 🚀 Getting Started (Linux Only)

### Prerequisites
- Godot 4.0 or later
- Tailscale installed and running
- Linux x86_64
- C++ build tools (GCC/Clang)

### Installation

1. **Clone the repository**
```bash
git clone https://github.com/johnson270cpu/godot-tailscale-p2p.git
cd godot-tailscale-p2p
```

2. **Build the extension**
```bash
scons
```

3. **Copy to your Godot project**
```bash
cp bin/libgodot_tailscale_p2p.so your_project/bin/
```

## ⚠️ Important Notes

- This project is **EXPERIMENTAL** - APIs and behavior may change
- Currently **Linux x86_64 only**
- Uses placeholder IP generation for testing
- Security audit recommended before any production use
- Requires Tailscale to be installed and running

## 🤝 Contributing

We're actively seeking contributors! Here's how you can help:

### Urgent Needs
- **Windows Port**: Build system setup, socket integration
- **macOS Port**: Build configuration, platform testing
- **Android Port**: NDK integration
- **Network Specialists**: Protocol optimization
- **Documentation**: Tutorial writing, examples

### How to Contribute

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

See [CONTRIBUTING.md](CONTRIBUTING.md) for detailed guidelines.

## 📋 Development Roadmap

- [ ] Windows support
- [ ] macOS support
- [ ] Android support
- [ ] LAN auto-discovery (mDNS)
- [ ] Custom network backends
- [ ] WebSocket support for web builds
- [ ] Example projects
- [ ] Performance optimization
- [ ] Production-ready security audit

## 📄 License

MIT License - see [LICENSE](LICENSE) file for details.

## 💬 Support

- **GitHub Issues**: Report bugs or request features
- **GitHub Discussions**: Ask questions and share ideas

## 🙏 Acknowledgments

- [Godot Engine](https://godotengine.org/)
- [Tailscale](https://tailscale.com/)

---

⭐ If interested in this project, give it a star to help us reach contributors!