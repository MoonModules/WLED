# WLED-MM Wokwi Simulation Testing

This directory contains configuration and tests for running WLED-MM in the Wokwi ESP32 simulator with Playwright-based web interface testing.

## Overview

The Wokwi testing workflow:
1. Builds the WLED firmware for ESP32
2. Runs the firmware in the Wokwi ESP32 simulator
3. Uses Playwright to test the web interface
4. Verifies pages load without JavaScript errors

## Files

- `diagram.json` - Wokwi hardware configuration (ESP32 DevKit) with serial monitor settings
- `wokwi.toml` - Wokwi CLI configuration and port forwarding
- `prepare-firmware.sh` - Script to copy built firmware to test directory
- `run-simulator.sh` - Script to start the Wokwi simulator

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
- Check that firmware.bin exists in test/wokwi/
- Verify Wokwi CLI is installed: `wokwi-cli --version`
- Check Wokwi CLI logs for errors

### No serial output from firmware
- Verify `serialMonitor` configuration in diagram.json
- Check firmware.bin is valid:
  ```bash
  hexdump -C firmware.bin | head -4
  # Should show ESP32 magic byte 0xe9 at start
  ```
- Ensure firmware was copied: `ls -lh test/wokwi/firmware.bin`
- Check firmware build logs for errors

### Web server not accessible
- Wait 30-60 seconds for the ESP32 to boot and start WiFi
- Check that port 8080 is not already in use
- Verify port forwarding in wokwi.toml

### Tests fail
- Check Playwright report: `npx playwright show-report`
- Look for console errors in test output
- Verify firmware build completed successfully

## References

- [Wokwi Documentation](https://docs.wokwi.com/)
- [Wokwi CLI](https://docs.wokwi.com/wokwi-ci/getting-started)
- [Playwright Documentation](https://playwright.dev/)
