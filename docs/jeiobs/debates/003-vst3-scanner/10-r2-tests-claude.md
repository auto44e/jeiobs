# Round 2 test worker — Claude (summary)

_Raw peer correspondence, task 003 (2026-11-15/16). Codex tool-call output trimmed; prompt and final answer kept verbatim. AI-assisted, not human review._

## Prompt

````text
# Worker brief, round 2: JeiOBS task 003 — VST3 scanner robustness (TESTS)

Same rules as before: work ONLY in the worktree
`~/oss/jeiobs-wt/scanner-test` (branch
`wip/vst3-scanner-test`), edit only `tools/jeiobs/vst3-check/`, no network, no
push. Commit with `git -c user.name=auto44e -c user.email=auto44e@gmail.com commit`.

You wrote commit 9e23d4ee9 in an earlier session (you do not remember it; read
`git show 9e23d4ee9`). Integrated with the implementation, the suite printed
15 PASS / 0 FAIL in 45 s on the FIXED build, and 9 of the new checks failed on the
UNFIXED build, as intended. The fixed build is at
`~/oss/jeiobs-wt/scanner-impl/_build` (another worker is
changing its code right now, so a run there may occasionally see a rebuild; if a
run looks inconsistent, re-run). The unfixed build is
`~/oss/jeiobs-build/vst3`. DEPS:
`~/oss/jeiobs-deps`.

Facts about the fixed implementation you may rely on:
- Skip lines on the scanner's stdout: `[VST3 Scanner] Skipped <bundle path>: <reason>`
  where reason is one of `probe failed`, `probe timed out`,
  `probe timed out (killed)` (coordinator had to kill it), `probe produced no result`,
  `invalid probe result`, `probe nonce mismatch`, ... The OBS log repeats these lines
  (as `warning: [VST3 Scanner] Skipped ...`).
- Default probe timeout will become 30000 ms; `OBS_VST3_PROBE_TIMEOUT_MS` overrides.
- The probe child redirects its stdout/stderr to /dev/null before loading plugin
  code, so plugin output and grandchildren cannot reach the coordinator's pipe.

An adversarial review (Codex) of your tests found the following. I (the lead) have
decided what to change:

1. **Pin exact classes, not counts.** Assert the exact set of class IDs
   (ADelay `0CDBB66985D548A9BFD8371909D24BB3`, AGain Sample Accurate
   `C18D3C1E719E4E29924D3ECAA5E4DA18`, plus the third class in the AGain bundle —
   read it from a clean scan of the good bundles; do not guess) in `list.json`, and
   exact names in the OBS log, wherever the current checks count to 3.
2. **System plugin folders.** The scanner also searches `/usr/lib/vst3` and
   `/usr/local/lib/vst3`. Preflight: if either contains any `*.vst3`, print why and
   exit 2 (the check needs a machine without system VST3s, e.g. the clean-room
   container). Fail on any `cp` error when populating a sandbox.
3. **Prove the moduleinfo path was used.** Keep the zero-audio-class `infoonly`
   bundle, and add a second moduleinfo-only bundle (`infoone`) whose
   `moduleinfo.json` lists ONE audio-effect class with a unique made-up CID, also on
   top of bad-crash code. Expect 4 exact classes (the 3 good + that CID), no skip
   line for either, exit 0. Validate the new JSON with your moduleinfo-check helper
   (`classes=1 audio=1 discardable=0`).
4. **Tighter, reason-checked timeouts.** For the 2000 ms standalone hang checks,
   require the reason `probe timed out` (either variant) and elapsed < 6000 ms. For
   the host hang check, also require that OBS exited within 15 s of its start and
   that scanning had started (e.g. a scanner process existed during the run, or the
   log shows the scan began — find something reliable).
5. **Noisy fixture must prove its grandchild existed.** Have the grandchild write
   its PID to a marker file (e.g. `$HOME/noisy-grandchild.pid`; the probe inherits
   the sandbox HOME) before sleeping; the check requires the marker, then requires
   the scan finished in < 6000 ms even though that grandchild is still alive
   (check it is alive at that moment), then kills it and verifies it is gone.
6. **Fail closed on tooling.** Preflight `pgrep`, `pkill`, `timeout`, `python3`,
   `gcc`, `g++`; exit 2 if any is missing. Never treat a pgrep error as "0 processes".
7. Keep everything else: throwaway HOME, offscreen Qt, unreachable audio server,
   nothing outside /tmp and DEPS_DIR, no leftover processes on any path, PASS/FAIL
   lines with measured values, exit 0 only if every expectation holds.

Then run against BOTH builds and report the exact output of each: the fixed build
should be all PASS (if a timeout-reason check fails only because the other worker
has not landed `probe timed out (killed)` yet, say so); the unfixed build should
fail every new check and pass the original 6. Commit and print a summary plus any
doubts.
````

## Answer

