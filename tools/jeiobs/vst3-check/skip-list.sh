# Checks for task 005 (task 003 decision D6): the scanner remembers bundles
# that failed in vst3skip.json. Linux only. Sourced by run.sh after its own
# checks; uses its helpers (home, check, want, note, ms, scan_wait, kids, freeze,
# froze, pstate, skips, reap, gone, die) and variables (WORK, BIN, HERE, SAMPLES,
# GOOD, GOOD_IDS, ADELAY, TPID, T0, RC, MS, FROZE, KIDS, h).
#
# Every scan here runs the scanner alone with its cache in $h/c (vst3list.json,
# vst3skip.json, vst3scan.lock), a 2 s probe timeout and a 20 s limit. Bundles
# built from marked.c append their module's path to $HOME/loads.log whenever a
# probe loads them, so "not loaded again" is counted, not inferred. A check that
# this machine cannot run prints SKIP and does not change the exit status.
[ -n "${WORK-}" ] && declare -F check >/dev/null || { echo "skip-list.sh is sourced by run.sh" >&2; exit 2; }

# --- 5. Skip-list bundles ---------------------------------------------------------
F="$WORK/skip"
AGAIN=("$SAMPLES/again-sample-accurate.vst3") # 2 of GOOD's classes; ADelay comes from bundles below
AGAIN_IDS="C18D3C1E719E4E29924D3ECAA5E4DA18 CC48BF25529043DA80223510FFE8BD02"
mkdir -p "$F/adelay-as/scoped.vst3/Contents/x86_64-linux" &&
  cp "$SAMPLES/adelay.vst3/Contents/x86_64-linux/adelay.so" "$F/adelay-impl.so" || die "cannot prepare $F"
marked() { # MODE BUNDLE [ARCH_DIR [NAME]]: build marked.c in MODE (crash, hang or shim)
  # as $F/BUNDLE.vst3/Contents/ARCH_DIR/NAME.so (default: x86_64-linux, BUNDLE)
  local -a defs=()
  local dir="$F/$2.vst3/Contents/${3:-x86_64-linux}"
  case $1 in
    crash) ;;
    hang) defs=(-DHANG) ;;
    shim) defs=(-DSHIM "-DADELAY_SO=\"$F/adelay-impl.so\"") ;;
    *) die "marked: unknown mode $1" ;;
  esac
  mkdir -p "$dir" && gcc -shared -fPIC "${defs[@]}" "$HERE/marked.c" -o "$dir/${4:-$2}.so" ||
    die "marked.c ($1) failed to compile for $2"
}
for b in mcrash mcrash2 scoped; do marked crash "$b"; done
marked hang mhang
marked shim shim
# Binaries the SDK cannot resolve: no x86_64-linux directory (a module for
# another architecture only), and that directory without <stem>.so (another .so).
marked crash noarch aarch64-linux
marked crash noso x86_64-linux other
# ADelay as a bundle whose code is probed: another name and no moduleinfo.json.
cp "$F/adelay-impl.so" "$F/adelay-as/scoped.vst3/Contents/x86_64-linux/scoped.so" || die "cannot build adelay-as"

# --- 6. Skip-list runs and predicates ---------------------------------------------
SCAN=0 # scans so far in the current home; scan N writes scanN.out and scanN.err
shome() { home "$@"; C="$h/c"; SCAN=0; } # NAME BUNDLE...: like home; the cache is $C
sscan_start() { # [NAME=VALUE...] [--ARG...] [foreground] [ulimit-u]: start the scanner on $h.
  # foreground: timeout keeps it in this shell's process group, so a stopped orphan
  # probe gets no SIGHUP when the scanner dies. ulimit-u: RLIMIT_NPROC 1, so the
  # scanner cannot start a probe.
  local -a envs=() args=() pre=() opts=()
  local a
  for a in "$@"; do
    case $a in
      foreground) opts=(--foreground) ;;
      ulimit-u) pre=(bash -c 'ulimit -S -u 1 && exec "$@"' ulimit-u) ;;
      --*) args+=("$a") ;;
      *=*) envs+=("$a") ;;
      *) die "sscan: unknown argument $a" ;;
    esac
  done
  SCAN=$((SCAN + 1)); T0=$(ms)
  (cd "$BIN" && exec env HOME="$h" OBS_VST3_PROBE_TIMEOUT_MS=2000 "${envs[@]}" timeout "${opts[@]}" -k 5 20 \
    "${pre[@]}" ./obs-vst3-scanner "$C/vst3list.json" "${args[@]}") >"$h/scan$SCAN.out" 2>"$h/scan$SCAN.err" &
  TPID=$!
}
sscan() { sscan_start "$@"; scan_wait; }
SCANNER= # the scanner process of the scan in progress (the timeout wrapper's child)
scanner() { kids "$TPID"; SCANNER=${KIDS[0]-}; }
mod() { echo "$h/.vst3/$1.vst3/Contents/x86_64-linux/$1.so"; } # BUNDLE: the module the SDK loads
sl() { # SUBCOMMAND ARG...: skiplist.py, its summary noted
  local s; s=$(python3 -I "$HERE/skiplist.py" "$@") || { note "$s"; return 1; }; note "$s"; }
