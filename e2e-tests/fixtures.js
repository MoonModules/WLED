// @ts-check
const base = require('@playwright/test');
const fs = require('fs');
const path = require('path');

const CONSOLE_LOG_DIR = path.join(__dirname, '..', 'console-logs');

// Ensure the console-logs directory exists
if (!fs.existsSync(CONSOLE_LOG_DIR)) {
	fs.mkdirSync(CONSOLE_LOG_DIR, { recursive: true });
}

/**
 * Extended test fixture that captures all browser console messages
 * and saves them as downloadable CI artifacts.
 */
exports.test = base.test.extend({
	page: async ({ page }, use, testInfo) => {
		const consoleLogs = [];

		// Capture ALL console messages (log, warn, error, info, debug)
		page.on('console', msg => {
			const entry = `[${msg.type().toUpperCase()}] ${msg.text()}`;
			consoleLogs.push(entry);
		});

		// Capture uncaught page errors
		page.on('pageerror', error => {
			consoleLogs.push(`[PAGE_ERROR] ${error.message}`);
		});

		await use(page);

		// After test completes, write console logs to file
		if (consoleLogs.length > 0) {
			const safeName = testInfo.titlePath.join(' - ')
				.replace(/[^a-zA-Z0-9_\-. ]/g, '_')
				.replace(/\s+/g, '_');
			const uniqueSuffix = `${testInfo.project.name}-w${testInfo.workerIndex}-r${testInfo.retry}`;
			const logFile = path.join(CONSOLE_LOG_DIR, `${safeName}-${uniqueSuffix}.log`);
			const logContent = consoleLogs.join('\n') + '\n';
			fs.writeFileSync(logFile, logContent);

			// Also attach to test report for visibility in Playwright HTML report
			await testInfo.attach('console-logs', {
				body: logContent,
				contentType: 'text/plain',
			});
		}
	},
});

exports.expect = base.expect;
