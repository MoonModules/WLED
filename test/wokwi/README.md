# WLED-MM Wokwi Simulation Testing

This directory contains configuration and tests for running WLED-MM in the Wokwi ESP32 simulator with Playwright-based web interface testing.

## Overview

The Wokwi testing workflow:
1. Builds the WLED firmware for ESP32
2. Creates a combined firmware image with esptool.py
3. Runs the firmware in the Wokwi ESP32 simulator
4. Uses Playwright to test the web interface
5. Verifies pages load without JavaScript errors

## Files

- `diagram.json` - Wokwi hardware configuration (ESP32 DevKit) with serial monitor settings
- `wokwi.toml` - Wokwi CLI configuration using combined firmware image
- `prepare-firmware.sh` - Script to create combined firmware image using esptool.py
- `run-simulator.sh` - Script to start the Wokwi simulator
- `firmware-combined.bin` - Combined firmware image (bootloader + partitions + app)
- `firmware.elf` - Firmware with debug symbols (copied from build)

## Combined Firmware Image

**Wokwi's recommended approach** is to use a single combined firmware image that includes bootloader, partition table, and application. This ensures proper filesystem support and eliminates potential issues with separate flash files.

**wokwi.toml configuration:**
```toml
[wokwi]
firmware = "firmware-combined.bin"  # Combined image with everything
elf = "firmware.elf"                # Debug symbols
```

**Combined image structure:**
- `0x1000` - Bootloader (ESP32 second-stage bootloader)
- `0x8000` - Partition table (defines flash memory layout)
- `0x10000` - Application (main WLED firmware)

**How it's created:**
The `prepare-firmware.sh` script uses `esptool.py merge_bin` to combine the three components:
```bash
esptool.py --chip esp32 merge_bin \
    -o firmware-combined.bin \
    --flash_mode dio \
    --flash_freq 40m \
    --flash_size 4MB \
    0x1000 bootloader.bin \
    0x8000 partitions.bin \
    0x10000 firmware.bin
```

**Why this approach:**
- Recommended by Wokwi for reliable filesystem support
- Ensures correct alignment and offsets for all components
- Eliminates "partition not found" errors
- Single file is simpler and more reliable than multiple flash files
- Matches how real ESP32 devices are typically flashed

## Serial Monitor Configuration

The `diagram.json` file includes critical serial monitor configuration required for capturing firmware output in CI environments:

```json
"serialMonitor": {
  "display": "always",
  "newline": "lf"
}
```

**Why this is needed:**
- Without `display: "always"`, serial output is not captured in headless/CI mode
- The Wokwi simulator only records serial output when explicitly configured
- This is **required** for debugging boot issues and verifying firmware execution

**Configuration options:**
- `display: "always"` - Ensures serial output is captured even in headless mode (CI)
- `display: "auto"` - Only shows serial monitor when running interactively (not suitable for CI)
- `newline: "lf"` - Line ending format (LF for Unix-style, CRLF for Windows)

**Troubleshooting missing serial output:**
If you see empty serial logs (`boot-check-serial.log` or `serial.log`):
1. Verify `serialMonitor` section exists in `diagram.json`
2. Check firmware.bin has valid ESP32 header (starts with `0xe9` magic byte)
3. Ensure firmware was built successfully and copied to test directory
4. Review firmware build logs for compilation errors

## Running Tests Locally

### Prerequisites

1. Install Node.js dependencies:
   ```bash
   npm ci
   ```

2. Install Wokwi CLI:
   ```bash
   curl -L https://wokwi.com/ci/install.sh | sh
   ```

3. Install Playwright browsers:
   ```bash
   npx playwright install --with-deps chromium
   ```

### Build and Test

1. Build the web interface:
   ```bash
   npm run build
   ```

2. Build the firmware:
   ```bash
   pio run -e esp32_V4_wokwi_debug
   ```

3. Prepare firmware for testing:
   ```bash
   ./test/wokwi/prepare-firmware.sh esp32_V4_wokwi_debug
   ```

4. Start the Wokwi simulator (in a separate terminal):
   ```bash
   cd test/wokwi
   ./run-simulator.sh
   ```

5. Run Playwright tests (in another terminal):
   ```bash
   npm run test:wokwi
   ```

## CI Integration

The GitHub Actions workflow (`.github/workflows/wokwi-test.yml`) automatically runs these tests on:
- Push to `mdev` branch
- Pull requests to `mdev` branch
- Manual workflow dispatch

## Test Cases

The Playwright tests (`test/playwright/wokwi-basic.spec.js`) verify:
- Main index page loads without errors
- Settings pages load without errors
- Edit page loads without errors
- JSON API endpoints respond correctly

## Boot Validation Scenarios

Wokwi CLI supports test scenarios that can validate firmware boot without requiring a full Playwright test suite. Two scenarios are provided:

### Quick Boot Check (`scenarios/boot-check.yaml`)
A fast 15-second validation that ensures WLED boots without immediate crashes.

