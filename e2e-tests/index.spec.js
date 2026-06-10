// @ts-check
const { test, expect } = require('./fixtures');

/**
 * Test that the main index page loads without JavaScript errors.
 * The WLED web UI is served from compiled-in firmware data.
 *
 * NOTE: We navigate to '/sliders' (not '/') because '/' calls serveIndexOrWelcome()
 * which serves the welcome page when no WiFi config is saved (e.g. fresh boot in QEMU).
 * '/sliders' calls serveIndex() directly, always serving the full main UI.
 */
test.describe('WLED Index Page', () => {
  test('should load main UI without JavaScript errors', async ({ page }) => {
    const consoleErrors = [];
    const pageErrors = [];

    // Listen for console errors
    page.on('console', msg => {
      if (msg.type() === 'error') {
        consoleErrors.push(msg.text());
      }
    });

    // Listen for page errors (uncaught exceptions)
    page.on('pageerror', error => {
      pageErrors.push(error.message);
    });

    // Use /sliders — always serves the built-in main UI, bypasses welcome page check
    await page.goto('/sliders');
    await page.waitForLoadState('load');
    
    // Wait a bit for initial JavaScript to execute
    await page.waitForTimeout(3000);

    // Title is "WLED" from <title>WLED</title>, or device name (also contains WLED)
    await expect(page).toHaveTitle(/WLED/);
    
    // Check for JavaScript errors
    expect(pageErrors, `Page errors found: ${pageErrors.join(', ')}`).toHaveLength(0);
    
    // Console errors are informational only for now - many expected due to missing API
    if (consoleErrors.length > 0) {
      console.log(`Console errors (informational): ${consoleErrors.length} errors`);
    }
  });

  test('should have basic UI elements', async ({ page }) => {
    // Use /sliders — always serves the built-in main UI
    await page.goto('/sliders');
    await page.waitForLoadState('load');
    await page.waitForTimeout(3000);

    // Both `#picker` (color wheel) and `#sliders` are defined in index.htm
    await expect(page.locator('`#picker`')).toBeAttached();
    await expect(page.locator('`#sliders`')).toBeAttached();
  });

  test('JSON API /json/info should return valid data', async ({ page }) => {
    const response = await page.request.get('/json/info');
    expect(response.status()).toBe(200);
    const json = await response.json();
    // Check basic fields exist
    expect(json).toHaveProperty('ver');
    expect(json).toHaveProperty('vid');
    expect(json).toHaveProperty('leds');
  });

  test('JSON API /json/state should return valid data', async ({ page }) => {
    const response = await page.request.get('/json/state');
    expect(response.status()).toBe(200);
    const json = await response.json();
    // Check basic state fields
    expect(json).toHaveProperty('on');
    expect(json).toHaveProperty('bri');
  });
});
