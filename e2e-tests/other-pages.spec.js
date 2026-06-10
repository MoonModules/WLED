// @ts-check
const { test, expect } = require('./fixtures');

/**
 * Test other WLED pages load without JavaScript errors.
 * These pages are served from compiled-in firmware data via routes defined
 * in wled_server.cpp, NOT from LittleFS filesystem paths.
 */
test.describe('WLED Other Pages', () => {
  const otherPages = [
    { path: '/simple.htm', name: 'Simple Control' },
    { path: '/welcome', name: 'Welcome Page' },
    { path: '/update', name: 'Update Page' },
    { path: '/liveview', name: 'Live View' },
  ];

  for (const { path, name } of otherPages) {
    test(`${name} (${path}) should load without JavaScript errors`, async ({ page }) => {
      const pageErrors = [];

      // Listen for page errors (uncaught exceptions)
      page.on('pageerror', error => {
        pageErrors.push(error.message);
      });

      await page.goto(path);
      await page.waitForLoadState('load');
      await page.waitForTimeout(2000);
      
      // Check that the page loaded (these pages may have different titles)
      const title = await page.title();
      expect(title).toBeTruthy();
      
      // Check for JavaScript uncaught exceptions
      expect(pageErrors, `Page errors in ${name}: ${pageErrors.join(', ')}`).toHaveLength(0);
    });
  }
});
