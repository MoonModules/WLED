Import('env')
import os
from datetime import datetime, timezone

# Only set VERSION if this is a nightly build (indicated by environment variable)
if os.environ.get('WLED_NIGHTLY_BUILD') == 'true':
    # VERSION format: yymmddb (b = build number, 0 for nightly)
    version_code = datetime.now(timezone.utc).strftime("%y%m%d") + "0"
    env.Append(BUILD_FLAGS=[f"-DWLED_BUILD_VERSION={version_code}"])
    print(f"Nightly build: Setting VERSION to {version_code}")

