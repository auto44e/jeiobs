# Round 1 test worker — Claude (summary)

_Raw peer correspondence, task 003 (2026-11-15/16). Codex tool-call output trimmed; prompt and final answer kept verbatim. AI-assisted, not human review._

## Prompt

````text
# Worker brief: JeiOBS task 003 — VST3 scanner robustness (TESTS)

You are the test worker. Work ONLY inside this git worktree:
`~/oss/jeiobs-wt/scanner-test` (branch
`wip/vst3-scanner-test`). Edit only files under `tools/jeiobs/vst3-check/`. No
network, no push. When done, `git add tools/jeiobs/vst3-check` and commit with
`git -c user.name=auto44e -c user.email=auto44e@gmail.com commit -m "..."`.

Read first: `tools/jeiobs/vst3-check/run.sh` (and its probes),
`docs/jeiobs/tasks/003-vst3-scanner-robustness.md`, and the scanner/host code in
`plugins/obs-vst3/` (`vst3-scan/vst3-scanner.cpp`, `VST3Scanner.cpp`,
`obs-vst3.cpp` scan section ~lines 169-353, `plugin-main.cpp`).

Another worker is implementing this design in parallel (you will not see its code):
- The scanner becomes a coordinator that never loads plugin code. Each bundle that
  needs a code load is probed in a child: `obs-vst3-scanner --probe <bundle>
  <result-file> <nonce> <timeout-ms>`, one at a time. The child has a watchdog that
  hard-exits at the timeout. A crash/timeout/garbage result skips only that bundle,
  logged once on the scanner's stdout as
  `[VST3 Scanner] Skipped <path>: <reason>`. Others are kept. The scan always ends
  with `VST3_SCAN_DONE` (exit 0) when any bundle exists, even if all probes failed.
- Per-probe timeout default 10000 ms, override with env `OBS_VST3_PROBE_TIMEOUT_MS`.
- Bundles with a valid `Contents/Resources/moduleinfo.json` that is not
  discardable are read WITHOUT loading code (even if it lists zero audio classes).
- libobs gains `os_process_pipe_kill`; on module unload OBS kills a running scan
  before joining, so OBS exits promptly even while a probe hangs, and no
  scanner/probe process is left behind.

## Your task
Update `run.sh` so the new expectations are real checks, and add fixtures:
1. Turn the INFO lines into checks (set `OBS_VST3_PROBE_TIMEOUT_MS=2000` for these
   runs):
   - standalone scanner with good + crash bundles: exit 0, 3 classes listed, one
     "Skipped" line naming crash.vst3.
   - standalone scanner with good + hang bundles: exit 0 within ~10 s (use
     `timeout 20`), 3 classes, one "Skipped" line naming hang.vst3.
   - standalone scanner with good + crash + hang + invalid: 3 classes, finishes.
   - host: OBS with good + hang bundles and a LONG probe timeout (e.g. 60000 so the
     scan is still running when quit-probe closes the window ~2 s after load) must
     exit 0 within the run's timeout (e.g. 30 s), log 0 memory leaks, and leave no
     `obs-vst3-scanner` process for that HOME afterwards (check with `pgrep -f`
     on a string unique to that HOME; kill leftovers in cleanup either way so the
     script never leaves processes).
   - host: OBS with good + crash bundles: after the scan, the 3 good classes are
     logged as available (`[VST3 Scanner]   <name>` lines, as the existing check
     counts) — today it logs "Pipe closed before result" and none.
2. New fixtures (small C files next to bad-crash.c / bad-hang.c, built by run.sh
   the same way):
   - `bad-noisy.c`: constructor writes junk (including the literal text
     `VST3_SCAN_DONE` and `VST3_PROBE`) to stdout/stderr, then forks a grandchild
     that sleeps 30 s holding inherited fds, then the constructor raises SIGSEGV.
     Expect: skipped, the scan still finishes within the timeout + a few seconds,
     the 3 good classes listed (no output confusion). Make sure cleanup kills the
     sleeping grandchild (e.g. by a unique argv/name or process group) so the
     script leaves nothing behind.
   - `bad-exit0.c`: constructor calls `_exit(0)` (clean exit, no result). Expect
     skipped, good ones listed.
   - A moduleinfo-only bundle: `Contents/Resources/moduleinfo.json` that is VALID
     for Steinberg's moduleinfo parser, not discardable, listing zero audio-effect
     classes, whose `.so` is bad-crash. Expect: scanner exit 0, no "Skipped" line
     for it, 3 classes (proves no code was loaded). Look at the SDK's moduleinfo
     format/parser under
     `~/oss/jeiobs-deps/vst3-full/vst3sdk/public.sdk/source/vst/moduleinfo/`
     to write a minimal valid file (Factory Info flags, Classes list, etc.).
