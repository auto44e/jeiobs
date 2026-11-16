#!/usr/bin/env bash
# Run inside the clean-room container (see Dockerfile). Follows
# docs/jeiobs/BUILDING.md on a fresh GitHub clone of REF and prints STEP lines.
# Keep the step 4 command in sync with BUILDING.md.
set -uo pipefail
REF="${1:-origin/main}"
step() { echo "STEP $1: $2"; }
git config --global --add safe.directory "*"
cd ~ && git clone -q --filter=blob:none https://github.com/auto44e/jeiobs.git jeiobs && cd jeiobs && git switch -q --detach "$REF" || { step clone FAIL; exit 1; }
git log --oneline -1
python3 tools/jeiobs/check_scaffold.py && step scaffold PASS || step scaffold FAIL
git submodule update --init --depth 1 plugins/obs-browser plugins/obs-websocket >/dev/null 2>&1 \
  && step submodules PASS || step submodules FAIL
start=$(date +%s)
tools/jeiobs/linux-deps.sh > ~/deps.log 2>&1 && step deps "PASS ($(( $(date +%s) - start ))s)" \
  || { step deps FAIL; tail -20 ~/deps.log; exit 1; }
D=$PWD/../jeiobs-deps; FF=$D/ffmpeg-8.0.3; MB=$D/mbedtls-3.6.7; Q=$D/Qt/6.11.1/gcc_64
# Step 4 exactly as documented.
PKG_CONFIG_PATH=$FF/lib/pkgconfig:$MB/lib/pkgconfig \
cmake --preset ubuntu -B ../jeiobs-build/ubuntu \
  "-DCMAKE_PREFIX_PATH=$Q;$FF;$MB" -DQt6_DIR=$Q/lib/cmake/Qt6 \
  "-DCMAKE_BUILD_RPATH=$Q/lib;$FF/lib;$MB/lib" -DENABLE_BROWSER=OFF \
  -DSTEINBERG_VST3_PATH=$D/vst3 > ~/configure.log 2>&1 \
  && step configure PASS || { step configure FAIL; grep -m3 -A2 "CMake Error" ~/configure.log; exit 1; }
start=$(date +%s)
cmake --build ../jeiobs-build/ubuntu -j"$(nproc)" > ~/build.log 2>&1 \
  && step build "PASS ($(( $(date +%s) - start ))s, $(grep -c '^\[' ~/build.log) steps, $(grep -c 'warning:' ~/build.log) warnings)" \
  || { step build FAIL; grep -m5 "error" ~/build.log; exit 1; }
S=$(mktemp -d); cd ../jeiobs-build/ubuntu/rundir/Debug/bin
LD_LIBRARY_PATH=$FF/lib:$MB/lib:$Q/lib ./obs --version
LD_LIBRARY_PATH=$FF/lib:$MB/lib:$Q/lib HOME=$S XDG_CONFIG_HOME=$S/.config QT_QPA_PLATFORM=offscreen \
  timeout 25 xvfb-run -a ./obs --disable-updater --disable-missing-files-check > ~/smoke.log 2>&1; rc=$?
# 124 = still running at the timeout, which is the expected end of a smoke run.
grep -q "OBS-VST3 filter loaded successfully" ~/smoke.log && step vst3-module-loads PASS || step vst3-module-loads FAIL
[ "$rc" = 124 ] && grep -q "JeiOBS\|OBS " ~/smoke.log && step smoke "PASS (ran 25 s)" || step smoke "FAIL rc=$rc"