about() { # BUNDLE SUBCOMMAND ARG...: the same, noted as about BUNDLE
  local b=$1 s; shift; s=$(python3 -I "$HERE/skiplist.py" "$@") || { note "$b $s"; return 1; }; note "$b $s"; }
# Predicates on the last scan in $h and on the files it left.
ran() { # the scan exited 0 and said VST3_SCAN_DONE once
  local d; d=$(grep -c '^VST3_SCAN_DONE$' "$h/scan$SCAN.out" 2>/dev/null)
  note "s$SCAN: exit $RC"; [ "$d" = 1 ] || note "VST3_SCAN_DONE x${d:-?}"; [ "$RC" = 0 ] && [ "$d" = 1 ]; }
ids() { local s; s=$(python3 -I "$HERE/results.py" ids "$C/vst3list.json" "$@") || { note "$s"; return 1; }; note "$s"; }
line() { skips "scan$SCAN.out" "$@"; } # BUNDLE N [MODE]: this scan's Skipped lines for BUNDLE
loaded() { about "$1" loads "$h/loads.log" "$(mod "$1")" "$2"; } # BUNDLE N: loads of its module so far
nocode() { # BUNDLE: no module inside BUNDLE was ever loaded
  local n; n=$(grep -cF -- "$(realpath -m "$h/.vst3/$1.vst3")/" "$h/loads.log" 2>/dev/null)
  note "$1 ${n:-0} module load(s)"; [ "${n:-0}" = 0 ]; }
entry() { about "$1" entry "$C/vst3skip.json" "$h/.vst3/$1.vst3" "$(mod "$1")" "$2"; } # BUNDLE FAILURES
noentry() { about "$1" noentry "$C/vst3skip.json" "$h/.vst3/$1.vst3"; }
backoff() { about "$1" backoff "$C/vst3skip.json" "$h/.vst3/$1.vst3"; }
listed_at() { about "$2" at "$C/vst3list.json" "$1" "$h/.vst3/$2.vst3"; } # ID BUNDLE
nskipped() { sl nskipped "$h/scan$SCAN.out" "$@"; } # [N]
# Edits between scans; each notes what it did.
moved_past() { about "$1" past "$C/vst3skip.json" "$h/.vst3/$1.vst3"; } # BUNDLE: retry_after
OLDVER=
setver() { # BUNDLE: give its entry another obs_version; OLDVER is the one it had
  OLDVER=$(python3 -I "$HERE/skiplist.py" version "$C/vst3skip.json" "$h/.vst3/$1.vst3" 0.0.0-vst3check) ||
    { note "$1 obs_version: $OLDVER"; OLDVER=; return 1; }
  note "$1 obs_version $OLDVER set to 0.0.0-vst3check"; }
version_is() { # BUNDLE VERSION: its entry's obs_version is VERSION
  local v; v=$(python3 -I "$HERE/skiplist.py" version "$C/vst3skip.json" "$h/.vst3/$1.vst3") ||
    { note "$1 obs_version: $v"; return 1; }
  note "$1 obs_version $v"; [ -n "$2" ] && [ "$v" = "$2" ]; }
