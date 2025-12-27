#!/bin/bash
# Script to run Wokwi simulator with the built firmware
# This script starts the Wokwi CLI simulator and waits for it to be ready

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WOKWI_TIMEOUT=${WOKWI_TIMEOUT:-120}

cd "$SCRIPT_DIR"

# Check if firmware exists
if [ ! -f "firmware.bin" ]; then
    echo "Error: firmware.bin not found in $SCRIPT_DIR"
    echo "Please run prepare-firmware.sh first"
    exit 1
fi

echo "Starting Wokwi simulator..."
echo "Timeout: ${WOKWI_TIMEOUT} seconds"

# Run wokwi-cli with timeout
# The simulator will forward port 80 to localhost:8080
wokwi-cli --timeout ${WOKWI_TIMEOUT}000 diagram.json
