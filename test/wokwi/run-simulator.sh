#!/bin/bash
# Script to run Wokwi simulator with the built firmware
# This script starts the Wokwi CLI simulator and waits for it to be ready

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WOKWI_TIMEOUT=${WOKWI_TIMEOUT:-300}

cd "$SCRIPT_DIR"

# Check if firmware exists
if [ ! -f "firmware.bin" ]; then
    echo "Error: firmware.bin not found in $SCRIPT_DIR"
    echo "Please run prepare-firmware.sh first"
    exit 1
fi

echo "Starting Wokwi simulator..."
echo "Timeout: ${WOKWI_TIMEOUT} seconds"
echo "Web server will be available at http://localhost:9080"
echo "Serial output will be displayed below"
echo "=================================="
echo ""

# Run wokwi-cli with timeout (in milliseconds) and scenario flag for better output
# The simulator will forward port 80 to localhost:9080
# Note: wokwi-cli runs in foreground, so this needs to be backgrounded or run in a separate process
# When run from the directory containing diagram.json and wokwi.toml, wokwi-cli will find them automatically
# Serial output goes to stdout, diagnostic messages go to stderr
wokwi-cli --timeout ${WOKWI_TIMEOUT}000 .
