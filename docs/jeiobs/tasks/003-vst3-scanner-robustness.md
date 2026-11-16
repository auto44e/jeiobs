# Task 003: VST3 scanner robustness

Status: **Done on Linux** (branch `jeiobs/vst3-scanner-robustness`). Owner
approved changing inherited upstream code for this on 2026-11-15. Windows and
macOS are written for but **not compiled or run**.

## Problem

The runtime check in [task 001](001-vst3.md#runtime-evidence-2026-11-15-branch-jeiobsvst3-runtime-check)
(`tools/jeiobs/vst3-check/run.sh`) found these inherited behaviors on Linux:

1. **One crashing plugin hides all plugins.** `obs-vst3-scanner` loaded every
   bundle in one process. If one bundle segfaulted, the scanner exited 139 before
   writing the list. OBS survived ("Pipe closed before result"), but no VST3
   plugin could be selected.
2. **No scan timeout.** A bundle that hangs on load kept the scanner alive
   indefinitely.
3. **A hung scan blocked OBS exit.** `freeVst3List()` joined the scan thread
   from `obs_module_unload`, which waited for the hung scanner.

## Design

Agreed from two independent design reviews (Codex and Claude, read-only), then
adjusted after three rounds of adversarial code review.

- **The scanner is a coordinator that never loads plugin code.** Bundles with a
  valid, non-discardable `moduleinfo.json` are read without loading code (also
  when it lists no audio class). Every other bundle is loaded in a child,
  `obs-vst3-scanner --probe <bundle> <result file> <nonce> <timeout ms>
  <coordinator pid>`, one at a time. All code-load sites in `VST3Scanner` go
  through one probe hook, so the coordinator cannot load code by accident.
- **Probe child.** It starts a watchdog thread and waits until it runs (on
  Windows a new thread needs the loader lock, so this must happen before
  `LoadLibrary`). The watchdog hard-exits (`_exit` / `TerminateProcess`, code
  86) at the timeout or when the coordinator is gone (`getppid()` change on
  POSIX, the coordinator's process handle on Windows). Before loading the plugin,
  the child points its stdout/stderr at `/dev/null` (POSIX) or `NUL` (Windows,
  closing the original pipe handles), so plugin output and helper processes
  cannot reach or hold the coordinator's pipe. The result is a JSON file written atomically
  with the nonce; the child then hard-exits without running plugin unload code.
- **Coordinator.** Drains each probe's stdout and stderr concurrently, then
  waits for the probe to exit with a bound of timeout + `max(5 s, timeout/4)`
  from spawn, and kills it if it is still there (the watchdog may be unable to
  run, e.g. a stopped process). It accepts a result
  only with exit status 0, a readable file, the right nonce and non-empty class
  IDs. Anything else skips only that bundle, logged once:
  `[VST3 Scanner] Skipped <path>: <reason>` (`probe failed`, `probe timed out`,
  `probe timed out (killed)`, `probe produced no result`, ...). The host repeats
  the scanner's `[VST3 Scanner]` lines in the OBS log. `VST3_SCAN_DONE` is printed only if the cache was
  saved. Stale probe files older than an hour are removed at start.
- **Timeout.** 30 s per probe by default (slow but valid plugins on a cold
  start); `OBS_VST3_PROBE_TIMEOUT_MS` overrides it (clamped to 0.5–120 s). There
  is no overall deadline: a scan with many hanging plugins takes longer, but OBS
  is usable meanwhile (filters fall back to their saved plugin path) and exit is
  bounded by the kill below.
- **Host.** Two new libobs exports: `os_process_pipe_kill()` (POSIX `SIGKILL`,
  Windows `TerminateProcess`) and `os_process_pipe_wait(pp, timeout_ms)`
  (POSIX `waitid(…WNOWAIT)` polling, Windows `WaitForSingleObject`). Neither
  reaps; `os_process_pipe_destroy` still does, so a kill can never hit a reused
  PID. On
  unload, `cancelVst3Scan()` sets a flag and kills the coordinator under a mutex,
  then joins. Probes notice the coordinator is gone and exit.
- **Discovery.** Sorted and deterministic; permission errors and missing
  folders do not abort the scan. On Windows, bundle binaries are taken only from
  the build's architecture folder (`x86_64-win`; `arm64-win`; ARM64EC:
  `arm64ec-win`, `arm64x-win`, `x86_64-win`); single-file `.vst3` DLLs are kept.
- Also fixed: a discardable bundle that failed was loaded twice; a cache with
  zero usable plugins triggered a full re-probe.

## Evidence (Linux, Ubuntu 24.04, `ENABLE_VST3=ON`, warnings as errors)

`run.sh` now has 16 checks. Fixtures: the SDK samples (ADelay, AGain Sample
Accurate, Advanced Tutorial: class IDs pinned), `invalid` (random bytes),
`crash` (SIGSEGV on load), `hang` (never returns), `noisy` (writes fake markers
and junk, forks a grandchild that holds every inherited fd for 30 s, then
crashes), `exit0` (clean exit on load, no result), two moduleinfo-only
bundles on top of crashing code (zero audio classes; one made-up class), and a
hang probe frozen with SIGSTOP inside the plugin so that only the coordinator can
end it.

| Build | Result |
| --- | --- |
| this branch | **16 PASS, 0 FAIL**, about 50 s, no process left. Hang probe skipped in 2.1 s with a 2 s timeout; frozen probe killed by the coordinator at 7.0 s; OBS quits mid-scan in 1.4–2.8 s with 0 leaks while the scanner runs; the noisy grandchild is alive while the scan finishes in about 30 ms. |
| this branch, clean Ubuntu 24.04 container (`tools/jeiobs/cleanroom`, under `xvfb-run`) | **15 PASS, 0 FAIL** at `73b9c2190` (before the frozen-probe check existed) |
| round-2 code (`e5d433625`) | the frozen-probe check FAILS (still running at 20 s), the other 15 PASS |
| unfixed (main) | the 6 original checks PASS, **all new checks FAIL** (crash: exit 139 and no list; hang: still running at 20 s; OBS blocked at exit until the 30 s timeout). |

The script exits 2 (no verdict) if a needed tool is missing, if `pgrep`/`pkill`
do not work, or if `/usr/lib/vst3` or `/usr/local/lib/vst3` contain plugins,
which would make the class checks unreliable.

## Not done / follow-ups

- **Windows and macOS were not compiled or run.** Risks: handle inheritance
  and process spawning through libobs pipes, `TerminateProcess` and loader-lock
  behavior, the architecture-folder filter, macOS bundle loading per probe
  (Gatekeeper prompts now happen per probed bundle).
- Remember failed bundles between launches (a skip list keyed by path, mtime and
  size) so a hanging plugin does not cost a timeout on every start.
- Pre-existing upstream races, unchanged: `vst3Update` can read `list` while a
  rescan deletes it, and a rescan holds a strong source reference.

## Related findings (separate)

- `again-sample-accurate` silence is caused by the SDK sample (processor gain
  defaults to 0), not the host. The host does skip the controller sync
  (`getState` → `setComponentState`) after creating a plugin; fixing that is a
  small separate change and will not make AGain audible.
- ADelay's 68–100 ms of extra delay is not attributed; see task 001.
- `VST3Plugin::init` assigns `sampleRate = sampleRate;` (the parameter to
  itself). Harmless today because the member is not read.

## Decisions

Recorded in the [DECISIONS](../DECISIONS.md) format (swept in 2026-11-16, after
the merge). Correspondence: [debates/003-vst3-scanner](../debates/003-vst3-scanner/).

**D1. One probe process per code-loading bundle, sequential.** Why: a plugin
crash or hang can only be contained by a process boundary. Both design reviews
([02](../debates/003-vst3-scanner/02-design-codex.md),
[03](../debates/003-vst3-scanner/03-design-claude.md)) chose it. If otherwise:
an in-process scan with signal handlers cannot survive a hang or heap
corruption. Probing in parallel would leak pipe handles between probes on Windows
and make the order nondeterministic. Flex: parallel probes with a small pool if
first scans with many plugins become too slow; needs per-probe handle hygiene
first.

**D2. Result file plus nonce, not stdout markers.** Why: plugin code can write
anything to stdout, so a marker can be forged (the `noisy` fixture does exactly
that). If otherwise (Claude's stdout framing): every probe would need private
descriptors and a parser robust to arbitrary bytes. Flex: firm unless libobs
gains a private IPC channel.

**D3. libobs gains `os_process_pipe_kill` and `os_process_pipe_wait`.** Why: a
cancel file cannot interrupt a blocked read. Accepting the wait leaves exit
bounded only by the probe timeout. Round 2 showed a timer alone fails on POSIX
once the probe drops its pipe
([11](../debates/003-vst3-scanner/11-r2-review-impl-claude.md)). If otherwise:
OBS can hang at exit, and a frozen probe blocks the scan forever. Flex: firm.
Both are additive exports and could be offered upstream.

**D4. No overall scan deadline.** Why: it can only act between probes, it would
silently cut off large first scans, and the kill bounds shutdown anyway (Claude,
[03](../debates/003-vst3-scanner/03-design-claude.md)). If otherwise (Codex's
30 s overall): a machine with many slow plugins would never finish a first scan.
Flex: revisit together with D6.

**D5. 30 s per-probe default.** Why: slow but valid plugins on a cold start
(antivirus, licence checks) must not be dropped
([07](../debates/003-vst3-scanner/07-r1-review-impl-claude.md)). If otherwise
(3–10 s): valid plugins get skipped on slow machines and look broken. Flex:
lower it once D6 remembers failures.

**D6. Failed bundles are not remembered yet.** Why: it needs a fingerprint
schema (path, mtime, size) and its own tests; it was kept out of an already
large PR. If otherwise: every launch pays the timeout for each hanging plugin.
Flex: **next task**. A skip list beside `vst3list.json`, retried on Rescan or
when the fingerprint changes.