Commit `8ff5a321e` is on `wip/vst3-scanner-test` (not pushed). With this version the fixed build passes all 15 checks, and the unfixed build passes the 6 original checks and fails all 9 new ones. No scanner, probe or fixture process was left after any run.

**Fixed build** (`scanner-impl/_build`, commit be2bdff9e), exit 0. This version ran twice there and both runs were 15/15. Earlier runs of the script took about 43 s. Output of the last run:
```
== scanner (standalone, OBS_VST3_PROBE_TIMEOUT_MS=2000)
PASS  no plugins: exit 1, NO_VST3  (exit 1)
PASS  good plugins: the 3 classes listed  (exit 0, the 3 expected classes)
PASS  invalid bundle skipped, good still listed  (exit 0, the 3 expected classes)
PASS  crash bundle: skipped once, 3 classes kept  (exit 0, 22 ms, the 3 expected classes, VST3_SCAN_DONE x1, crash skipped x1 (probe failed))
PASS  hang bundle: probe times out, scan done in < 6 s, skipped once, 3 classes kept, no process left  (exit 0, 2128 ms, the 3 expected classes, VST3_SCAN_DONE x1, hang skipped x1 (probe timed out), 0 left)
PASS  crash + hang + invalid: scan done in < 6 s, each bad one skipped once (hang as timeout), 3 classes kept, no process left  (exit 0, 2150 ms, the 3 expected classes, VST3_SCAN_DONE x1, crash skipped x1 (probe failed), hang skipped x1 (probe timed out), invalid skipped x1 (probe failed), 0 left)
PASS  noisy bundle (junk output, live fd-holding grandchild, crash): scan done in < 6 s, skipped once, 3 classes kept  (grandchild 2028484 alive, exit 0, 26 ms, the 3 expected classes, VST3_SCAN_DONE x1, noisy skipped x1 (probe failed), grandchild gone after SIGKILL, 0 left)
PASS  exit0 bundle (clean exit, no result): skipped once, 3 classes kept  (exit 0, 22 ms, the 3 expected classes, VST3_SCAN_DONE x1, exit0 skipped x1 (probe produced no result))
PASS  moduleinfo-only bundles (no audio class; one): code not loaded, neither skipped, 3 classes + infoone's listed  (exit 0, 13 ms, the 4 expected classes, VST3_SCAN_DONE x1, infoonly skipped x0, infoone skipped x0)
== host (real OBS, quit after load)
PASS  loads, lists the 3 plugins, exits cleanly, 0 leaks  (exit 0, the 3 expected names listed, 0 leak count, 4850 ms)
PASS  crash bundle: OBS survives and exits cleanly  (exit 0, 0 leak count, 2980 ms)
PASS  crash bundle: OBS still lists the 3 good classes  (the 3 expected names listed, crash skipped x1 (probe failed))
PASS  hang bundle: OBS quits mid-scan with exit 0 within 15 s, 0 leaks, no scanner left  (exit 0, 2850 ms, scanner seen (max 2 process(es)), 1 scanner(s) running at the quit request, scan interrupted, 0 leak count, 0 left)
PASS  noisy bundle: OBS lists the 3 good classes, exits 0 within 25 s while the grandchild lives, 0 leaks  (grandchild 2029183 alive, exit 0, 2876 ms, the 3 expected names listed, 0 leak count, grandchild gone after SIGKILL, 0 left)
== audio through the filter (tone onset in the final mix)
PASS  ADelay delays the tone by ~1 s (0.95-1.15), level kept, no NaN/Inf  (onset dry 0.5647 s, ADelay 1.6464 s, 1 line(s) with peak ~0.5 and no NaN/Inf)
work dir: /tmp/jeiobs-vst3check.d8CG
0 failed expectation(s)
```