flip() { # BUNDLE: change one byte of its module, then touch -r it back to its old mtime
  local so s; so=$(mod "$1")
  cp -p "$so" "$h/flip.orig" && python3 -I "$HERE/skiplist.py" flip "$so" >/dev/null &&
    touch -r "$h/flip.orig" "$so" || die "cannot change a byte of $so"
  s=$(python3 -I "$HERE/skiplist.py" flipped "$so" "$h/flip.orig") || die "no same-size, same-mtime change of $so: $s"
  note "$1 module changed: $s"; }
corrupt() { # [bak]: vst3skip.json becomes a torn write; with bak, its valid content is
  # saved as vst3skip.json.bak first, otherwise any vst3skip.json.bak is removed
  local bad=0
  if [ "${1-}" = bak ]; then
    cp "$C/vst3skip.json" "$C/vst3skip.json.bak" 2>/dev/null || { note "no vst3skip.json to keep as .bak"; bad=1; }
  else
    rm -f "$C/vst3skip.json.bak" || die "cannot remove $C/vst3skip.json.bak"
  fi
  printf '{"version": 1, "bundles": [{"path": ' >"$C/vst3skip.json" || die "cannot write $C/vst3skip.json"
  if [ "${1-}" = bak ]; then note "vst3skip.json torn, valid .bak"; else note "vst3skip.json torn, no .bak"; fi
  [ "$bad" = 0 ]; }
snapc() { python3 -I "$HERE/skiplist.py" snap "$C" "$h/c.snap" >/dev/null || die "cannot snapshot $C"; }
plant() { # NAME...: files in $C, last modified 2 h ago
  local n; for n in "$@"; do printf 'stale\n' >"$C/$n" && touch -d '2 hours ago' "$C/$n" || die "cannot create $C/$n"; done; }
removed() { # NAME...: none of them is left in $C
  local n left=(); for n in "$@"; do [ ! -e "$C/$n" ] || left+=("$n"); done
  if [ "${#left[@]}" = 0 ]; then note "removed $*"; else note "left: ${left[*]}"; return 1; fi; }
kept() { if [ -e "$C/$1" ]; then note "$1 kept"; else note "$1 removed"; return 1; fi; }
# The lock. A holder is a flock process; its child carries $h in its command
# line, so reap and the EXIT trap end it like any process of this run.
HOLDER=
hold() { # SECS: another process takes $C/vst3scan.lock (flock -x) and keeps it for SECS
  rm -f "$h/held"
  flock -o -x "$C/vst3scan.lock" bash -c ': >"$1" && exec -a "obs-vst3-scanner lock holder $1/" sleep "$2"' \
    hold "$h/held" "$1" </dev/null >/dev/null 2>&1 &
  HOLDER=$!
  local i; for ((i = 0; i < 60; i++)); do [ -e "$h/held" ] && break; sleep 0.05; done
  [ -e "$h/held" ] || die "flock could not take $C/vst3scan.lock"
  note "flock holds vst3scan.lock for $1 s"; }
release() { # end the holder, which frees the lock
  [ -n "$HOLDER" ] || return 0
  kids "$HOLDER"; [ "${#KIDS[@]}" = 0 ] || kill -KILL "${KIDS[@]}" 2>/dev/null
  wait "$HOLDER" 2>/dev/null; HOLDER=; }
