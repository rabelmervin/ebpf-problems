#!/bin/bash
# setup.sh - Install dependencies and setup eBPF program

set -e

echo "=========================================="
echo "eBPF TCP Packet Dropper - Setup"
echo "=========================================="
echo ""

# Check if running as root
if [ "$EUID" -eq 0 ]; then 
    echo "❌ Please run this script WITHOUT sudo"
    echo "   It will ask for sudo when needed"
    exit 1
fi

echo "📦 [1/3] Installing dependencies..."
sudo apt-get update -qq
sudo apt-get install -y \
    clang \
    llvm \
    libbpf-dev \
    linux-headers-$(uname -r) \
    build-essential \
    gcc \
    netcat

echo ""
echo "🔍 [2/3] Checking kernel version..."
KERNEL_VERSION=$(uname -r | cut -d. -f1)
echo "   Kernel version: $(uname -r)"
if [ "$KERNEL_VERSION" -lt 5 ]; then
    echo "   ⚠️  Warning: Kernel < 5.x, XDP might not work optimally"
else
    echo "   ✓ Kernel version OK"
fi

echo ""
echo "🔨 [3/3] Compiling eBPF program..."
make clean 2>/dev/null || true
make

echo ""
echo "🌐 Detecting network interface..."
INTERFACE=$(ip route | grep default | awk '{print $5}' | head -1)

if [ -z "$INTERFACE" ]; then
    INTERFACE=$(ip link | grep -E "^[0-9]+" | grep -v "lo:" | head -1 | awk -F': ' '{print $2}')
fi

echo "   Detected: $INTERFACE"

echo ""
echo "=========================================="
echo "✅ Setup Complete!"
echo "=========================================="
echo ""
echo "�� Usage:"
echo ""
echo "  Default port (4040):"
echo "    sudo ./tcp_drop_user $INTERFACE"
echo ""
echo "  Custom port (e.g., 8080):"
echo "    sudo ./tcp_drop_user $INTERFACE 8080"
echo ""
echo "🧪 To test:"
echo ""
echo "  Terminal 1:"
echo "    sudo ./tcp_drop_user $INTERFACE 4040"
echo ""
echo "  Terminal 2:"
echo "    nc -v 127.0.0.1 4040"
echo "    (Should timeout - packets are being dropped!)"
echo ""
echo "Press Ctrl+C to stop"
echo "=========================================="
