extends Node2D

var peer = null
var is_host = false
var host_button = null
var join_button = null
var message_input = null
var send_button = null
var chat_display = null
var status_label = null

func _ready():
	print("=== Godot Tailscale Extension Test ===")
	setup_ui()
	
	# Check if extension loaded
	if ClassDB.class_exists("TailscaleMultiplayerPeer"):
		print("SUCCESS: TailscaleMultiplayerPeer found!")
		chat_display.add_text("[System] ✅ Extension loaded!\n")
	else:
		print("FAIL: TailscaleMultiplayerPeer not found")
		chat_display.add_text("[System] ❌ Extension not loaded\n")

func setup_ui():
	# Status label
	status_label = Label.new()
	status_label.text = "Status: Not connected"
	status_label.position = Vector2(10, 10)
	add_child(status_label)
	
	# Host button
	host_button = Button.new()
	host_button.text = "HOST GAME"
	host_button.position = Vector2(10, 40)
	host_button.size = Vector2(120, 40)
	host_button.pressed.connect(_on_host_pressed)
	add_child(host_button)
	
	# Join button
	join_button = Button.new()
	join_button.text = "JOIN GAME"
	join_button.position = Vector2(140, 40)
	join_button.size = Vector2(120, 40)
	join_button.pressed.connect(_on_join_pressed)
	add_child(join_button)
	
	# Chat display
	chat_display = RichTextLabel.new()
	chat_display.position = Vector2(10, 90)
	chat_display.size = Vector2(460, 250)
	add_child(chat_display)
	
	# Message input
	message_input = LineEdit.new()
	message_input.position = Vector2(10, 350)
	message_input.size = Vector2(350, 30)
	add_child(message_input)
	
	# Send button
	send_button = Button.new()
	send_button.text = "SEND"
	send_button.position = Vector2(370, 350)
	send_button.size = Vector2(100, 30)
	send_button.pressed.connect(_on_send_pressed)
	add_child(send_button)
	
	chat_display.add_text("[System] UI Ready - Click HOST or JOIN\n")

func _on_host_pressed():
	if peer:
		chat_display.add_text("[System] Already hosting/connected\n")
		return
	
	is_host = true
	peer = TailscaleMultiplayerPeer.new()
	peer.start_server(9999)
	multiplayer.multiplayer_peer = peer
	status_label.text = "Status: HOSTING on port 9999"
	chat_display.add_text("[System] 🟢 Server started! Waiting for players...\n")
	chat_display.add_text("[System] Your virtual IP: " + peer.get_tailscale_ip() + "\n")

func _on_join_pressed():
	if peer:
		chat_display.add_text("[System] Already connected/hosting\n")
		return
	
	is_host = false
	peer = TailscaleMultiplayerPeer.new()
	
	# For local testing, use 127.0.0.1
	# For real testing, you'd enter the host's Tailscale IP
	var host_ip = "127.0.0.1"
	
	chat_display.add_text("[System] Connecting to " + host_ip + ":9999...\n")
	peer.connect_to_peer(host_ip, 9999)
	multiplayer.multiplayer_peer = peer
	status_label.text = "Status: CONNECTED to " + host_ip
	chat_display.add_text("[System] 🔗 Connected to server!\n")

func _on_send_pressed():
	var msg = message_input.text
	if msg == "":
		return
	
	if not peer or not peer.is_connected():
		chat_display.add_text("[System] Not connected! Host or join first.\n")
		return
	
	chat_display.add_text("[You] " + msg + "\n")
	message_input.text = ""
	
	# Send via RPC
	rpc("_receive_message", msg)

@rpc("any_peer", "reliable")
func _receive_message(msg):
	var sender_name = "Client" if is_host else "Host"
	chat_display.add_text("[" + sender_name + "] " + msg + "\n")