3. Keep all existing checks and the run.sh contract: throwaway HOME per run,
   `QT_QPA_PLATFORM=offscreen`, unreachable audio server, nothing written outside
   `/tmp` and DEPS_DIR, exit 0 only if every expectation holds. Keep it readable
   (it is evidence for a PR). Print PASS/FAIL lines.
4. Run it against the CURRENT (unfixed) build:
   `tools/jeiobs/vst3-check/run.sh ~/oss/jeiobs-build/vst3 ~/oss/jeiobs-deps`
   The new checks for crash/hang/noisy/exit0 SHOULD FAIL there (the moduleinfo one
   may pass or fail; report which) and the existing 7 should still PASS. Also make
   sure the script finishes in bounded time on the unfixed build and leaves no
   processes (`pgrep -af obs-vst3-scanner` and your grandchild marker after the
   run). Report the exact output.
5. Commit, then print: what you changed, the run output, and any doubts about the
   expectations (e.g. timing margins).
````

## Answer

Committed as `9e23d4ee9` on `wip/vst3-scanner-test`. Against the unfixed build, all 6 existing checks pass and all 9 new ones fail, as intended. The moduleinfo-only check also fails there. The run finished in 143 s and left no processes behind.

## What changed (only `tools/jeiobs/vst3-check/`)
- **`run.sh`**:
  - **Standalone scanner checks:** each run sets `OBS_VST3_PROBE_TIMEOUT_MS=2000`. Each new check requires: exit 0 within 10 s, 3 classes in `list.json`, exactly one `VST3_SCAN_DONE` on stdout, and each bad bundle named N times as `[VST3 Scanner] Skipped $HOME/.vst3/<name>.vst3: ` on stdout.
  - **New host checks:** crash bundle → OBS still lists the 3 good classes; hang bundle with a 60 s probe timeout → OBS quits mid-scan with exit 0 within 30 s, 0 leaks, no scanner or probe left; noisy bundle → OBS lists the 3 classes and exits within 25 s.
  - **Process safety:** every run uses `timeout -k 5` in the background, so an interrupted run can stop it. Leftovers are found by the sandbox HOME path in their command line and killed after each case and on exit. Core dumps are disabled (`core_pattern` here is `core`, relative to the working directory, i.e. BIN). Any inherited `OBS_VST3_PROBE_TIMEOUT_MS` is unset.
  - PASS/FAIL lines now show the measured values in parentheses.
- **`bad-noisy.c`**: writes fake `VST3_SCAN_DONE`, `NO_VST3`, `VST3_PROBE` and binary junk to both stdout and stderr. It then forks a grandchild named `vst3chk-noisy` that holds every inherited fd for 30 s, and raises SIGSEGV.
- **`bad-exit0.c`**: calls `_exit(0)` while loading.
- **`infoonly-moduleinfo.json`** plus **`moduleinfo-check.cpp`**: the `infoonly` bundle is this moduleinfo file on top of bad-crash's code. `run.sh` compiles the SDK's own parser and stops with exit 2 unless the file parses as `classes=1 audio=0 discardable=0`. I also checked the parser on the real sample bundles (ADelay 1 + AGain 2 audio classes), on a broken key (rejected) and on a flipped flag (detected).

