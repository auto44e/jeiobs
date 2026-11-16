#!/usr/bin/env bash
# Reproducible runtime check for the obs-vst3 host (tasks 001 and 003). Linux only.
#
#   tools/jeiobs/vst3-check/run.sh BUILD_DIR [DEPS_DIR]
#
# BUILD_DIR: an ENABLE_VST3=ON build (see docs/jeiobs/BUILDING.md).
# DEPS_DIR:  from tools/jeiobs/linux-deps.sh (default ../jeiobs-deps).
#
# Needs a machine without system VST3s (the scanner also searches /usr/lib/vst3
# and /usr/local/lib/vst3), e.g. the clean-room container. Every OBS run uses a
# throwaway HOME and the offscreen Qt platform, and loads plugins only from that
# HOME. Nothing outside /tmp and DEPS_DIR is written. Builds two GUI-free
# Steinberg sample effects (VST SDK 3.8.0, MIT; 3 classes), seven broken or
# moduleinfo-only bundles and two throwaway OBS test plugins (probes).
# Every run is time-limited, and any scanner, probe or fixture process the run
# leaves behind is killed, also one that a check has stopped (SIGSTOP) on
# purpose. Results are printed as PASS/FAIL lines; exit status
# is 0 only if every expectation holds, 1 if one fails, 2 if the check cannot run.
set -uo pipefail
ulimit -c 0                     # crashing fixtures must not leave core files in BIN
unset OBS_VST3_PROBE_TIMEOUT_MS # each run below sets the probe timeout it needs
die() { echo "$*" >&2; exit 2; } # the check cannot run, or cannot trust its tools

# --- 0. Preflight: tools, and no system VST3 that every scan would also see ---
for t in pgrep pkill timeout python3 gcc g++; do
  command -v "$t" >/dev/null || die "missing tool: $t"
done
for d in /usr/lib/vst3 /usr/local/lib/vst3; do
  [ -e "$d" ] || continue
  f="$(find -H "$d" -name '*.vst3' -print -quit)" || die "cannot search $d for VST3 bundles"
  [ -z "$f" ] || die "system VST3 $f would be in every scan; run this on a machine without" \
    "system VST3s, e.g. the clean-room container"
done
BUILD="$(cd "${1:?usage: run.sh BUILD_DIR [DEPS_DIR]}" && pwd)" || die "no build dir $1"
HERE="$(cd "$(dirname "$0")" && pwd)"
SRCROOT="$(cd "$HERE/../../.." && pwd)"
DEPS="$(cd "${2:-$SRCROOT/../jeiobs-deps}" && pwd)" || die "no deps dir ${2:-$SRCROOT/../jeiobs-deps}"
Q="$DEPS/Qt/6.11.1/gcc_64"
BIN="$BUILD/rundir/Debug/bin"
LIB="$BUILD/rundir/Debug/lib/x86_64-linux-gnu"
[ -x "$BIN/obs-vst3-scanner" ] || die "No obs-vst3-scanner in $BIN (ENABLE_VST3=ON build?)"
export LD_LIBRARY_PATH="$DEPS/ffmpeg-8.0.3/lib:$DEPS/mbedtls-3.6.7/lib:$Q/lib"
WORK="$(mktemp -d /tmp/jeiobs-vst3check.XXXX)" || die "cannot create a work dir in /tmp"

