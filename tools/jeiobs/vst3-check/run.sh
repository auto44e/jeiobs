#!/usr/bin/env bash
# Reproducible runtime check for the obs-vst3 host (tasks 001 and 003). Linux only.
#
#   tools/jeiobs/vst3-check/run.sh BUILD_DIR [DEPS_DIR]
#
# BUILD_DIR: an ENABLE_VST3=ON build (see docs/jeiobs/BUILDING.md).
# DEPS_DIR:  from tools/jeiobs/linux-deps.sh (default ../jeiobs-deps).
#
# Every OBS run uses a throwaway HOME and the offscreen Qt platform, and loads
# plugins only from that HOME. Nothing outside /tmp and DEPS_DIR is written.
# Builds two GUI-free Steinberg sample effects (VST SDK 3.8.0, MIT; 3 classes),
# six deliberately broken bundles and two throwaway OBS test plugins (probes).
# Every run is time-limited, and any scanner, probe or fixture process the run
# leaves behind is killed. Results are printed as PASS/FAIL lines; exit status
# is 0 only if every expectation holds.
set -uo pipefail
ulimit -c 0                     # crashing fixtures must not leave core files in BIN
unset OBS_VST3_PROBE_TIMEOUT_MS # each run below sets the probe timeout it needs

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
# check LABEL CONDITION [DETAIL]: helpers run by CONDITION may add to DETAIL.
check() {
  DETAIL=
  if eval "$2"; then local r=PASS; else local r=FAIL; FAILS=$((FAILS + 1)); fi
  DETAIL+="${3:+${DETAIL:+, }$3}"
  echo "$r  $1${DETAIL:+  ($DETAIL)}"
}
[ -x "$BIN/obs-vst3-scanner" ] || { echo "No obs-vst3-scanner in $BIN (ENABLE_VST3=ON build?)" >&2; exit 2; }

# The command lines of the scanner, its probes and bad-noisy's grandchild all
# contain the sandbox HOME, so this pattern finds exactly one run's processes.
mine() { pgrep -f -- "obs-vst3-scanner.*$1/"; }
reap() { pkill -KILL -f -- "obs-vst3-scanner.*$1/"; true; }
left() { # HOME: how many such processes remain after a grace of 20 polls (~3 s)
  local i; for i in $(seq 20); do mine "$1" >/dev/null || break; sleep 0.1; done; mine "$1" | wc -l; }
TPID= # the timeout wrapper of the run in progress; it signals its whole group
cleanup() { [ -n "$TPID" ] && kill "$TPID" 2>/dev/null; reap "$WORK"; }
trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM

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
bad() { # SOURCE BUNDLE: build bad-SOURCE.c as BUNDLE's module
  mk "$2" && gcc -shared -fPIC "$HERE/bad-$1.c" -o "$WORK/bad/$2.vst3/Contents/x86_64-linux/$2.so" ||
    { echo "bad-$1.c failed to compile" >&2; exit 2; }; }
mk invalid && head -c 4096 /dev/urandom >"$WORK/bad/invalid.vst3/Contents/x86_64-linux/invalid.so"
for b in crash hang noisy exit0; do bad $b $b; done
# infoonly: bad-crash's code under a valid, non-discardable moduleinfo.json that
# lists no audio class. The SDK's own parser confirms the file says just that.
bad crash infoonly
mkdir -p "$WORK/bad/infoonly.vst3/Contents/Resources" &&
  cp "$HERE/infoonly-moduleinfo.json" "$WORK/bad/infoonly.vst3/Contents/Resources/moduleinfo.json"
g++ -std=c++17 -I"$SDK" "$HERE/moduleinfo-check.cpp" "$SDK/public.sdk/source/vst/moduleinfo/moduleinfoparser.cpp" \
  -o "$WORK/moduleinfo-check" || { echo "moduleinfo-check failed to compile" >&2; exit 2; }
[ "$("$WORK/moduleinfo-check" "$HERE/infoonly-moduleinfo.json")" = "classes=1 audio=0 discardable=0" ] ||
  { echo "infoonly-moduleinfo.json is not a valid moduleinfo without audio classes" >&2; exit 2; }
GEN="$(dirname "$(find "$BUILD" -name obsconfig.h | head -1)")"
for p in dsp quit; do
  g++ -std=c++17 -fPIC -shared "$HERE/$p-probe.cpp" -o "$WORK/$p-probe.so" \
    -I"$GEN" -I"$SRCROOT/libobs" -I"$SRCROOT/frontend/api" \
    -I"$Q/include" -I"$Q/include/QtCore" -I"$Q/include/QtGui" -I"$Q/include/QtWidgets" \
    -L"$LIB" -l:libobs.so.30 -l:libobs-frontend-api.so.30 -L"$Q/lib" -lQt6Widgets -lQt6Gui -lQt6Core ||
    { echo "probe $p failed to compile" >&2; exit 2; }
