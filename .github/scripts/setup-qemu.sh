#!/bin/bash
# Setup QEMU ESP32 emulation environment
# This script downloads and sets up QEMU for ESP32

set -e

QEMU_DIR="qemu-esp32"

echo "Setting up QEMU ESP32..."

# Create directory for QEMU
mkdir -p ${QEMU_DIR}

# Check if QEMU is already installed
if [ -f "${QEMU_DIR}/qemu-system-xtensa" ]; then
    echo "QEMU ESP32 already installed"
    echo "QEMU binary: ${QEMU_DIR}/qemu-system-xtensa"
    exit 0
fi

# Try multiple QEMU sources in order of preference
echo "Attempting to download QEMU ESP32..."

# List of potential QEMU download URLs to try
# Using the latest stable releases from Espressif
QEMU_URLS=(
    "esp-develop-9.2.2-20250817|https://github.com/espressif/qemu/releases/download/esp-develop-9.2.2-20250817/qemu-xtensa-softmmu-esp_develop_9.2.2_20250817-x86_64-linux-gnu.tar.xz"
    "esp-develop-9.1.0-20240606|https://github.com/espressif/qemu/releases/download/esp-develop-9.1.0-20240606/qemu-xtensa-softmmu-esp_develop_9.1.0_20240606-x86_64-linux-gnu.tar.xz"
    "esp-develop-9.0.0-20231220|https://github.com/espressif/qemu/releases/download/esp-develop-9.0.0-20231220/qemu-xtensa-softmmu-esp_develop_9.0.0_20231220-x86_64-linux-gnu.tar.xz"
)

DOWNLOAD_SUCCESS=false

for ENTRY in "${QEMU_URLS[@]}"; do
    VERSION="${ENTRY%%|*}"
    URL="${ENTRY##*|}"
    
    echo "Trying version ${VERSION}..."
    echo "URL: ${URL}"
    
    if wget --spider -q "${URL}" 2>/dev/null; then
        echo "Found available version: ${VERSION}"
        echo "Downloading from ${URL}..."
        
        if wget -q "${URL}" -O qemu.tar.xz; then
            echo "Download successful, extracting..."
            if tar -xf qemu.tar.xz -C ${QEMU_DIR} --strip-components=1; then
                rm qemu.tar.xz
                DOWNLOAD_SUCCESS=true
                echo "QEMU ESP32 version ${VERSION} installed successfully"
                break
            else
                echo "Extraction failed, trying next source..."
                rm -f qemu.tar.xz
            fi
        else
            echo "Download failed, trying next source..."
            rm -f qemu.tar.xz
        fi
    else
        echo "Version ${VERSION} not available, trying next..."
    fi
done

if [ "$DOWNLOAD_SUCCESS" = false ]; then
    echo "ERROR: Could not download QEMU ESP32 from any source"
    echo "Please check https://github.com/espressif/qemu/releases for available versions"
    exit 1
fi

# Make QEMU executable (try both possible locations)
if [ -f "${QEMU_DIR}/qemu-system-xtensa" ]; then
    chmod +x ${QEMU_DIR}/qemu-system-xtensa
    QEMU_BIN="${QEMU_DIR}/qemu-system-xtensa"
elif [ -f "${QEMU_DIR}/bin/qemu-system-xtensa" ]; then
    chmod +x ${QEMU_DIR}/bin/qemu-system-xtensa
    # Create symlink for easier access
    ln -sf bin/qemu-system-xtensa ${QEMU_DIR}/qemu-system-xtensa
    QEMU_BIN="${QEMU_DIR}/bin/qemu-system-xtensa"
else
    echo "ERROR: Could not find qemu-system-xtensa binary"
    exit 1
fi

echo "QEMU ESP32 setup complete"
echo "QEMU binary: ${QEMU_BIN}"

# Verify QEMU can run by checking for required libraries
echo "Verifying QEMU dependencies..."
if ! ldd "${QEMU_BIN}" | grep -q "not found"; then
    echo "All required libraries found"
    ${QEMU_BIN} --version
else
    echo "WARNING: Missing required libraries:"
    ldd "${QEMU_BIN}" | grep "not found"
    echo ""
    echo "Install missing dependencies with:"
    echo "  sudo apt-get update"
    echo "  sudo apt-get install -y libsdl2-2.0-0 libpixman-1-0 libglib2.0-0"
    exit 1
fi