# The command lines of the scanner, its probes and bad-noisy's grandchild all
# contain the sandbox HOME, so this pattern finds exactly one run's processes.
# A pgrep or pkill failure stops the check; it never counts as "none left".
nprocs() { # DIR: set NP to the number of such processes under DIR
  local out rc pids
  out="$(pgrep -f -- "obs-vst3-scanner.*$1/")"; rc=$?
  case $rc in
    0) mapfile -t pids <<<"$out"; NP=${#pids[@]} ;;
    1) NP=0 ;;
    *) die "pgrep failed with status $rc" ;;
  esac
}
reap() { # DIR: SIGCONT, then SIGKILL all of them, so none is left stopped
  local s rc
  for s in CONT KILL; do
    pkill "-$s" -f -- "obs-vst3-scanner.*$1/"; rc=$?
    [ "$rc" -le 1 ] || die "pkill failed with status $rc"
  done
}
TPID= # the timeout wrapper of the run in progress; it signals its whole group
cleanup() { if [ -n "$TPID" ]; then kill "$TPID" 2>/dev/null; wait "$TPID" 2>/dev/null; fi; reap "$WORK"; }
trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM
# Self-test: a decoy shaped like one of this run's processes must be counted,
# then killed, or no "0 left" below would mean anything. (Started from a
# subshell, so it is not a job of this shell and its death is not announced.)
( (exec -a "obs-vst3-scanner $WORK/selftest/" sleep 30) </dev/null >/dev/null 2>&1 & )
for ((i = 0; i < 50; i++)); do nprocs "$WORK/selftest"; [ "$NP" = 1 ] && break; sleep 0.1; done
[ "$NP" = 1 ] || die "pgrep -f found $NP instead of 1 decoy process"
reap "$WORK/selftest"
for ((i = 0; i < 30; i++)); do nprocs "$WORK/selftest"; [ "$NP" = 0 ] && break; sleep 0.1; done
[ "$NP" = 0 ] || die "pkill -f did not stop the decoy process"

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
    die "SDK sample build failed, see $WORK/sdk.log"
fi
GOOD=("$SAMPLES/adelay.vst3" "$SAMPLES/again-sample-accurate.vst3")
# The audio classes in GOOD as a clean scan lists them (ids) and OBS logs them (names).
ADELAY=0CDBB66985D548A9BFD8371909D24BB3
GOOD_IDS="$ADELAY C18D3C1E719E4E29924D3ECAA5E4DA18 CC48BF25529043DA80223510FFE8BD02"
GOOD_NAMES=("ADelay" "AGain Sample Accurate" "Advanced Tutorial")

# --- 2. Broken and moduleinfo-only bundles, probes ----------------------------
mk() { mkdir -p "$WORK/bad/$1.vst3/Contents/x86_64-linux" || die "cannot create bundle $1"; }
bad() { # SOURCE BUNDLE: build bad-SOURCE.c as BUNDLE's module
  mk "$2"; gcc -shared -fPIC "$HERE/bad-$1.c" -o "$WORK/bad/$2.vst3/Contents/x86_64-linux/$2.so" ||
    die "bad-$1.c failed to compile"; }
mk invalid
head -c 4096 /dev/urandom >"$WORK/bad/invalid.vst3/Contents/x86_64-linux/invalid.so" || die "cannot write invalid.so"
for b in crash hang noisy exit0; do bad $b $b; done
# infoonly and infoone: bad-crash's code under a valid, non-discardable
# moduleinfo.json with one class, which is not an audio class (infoonly) or is
# one (infoone). The SDK's own parser confirms each file says just that.
g++ -std=c++17 -I"$SDK" "$HERE/moduleinfo-check.cpp" "$SDK/public.sdk/source/vst/moduleinfo/moduleinfoparser.cpp" \
  -o "$WORK/moduleinfo-check" || die "moduleinfo-check failed to compile"
modinfo() { # BUNDLE AUDIO_CLASSES
  [ "$("$WORK/moduleinfo-check" "$HERE/$1-moduleinfo.json")" = "classes=1 audio=$2 discardable=0" ] ||
    die "$1-moduleinfo.json is not a valid non-discardable moduleinfo with 1 class, $2 of them audio"
  bad crash "$1"
  mkdir -p "$WORK/bad/$1.vst3/Contents/Resources" &&
    cp "$HERE/$1-moduleinfo.json" "$WORK/bad/$1.vst3/Contents/Resources/moduleinfo.json" ||
    die "cannot install $1's moduleinfo.json"
}
modinfo infoonly 0
modinfo infoone 1
INFOONE=6A65696F627376737433636865636B32 # the one class in infoone-moduleinfo.json
[ "$(grep -c "\"CID\": \"$INFOONE\"" "$HERE/infoone-moduleinfo.json")" = 1 ] ||
  die "infoone-moduleinfo.json does not declare class $INFOONE"
