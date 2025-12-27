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
    echo "Example: $0 esp32dev_compat"
    exit 1
fi

ENV_NAME=$1
FIRMWARE_BIN="$PROJECT_ROOT/.pio/build/$ENV_NAME/firmware.bin"
FIRMWARE_ELF="$PROJECT_ROOT/.pio/build/$ENV_NAME/firmware.elf"

# Check if firmware exists
if [ ! -f "$FIRMWARE_BIN" ]; then
    echo "Error: Firmware binary not found at $FIRMWARE_BIN"
    echo "Please build the firmware first: pio run -e $ENV_NAME"
    exit 1
fi

# Copy firmware to test directory
echo "Copying firmware from $ENV_NAME to test directory..."
cp "$FIRMWARE_BIN" "$WOKWI_DIR/firmware.bin"

if [ -f "$FIRMWARE_ELF" ]; then
    cp "$FIRMWARE_ELF" "$WOKWI_DIR/firmware.elf"
    echo "Copied firmware.bin and firmware.elf"
else
    echo "Warning: firmware.elf not found, copying only firmware.bin"
fi

echo "Firmware prepared successfully!"
echo "Location: $WOKWI_DIR/firmware.bin"
