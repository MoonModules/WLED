# WLED End-to-End (E2E) Tests

This directory contains Playwright-based end-to-end tests for the WLED web interface.

## Purpose

These tests verify that:
1. All web pages load without JavaScript errors when served from ESP32
2. Basic UI elements are present and functional
3. Pages can be navigated without issues
4. The web interface works correctly when served from the ESP32 firmware running in QEMU

**Important**: The WLED web UI is tightly coupled to the backend, so tests must run against the actual ESP32 firmware running in QEMU emulation.

## Running Tests Locally

### Prerequisites

```bash
# Install Node.js dependencies
npm ci

# Install Playwright browsers
npx playwright install --with-deps chromium

# Install PlatformIO for building firmware
pip install -r requirements.txt
```

### Test with QEMU ESP32 Emulator

Test the actual firmware running in QEMU ESP32 emulator:

**Important**: Use the V4 Sound Reactive ethernet build for QEMU testing, as WiFi is not emulated and causes crashes.

1. **Build the firmware**:
   ```bash
   npm run build  # Build web UI
   pio run -e esp32_4MB_V4_S_eth  # Build V4 SR ethernet firmware (WiFi disabled, 15+ min first time)
   ```

2. **Setup QEMU** (first time only):
   ```bash
   bash .github/scripts/setup-qemu.sh
   ```

3. **Run firmware in QEMU** (in a separate terminal):
   ```bash
   bash .github/scripts/run-qemu.sh .pio/build/esp32_4MB_V4_S_eth qemu-esp32 8080
   ```
   
   Wait ~30-45 seconds for ESP32 to boot and start the web server.

4. **Run tests**:
   ```bash
   WLED_BASE_URL=http://localhost:8080 npm run test:e2e
   ```

### Test with Real Hardware

To test against a real ESP32 device:

1. Flash firmware to your ESP32
2. Note the device IP address
3. Run tests:
   ```bash
   WLED_BASE_URL=http://<device-ip> npm run test:e2e
   ```

### Other Test Commands

```bash
# Run tests in UI mode (interactive)
WLED_BASE_URL=http://localhost:8080 npm run test:e2e:ui

# Run tests in debug mode
WLED_BASE_URL=http://localhost:8080 npm run test:e2e:debug

# Run a specific test file
WLED_BASE_URL=http://localhost:8080 npx playwright test e2e-tests/index.spec.js
```

## Test Structure

- `index.spec.js` - Tests for the main WLED UI page
- `settings.spec.js` - Tests for all settings pages
- `other-pages.spec.js` - Tests for other pages (simple, welcome, update, liveview)

## What Tests Check

Each test verifies:
- ✅ Page loads successfully from ESP32
- ✅ No JavaScript uncaught exceptions (page errors)
- ✅ Required UI elements are present
- ✅ Backend API endpoints respond correctly

## CI/CD Integration

The tests run automatically in GitHub Actions via `.github/workflows/qemu-e2e-test.yml`:

1. **build-firmware** - Builds ESP32 firmware with embedded web UI
2. **test-qemu** - Runs firmware in QEMU and tests with Playwright

## Viewing Test Results

After running tests:
- Console output shows pass/fail status
- HTML report: `playwright-report/index.html`
- Screenshots of failures (if any): `test-results/`

Open the HTML report:
```bash
npx playwright show-report
```

## Troubleshooting

**QEMU fails to start:**
- Ensure QEMU is installed: `bash .github/scripts/setup-qemu.sh`
- Check QEMU logs: `cat qemu-output.log`
- Verify firmware was built successfully

**Tests fail with connection errors:**
- Wait longer for ESP32 to boot (30-45 seconds minimum)
- Check if port 80 is accessible: `curl http://localhost/`
- Verify QEMU is still running: `ps aux | grep qemu`

**Tests timeout:**
- QEMU emulation is slow - tests have 45 second timeouts
- Real hardware is faster - adjust timeouts if needed
- Check QEMU output for boot errors

**Settings pages show "PIN required":**
- This is expected when WLED security PIN is enabled
- Tests verify the page loads even when authentication is required
- The PIN feature is working correctly

## QEMU Limitations

ESP32 QEMU emulation has limitations:
- **Network**: User-mode networking only (no raw ethernet)
- **WiFi**: Not emulated (returns mock data)
- **Peripherals**: Many are stubbed (LEDs, I2C, etc.)
- **Performance**: Slower than real hardware

Despite these limitations, QEMU is sufficient for testing:
- Web UI loads correctly
- JavaScript executes without errors  
- API endpoints respond
- Page navigation works

## Adding New Tests

1. Create a new `.spec.js` file in `e2e-tests/`
2. Follow the existing test pattern
3. Always check for page errors (uncaught exceptions)
4. Test against QEMU/hardware, not static files
5. Run tests locally before committing

Example:
```javascript
const { test, expect } = require('@playwright/test');

test('my new test', async ({ page }) => {
  const pageErrors = [];
  page.on('pageerror', error => {
    pageErrors.push(error.message);
  });
  
  await page.goto('/my-page.htm');
  await page.waitForLoadState('load');
  await page.waitForTimeout(2000);
  
  expect(pageErrors).toHaveLength(0);
});
```

## Future Enhancements

- [ ] Add JSON API endpoint validation tests
- [ ] Test WebSocket connections for real-time updates
- [ ] Add visual regression testing
- [ ] Test on multiple browsers (Firefox, Safari)
- [ ] Add performance/load testing
- [ ] Test with real ESP32 hardware in CI (if available)
- [ ] Improve QEMU boot time