GEN="$(dirname "$(find "$BUILD" -name obsconfig.h | head -1)")"
for p in dsp quit; do
  g++ -std=c++17 -fPIC -shared "$HERE/$p-probe.cpp" -o "$WORK/$p-probe.so" \
    -I"$GEN" -I"$SRCROOT/libobs" -I"$SRCROOT/frontend/api" \
    -I"$Q/include" -I"$Q/include/QtCore" -I"$Q/include/QtGui" -I"$Q/include/QtWidgets" \
    -L"$LIB" -l:libobs.so.30 -l:libobs-frontend-api.so.30 -L"$Q/lib" -lQt6Widgets -lQt6Gui -lQt6Core ||
    die "probe $p failed to compile"
done
python3 -I - "$WORK/burst.wav" <<'EOF' || die "cannot write burst.wav"
import math, struct, sys, wave
sr = 48000
w = wave.open(sys.argv[1], "wb"); w.setnchannels(2); w.setsampwidth(2); w.setframerate(sr)
w.writeframes(b"".join(struct.pack("<hh", s, s) for s in (
    int(0.5 * math.sin(2 * math.pi * 1000 * i / sr) * 32767) if 0.5 <= i / sr < 1.0 else 0
    for i in range(int(sr * 4.0)))))
w.close()
EOF

# --- 3. Runs ------------------------------------------------------------------
home() { # NAME BUNDLE...: fresh sandbox HOME $h whose ~/.vst3 holds copies of BUNDLEs
  h="$WORK/home-$1"; shift
  rm -rf "$h" && mkdir -p "$h/.vst3" || die "cannot create $h"
  local b; for b in "$@"; do cp -r "$b" "$h/.vst3/" || die "cannot copy $b into $h"; done
}
# scan and obs run in the background so an interrupted check can stop them
# (TPID). Both set RC (exit status) and MS (wall-clock milliseconds).
ms() { echo $(( $(date +%s%N) / 1000000 )); }
scan() { # SECS: the scanner alone on $h, probe timeout 2 s; stdout scan.out, stderr scan.err
  scan_start "$1"; scan_wait; }
scan_start() { # SECS: start that scan and return while it runs; scan_wait ends it
  T0=$(ms)
  (cd "$BIN" && exec env HOME="$h" OBS_VST3_PROBE_TIMEOUT_MS=2000 timeout -k 5 "$1" \
    ./obs-vst3-scanner "$h/list.json") >"$h/scan.out" 2>"$h/scan.err" & TPID=$!; }
scan_wait() { wait "$TPID" 2>/dev/null; RC=$?; TPID=; MS=$(( $(ms) - T0 )); }
# freeze BUNDLE: during a scan_start scan, SIGSTOP the scanner's --probe child
# for $h/.vst3/BUNDLE.vst3 once that child runs BUNDLE's code, so the probe's
# own watchdog can never end it. It must be in that code: a probe redirects its
# stdio before it loads the bundle, and only then does nothing but the scanner's
# own kill bound the scanner's wait. Polls every 50 ms until 3 s into the scan.
# Sets FROZE to the stopped PID (empty if none) and FROZE_NOTE to what happened.
kids() { # PID: set KIDS to PID's child processes
  local out rc; out="$(pgrep -P "$1")"; rc=$?
  case $rc in 0) mapfile -t KIDS <<<"$out" ;; 1) KIDS=() ;; *) die "pgrep failed with status $rc" ;; esac; }
