#!/usr/bin/env bash
set -euo pipefail

# Keep verification memory-safe on desktop workloads.
WHACKER_BUILD_JOBS="${WHACKER_BUILD_JOBS:-2}"

echo "[1/5] Build debug tree"
cmake --build build -j"${WHACKER_BUILD_JOBS}"

echo "[2/5] Test debug tree"
rm -f build/Testing/Temporary/LastTestsFailed.log
ctest --test-dir build --output-on-failure

echo "[3/5] Build strict tree"
cmake --build /tmp/whacker_warnings8 -j"${WHACKER_BUILD_JOBS}" 2>&1 | tee /tmp/whacker_warnings8.log

echo "[4/5] Test strict tree"
rm -f /tmp/whacker_warnings8/Testing/Temporary/LastTestsFailed.log
ctest --test-dir /tmp/whacker_warnings8 --output-on-failure

echo "[5/5] Scan strict build log for compiler warnings"
rg -n "warning:" /tmp/whacker_warnings8.log
