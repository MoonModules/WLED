# WLED-MM Wokwi Simulation Testing

This directory contains configuration and tests for running WLED-MM in the Wokwi ESP32 simulator with Playwright-based web interface testing.

## Overview

The Wokwi testing workflow:
1. Builds the WLED firmware for ESP32
2. Runs the firmware in the Wokwi ESP32 simulator
3. Uses Playwright to test the web interface
4. Verifies pages load without JavaScript errors

## Files

- `diagram.json` - Wokwi hardware configuration (ESP32 DevKit)
- `wokwi.toml` - Wokwi CLI configuration and port forwarding
- `prepare-firmware.sh` - Script to copy built firmware to test directory
- `run-simulator.sh` - Script to start the Wokwi simulator

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
   pio run -e esp32dev_compat
   ```

3. Prepare firmware for testing:
   ```bash
   ./test/wokwi/prepare-firmware.sh esp32dev_compat
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

## Extending Tests

To add more tests:
1. Edit `test/playwright/wokwi-basic.spec.js`
2. Add new test cases using Playwright's `test()` function
3. Follow the existing pattern of checking for console errors

## Troubleshooting

### Simulator doesn't start
- Check that firmware.bin exists in test/wokwi/
- Verify Wokwi CLI is installed: `wokwi-cli --version`
- Check Wokwi CLI logs for errors

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