isprobe() { # PID BUNDLE: PID runs with the arguments --probe $h/.vst3/BUNDLE.vst3 ...
  local a; mapfile -d '' -t a 2>/dev/null <"/proc/$1/cmdline" || return 1
  [ "${a[1]-}" = --probe ] && [ "${a[2]-}" = "$h/.vst3/$2.vst3" ]; }
FROZE= FROZE_NOTE=
freeze() {
  local so="/$1.vst3/Contents/x86_64-linux/$1.so" scanners s p st i
  FROZE= FROZE_NOTE="no $1 probe ran $1.so within 3 s"
  while [ $(( $(ms) - T0 )) -lt 3000 ]; do
    kill -0 "$TPID" 2>/dev/null || { FROZE_NOTE="scan ended before a $1 probe ran $1.so"; return; }
    kids "$TPID"; scanners=("${KIDS[@]}") # the scanner: the timeout wrapper's child
    for s in "${scanners[@]}"; do
      kids "$s"
      for p in "${KIDS[@]}"; do
        isprobe "$p" "$1" && grep -qF -- "$so" "/proc/$p/maps" 2>/dev/null || continue
        # Only count a stop this check caused: not already stopped, and kill worked.
        st=$(pstate "$p")
        if [ "$st" = T ] || ! kill -STOP "$p" 2>/dev/null; then
          FROZE_NOTE="$1 probe $p already stopped or could not be stopped (state $st)"; return
        fi
        for ((i = 0; i < 20; i++)); do st=$(pstate "$p"); [ "$st" = T ] && break; sleep 0.05; done
        if [ "$st" = T ] && isprobe "$p" "$1"; then
          FROZE=$p FROZE_NOTE="$1 probe stopped in $1.so at $(( $(ms) - T0 )) ms"
        else
          kill -CONT "$p" 2>/dev/null # not stopped, or no longer that probe: undo
          FROZE_NOTE="$1 probe $p not stopped (state $st)"
        fi
        return
      done
    done
    sleep 0.05
  done; }
obs() { # PROBE SECS [ENV...]: real OBS on $h with one test plugin; also sets SEEN
  local p="$1" t="$2" t0 i; shift 2
  mkdir -p "$h/.config/obs-studio/plugins/$p/bin/64bit" &&
    cp "$WORK/$p.so" "$h/.config/obs-studio/plugins/$p/bin/64bit/" || die "cannot install $p into $h"
  t0=$(ms)
  # No audio server: the default profile's Desktop Audio capture would otherwise
  # mix whatever this machine is playing into the measured output.
  (cd "$BIN" && exec env "$@" HOME="$h" XDG_CONFIG_HOME="$h/.config" QT_QPA_PLATFORM=offscreen \
    PULSE_SERVER=unix:/nonexistent/jeiobs-vst3check PIPEWIRE_REMOTE=/nonexistent/jeiobs-vst3check \
    timeout -k 5 "$t" ./obs --disable-updater --disable-missing-files-check) >"$h/obs.log" 2>&1 & TPID=$!
  # Meanwhile count this sandbox's scanner processes every 0.1 s; SEEN is the
  # most at once. Bounded: timeout -k 5 ends the run within SECS + 5 s.
  SEEN=0
  for ((i = 0; i < (t + 10) * 10; i++)); do
    kill -0 "$TPID" 2>/dev/null || break
    nprocs "$h"; [ "$NP" -gt "$SEEN" ] && SEEN=$NP
    sleep 0.1
  done
  wait "$TPID" 2>/dev/null; RC=$?; TPID=; MS=$(( $(ms) - t0 )); }

