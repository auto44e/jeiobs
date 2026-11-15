#!/usr/bin/env bash
# Reproducible runtime check for the obs-vst3 host (task 001). Linux only.
#
#   tools/jeiobs/vst3-check/run.sh BUILD_DIR [DEPS_DIR]
#
# BUILD_DIR: an ENABLE_VST3=ON build (see docs/jeiobs/BUILDING.md).
# DEPS_DIR:  from tools/jeiobs/linux-deps.sh (default ../jeiobs-deps).
#
# Every OBS run uses a throwaway HOME and the offscreen Qt platform, and loads
# plugins only from that HOME. Nothing outside /tmp and DEPS_DIR is written.
# Builds two GUI-free Steinberg sample effects (VST SDK 3.8.0, MIT), three
# deliberately broken bundles, and three throwaway test plugins (probes).
# Results are printed; exit status is 0 only if every expectation holds.
set -uo pipefail

BUILD="$(cd "${1:?usage: run.sh BUILD_DIR [DEPS_DIR]}" && pwd)"
HERE="$(cd "$(dirname "$0")" && pwd)"
SRCROOT="$(cd "$HERE/../../.." && pwd)"
DEPS="$(cd "${2:-$SRCROOT/../jeiobs-deps}" && pwd)"
Q="$DEPS/Qt/6.11.1/gcc_64"
BIN="$BUILD/rundir/Debug/bin"
LIB="$BUILD/rundir/Debug/lib/x86_64-linux-gnu"
WORK="$(mktemp -d /tmp/jeiobs-vst3check.XXXX)"
export LD_LIBRARY_PATH="$DEPS/ffmpeg-8.0.3/lib:$DEPS/mbedtls-3.6.7/lib:$Q/lib"
FAILS=0
check() { if eval "$2"; then echo "PASS  $1"; else echo "FAIL  $1"; FAILS=$((FAILS + 1)); fi; }
[ -x "$BIN/obs-vst3-scanner" ] || { echo "No obs-vst3-scanner in $BIN (ENABLE_VST3=ON build?)" >&2; exit 2; }

# --- 1. Sample effects from the pinned SDK (VSTGUI and hosting examples off) --
SDK="$DEPS/vst3-full/vst3sdk"
if [ "$(git -C "$SDK" rev-parse HEAD 2>/dev/null)" != 9fad9770f2ae8542ab1a548a68c1ad1ac690abe0 ]; then
  rm -rf "$SDK" && mkdir -p "$DEPS/vst3-full"
  git clone -q --filter=blob:none --no-checkout https://github.com/steinbergmedia/vst3sdk.git "$SDK"
  git -C "$SDK" -c advice.detachedHead=false checkout -q -f 9fad9770f2ae8542ab1a548a68c1ad1ac690abe0
  git -C "$SDK" submodule update --init -q --depth 1 base pluginterfaces public.sdk cmake
fi
SAMPLES="$SDK/build/VST3/Release"
if [ ! -f "$SAMPLES/adelay.vst3/Contents/x86_64-linux/adelay.so" ]; then
  cmake -S "$SDK" -B "$SDK/build" -G Ninja -DCMAKE_BUILD_TYPE=Release \
    -DSMTG_ENABLE_VSTGUI_SUPPORT=OFF -DSMTG_ENABLE_VST3_HOSTING_EXAMPLES=OFF \
    -DSMTG_CREATE_PLUGIN_LINK=OFF -DSMTG_RUN_VST_VALIDATOR=OFF >"$WORK/sdk.log" 2>&1 &&
    ninja -C "$SDK/build" adelay again-sample-accurate >>"$WORK/sdk.log" 2>&1 ||
    { echo "SDK sample build failed, see $WORK/sdk.log" >&2; exit 2; }
fi

# --- 2. Broken bundles and probes ---------------------------------------------
mk() { mkdir -p "$WORK/bad/$1.vst3/Contents/x86_64-linux"; }
mk invalid && head -c 4096 /dev/urandom >"$WORK/bad/invalid.vst3/Contents/x86_64-linux/invalid.so"
mk crash && gcc -shared -fPIC "$HERE/bad-crash.c" -o "$WORK/bad/crash.vst3/Contents/x86_64-linux/crash.so"
mk hang && gcc -shared -fPIC "$HERE/bad-hang.c" -o "$WORK/bad/hang.vst3/Contents/x86_64-linux/hang.so"
GEN="$(dirname "$(find "$BUILD" -name obsconfig.h | head -1)")"
for p in dsp quit; do
  g++ -std=c++17 -fPIC -shared "$HERE/$p-probe.cpp" -o "$WORK/$p-probe.so" \
    -I"$GEN" -I"$SRCROOT/libobs" -I"$SRCROOT/frontend/api" \
    -I"$Q/include" -I"$Q/include/QtCore" -I"$Q/include/QtGui" -I"$Q/include/QtWidgets" \
    -L"$LIB" -l:libobs.so.30 -l:libobs-frontend-api.so.30 -L"$Q/lib" -lQt6Widgets -lQt6Gui -lQt6Core ||
    { echo "probe $p failed to compile" >&2; exit 2; }
