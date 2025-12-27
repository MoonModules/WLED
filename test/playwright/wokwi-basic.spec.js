const { test, expect } = require('@playwright/test');

/**
 * Basic WLED-MM Web Interface Tests
 * These tests verify that the web interface loads correctly
 * and doesn't have JavaScript errors on basic pages.
 */

test.describe('WLED-MM Basic Web Interface', () => {
  let consoleErrors = [];
  let pageErrors = [];

  test.beforeEach(async ({ page }) => {
    // Reset error collectors
    consoleErrors = [];
    pageErrors = [];

    // Listen for console errors
    page.on('console', msg => {
      if (msg.type() === 'error') {
        consoleErrors.push(msg.text());
      }
    });

    // Listen for page errors
    page.on('pageerror', error => {
      pageErrors.push(error.message);
    });
  });

  test('should load main index page without errors', async ({ page }) => {
    await page.goto('/');
    
    // Wait for page to be loaded
    await page.waitForLoadState('networkidle');
    
    // Check for page title or main content
    const title = await page.title();
    expect(title).toBeTruthy();
    
    // Verify no JavaScript errors occurred
    expect(consoleErrors).toHaveLength(0);
    expect(pageErrors).toHaveLength(0);
    
    console.log('Main page loaded successfully');
  });

  test('should load settings page without errors', async ({ page }) => {
    await page.goto('/settings.htm');
    
    // Wait for page to be loaded
    await page.waitForLoadState('networkidle');
    
    // Verify no JavaScript errors occurred
    expect(consoleErrors).toHaveLength(0);
    expect(pageErrors).toHaveLength(0);
    
    console.log('Settings page loaded successfully');
  });

  test('should load WiFi settings page without errors', async ({ page }) => {
    await page.goto('/settings/wifi');
    
    // Wait for page to be loaded
    await page.waitForLoadState('networkidle');
    
    // Verify no JavaScript errors occurred
    expect(consoleErrors).toHaveLength(0);
    expect(pageErrors).toHaveLength(0);
    
    console.log('WiFi settings page loaded successfully');
  });

  test('should load LED settings page without errors', async ({ page }) => {
    await page.goto('/settings/leds');
    
    // Wait for page to be loaded
    await page.waitForLoadState('networkidle');
    
    // Verify no JavaScript errors occurred
    expect(consoleErrors).toHaveLength(0);
    expect(pageErrors).toHaveLength(0);
    
    console.log('LED settings page loaded successfully');
  });

  test('should load UI settings page without errors', async ({ page }) => {
    await page.goto('/settings/ui');
    
    // Wait for page to be loaded
    await page.waitForLoadState('networkidle');
    
    // Verify no JavaScript errors occurred
    expect(consoleErrors).toHaveLength(0);
    expect(pageErrors).toHaveLength(0);
    
    console.log('UI settings page loaded successfully');
  });

  test('should load edit page without errors', async ({ page }) => {
    await page.goto('/edit.htm');
    
    // Wait for page to be loaded
    await page.waitForLoadState('networkidle');
    
    // Verify no JavaScript errors occurred
    expect(consoleErrors).toHaveLength(0);
    expect(pageErrors).toHaveLength(0);
    
    console.log('Edit page loaded successfully');
  });

  test('should be able to check JSON API info', async ({ page }) => {
    const response = await page.goto('/json/info');
    
    expect(response?.status()).toBe(200);
    
    const json = await response?.json();
    expect(json).toBeTruthy();
    expect(json.ver).toBeTruthy(); // Should have version
    
    console.log('JSON API responding correctly, version:', json.ver);
  });

  test('should be able to check JSON API state', async ({ page }) => {
    const response = await page.goto('/json/state');
    
    expect(response?.status()).toBe(200);
    
    const json = await response?.json();
    expect(json).toBeTruthy();
    expect(json.on).toBeDefined(); // Should have on/off state
    
    console.log('JSON state API responding correctly');
  });
});
