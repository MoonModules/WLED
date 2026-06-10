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
    { path: '/settings', name: 'Main Settings', title: 'WLED Settings' },
    { path: '/settings/wifi', name: 'WiFi Settings', title: 'Wi-Fi Settings' },
    { path: '/settings/leds', name: 'LED Settings', title: 'LED Settings' },
    { path: '/settings/ui', name: 'UI Settings', title: 'UI Settings' },
    { path: '/settings/sync', name: 'Sync Settings', title: 'Sync Settings' },
    { path: '/settings/time', name: 'Time Settings', title: 'Time Settings' },
    { path: '/settings/sec', name: 'Security Settings', title: 'Misc Settings' },
    { path: '/settings/um', name: 'Usermod Settings', title: 'Usermod Settings' },
    { path: '/settings/2D', name: '2D Settings', title: '2D Set-up' },
  ];

  for (const { path, name, title } of settingsPages) {
    test(`${name} (${path}) should load without JavaScript errors`, async ({ page }) => {
      const pageErrors = [];

      // Listen for page errors (uncaught exceptions)
      page.on('pageerror', error => {
        pageErrors.push(error.message);
      });

      await page.goto(path);
      await page.waitForLoadState('load');
      await page.waitForTimeout(2000);
      
      // Check that the page loaded with expected title
      await expect(page).toHaveTitle(title);
      
      // Check for JavaScript uncaught exceptions
      expect(pageErrors, `Page errors in ${name}: ${pageErrors.join(', ')}`).toHaveLength(0);
    });
  }
});
