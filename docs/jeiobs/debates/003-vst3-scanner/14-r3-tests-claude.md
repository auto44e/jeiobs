# Round 3 test worker — Claude (summary)

_Raw peer correspondence, task 003 (2026-11-15/16). Codex tool-call output trimmed; prompt and final answer kept verbatim. AI-assisted, not human review._

## Prompt

````text
# Worker brief, round 3: JeiOBS task 003 — VST3 scanner robustness (TESTS)

Same rules: work ONLY in `~/oss/jeiobs-wt/scanner-test`
(branch `wip/vst3-scanner-test-r3`, already at the current integration head), edit
only `tools/jeiobs/vst3-check/`, no network, no push. Commit with
`git -c user.name=auto44e -c user.email=auto44e@gmail.com commit`.

You wrote earlier rounds of this suite (you don't remember them; read
`git log --oneline -10 -- tools/jeiobs/vst3-check` and the files). The lead has since
tightened it (commit e5d433625: invalid-bundle check requires its skip line; hang
bounds 8 s; quit-probe waits for a running scanner child when
`QUITPROBE_WAIT_SCANNER=1`). It passes 15/15 on the fixed build
(`~/oss/jeiobs-wt/scanner-impl/_build`).

A review found that the coordinator's own kill of a stuck probe never worked on
POSIX: once the probe redirects its stdio, nothing bounds the wait. Nothing in the
suite caught it, because a hanging fixture is always ended by the probe's own
watchdog. Another worker is fixing it right now (bounded process wait + kill; the
skip reason when the coordinator kills is `probe timed out (killed)`; kill margin
is `max(5000 ms, timeout/4)`, so with a 2000 ms timeout the coordinator kills at
about 7 s after spawn).

## Your task
Add ONE standalone scanner check that would have caught it:
- Sandbox with the 3 good classes + `hang.vst3`, `OBS_VST3_PROBE_TIMEOUT_MS=2000`.
- Start the scanner in the background, wait until its `--probe` child for the
  hang bundle exists (identify it precisely: a process whose command line contains
  `--probe` AND this sandbox's HOME path; poll every 50 ms, give up after 3 s and
  FAIL), then `kill -STOP` it (this simulates a probe whose watchdog cannot run).
- Expect: scanner exit 0 within 12 s, the 3 exact classes, exactly one Skipped line
  for hang with the reason `probe timed out (killed)` (exact variant), VST3_SCAN_DONE
  once, and no process left (SIGCONT + SIGKILL anything left in cleanup so the script
  never leaves a stopped process behind, on every path).
- Label it clearly, e.g. "frozen probe: coordinator kills it, scan done in < 12 s".
Keep all other checks and the script's contract unchanged.

The fixed build will not pass this until the other worker lands its change (it may
land while you work; rebuilds happen in that `_build`). Run the suite against the
CURRENT state of `scanner-impl/_build` and report: if the implementation change has
not landed yet, this new check should FAIL (show it) and the other 15 should pass;
if it has landed, all 16 should pass. Also confirm on the unfixed build
(`~/oss/jeiobs-build/vst3`) that it fails and leaves no
stopped process. Commit and print the outputs.
````

## Answer

Committed as `a7a527d8a` on `wip/vst3-scanner-test-r3`, author auto44e, not pushed. The other worker's fix (8edfb74f6) was already in `scanner-impl/_build` when I ran the suite, so all 16 checks pass there. On a build of the code just before that fix, only the new check fails.

**What changed** (only in `tools/jeiobs/vst3-check/`):
- **New check:** "frozen probe (SIGSTOP inside hang.so): coordinator kills it, scan done in < 12 s, skipped once as killed, 3 classes kept, no process left".
- **Finding the probe:** a child of the scanner whose arguments are exactly `--probe` and this sandbox's `hang.vst3`. It polls every 50 ms and fails after 3 s.
- **One change from your brief:** I stop the probe only once it has loaded `hang.so`, not as soon as it exists. If it is stopped before it redirects its stdio, the old kill timer still ends it, and the pre-fix build could pass by luck. After the stop the check requires the probe's state to be T (stopped).
- **Exact reason:** `results.py` has a new `killed` mode that accepts only `probe timed out (killed)`.
- **Cleanup:** `reap()` now sends SIGCONT, then SIGKILL, both after each check and at exit. `scan()` is split into `scan_start` and `scan_wait`. The other checks behave as before.