done
python3 -I - "$WORK/burst.wav" <<'EOF'
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
# scan and obs run in the background so an interrupted check can stop them
# (TPID). Both set RC (exit status) and MS (wall-clock milliseconds).
ms() { echo $(( $(date +%s%N) / 1000000 )); }
scan() { # HOME SECS: the scanner alone, probe timeout 2 s; stdout scan.out, stderr scan.err
  local h="$1" t0; t0=$(ms)
  (cd "$BIN" && exec env HOME="$h" OBS_VST3_PROBE_TIMEOUT_MS=2000 timeout -k 5 "$2" \
    ./obs-vst3-scanner "$h/list.json") >"$h/scan.out" 2>"$h/scan.err" & TPID=$!
  wait "$TPID" 2>/dev/null; RC=$?; TPID=; MS=$(( $(ms) - t0 )); }
nplug() { python3 -I -c "import json,sys;print(len(json.load(open(sys.argv[1]))['plugins']))" "$1/list.json" 2>/dev/null || echo 0; }
# kept HOME [BUNDLE=N]...: the scan exited 0 within 10 s, its list holds the 3
# good classes, stdout has one VST3_SCAN_DONE, and stdout names each BUNDLE N
# times as "[VST3 Scanner] Skipped <path>: <reason>".
kept() {
  local h="$1" ok=0 e n d; shift
  n=$(nplug "$h"); d=$(grep -c VST3_SCAN_DONE "$h/scan.out")
  DETAIL+="${DETAIL:+, }exit $RC, $n classes, $MS ms, VST3_SCAN_DONE x$d"
  [ "$RC" = 0 ] && [ "$n" = 3 ] && [ "$MS" -le 10000 ] && [ "$d" = 1 ] || ok=1
  for e in "$@"; do
    n=$(grep -c -- "\[VST3 Scanner\] Skipped $h/.vst3/${e%=*}.vst3: " "$h/scan.out")
    DETAIL+=", ${e%=*} skipped x$n"; [ "$n" = "${e#*=}" ] || ok=1
  done
  return $ok; }
obs() { # HOME PROBE SECS [ENV...]
  local h="$1" p="$2" t="$3" t0; shift 3
  mkdir -p "$h/.config/obs-studio/plugins/$p/bin/64bit" && cp "$WORK/$p.so" "$h/.config/obs-studio/plugins/$p/bin/64bit/"
  t0=$(ms)
  # No audio server: the default profile's Desktop Audio capture would otherwise
  # mix whatever this machine is playing into the measured output.
  (cd "$BIN" && exec env "$@" HOME="$h" XDG_CONFIG_HOME="$h/.config" QT_QPA_PLATFORM=offscreen \
    PULSE_SERVER=unix:/nonexistent/jeiobs-vst3check PIPEWIRE_REMOTE=/nonexistent/jeiobs-vst3check \
    timeout -k 5 "$t" ./obs --disable-updater --disable-missing-files-check) >"$h/obs.log" 2>&1 & TPID=$!
  wait "$TPID" 2>/dev/null; RC=$?; TPID=; MS=$(( $(ms) - t0 )); }
listed() { grep -c 'VST3 Scanner\]   ' "$1/obs.log"; }
GOOD=("$SAMPLES/adelay.vst3" "$SAMPLES/again-sample-accurate.vst3")
ADELAY=0CDBB66985D548A9BFD8371909D24BB3

echo "== scanner (standalone, OBS_VST3_PROBE_TIMEOUT_MS=2000)"
h=$(home empty); scan "$h" 20; check "no plugins: exit 1, NO_VST3"         "[ $RC = 1 ] && grep -q NO_VST3 '$h/scan.out'" "exit $RC"
h=$(home good "${GOOD[@]}"); scan "$h" 20; check "good plugins: 3 classes listed" "[ $RC = 0 ] && [ \$(nplug '$h') = 3 ]" \
  "exit $RC, $(nplug "$h") classes"
h=$(home invalid "${GOOD[@]}" "$WORK/bad/invalid.vst3"); scan "$h" 20
check "invalid bundle skipped, good still listed"  "[ $RC = 0 ] && [ \$(nplug '$h') = 3 ]" "exit $RC, $(nplug "$h") classes"
h=$(home crash "${GOOD[@]}" "$WORK/bad/crash.vst3"); scan "$h" 20
check "crash bundle: skipped once, 3 classes kept" "kept '$h' crash=1"
h=$(home hang "${GOOD[@]}" "$WORK/bad/hang.vst3"); scan "$h" 20; n=$(left "$h")
check "hang bundle: probe times out, skipped once, 3 classes kept, no process left" "kept '$h' hang=1 && [ $n = 0 ]" "$n left"
h=$(home mixed "${GOOD[@]}" "$WORK/bad/"{crash,hang,invalid}.vst3); scan "$h" 30; n=$(left "$h")
check "crash + hang + invalid: each bad one skipped once, 3 classes kept, no process left" \
  "kept '$h' crash=1 hang=1 && [ $n = 0 ]" "$n left"