# --- 4. Expectations ----------------------------------------------------------
# check LABEL STEPS: STEPS is shell code made of `want PREDICATE ARGS...`. Every
# step runs, even after one fails, and adds what it measured to DETAIL; the
# check passes only if at least one step ran and none failed.
FAILS=0
check() {
  DETAIL= BAD=0 NW=0
  eval "$2"
  [ "$NW" -gt 0 ] || BAD=1
  if [ "$BAD" = 0 ]; then echo "PASS  $1  ($DETAIL)"; else FAILS=$((FAILS + 1)); echo "FAIL  $1  ($DETAIL)"; fi
}
want() { NW=$((NW + 1)); "$@" || BAD=1; return 0; }
also() { "$@" || true; } # measure, but do not require
note() { DETAIL+="${DETAIL:+, }$*"; }
# Predicates on the last run in $h.
status() { note "exit $RC"; [ "$RC" = "$1" ]; }
within() { note "$MS ms"; [ "$MS" -lt "$1" ]; } # LIMIT_MS
has() { grep -qF -- "$2" "$h/$1" || { note "no '$2' in $1"; return 1; }; } # FILE TEXT
classes() { local s; s=$(python3 -I "$HERE/results.py" ids "$h/list.json" "$@") || { note "$s"; return 1; }; note "$s"; }
names() { local s; s=$(python3 -I "$HERE/results.py" names "$h/obs.log" "$@") || { note "$s"; return 1; }; note "$s"; }
skips() { # FILE BUNDLE N [timeout]: N "Skipped <path>:" lines name $h/.vst3/BUNDLE.vst3
  local s; s=$(python3 -I "$HERE/results.py" skips "$h/$1" "$h/.vst3/$2.vst3" "${@:3}") ||
    { note "$2 $s"; return 1; }; note "$2 $s"; }
done1() { local d; d=$(grep -c VST3_SCAN_DONE "$h/scan.out" 2>/dev/null); note "VST3_SCAN_DONE x${d:-?}"; [ "$d" = 1 ]; }
kept() { # LIMIT_MS ID...: the scan exited 0 in time, listed exactly ID..., said VST3_SCAN_DONE once
  want status 0; want within "$1"; shift; want classes "$@"; want done1; }
leaks0() {
  local n; n=$(grep -oE 'Number of memory leaks: [0-9]+$' "$h/obs.log" 2>/dev/null | grep -oE '[0-9]+$')
  note "${n:-no} leak count"; [ "$n" = 0 ]; }
gone() { # none of $h's scanner, probe or fixture processes is left after a grace of ~3 s
  local i; for ((i = 0; i < 30; i++)); do nprocs "$h"; [ "$NP" = 0 ] && break; sleep 0.1; done
  note "$NP left"; [ "$NP" = 0 ]; }
scanning() { # the scanner was seen during the OBS run, and still ran when quit-probe asked to quit
  local q; q=$(grep -oE 'obs-vst3-scanner children running: -?[0-9]+$' "$h/obs.log" 2>/dev/null | grep -oE -- '-?[0-9]+$')
  note "scanner seen (max $SEEN process(es)), ${q:-no count of} scanner(s) running at the quit request"
  [ "$SEEN" -ge 1 ] && [ -n "$q" ] && [ "$q" -ge 1 ]; }
interrupted() {
  grep -q 'Available plugins' "$h/obs.log" 2>/dev/null
  case $? in 0) note "scan finished"; return 1 ;; 1) note "scan interrupted" ;; *) note "obs.log unreadable"; return 1 ;; esac; }
froze() { note "$FROZE_NOTE"; [ -n "$FROZE" ]; } # freeze stopped the probe it was after
pstate() { local a b s; { read -r a b s _ <"/proc/$1/stat"; } 2>/dev/null && echo "$s" || echo gone; }
GC= # bad-noisy's grandchild in $h, once alive() has confirmed it is that process
alive() { # the grandchild wrote its PID marker and is running now
  local pid st comm; GC=; pid=$(cat "$h/noisy-grandchild.pid" 2>/dev/null)
  case $pid in '' | *[!0-9]*) note "no grandchild PID marker"; return 1 ;; esac
  st=$(pstate "$pid"); comm=$(cat "/proc/$pid/comm" 2>/dev/null)
  if [ "$st" != gone ] && [ "$st" != Z ] && [ "$comm" = vst3chk-noisy ] &&
    tr '\0' ' ' 2>/dev/null <"/proc/$pid/cmdline" | grep -qF -- "$h/"; then
    GC=$pid; note "grandchild $pid alive"
  else
    note "grandchild $pid not running (state $st, name ${comm:-none})"; return 1
  fi; }
