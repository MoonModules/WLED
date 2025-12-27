/**
 * Simple test to verify cdata.js build process works
 */

const fs = require("fs");
const path = require("path");

console.log("Running cdata.js build test...");

// Check if required files exist
const requiredFiles = [
  "wled00/data/index.htm",
  "wled00/data/settings.htm",
  "wled00/html_ui.h"
];

let allFilesExist = true;

for (const file of requiredFiles) {
  const filePath = path.join(__dirname, "..", file);
  if (!fs.existsSync(filePath)) {
    console.error(`❌ Required file not found: ${file}`);
    allFilesExist = false;
  } else {
    console.log(`✓ Found: ${file}`);
  }
}

if (!allFilesExist) {
  console.error("\n❌ Some required files are missing. Please run 'npm run build' first.");
  process.exit(1);
}

console.log("\n✓ All tests passed!");
process.exit(0);