# bad-noisy prints a fake VST3_SCAN_DONE, then crashes; its grandchild keeps
# every inherited fd open for 30 s.
h=$(home noisy "${GOOD[@]}" "$WORK/bad/noisy.vst3"); scan "$h" 20
check "noisy bundle (junk output, fd-holding grandchild, crash): skipped once, 3 classes kept" "kept '$h' noisy=1"
reap "$h"
h=$(home exit0 "${GOOD[@]}" "$WORK/bad/exit0.vst3"); scan "$h" 20
check "exit0 bundle (clean exit, no result): skipped once, 3 classes kept" "kept '$h' exit0=1"
# Loading infoonly's code would crash; the crash check above shows that a probe
# crash is reported, so no Skipped line here means its code was not loaded.
h=$(home infoonly "${GOOD[@]}" "$WORK/bad/infoonly.vst3"); scan "$h" 20
check "moduleinfo-only bundle (no audio class): code not loaded, not skipped, 3 classes kept" "kept '$h' infoonly=0"

echo "== host (real OBS, quit after load)"
h=$(home hgood "${GOOD[@]}"); obs "$h" quit-probe 60
check "loads, lists 3 plugins, exits cleanly, 0 leaks" \
  "[ $RC = 0 ] && grep -q 'OBS-VST3 filter loaded successfully' '$h/obs.log' && [ \$(grep -c 'VST3 Scanner\]   ' '$h/obs.log') = 3 ] && grep -q 'memory leaks: 0' '$h/obs.log'" \
  "exit $RC, $(listed "$h") listed, $MS ms"
h=$(home hcrash "${GOOD[@]}" "$WORK/bad/crash.vst3"); obs "$h" quit-probe 60
check "crash bundle: OBS survives and exits cleanly" "[ $RC = 0 ] && grep -q 'memory leaks: 0' '$h/obs.log'" "exit $RC, $MS ms"
check "crash bundle: OBS still lists the 3 good classes" "[ $(listed "$h") = 3 ]" \
  "$(listed "$h") listed, $(grep -c 'crash\.vst3' "$h/obs.log") log line(s) name crash.vst3"
# The 60 s probe timeout keeps the scan running past the quit (~2 s after
# loading), so OBS can only exit promptly if it stops the scan.
h=$(home hhang "${GOOD[@]}" "$WORK/bad/hang.vst3"); obs "$h" quit-probe 30 OBS_VST3_PROBE_TIMEOUT_MS=60000; n=$(left "$h")
grep -q 'Available plugins' "$h/obs.log" && s=finished || s=interrupted
check "hang bundle: OBS quits mid-scan with exit 0 within 30 s, 0 leaks, no scanner left" \
  "[ $RC = 0 ] && [ $s = interrupted ] && grep -q 'memory leaks: 0' '$h/obs.log' && [ $n = 0 ]" \
  "exit $RC, $MS ms, scan $s, $n left"
reap "$h"
# A fake VST3_SCAN_DONE and pipes held open for 30 s must not confuse OBS.
h=$(home hnoisy "${GOOD[@]}" "$WORK/bad/noisy.vst3"); obs "$h" quit-probe 25
check "noisy bundle: OBS lists the 3 good classes, exits 0 within 25 s, 0 leaks" \
  "[ $RC = 0 ] && [ $(listed "$h") = 3 ] && grep -q 'memory leaks: 0' '$h/obs.log'" "exit $RC, $(listed "$h") listed, $MS ms"
reap "$h"

echo "== audio through the filter (tone onset in the final mix)"
onset() { grep -oE 'onset_s=[0-9.]+' "$1/obs.log" | cut -d= -f2; }
h=$(home dry "${GOOD[@]}"); obs "$h" dsp-probe 60 DSP_WAV="$WORK/burst.wav"; dry=$(onset "$h")
h=$(home wet "${GOOD[@]}"); obs "$h" dsp-probe 60 DSP_WAV="$WORK/burst.wav" DSP_FILTER=$ADELAY DSP_PATH=unused; wet=$(onset "$h")
wetok=$(grep -cE 'peak=0\.(49|50)[0-9]* nonfinite=0' "$h/obs.log")
echo "INFO  onset dry=${dry:-none}s ADelay=${wet:-none}s"
check "ADelay delays the tone by ~1 s (0.95-1.15), level kept, no NaN/Inf" \
  "[ -n '$dry' ] && [ -n '$wet' ] && [ $wetok = 1 ] && python3 -I -c 'import sys;d=$wet-$dry;sys.exit(0 if 0.95<=d<=1.15 else 1)'"

echo "work dir: $WORK"
echo "$FAILS failed expectation(s)"
[ "$FAILS" = 0 ]
