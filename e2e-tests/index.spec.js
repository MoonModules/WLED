// @ts-check
const { test, expect } = require('./fixtures');

/**
 * Test that the main index page loads without JavaScript errors.
 * The WLED web UI is served from compiled-in firmware data via the '/' route.
 * The '/index.htm' path only serves from LittleFS (custom uploaded UI) and
 * won't work when there's no filesystem content.
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

    // The main UI is served at '/' (or '/sliders') from compiled-in PAGE_index
    await page.goto('/');
    
    // Wait for page to be loaded (don't wait for networkidle as API calls may hang)
    await page.waitForLoadState('load');
    
    // Wait a bit for initial JavaScript to execute
    await page.waitForTimeout(3000);
    
    // Check that the page title is set
    await expect(page).toHaveTitle(/WLED/);
    
    // Check for JavaScript errors
    expect(pageErrors, `Page errors found: ${pageErrors.join(', ')}`).toHaveLength(0);
    
    // Console errors are informational only for now - many expected due to missing API
    if (consoleErrors.length > 0) {
      console.log(`Console errors (informational): ${consoleErrors.length} errors`);
    }
  });

  test('should have basic UI elements', async ({ page }) => {
    await page.goto('/');
    await page.waitForLoadState('load');
    await page.waitForTimeout(3000);
    
    // Check for the picker container (color wheel)
    const pickerContainer = page.locator('#picker');
    await expect(pickerContainer).toBeAttached();
    
    // Check for the controls/sliders container
    const controls = page.locator('#sliders');
    await expect(controls).toBeAttached();
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
