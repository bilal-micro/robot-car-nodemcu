import os
import django
from django.urls import path
import asyncio
import websockets
import socket
import threading
import time
from django.core.asgi import get_asgi_application
from channels.routing import ProtocolTypeRouter, URLRouter
from channels.generic.websocket import AsyncWebsocketConsumer
import subprocess

os.environ.setdefault("DJANGO_SETTINGS_MODULE", "myproject.settings")
django.setup()

# WebSocket Clients Storage
websocket_clients = set()

class WebSocketConsumer(AsyncWebsocketConsumer):
    async def connect(self):
        """Accept WebSocket connection and store client."""
        await self.accept()
        websocket_clients.add(self)
        print(f"🔗 WebSocket client connected ({len(websocket_clients)} total)")

    async def disconnect(self, close_code):
        """Remove WebSocket client on disconnect."""
        websocket_clients.remove(self)
        print(f"🔌 WebSocket client disconnected ({len(websocket_clients)} remaining)")

    async def receive(self, text_data=None, bytes_data=None):
        print(text_data)

    async def send_tcp_data(self, message):
        """Send TCP received data to WebSocket clients."""
        print("Sending To NodeMCU  " , message)
        await self.send(message)

# WebSocket Routing
websocket_urlpatterns = [
    path('ws', WebSocketConsumer.as_asgi()),
]

application = ProtocolTypeRouter({
    "http": get_asgi_application(),
    "websocket": URLRouter(websocket_urlpatterns),
})

# TCP Server
TCP_HOST = "0.0.0.0"
TCP_PORT = 6019


async def tcp_server():
    """Receive TCP data and send it to all WebSocket clients."""
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind((TCP_HOST, TCP_PORT))
    server.listen(3)
    print(f"✅ TCP Server running on {TCP_HOST}:{TCP_PORT}")

    loop = asyncio.get_event_loop()

    while True:
        conn, addr = await loop.run_in_executor(None, server.accept)
        print(f"📡 TCP Connection established from {addr}")

        asyncio.create_task(handle_tcp_client(conn, addr))  # Run in a non-blocking way


def get_pid_by_port(port):
    """Find process ID (PID) using the given port."""
    try:
        result = subprocess.run(
            ["sudo", "netstat", "-tulnp"], capture_output=True, text=True
        )
        for line in result.stdout.split("\n"):
            if f":{port}" in line:
                parts = line.split()
                pid_info = parts[-1]  # Last column has PID/program
                pid = pid_info.split("/")[0]  # Extract PID from '1234/python'
                return int(pid)
    except Exception as e:
        print(f"Error getting PID: {e}")
    return None

def kill_process(pid):
    """Kill process by PID."""
    try:
        os.system(f"sudo kill -9 {pid}")
        print(f"✅ Killed process {pid} on port {TCP_PORT}")
    except Exception as e:
        print(f"Error killing process {pid}: {e}")


async def handle_tcp_client(conn, addr):
    """Handle each TCP client asynchronously and read one line at a time."""
    loop = asyncio.get_event_loop()
    reader = conn.makefile("r")  # Buffered reader for line-by-line reading

    try:
        while True:
            line = await loop.run_in_executor(None, reader.readline)  # Read one line
            if not line:
                break  # Client disconnected
            
            message = line.strip()  # Remove extra spaces/newlines
            print(f"📩 Received from TCP: {message}")

            # Forward to WebSocket clients (if needed)
            disconnected_clients = set()
            for client in websocket_clients:
                try:
                    # time.sleep(1)
                    da = str(message).split('\n')[0]
                    x = da.split(',')[0].split('.')[0]
                    y = da.split(',')[1].split('.')[0]
                    await client.send_tcp_data(f'{x},{y}')
                except:
                    disconnected_clients.add(client)

            websocket_clients.difference_update(disconnected_clients)

    except Exception as e:
        print(f"❌ Error: {e}")

    finally:
        conn.close()
        print(f"🔌 TCP Connection closed {addr}")
# Run TCP server in a separate thread
pid = get_pid_by_port(TCP_PORT)
if pid:
    print(f"⚠️ Process {pid} detected on port {TCP_PORT}. Checking timeout...")
    # Kill the process if needed
    kill_process(pid)
else:
    print(f"✅ No process found on port {TCP_PORT}")

threading.Thread(target=lambda: asyncio.run(tcp_server()), daemon=True).start()
