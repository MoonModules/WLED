#!/bin/bash
# Script to prepare firmware for Wokwi testing
# This copies the built firmware to the test directory

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
WOKWI_DIR="$PROJECT_ROOT/test/wokwi"

# Check if environment is specified
if [ -z "$1" ]; then
    echo "Usage: $0 <environment>"
    echo "Example: $0 esp32_V4_wokwi_debug"
    exit 1
fi

ENV_NAME=$1
BUILD_DIR="$PROJECT_ROOT/.pio/build/$ENV_NAME"
FIRMWARE_BIN="$BUILD_DIR/firmware.bin"
FIRMWARE_ELF="$BUILD_DIR/firmware.elf"
BOOTLOADER_BIN="$BUILD_DIR/bootloader.bin"
PARTITIONS_BIN="$BUILD_DIR/partitions.bin"

# Check if firmware exists
if [ ! -f "$FIRMWARE_BIN" ]; then
    echo "Error: Firmware binary not found at $FIRMWARE_BIN"
    echo "Please build the firmware first: pio run -e $ENV_NAME"
    exit 1
fi

# Copy firmware to test directory
echo "Copying firmware from $ENV_NAME to test directory..."
cp "$FIRMWARE_BIN" "$WOKWI_DIR/firmware.bin"
echo "✓ Copied firmware.bin"

if [ -f "$FIRMWARE_ELF" ]; then
    cp "$FIRMWARE_ELF" "$WOKWI_DIR/firmware.elf"
    echo "✓ Copied firmware.elf"
else
    echo "⚠ Warning: firmware.elf not found"
fi

# Copy bootloader and partitions (required for filesystem support)
if [ -f "$BOOTLOADER_BIN" ]; then
    cp "$BOOTLOADER_BIN" "$WOKWI_DIR/bootloader.bin"
    echo "✓ Copied bootloader.bin"
else
    echo "⚠ Warning: bootloader.bin not found at $BOOTLOADER_BIN"
fi

if [ -f "$PARTITIONS_BIN" ]; then
    cp "$PARTITIONS_BIN" "$WOKWI_DIR/partitions.bin"
    echo "✓ Copied partitions.bin"
else
    echo "⚠ Warning: partitions.bin not found at $PARTITIONS_BIN"
fi

echo ""
echo "Firmware prepared successfully!"
echo "Files in $WOKWI_DIR:"
ls -lh "$WOKWI_DIR"/*.bin "$WOKWI_DIR"/*.elf 2>/dev/null || true