**Features:**
- Simple delay-based validation
- Total runtime: ~15 seconds
- Fails if simulator crashes or hangs during boot
- Perfect for CI pre-flight checks
- Tolerates transient network issues (code 1006 errors)

**Usage:**
```bash
cd test/wokwi
~/.wokwi-ci/bin/wokwi-cli --timeout 30000 --scenario scenarios/boot-check.yaml .
```

**Note:** The CLI timeout (30 seconds) is set higher than the scenario timeout (15 seconds) to account for network delays and API connection time.

### Comprehensive Boot Validation (`scenarios/boot-full.yaml`)
A thorough 30-second validation with extended timing for WiFi AP and HTTP server initialization.

**Features:**
- Allows full system initialization
- Total runtime: ~30 seconds
- More detailed validation
- Better for local testing and troubleshooting

**Usage:**
```bash
cd test/wokwi
~/.wokwi-ci/bin/wokwi-cli --timeout 40000 --scenario scenarios/boot-full.yaml .
```

### Creating Custom Scenarios

You can create your own scenario files in YAML format:

```yaml
name: "Custom Test"
version: 1
timeout: 15000  # milliseconds

steps:
  - name: "Description of step"
    delay: 5s  # wait 5 seconds (note: time units required!)
```

The scenario will fail if:
- The simulator crashes during execution
- The timeout is exceeded
- Any step encounters an error

## Extending Tests

To add more tests:
1. Edit `test/playwright/wokwi-basic.spec.js`
2. Add new test cases using Playwright's `test()` function
3. Follow the existing pattern of checking for console errors
4. Create custom scenario files in `scenarios/` directory

## Troubleshooting

### Simulator doesn't start
- Check that firmware-combined.bin exists in test/wokwi/
- Verify Wokwi CLI is installed: `wokwi-cli --version`
- Check Wokwi CLI logs for errors

### No serial output from firmware
- Verify `serialMonitor` configuration in diagram.json
- Check firmware-combined.bin is valid:
  ```bash
  hexdump -C firmware-combined.bin | head -16
  # Should show bootloader at 0x1000, partitions at 0x8000, app at 0x10000
  ```
- Ensure combined image was created: `ls -lh test/wokwi/firmware-combined.bin`
- Check firmware build logs for errors

### Filesystem/partition errors
**Error:** `partition "spiffs" could not be found`

**Cause:** Missing or incorrect combined firmware image

**Solutions:**
1. Verify firmware-combined.bin exists and has correct structure:
   ```bash
   python3 << 'EOF'
   with open('test/wokwi/firmware-combined.bin', 'rb') as f:
       data = f.read()
   print(f"Size: {len(data)} bytes")
   print(f"Bootloader at 0x1000: {data[0x1000:0x1004].hex()}")
   print(f"Partitions at 0x8000: {data[0x8000:0x8004].hex()}")
   print(f"App at 0x10000: {data[0x10000:0x10004].hex()}")
   EOF
   ```

2. Rebuild combined image:
   ```bash
   cd test/wokwi
   ./prepare-firmware.sh esp32_V4_wokwi_debug
   ```

3. Verify esptool.py is installed:
   ```bash
   pip install esptool
   ```

4. Check that bootloader.bin, partitions.bin, and firmware.bin exist in build directory:
   ```bash
   ls -lh .pio/build/esp32_V4_wokwi_debug/{bootloader.bin,partitions.bin,firmware.bin}
   # May also check: .pio/build/esp32_V4_wokwi_debug/bootloader/bootloader.bin
   ```

4. Check the partition table source CSV file:
   ```bash
   # The esp32_V4_wokwi_debug build uses:
   cat tools/WLED_ESP32_4MB_256KB_FS.csv
   # Should include a line with: spiffs, data, spiffs, ...
   ```

5. Rebuild firmware to regenerate partition files:
   ```bash
   pio run -e esp32_V4_wokwi_debug --target clean
   pio run -e esp32_V4_wokwi_debug
   ```

6. Check wokwi.toml flash configuration:
   ```toml
   [wokwi]
   partitions = "partitions.bin"
   
   [[wokwi.flashFiles]]
   offset = 0x1000
   file = "bootloader.bin"
   
   [[wokwi.flashFiles]]
   offset = 0x8000
   file = "partitions.bin"
   ```

**Note:** The bootloader and partition table are essential for filesystem support. Without them, SPIFFS cannot mount and the firmware will show "partition not found" errors.

### Web server not accessible
- Wait 30-60 seconds for the ESP32 to boot and start WiFi
- Check that port 9080 is not already in use
- Verify port forwarding in wokwi.toml

### Tests fail
- Check Playwright report: `npx playwright show-report`
- Look for console errors in test output
- Verify firmware build completed successfully

## References

- [Wokwi Documentation](https://docs.wokwi.com/)
- [Wokwi CLI](https://docs.wokwi.com/wokwi-ci/getting-started)
- [Playwright Documentation](https://playwright.dev/)
