#!/usr/bin/env python3
import socket
import sys

PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 4040

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind(('127.0.0.1', PORT))
sock.listen(1)
print(f"tcp_server listening on 127.0.0.1:{PORT}")
try:
    while True:
        conn, addr = sock.accept()
        print(f"accepted connection from {addr} on port {PORT}")
        try:
            conn.sendall(b"hello\n")
        except Exception:
            pass
        conn.close()
except KeyboardInterrupt:
    pass
