#!/bin/bash
# Run WLED firmware in QEMU ESP32
# This script starts QEMU with the compiled firmware and enables network access
#
# Note: QEMU ESP32 emulation has limitations:
# - Not all peripherals are fully emulated (WiFi, I2C, some GPIOs)
# - Some firmware features may crash in QEMU but work on real hardware
# - This is expected behavior for testing web UI functionality

set -e

FIRMWARE_DIR="${1:-.pio/build/esp32dev}"
QEMU_DIR="${2:-qemu-esp32}"
HTTP_PORT="${3:-8080}"  # Default to 8080 (non-privileged port)

if [ ! -d "$FIRMWARE_DIR" ]; then
    echo "Error: Firmware directory not found: $FIRMWARE_DIR"
    exit 1
fi

if [ ! -f "${QEMU_DIR}/qemu-system-xtensa" ] && [ ! -f "${QEMU_DIR}/bin/qemu-system-xtensa" ]; then
    echo "Error: QEMU not found at ${QEMU_DIR}/qemu-system-xtensa or ${QEMU_DIR}/bin/qemu-system-xtensa"
    echo "Please run setup-qemu.sh first"
    exit 1
fi

# Determine QEMU binary location
if [ -f "${QEMU_DIR}/qemu-system-xtensa" ]; then
    QEMU_BIN="${QEMU_DIR}/qemu-system-xtensa"
else
    QEMU_BIN="${QEMU_DIR}/bin/qemu-system-xtensa"
fi

# Check for required firmware files
BOOTLOADER="${FIRMWARE_DIR}/bootloader.bin"
PARTITIONS="${FIRMWARE_DIR}/partitions.bin"
FIRMWARE="${FIRMWARE_DIR}/firmware.bin"

if [ ! -f "$BOOTLOADER" ]; then
    echo "Error: Bootloader not found: $BOOTLOADER"
    exit 1
fi

if [ ! -f "$FIRMWARE" ]; then
    echo "Error: Firmware not found: $FIRMWARE"
    exit 1
fi

echo "Starting QEMU ESP32 with WLED firmware"
echo "Firmware directory: $FIRMWARE_DIR"
echo "HTTP will be accessible at: http://localhost:${HTTP_PORT}"

# Create a merged flash image as QEMU expects
FLASH_IMAGE="/tmp/wled_flash.bin"
echo "Creating flash image at $FLASH_IMAGE"

# Create a 4MB flash image (0x400000 bytes)
dd if=/dev/zero of="$FLASH_IMAGE" bs=1M count=4 2>/dev/null

# Write bootloader at 0x1000
if [ -f "$BOOTLOADER" ]; then
    dd if="$BOOTLOADER" of="$FLASH_IMAGE" bs=1 seek=$((0x1000)) conv=notrunc 2>/dev/null
fi

# Write partitions at 0x8000
if [ -f "$PARTITIONS" ]; then
    dd if="$PARTITIONS" of="$FLASH_IMAGE" bs=1 seek=$((0x8000)) conv=notrunc 2>/dev/null
fi

# Write firmware at 0x10000
dd if="$FIRMWARE" of="$FLASH_IMAGE" bs=1 seek=$((0x10000)) conv=notrunc 2>/dev/null

echo "Flash image created successfully"

# Run QEMU ESP32
# Note: ESP32 in QEMU has limited peripheral support
# Network configuration uses user-mode networking with port forwarding
echo "Starting QEMU..."
${QEMU_BIN} \
    -nographic \
    -machine esp32 \
    -drive file=${FLASH_IMAGE},if=mtd,format=raw \
    -nic user,model=open_eth,hostfwd=tcp::${HTTP_PORT}-:80 \
    -serial mon:stdio &

QEMU_PID=$!
echo "QEMU started with PID: $QEMU_PID"
echo $QEMU_PID > qemu.pid

# Wait for QEMU to initialize
echo "Waiting for QEMU to initialize (30 seconds)..."
sleep 30

# Check if QEMU is still running
if ! kill -0 $QEMU_PID 2>/dev/null; then
    echo "Error: QEMU process died"
    exit 1
fi

echo "QEMU is running"
echo "To stop QEMU: kill $QEMU_PID"
echo "Or use: kill \$(cat qemu.pid)"

# Wait for QEMU process
wait $QEMU_PID
