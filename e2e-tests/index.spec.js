// @ts-check
const { test, expect } = require('@playwright/test');

/**
 * Test that the main index page loads without JavaScript errors
 */
test.describe('WLED Index Page', () => {
  test('should load index.htm without JavaScript errors', async ({ page }) => {
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

    await page.goto('/index.htm');
    
    // Wait for page to be loaded (don't wait for networkidle as API calls may hang)
    await page.waitForLoadState('load');
    
    // Wait a bit for initial JavaScript to execute
    await page.waitForTimeout(2000);
    
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
    await page.goto('/index.htm');
    await page.waitForLoadState('load');
    await page.waitForTimeout(2000);
    
    // Check for the picker container (it should at least be in the HTML)
    const pickerContainer = await page.locator('#picker');
    await expect(pickerContainer).toBeAttached();
    
    // Check for the controls container
    const controls = await page.locator('#sliders');
    await expect(controls).toBeAttached();
  });
});