lockfd() { # PID: PID has a file descriptor open on $C/vst3scan.lock
  local f lock; lock=$(realpath -m "$C/vst3scan.lock")
  for f in /proc/"$1"/fd/*; do [ "$(readlink "$f" 2>/dev/null)" = "$lock" ] && return 0; done; return 1; }
holds() { # PID WHO: PID (a WHO) has vst3scan.lock open
  [ -n "$1" ] && [ -d "/proc/$1/fd" ] || { note "no $2 to inspect"; return 1; }
  if lockfd "$1"; then note "$2 $1 has vst3scan.lock open"; else note "$2 $1 has no fd on vst3scan.lock"; return 1; fi; }
nolock() { # PID WHO: PID (a WHO) runs and has no fd on vst3scan.lock (/proc/PID/fd)
  [ -n "$1" ] && [ -d "/proc/$1/fd" ] || { note "no $2 to inspect"; return 1; }
  if lockfd "$1"; then note "$2 $1 has vst3scan.lock open"; return 1; else note "$2 $1 has no fd on vst3scan.lock"; fi; }
lockstate() { # free|busy: whether flock -n can take vst3scan.lock now
  local rc; flock -n -E 75 "$C/vst3scan.lock" true; rc=$?
  case $rc in 0) note "vst3scan.lock free" ;; 75) note "vst3scan.lock busy" ;; *) note "flock -n failed ($rc)" ;; esac
  case $1 in free) [ "$rc" = 0 ] ;; busy) [ "$rc" = 75 ] ;; *) return 1 ;; esac; }
running() { [ -n "$1" ] && case $(pstate "$1") in gone | Z) false ;; *) true ;; esac; } # PID
released_first() { # the holder had ended when the scan ended, so the scanner waited for it
  if running "$HOLDER"; then note "lock still held when the scan ended ($MS ms)"; return 1; fi
  note "scan ended after the lock was freed ($MS ms)"; }
blocked_at() { # MS: MS into the scan it still runs, without VST3_SCAN_DONE, and $C is as snapped
  while [ $(($(ms) - T0)) -lt "$1" ]; do sleep 0.05; done
  local bad=0
  running "$TPID" || { note "scan over before $1 ms"; bad=1; }
  ! grep -q '^VST3_SCAN_DONE$' "$h/scan$SCAN.out" || { note "VST3_SCAN_DONE before $1 ms"; bad=1; }
  sl same "$C" "$h/c.snap" || bad=1
  [ "$bad" = 1 ] || note "still waiting at $1 ms"
  [ "$bad" = 0 ]; }
waiting() { # the scanner opens vst3scan.lock within 3 s and still waits 0.5 s later
  local i
  for ((i = 0; i < 60; i++)); do
    scanner; [ -n "$SCANNER" ] && lockfd "$SCANNER" && break
    running "$TPID" || { note "scan ended without waiting for the lock"; return 1; }
    SCANNER=; sleep 0.05
  done
  [ -n "$SCANNER" ] || { note "the scanner did not open vst3scan.lock within 3 s"; return 1; }
  sleep 0.5
  if running "$SCANNER" && ! grep -q '^VST3_SCAN_DONE$' "$h/scan$SCAN.out"; then
    note "scanner $SCANNER waits for vst3scan.lock"
  else
    note "scanner $SCANNER did not wait"; return 1
  fi; }
cancel() { # SIGKILL the scanner, as the host's cancel does
  scanner
  if running "$SCANNER" && kill -KILL "$SCANNER" 2>/dev/null; then scan_wait; note "scanner killed (exit $RC)"
  else scan_wait; note "no scanner left to kill (exit $RC)"; fi; }
orphan() { # the frozen probe outlived its scanner: still stopped, now with another parent
  local a b st pp
  [ -n "$FROZE" ] && { read -r a b st pp _ <"/proc/$FROZE/stat"; } 2>/dev/null || { note "no frozen probe left"; return 1; }
  note "probe $FROZE state $st, parent $pp"; [ "$st" = T ] && [ "$pp" != "$SCANNER" ]; }
SKIPS=0
skipped() { SKIPS=$((SKIPS + 1)); echo "SKIP  $1  ($2)"; } # LABEL WHY

echo "== skip list (task 005; scanner alone, cache in \$h/c, OBS_VST3_PROBE_TIMEOUT_MS=2000)"
shome skip-crash "${GOOD[@]}" "$F/mcrash.vst3"
check "#1 crash: probed and remembered (scan 1), then skipped as remembered without a load (scan 2), VST3_SKIPPED 1, 3 classes kept" '
  sscan; want ran; want ids $GOOD_IDS; want loaded mcrash 1; want line mcrash 1 fresh; want entry mcrash 1
  sscan; want ran; want ids $GOOD_IDS; want loaded mcrash 1; want line mcrash 1 remembered; want nskipped 1
  want entry mcrash 1'
check "#3 --retry-failed: the remembered crash is probed again (failures 2), then skipped as remembered" '
  sscan --retry-failed; want ran; want ids $GOOD_IDS; want loaded mcrash 2; want line mcrash 1 fresh
  want entry mcrash 2
  sscan; want ran; want loaded mcrash 2; want line mcrash 1 remembered'

shome skip-hang "${GOOD[@]}" "$F/mhang.vst3"
check "#2 hang: timed out and probed on scans 1 and 2 (failures 1, 2; retry_after = last failure + 7 d), skipped as remembered on scan 3" '
  sscan; want ran; want loaded mhang 1; want line mhang 1 timeout; want entry mhang 1
  sscan; want ran; want loaded mhang 2; want line mhang 1 timeout; want entry mhang 2; want backoff mhang
  sscan; want ran; want ids $GOOD_IDS; want loaded mhang 2; want line mhang 1 remembered; want nskipped 1
  note "$MS ms"; want gone'
check "#2 hang: retry_after moved into the past -> probed again (failures 3), backed off again" '
  want moved_past mhang
  sscan; want ran; want loaded mhang 3; want line mhang 1 timeout; want entry mhang 3; want backoff mhang'
check "#2 hang: obs_version changed -> probed once despite the backoff (failures kept: 4, version rewritten), then skipped" '
  want setver mhang
  sscan; want ran; want loaded mhang 4; want line mhang 1 timeout; want entry mhang 4; want version_is mhang "$OLDVER"
  sscan; want ran; want loaded mhang 4; want line mhang 1 remembered; want gone'

shome skip-flip "${GOOD[@]}" "$F/mcrash.vst3"
check "#4 fingerprint: one byte changed, size and mtime kept (touch -r) -> probed again with failures reset to 1, then skipped" '
  sscan; want ran; want loaded mcrash 1; want entry mcrash 1
  sscan --retry-failed; want ran; want loaded mcrash 2; want entry mcrash 2
  sscan; want ran; want loaded mcrash 2; want line mcrash 1 remembered
  flip mcrash
  sscan; want ran; want loaded mcrash 3; want line mcrash 1 fresh; want entry mcrash 1
  sscan; want ran; want loaded mcrash 3; want line mcrash 1 remembered'

# The shim crashes only while VST3CHK_SHIM_CRASH is set; it is not the binary
# that changes, so only Rescan (--retry-failed) can bring ADelay back.
shome skip-shim "${AGAIN[@]}" "$F/shim.vst3"
check "#5 shim to ADelay that crashes with VST3CHK_SHIM_CRASH=1: remembered, still skipped without it, listed and forgotten after --retry-failed" '
  sscan VST3CHK_SHIM_CRASH=1; want ran; want ids $AGAIN_IDS; want loaded shim 1; want line shim 1 fresh
  want entry shim 1
  sscan; want ran; want ids $AGAIN_IDS; want loaded shim 1; want line shim 1 remembered
  sscan --retry-failed; want ran; want ids $AGAIN_IDS $ADELAY; want listed_at $ADELAY shim; want loaded shim 2
  want line shim 0; want noentry shim
  sscan; want ran; want ids $AGAIN_IDS $ADELAY; want loaded shim 2'

# RLIMIT_NPROC does not bind root, so as root the probe would start.
if [ "$(id -u)" = 0 ]; then
  skipped "#6 scanner-side failure (ulimit -u 1)" "running as root, where RLIMIT_NPROC does not apply"
else
  shome skip-nproc "${GOOD[@]}" "$F/mcrash.vst3"
  check "#6 scanner-side failure (ulimit -u 1, no probe can start): nothing recorded; the next scan probes and remembers it" '
    sscan ulimit-u; want ran; want ids $GOOD_IDS; want loaded mcrash 0; want line mcrash 1 fresh; want noentry mcrash
    sscan; want ran; want loaded mcrash 1; want entry mcrash 1
    sscan; want ran; want loaded mcrash 1; want line mcrash 1 remembered'
fi

shome skip-unresolved "${GOOD[@]}" "$F/"{noarch,noso,mcrash}.vst3
check "#7 unresolved binary (no x86_64-linux/; no <stem>.so): probed on both scans, never remembered, no code loaded; a crash beside them is remembered" '
  sscan; want ran; want ids $GOOD_IDS; want line noarch 1 fresh; want line noso 1 fresh; want noentry noarch
  want noentry noso; want loaded mcrash 1; want entry mcrash 1
  sscan; want ran; want line noarch 1 fresh; want line noso 1 fresh; want noentry noarch; want noentry noso
  want loaded mcrash 1; want line mcrash 1 remembered; want nocode noarch; want nocode noso'

shome skip-corrupt "${GOOD[@]}" "$F/mcrash.vst3"
check "#8 corrupt vst3skip.json: with a valid .bak still skipped (no load); without .bak probed once more and rewritten" '
  sscan; want ran; want loaded mcrash 1; want entry mcrash 1
  want corrupt bak
  sscan; want ran; want ids $GOOD_IDS; want loaded mcrash 1; want line mcrash 1 remembered; want entry mcrash 1
  want corrupt
  sscan; want ran; want ids $GOOD_IDS; want loaded mcrash 2; want line mcrash 1 fresh; want entry mcrash 1'
check "#8 stale temp files (2 h old probe status marker and vst3list.json.tmp-<pid>): removed by the next scan, an unrelated file kept" '
  plant probe-1-1.json.status vst3list.json.tmp-1 unrelated.txt
  sscan; want ran; want removed probe-1-1.json.status vst3list.json.tmp-1; want kept unrelated.txt'

shome skip-lock "${GOOD[@]}" "$F/mcrash.vst3"
check "#9 lock held by another process for 4 s: the scanner waits (cache untouched at 1 s), then finishes with both files valid" '
  sscan; want ran; want entry mcrash 1
  hold 4; snapc
  sscan_start; want blocked_at 1000; scan_wait; want released_first; want ran; want ids $GOOD_IDS; want entry mcrash 1
  want line mcrash 1 remembered; want loaded mcrash 1; release; want gone'
check "#9 SIGKILL (host cancel) while waiting for the lock: cache dir unchanged (both files, stale temp files), no process left" '
  plant probe-1-1.json probe-1-1.json.tmp probe-1-1.json.status vst3list.json.tmp-1 vst3skip.json.tmp
  hold 10; snapc
  sscan_start; want waiting; cancel; want sl same "$C" "$h/c.snap"; release; want gone'
reap "$h"

# A frozen probe whose scanner is killed stays alive (stopped, no SIGHUP:
# foreground), so a lock it inherited would still be held.
shome skip-orphan "${GOOD[@]}" "$F/mhang.vst3"
check "#9 the scanner holds vst3scan.lock, its probe (frozen in mhang.so) has no fd on it; with the scanner killed the lock is free while the probe lives" '
  sscan_start foreground OBS_VST3_PROBE_TIMEOUT_MS=10000; freeze mhang; want froze; scanner
  want holds "$SCANNER" scanner; want nolock "$FROZE" probe; want lockstate busy
  cancel; want orphan; want lockstate free; reap "$h"; want gone'
reap "$h"

shome skip-shape "${GOOD[@]}" "$F/"{mcrash,mcrash2}.vst3
check "#10 vst3list.json keeps only version 1 and plugins (v1 class keys); VST3_SKIPPED 2 before VST3_SCAN_DONE for two remembered crashes" '
  sscan; want ran; want sl v1list "$C/vst3list.json"; want nskipped; want loaded mcrash 1; want loaded mcrash2 1
  sscan; want ran; want sl v1list "$C/vst3list.json"; want nskipped 2; want loaded mcrash 1; want loaded mcrash2 1
  want line mcrash 1 remembered; want line mcrash2 1 remembered'

# Not an in-place replacement (a cached bundle replaced in place is not
# re-probed, a D6 non-goal): the path is gone for one scan in between.
shome skip-scoped "${AGAIN[@]}" "$F/adelay-as/scoped.vst3"
check "#11 valid, then failing at one path P (P removed for a scan in between): P not listed, remembered, one load over two scans" '
  sscan; want ran; want ids $AGAIN_IDS $ADELAY; want listed_at $ADELAY scoped
  rm -rf "$h/.vst3/scoped.vst3" || die "cannot remove P"
  sscan; want ran; want ids $AGAIN_IDS
  cp -r "$F/scoped.vst3" "$h/.vst3/" || die "cannot install the crash at P"
  sscan; want ran; want ids $AGAIN_IDS; want loaded scoped 1; want line scoped 1 fresh; want entry scoped 1
  sscan; want ran; want ids $AGAIN_IDS; want loaded scoped 1; want line scoped 1 remembered'
