#!/bin/bash
# Setup QEMU ESP32 emulation environment
# This script downloads and sets up QEMU for ESP32

set -e

QEMU_VERSION="esp-develop-20220919"
QEMU_URL="https://github.com/espressif/qemu/releases/download/${QEMU_VERSION}/qemu-${QEMU_VERSION}-x86_64-linux-gnu.tar.xz"
QEMU_DIR="qemu-esp32"

echo "Setting up QEMU ESP32..."

# Create directory for QEMU
mkdir -p ${QEMU_DIR}

# Download QEMU ESP32 if not already present
if [ ! -f "${QEMU_DIR}/qemu-system-xtensa" ]; then
    echo "Downloading QEMU ESP32 from ${QEMU_URL}..."
    wget -q ${QEMU_URL} -O qemu.tar.xz
    
    echo "Extracting QEMU..."
    tar -xf qemu.tar.xz -C ${QEMU_DIR} --strip-components=1
    
    # Cleanup
    rm qemu.tar.xz
    
    echo "QEMU ESP32 installed successfully"
else
    echo "QEMU ESP32 already installed"
fi

# Make QEMU executable
chmod +x ${QEMU_DIR}/qemu-system-xtensa

echo "QEMU ESP32 setup complete"
echo "QEMU binary: ${QEMU_DIR}/qemu-system-xtensa"