killed() { # SIGKILL the grandchild; it must have exited within ~3 s
  [ -n "$GC" ] || { note "no grandchild to kill"; return 1; }
  local i st; kill -KILL "$GC" 2>/dev/null
  for ((i = 0; i < 30; i++)); do st=$(pstate "$GC"); [ "$st" = gone ] || [ "$st" = Z ] && break; sleep 0.1; done
  case $st in gone) note "grandchild gone after SIGKILL" ;; Z) note "grandchild exited (unreaped) after SIGKILL" ;;
    *) note "grandchild still running (state $st) after SIGKILL"; return 1 ;; esac; }

echo "== scanner (standalone, OBS_VST3_PROBE_TIMEOUT_MS=2000)"
home empty; scan 20
check "no plugins: exit 1, NO_VST3" 'want status 1; want has scan.out NO_VST3'
home good "${GOOD[@]}"; scan 20
check "good plugins: the 3 classes listed" 'want status 0; want classes $GOOD_IDS'
home invalid "${GOOD[@]}" "$WORK/bad/invalid.vst3"; scan 20
check "invalid bundle skipped once, good still listed" 'kept 10000 $GOOD_IDS; want skips scan.out invalid 1'
home crash "${GOOD[@]}" "$WORK/bad/crash.vst3"; scan 20
check "crash bundle: skipped once, 3 classes kept" 'kept 10000 $GOOD_IDS; want skips scan.out crash 1'
home hang "${GOOD[@]}" "$WORK/bad/hang.vst3"; scan 20
check "hang bundle: probe times out, scan done in < 8 s, skipped once, 3 classes kept, no process left" \
  'kept 8000 $GOOD_IDS; want skips scan.out hang 1 timeout; want gone'
home mixed "${GOOD[@]}" "$WORK/bad/"{crash,hang,invalid}.vst3; scan 20
check "crash + hang + invalid: scan done in < 8 s, each bad one skipped once (hang as timeout), 3 classes kept, no process left" \
  'kept 8000 $GOOD_IDS; want skips scan.out crash 1; want skips scan.out hang 1 timeout
   want skips scan.out invalid 1; want gone'
# bad-noisy prints a fake VST3_SCAN_DONE and more on both streams, then crashes;
# its grandchild keeps every inherited fd open for 30 s. The scan must be done
# long before that, while the grandchild still runs.
home noisy "${GOOD[@]}" "$WORK/bad/noisy.vst3"; scan 20
check "noisy bundle (junk output, live fd-holding grandchild, crash): scan done in < 8 s, skipped once, 3 classes kept" \
  'want alive; kept 8000 $GOOD_IDS; want skips scan.out noisy 1; want killed; want gone'
reap "$h"
home exit0 "${GOOD[@]}" "$WORK/bad/exit0.vst3"; scan 20
check "exit0 bundle (clean exit, no result): skipped once, 3 classes kept" 'kept 10000 $GOOD_IDS; want skips scan.out exit0 1'
# Loading infoonly's or infoone's code would crash a probe, which the crash
# check shows is reported, so no Skipped line means neither was loaded; infoone's
# class can only have come from its moduleinfo.json.
home info "${GOOD[@]}" "$WORK/bad/"{infoonly,infoone}.vst3; scan 20
check "moduleinfo-only bundles (no audio class; one): code not loaded, neither skipped, 3 classes + infoone's listed" \
  'kept 10000 $GOOD_IDS $INFOONE; want skips scan.out infoonly 0; want skips scan.out infoone 0'