**Unfixed build** (`jeiobs-build/vst3`), exit 1, 2m10s:
```
== scanner (standalone, OBS_VST3_PROBE_TIMEOUT_MS=2000)
PASS  no plugins: exit 1, NO_VST3  (exit 1)
PASS  good plugins: the 3 classes listed  (exit 0, the 3 expected classes)
PASS  invalid bundle skipped, good still listed  (exit 0, the 3 expected classes)
FAIL  crash bundle: skipped once, 3 classes kept  (exit 139, 12 ms, unreadable (FileNotFoundError: [Errno 2] No such file or directory: '/tmp/jeiobs-vst3check.dVja/home-crash/list.json'), VST3_SCAN_DONE x0, crash skipped x0)
FAIL  hang bundle: probe times out, scan done in < 6 s, skipped once, 3 classes kept, no process left  (exit 124, 20006 ms, unreadable (FileNotFoundError: [Errno 2] No such file or directory: '/tmp/jeiobs-vst3check.dVja/home-hang/list.json'), VST3_SCAN_DONE x0, hang skipped x0, 0 left)
FAIL  crash + hang + invalid: scan done in < 6 s, each bad one skipped once (hang as timeout), 3 classes kept, no process left  (exit 124, 20006 ms, unreadable (FileNotFoundError: [Errno 2] No such file or directory: '/tmp/jeiobs-vst3check.dVja/home-mixed/list.json'), VST3_SCAN_DONE x0, crash skipped x0, hang skipped x0, invalid skipped x0, 0 left)
FAIL  noisy bundle (junk output, live fd-holding grandchild, crash): scan done in < 6 s, skipped once, 3 classes kept  (grandchild 2031781 alive, exit 139, 13 ms, unreadable (FileNotFoundError: [Errno 2] No such file or directory: '/tmp/jeiobs-vst3check.dVja/home-noisy/list.json'), VST3_SCAN_DONE x1, noisy skipped x0, grandchild gone after SIGKILL, 0 left)
FAIL  exit0 bundle (clean exit, no result): skipped once, 3 classes kept  (exit 0, 15 ms, unreadable (FileNotFoundError: [Errno 2] No such file or directory: '/tmp/jeiobs-vst3check.dVja/home-exit0/list.json'), VST3_SCAN_DONE x0, exit0 skipped x0)
FAIL  moduleinfo-only bundles (no audio class; one): code not loaded, neither skipped, 3 classes + infoone's listed  (exit 139, 16 ms, unreadable (FileNotFoundError: [Errno 2] No such file or directory: '/tmp/jeiobs-vst3check.dVja/home-info/list.json'), VST3_SCAN_DONE x0, infoonly skipped x0, infoone skipped x0)
== host (real OBS, quit after load)
PASS  loads, lists the 3 plugins, exits cleanly, 0 leaks  (exit 0, the 3 expected names listed, 0 leak count, 4816 ms)
PASS  crash bundle: OBS survives and exits cleanly  (exit 0, 0 leak count, 2902 ms)
FAIL  crash bundle: OBS still lists the 3 good classes  (0 names listed, missing [ADelay, AGain Sample Accurate, Advanced Tutorial], unexpected [none], crash skipped x0)
FAIL  hang bundle: OBS quits mid-scan with exit 0 within 15 s, 0 leaks, no scanner left  (exit 124, 30178 ms, scanner seen (max 1 process(es)), 1 scanner(s) running at the quit request, scan interrupted, 0 leak count, 0 left)
FAIL  noisy bundle: OBS lists the 3 good classes, exits 0 within 25 s while the grandchild lives, 0 leaks  (grandchild 2033557 not running (state gone, name none), exit 124, 25155 ms, 0 names listed, missing [ADelay, AGain Sample Accurate, Advanced Tutorial], unexpected [none], 0 leak count, no grandchild to kill, 0 left)
== audio through the filter (tone onset in the final mix)
PASS  ADelay delays the tone by ~1 s (0.95-1.15), level kept, no NaN/Inf  (onset dry 0.5789 s, ADelay 1.6597 s, 1 line(s) with peak ~0.5 and no NaN/Inf)
work dir: /tmp/jeiobs-vst3check.dVja
9 failed expectation(s)
```

**Summary**
- **Third class:** `CC48BF25529043DA80223510FFE8BD02`, "Advanced Tutorial", read from a clean scan; both builds agree. Every check that counted to 3 now requires the exact class IDs in `list.json`, or the exact names in the OBS log. A new `results.py` does the parsing.
- **Preflight:** exits 2 if any of the six tools is missing or a system folder holds a `*.vst3`. It also starts a fake process that looks like one of the run's scanners and exits 2 unless pgrep counts it and pkill stops it. A pgrep/pkill error or a `cp` error also exits 2.
- **Fail-closed paths tried:** missing tool, pgrep that errors, pgrep that never matches, a `*.vst3` in a stand-in system folder, and a `cp` error. Each exits 2 with a message.
- **Timeout reason:** the fixed build always reported plain `probe timed out`. The check accepts both variants, so nothing waits on the other worker landing `(killed)`.

**Decisions and doubts**
1. **`infoone` shares a sandbox with `infoonly`.** On its own it would pass on the unfixed build, because that build already trusts a moduleinfo.json that lists an audio class. Together, `infoonly` makes the check fail there.
2. **Host hang "still scanning" is now counted inside OBS.** OBS exits about 0.3 s after the quit request, and sampling from outside missed that window in 1 of 3 runs, which made the check flaky. So `quit-probe.cpp` now counts OBS's running scanner children at the moment it asks OBS to quit. The outside sampling stays as the "scanner seen during the run" signal.
3. **One expectation beyond your list:** the crash + hang + invalid check now also requires a skip line for `invalid`, which its label already claimed. The fixed build reports `probe failed`.
4. **Log noise in the fixed build:** on a machine without the system folders, it now logs `warning: [VST3 Scanner] Directory iteration stopped at /usr/lib/vst3: No such file or directory`, and the same for `/usr/local/lib/vst3`. That is a warning for an optional folder that simply isn't there; worth passing to the implementation worker.
