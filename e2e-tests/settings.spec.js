// @ts-check
const { test, expect } = require('@playwright/test');

/**
 * Test that all settings pages load without JavaScript errors
 */
test.describe('WLED Settings Pages', () => {
  const settingsPages = [
    { path: '/settings.htm', name: 'Main Settings' },
    { path: '/settings_wifi.htm', name: 'WiFi Settings' },
    { path: '/settings_leds.htm', name: 'LED Settings' },
    { path: '/settings_ui.htm', name: 'UI Settings' },
    { path: '/settings_sync.htm', name: 'Sync Settings' },
    { path: '/settings_time.htm', name: 'Time Settings' },
    { path: '/settings_sec.htm', name: 'Security Settings' },
    { path: '/settings_dmx.htm', name: 'DMX Settings' },
    { path: '/settings_um.htm', name: 'Usermod Settings' },
    { path: '/settings_2D.htm', name: '2D Settings' },
    { path: '/settings_pin.htm', name: 'Pin Settings' },
  ];

  for (const { path, name } of settingsPages) {
    test(`${name} (${path}) should load without JavaScript errors`, async ({ page }) => {
      const pageErrors = [];

      // Listen for page errors (uncaught exceptions)
      page.on('pageerror', error => {
        pageErrors.push(error.message);
      });

      await page.goto(path);
      await page.waitForLoadState('load');
      await page.waitForTimeout(1000);
      
      // Check that the page loaded (has title)
      await expect(page).toHaveTitle(/WLED/);
      
      // Check for JavaScript uncaught exceptions
      expect(pageErrors, `Page errors in ${name}: ${pageErrors.join(', ')}`).toHaveLength(0);
    });
  }
});
