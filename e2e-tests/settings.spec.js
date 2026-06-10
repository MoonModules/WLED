// @ts-check
const { test, expect } = require('./fixtures');

/**
 * Test that all settings pages load without JavaScript errors.
 * WLED serves settings pages from compiled-in firmware data via URL routes:
 *   /settings       → main settings page
 *   /settings/wifi  → WiFi settings
 *   /settings/leds  → LED settings
 *   /settings/ui    → UI settings
 *   /settings/sync  → Sync interfaces
 *   /settings/time  → Time & macros
 *   /settings/sec   → Security & updates
 *   /settings/dmx   → DMX output
 *   /settings/um    → Usermods
 *   /settings/2D    → 2D configuration
 *
 * Note: The paths like /settings_wifi.htm are filesystem-based and won't work
 * without LittleFS content. Use the route-based URLs instead.
 */
test.describe('WLED Settings Pages', () => {
  const settingsPages = [
    { path: '/settings', name: 'Main Settings' },
    { path: '/settings/wifi', name: 'WiFi Settings' },
    { path: '/settings/leds', name: 'LED Settings' },
    { path: '/settings/ui', name: 'UI Settings' },
    { path: '/settings/sync', name: 'Sync Settings' },
    { path: '/settings/time', name: 'Time Settings' },
    { path: '/settings/sec', name: 'Security Settings' },
    { path: '/settings/um', name: 'Usermod Settings' },
    { path: '/settings/2D', name: '2D Settings' },
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
      await page.waitForTimeout(2000);
      
      // Check that the page loaded with WLED title
      await expect(page).toHaveTitle(/WLED/);
      
      // Check for JavaScript uncaught exceptions
      expect(pageErrors, `Page errors in ${name}: ${pageErrors.join(', ')}`).toHaveLength(0);
    });
  }
});
