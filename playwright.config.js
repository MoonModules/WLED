// @ts-check
const { defineConfig, devices } = require('@playwright/test');

/**
 * Configuration for testing WLED running in QEMU or on real hardware
 * The web UI requires the ESP32 backend, so we test against the actual firmware
 * 
 * Set WLED_BASE_URL environment variable to point to QEMU or hardware:
 * - QEMU: http://localhost:8080 (after running firmware in QEMU)
 * - Hardware: http://<device-ip>
 * 
 * @see https://playwright.dev/docs/test-configuration
 */
module.exports = defineConfig({
  testDir: './e2e-tests',
  fullyParallel: false, // Run tests sequentially to avoid overloading QEMU
  forbidOnly: !!process.env.CI,
  retries: process.env.CI ? 1 : 0,
  workers: 1, // Single worker to avoid race conditions with QEMU
  reporter: 'html',
  timeout: 45000, // 45 seconds per test (QEMU can be slow)
  use: {
    baseURL: process.env.WLED_BASE_URL || 'http://localhost',
    trace: 'on-first-retry',
    screenshot: 'only-on-failure',
    navigationTimeout: 20000, // 20 seconds for navigation (QEMU startup)
    actionTimeout: 15000, // 15 seconds for actions
  },

  projects: [
    {
      name: 'chromium',
      use: { ...devices['Desktop Chrome'] },
    },
  ],
});