done
python3 - "$WORK/burst.wav" <<'EOF'
import math, struct, sys, wave
sr = 48000
w = wave.open(sys.argv[1], "wb"); w.setnchannels(2); w.setsampwidth(2); w.setframerate(sr)
w.writeframes(b"".join(struct.pack("<hh", s, s) for s in (
    int(0.5 * math.sin(2 * math.pi * 1000 * i / sr) * 32767) if 0.5 <= i / sr < 1.0 else 0
    for i in range(int(sr * 4.0)))))
w.close()
EOF

# home NAME BUNDLES... : fresh sandbox HOME with the given ~/.vst3 contents
home() { local h="$WORK/home-$1"; shift; rm -rf "$h"; mkdir -p "$h/.vst3"; for b in "$@"; do cp -r "$b" "$h/.vst3/"; done; echo "$h"; }
scan() { (cd "$BIN" && HOME="$1" timeout "$2" ./obs-vst3-scanner "$1/list.json" >"$1/scan.out" 2>&1); echo $?; }
nplug() { python3 -c "import json,sys;print(len(json.load(open(sys.argv[1]))['plugins']))" "$1/list.json" 2>/dev/null || echo 0; }
obs() { # HOME PROBE TIMEOUT [ENV...]
  local h="$1" p="$2" t="$3"; shift 3
  mkdir -p "$h/.config/obs-studio/plugins/$p/bin/64bit" && cp "$WORK/$p.so" "$h/.config/obs-studio/plugins/$p/bin/64bit/"
  (cd "$BIN" && env "$@" HOME="$h" XDG_CONFIG_HOME="$h/.config" QT_QPA_PLATFORM=offscreen \
    timeout "$t" ./obs --disable-updater --disable-missing-files-check >"$h/obs.log" 2>&1); echo $?; }
GOOD=("$SAMPLES/adelay.vst3" "$SAMPLES/again-sample-accurate.vst3")
ADELAY=0CDBB66985D548A9BFD8371909D24BB3

echo "== scanner (standalone)"
h=$(home empty);   rc=$(scan "$h" 20); check "no plugins: exit 1, NO_VST3"           "[ $rc = 1 ] && grep -q NO_VST3 '$h/scan.out'"
h=$(home good "${GOOD[@]}"); rc=$(scan "$h" 20); check "good plugins: 3 classes listed"  "[ $rc = 0 ] && [ \$(nplug '$h') = 3 ]"
h=$(home invalid "${GOOD[@]}" "$WORK/bad/invalid.vst3"); rc=$(scan "$h" 20)
check "invalid bundle skipped, good still listed"  "[ $rc = 0 ] && [ \$(nplug '$h') = 3 ]"
h=$(home crash "${GOOD[@]}" "$WORK/bad/crash.vst3"); rc=$(scan "$h" 20)
echo "INFO  crash bundle: scanner exit $rc, $(nplug "$h") plugins listed (known: one crash hides all)"
h=$(home hang "${GOOD[@]}" "$WORK/bad/hang.vst3"); rc=$(scan "$h" 15)
echo "INFO  hang bundle: scanner exit $rc (124 = still hung at 15 s; known: no timeout)"

echo "== host (real OBS, quit after load)"
h=$(home hgood "${GOOD[@]}"); rc=$(obs "$h" quit-probe 60)
check "loads, lists 3 plugins, exits cleanly, 0 leaks" \
  "[ $rc = 0 ] && grep -q 'OBS-VST3 filter loaded successfully' '$h/obs.log' && [ \$(grep -c 'VST3 Scanner\]   ' '$h/obs.log') = 3 ] && grep -q 'memory leaks: 0' '$h/obs.log'"
h=$(home hcrash "${GOOD[@]}" "$WORK/bad/crash.vst3"); rc=$(obs "$h" quit-probe 60)
check "crash bundle: OBS survives and exits cleanly" "[ $rc = 0 ] && grep -q 'Pipe closed before result' '$h/obs.log'"
h=$(home hhang "${GOOD[@]}" "$WORK/bad/hang.vst3"); rc=$(obs "$h" quit-probe 45)
pkill -f "obs-vst3-scanner $h" 2>/dev/null
echo "INFO  hang bundle: OBS exit $rc (124 = shutdown blocked joining the scan thread; known)"

echo "== audio through the filter (tone onset in the final mix)"
onset() { grep -oE 'onset_s=[0-9.]+' "$1/obs.log" | cut -d= -f2; }
h=$(home dry "${GOOD[@]}"); obs "$h" dsp-probe 60 DSP_WAV="$WORK/burst.wav" >/dev/null; dry=$(onset "$h")
h=$(home wet "${GOOD[@]}"); obs "$h" dsp-probe 60 DSP_WAV="$WORK/burst.wav" DSP_FILTER=$ADELAY DSP_PATH=unused >/dev/null; wet=$(onset "$h")
wetok=$(grep -cE 'peak=0\.(49|50)[0-9]* nonfinite=0' "$h/obs.log")
echo "INFO  onset dry=${dry:-none}s ADelay=${wet:-none}s"
check "ADelay delays the tone by ~1 s (0.95-1.15), level kept, no NaN/Inf" \
  "[ -n '$dry' ] && [ -n '$wet' ] && [ $wetok = 1 ] && python3 -c 'import sys;d=$wet-$dry;sys.exit(0 if 0.95<=d<=1.15 else 1)'"

echo "work dir: $WORK"
echo "$FAILS failed expectation(s)"
[ "$FAILS" = 0 ]