# A probe frozen inside the bundle's code (as if its watchdog could not run)
# must still be ended by the scanner's own kill, long before the 20 s limit.
home frozen "${GOOD[@]}" "$WORK/bad/hang.vst3"; scan_start 20; freeze hang; scan_wait
check "frozen probe (SIGSTOP inside hang.so): coordinator kills it, scan done in < 12 s, skipped once as killed, 3 classes kept, no process left" \
  'want froze; kept 12000 $GOOD_IDS; want skips scan.out hang 1 killed; want gone'
reap "$h"

echo "== host (real OBS, quit after load)"
home hgood "${GOOD[@]}"; obs quit-probe 60
check "loads, lists the 3 plugins, exits cleanly, 0 leaks" \
  'want status 0; want has obs.log "OBS-VST3 filter loaded successfully"; want names "${GOOD_NAMES[@]}"; want leaks0; note "$MS ms"'
home hcrash "${GOOD[@]}" "$WORK/bad/crash.vst3"; obs quit-probe 60
check "crash bundle: OBS survives and exits cleanly" 'want status 0; want leaks0; note "$MS ms"'
check "crash bundle: OBS still lists the 3 good classes" 'want names "${GOOD_NAMES[@]}"; also skips obs.log crash 1'
reap "$h"
# The 60 s probe timeout keeps the scan running past the quit (~2 s after
# loading), so OBS can only exit promptly if it stops the scan. quit-probe
# counts OBS's running scanner children when it asks to quit, since OBS exits
# too soon after that for the 0.1 s sampling above to be sure to see it.
home hhang "${GOOD[@]}" "$WORK/bad/hang.vst3"; obs quit-probe 30 OBS_VST3_PROBE_TIMEOUT_MS=60000 QUITPROBE_WAIT_SCANNER=1
check "hang bundle: OBS quits mid-scan with exit 0 within 15 s, 0 leaks, no scanner left" \
  'want status 0; want within 15000; want scanning; want interrupted; want leaks0; want gone'
reap "$h"
# A fake VST3_SCAN_DONE and pipes held open for 30 s must not confuse OBS or
# make it wait for the grandchild.
home hnoisy "${GOOD[@]}" "$WORK/bad/noisy.vst3"; obs quit-probe 25
check "noisy bundle: OBS lists the 3 good classes, exits 0 within 25 s while the grandchild lives, 0 leaks" \
  'want alive; want status 0; want within 25000; want names "${GOOD_NAMES[@]}"; want leaks0; want killed; want gone'
reap "$h"

echo "== audio through the filter (tone onset in the final mix)"
onset() { grep -oE 'onset_s=[0-9.]+' "$h/obs.log" | cut -d= -f2; }
home dry "${GOOD[@]}"; obs dsp-probe 60 DSP_WAV="$WORK/burst.wav"; dry=$(onset)
home wet "${GOOD[@]}"; obs dsp-probe 60 DSP_WAV="$WORK/burst.wav" DSP_FILTER=$ADELAY DSP_PATH=unused; wet=$(onset)
wetok=$(grep -cE 'peak=0\.(49|50)[0-9]* nonfinite=0' "$h/obs.log")
delayed() {
  note "onset dry ${dry:-none} s, ADelay ${wet:-none} s, $wetok line(s) with peak ~0.5 and no NaN/Inf"
  [ -n "$dry" ] && [ -n "$wet" ] && [ "$wetok" = 1 ] &&
    python3 -I -c 'import sys; d = float(sys.argv[2]) - float(sys.argv[1]); sys.exit(0 if 0.95 <= d <= 1.15 else 1)' "$dry" "$wet"
}
check "ADelay delays the tone by ~1 s (0.95-1.15), level kept, no NaN/Inf" 'want delayed'

echo "work dir: $WORK"
echo "$FAILS failed expectation(s)"
[ "$FAILS" = 0 ]
