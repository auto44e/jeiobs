# Task 003: VST3 scanner robustness (proposed)

Status: **Proposed**, not started. It needs owner approval before any code
changes, because it changes inherited upstream host code from task 001.

## Problem

The runtime check in [task 001](001-vst3.md#runtime-evidence-2026-11-15-branch-jeiobsvst3-runtime-check)
(`tools/jeiobs/vst3-check/run.sh`) found these inherited behaviors on Linux:

1. **One crashing plugin hides all plugins.** `obs-vst3-scanner` loads every
   bundle in one process. If one bundle segfaults, the scanner exits 139 before
   writing the list. OBS survives ("Pipe closed before result"), but no VST3
   plugin can be selected.
2. **No scan timeout.** `getScanResult` in `plugins/obs-vst3/obs-vst3.cpp`
   blocks on `os_process_pipe_read` with no time limit. A bundle that hangs on
   load keeps the scanner alive indefinitely.
3. **A hung scan blocks OBS exit.** `freeVst3List()` joins `vst3ScanThread`
   from `obs_module_unload`. With a hung scanner, OBS does not exit; it exited
   about 1 s after I killed the scanner by hand.

Related findings (separate from this task):

4. `again-sample-accurate` silence is caused by the SDK sample (processor gain
   defaults to 0), not the host. The host does skip the controller sync
   (`getState` → `setComponentState`) after creating a plugin; fixing that is a
   small separate change and will not make AGain audible.
5. ADelay's 68–100 ms of extra delay is not attributed; see task 001.
6. `VST3Plugin::init` assigns `sampleRate = sampleRate;` (the parameter to
   itself). Harmless today because the member is not read.

## Possible approach (to agree first)

- Scan each bundle in its own scanner process, or have the scanner write
  partial results so one bad bundle cannot hide the others. Report the bad
  bundle by path in the log.
- Add a per-bundle or per-scan timeout and kill the scanner when it expires.
- On unload, terminate a still-running scanner before joining the thread.
- For items 4 and 5, first find the cause (host parameter initialization, block
  size, latency reporting) before changing anything.

## Acceptance

- `run.sh` turns the crash and hang `INFO` lines into checks: with a crashing or
  hanging bundle present, the 3 good classes are still listed, the scan finishes
  within the timeout, and OBS exits cleanly.
- The existing 7 checks still pass; `ENABLE_VST3=OFF` still builds.
- Keep the change separate from the upstream cherry-picks and offer it upstream
  if it holds up.
