Import('env')  # PlatformIO-specific import
import json
import os
from datetime import datetime, timezone

PACKAGE_FILE = "package.json"

with open(PACKAGE_FILE, "r") as package:
    version = json.load(package)["version"]

# Handle nightly build
if os.environ.get('WLED_NIGHTLY_BUILD') == 'true':
    # VERSION format: yymmddb (b = build number, 0 for nightly)
    version_code = datetime.now(timezone.utc).strftime("%y%m%d") + "0"
    env.Append(BUILD_FLAGS=[f"-DWLED_BUILD_VERSION={version_code}"])
    print(f"Nightly build: Setting VERSION to {version_code}")
    
    # Update version tag: replace "-mdev" with "-nightly" or append "-nightly" if no tag
    if "-mdev" in version:
        version = version.replace("-mdev", "-nightly")
    elif "-" in version:
        # Replace any existing tag with -nightly (handles multiple hyphens correctly)
        version = version.rsplit("-", 1)[0] + "-nightly"
    else:
        # No tag present, append -nightly
        version = version + "-nightly"
    print(f"Nightly build: Using version string {version}")

env.Append(BUILD_FLAGS=[f"-DWLED_VERSION={version}"])