**Fixed build (`scanner-impl/_build`, 8edfb74f6): 16 PASS / 0 FAIL, exit 0, 49 s, nothing left**
```
PASS  no plugins: exit 1, NO_VST3  (exit 1)
PASS  good plugins: the 3 classes listed  (exit 0, the 3 expected classes)
PASS  invalid bundle skipped once, good still listed  (exit 0, 33 ms, …, invalid skipped x1 (probe failed))
PASS  crash bundle: skipped once, 3 classes kept  (exit 0, 32 ms, …, crash skipped x1 (probe failed))
PASS  hang bundle: … < 8 s …  (exit 0, 2139 ms, …, hang skipped x1 (probe timed out), 0 left)
PASS  crash + hang + invalid: …  (exit 0, 2184 ms, …, 0 left)
PASS  noisy bundle …  (grandchild 2234112 alive, exit 0, 31 ms, …, grandchild gone after SIGKILL, 0 left)
PASS  exit0 bundle …  (exit 0, 31 ms, …, exit0 skipped x1 (probe produced no result))
PASS  moduleinfo-only bundles …  (exit 0, 12 ms, the 4 expected classes, …)
PASS  frozen probe (SIGSTOP inside hang.so): coordinator kills it, scan done in < 12 s, skipped once as killed, 3 classes kept, no process left  (hang probe stopped in hang.so at 57 ms, exit 0, 7028 ms, the 3 expected classes, VST3_SCAN_DONE x1, hang skipped x1 (probe timed out (killed)), 0 left)
PASS  loads, lists the 3 plugins, exits cleanly, 0 leaks  (exit 0, …, 5446 ms)
PASS  crash bundle: OBS survives and exits cleanly  (exit 0, 0 leak count, 3058 ms)
PASS  crash bundle: OBS still lists the 3 good classes
PASS  hang bundle: OBS quits mid-scan …  (exit 0, 1385 ms, …, 0 left)
PASS  noisy bundle: OBS lists the 3 good classes …  (exit 0, 2893 ms, …, 0 left)
PASS  ADelay delays the tone by ~1 s …  (onset dry 0.5779 s, ADelay 1.6450 s, …)
0 failed expectation(s)
```

**Pre-fix build (e5d433625, built into `/tmp/jeiobs-prefix-build`): the other 15 pass, the new check fails, nothing left**
```
FAIL  frozen probe (SIGSTOP inside hang.so): … (hang probe stopped in hang.so at 78 ms, exit 124, 20010 ms, unreadable (FileNotFoundError: … home-frozen/list.json), VST3_SCAN_DONE x0, hang skipped x0, 0 left)
1 failed expectation(s)
```

**`jeiobs-build/vst3`: the new check fails, nothing left, but it is a weak test.** That build is older than the probe design and starts no `--probe` processes, so it can't show this bug:
```
FAIL  frozen probe … (no hang probe ran hang.so within 3 s, exit 124, 20010 ms, …, 0 left)
11 failed expectation(s)
```
That is why I built the pre-fix code myself.

**Interrupting the script while the probe was stopped (pre-fix build):** SIGINT exited 130 and SIGTERM exited 143, each within 0.1 s, with nothing left. After every run, `ps` showed no process from the run, stopped or otherwise.

**Still open:**
- I ran each build once only.
- I didn't edit HANDOFF or the task docs, since the brief limited me to `tools/jeiobs/vst3-check/`. Any doc that says the suite has 15 checks now needs 16.
- My pre-fix build is still in `/tmp` (`jeiobs-prefix-build`, about 1 GB, and `jeiobs-prefix-src`). You can delete it or keep it for future negative tests.