## Run output (unfixed build `jeiobs-build/vst3`)
```
== scanner (standalone, OBS_VST3_PROBE_TIMEOUT_MS=2000)
PASS  no plugins: exit 1, NO_VST3  (exit 1)
PASS  good plugins: 3 classes listed  (exit 0, 3 classes)
PASS  invalid bundle skipped, good still listed  (exit 0, 3 classes)
FAIL  crash bundle: skipped once, 3 classes kept  (exit 139, 0 classes, 23 ms, VST3_SCAN_DONE x0, crash skipped x0)
FAIL  hang bundle: probe times out, skipped once, 3 classes kept, no process left  (exit 124, 0 classes, 20016 ms, VST3_SCAN_DONE x0, hang skipped x0, 0 left)
FAIL  crash + hang + invalid: each bad one skipped once, 3 classes kept, no process left  (exit 124, 0 classes, 30008 ms, VST3_SCAN_DONE x0, crash skipped x0, hang skipped x0, 0 left)
FAIL  noisy bundle (junk output, fd-holding grandchild, crash): skipped once, 3 classes kept  (exit 139, 0 classes, 28 ms, VST3_SCAN_DONE x1, noisy skipped x0)
FAIL  exit0 bundle (clean exit, no result): skipped once, 3 classes kept  (exit 0, 0 classes, 16 ms, VST3_SCAN_DONE x0, exit0 skipped x0)
FAIL  moduleinfo-only bundle (no audio class): code not loaded, not skipped, 3 classes kept  (exit 139, 0 classes, 16 ms, VST3_SCAN_DONE x0, infoonly skipped x0)
== host (real OBS, quit after load)
PASS  loads, lists 3 plugins, exits cleanly, 0 leaks  (exit 0, 3 listed, 5029 ms)
PASS  crash bundle: OBS survives and exits cleanly  (exit 0, 2947 ms)
FAIL  crash bundle: OBS still lists the 3 good classes  (0 listed, 0 log line(s) name crash.vst3)
FAIL  hang bundle: OBS quits mid-scan with exit 0 within 30 s, 0 leaks, no scanner left  (exit 124, 30077 ms, scan interrupted, 0 left)
FAIL  noisy bundle: OBS lists the 3 good classes, exits 0 within 25 s, 0 leaks  (exit 124, 0 listed, 25066 ms)
== audio through the filter (tone onset in the final mix)
INFO  onset dry=0.5763s ADelay=1.6500s
PASS  ADelay delays the tone by ~1 s (0.95-1.15), level kept, no NaN/Inf
work dir: /tmp/jeiobs-vst3check.lwSU
9 failed expectation(s)
```
After the run, `pgrep -af obs-vst3-scanner` matched only my own shell, `pgrep -a vst3chk-noisy` found nothing, and no core files were written. An earlier run gave the same results.

**Moduleinfo-only check: fails on the unfixed build (exit 139).** When `moduleinfo.json` lists zero audio classes, `tryReadModuleInfo` falls back to loading the code.

## Doubts and decisions for you
- **Not yet run against the fixed build.** I only tested the pass side of the helpers on synthetic output in the agreed format; all expected passes and fails came out right. Running it against the implementation build is the next step.
- **Strict format.** The checks require `Skipped` lines on stdout (the agreed `blog(LOG_WARNING)` does that) with the exact path followed by `: `, and exactly one `VST3_SCAN_DONE`. If the implementation differs, the details will show something like `crash skipped x0` with 3 classes listed.
- **Timing margins:**
  - Standalone: 10 s limit against a 2 s probe timeout; scans without bad bundles take 15–30 ms here.
  - Host: normal OBS runs take 3–5 s. The 25 s noisy limit stays below the grandchild's 30 s, which only starts once the scan does.
  - The leftover check waits about 3 s.
- **Host hang check also requires that "Available plugins" never appears.** That proves the scan was still running at quit, so the kill path is really what's being tested.
- **Host noisy check is my addition**, not in the brief. On the unfixed build it showed two problems: the fake `NO_VST3` made OBS log an empty plugin list, and the grandchild's held stderr blocked shutdown until the timeout.
- **Changed one existing check.** "OBS survives" no longer requires "Pipe closed before result", which the fix removes; it now requires 0 leaks.
- **What some checks don't cover:**
  - The moduleinfo-only check depends on the crash check to prove that a probe crash produces a `Skipped` line.
  - The invalid bundle has no `Skipped` requirement, because the design doesn't say a load failure counts as one.
  - The grandchild doesn't call `setsid`, so a design that kills the whole probe process group would kill it too.
  - I didn't add a garbage-result fixture: it would depend on the probe's argument order and could write outside `/tmp` if that order changed.
- **Doc mismatches (outside my edit scope):** `run.sh` had 6 checks, not the "7 PASS" the docs record. `001-vst3.md` (runtime evidence) and the task 003 acceptance text still describe the INFO lines and need updating.

My scratch outputs are in `/tmp/vst3chk-dev.rCSe/` (`run1.txt`, `run2.txt`).
