# Design review — Codex (adversarial)

_Raw peer correspondence, task 003 (2026-11-15/16). Codex tool-call output trimmed; prompt and final answer kept verbatim. AI-assisted, not human review._

## Prompt

````text
# JeiOBS task 003: VST3 scanner robustness — design brief

Repo: ~/oss/jeiobs (fork of OBS Studio 33.0.1; upstream VST3 host backported in
plugins/obs-vst3, by upstream authors). C++17, libobs util APIs. Platforms the
code must COMPILE on: Linux, Windows, macOS. Only Linux can be run/tested here.
Fork owner has approved deviating from upstream code for this.

## Current architecture (read the files; key points)
- Host plugin `obs-vst3.so` (plugins/obs-vst3/obs-vst3.cpp:169-353, plugin-main.cpp).
  On module load `retrieveVst3List(nullptr)` spawns the helper `obs-vst3-scanner
  <config>/vst3list.json` via libobs `os_process_pipe_create2(args, "r")`, then a
  std::thread `getScanResult` loops on blocking `os_process_pipe_read` (stdout)
  until it sees `VST3_SCAN_DONE\n` or `NO_VST3\n`, or the pipe closes. Then it
  loads the JSON list, reads all stderr, `os_process_pipe_destroy` (waitpid /
  WaitForSingleObject INFINITE), sets `vst3ScanDone`.
- `freeVst3List()` joins `vst3ScanThread`; called from `obs_module_unload` and
  from the "Rescan" button callback (only when scan is done).
- libobs pipe API (libobs/util/pipe.h) has NO kill, NO timeout, NO pid accessor.
- Scanner (plugins/obs-vst3/vst3-scan/vst3-scanner.cpp + VST3Scanner.cpp) runs in
  ONE process: for each bundle under /usr/lib/vst3, /usr/local/lib/vst3, ~/.vst3
  (platform equivalents) it either parses Contents/Resources/moduleinfo.json
  (no code loaded) or, if no moduleinfo or factory flag kClassesDiscardable,
  calls `VST3::Hosting::Module::create` (dlopen/LoadLibrary + GetPluginFactory)
  and enumerates classes (`addModuleClasses`). Cache path: `vst3CacheLoad` reuses
  the JSON but re-loads discardable modules and new bundles in-process too.
  Writes the JSON with obs_data_save_json_safe, prints VST3_SCAN_DONE.

## Observed failures (Linux, reproducible with tools/jeiobs/vst3-check/run.sh)
1. A bundle whose .so segfaults on load (constructor raise(SIGSEGV)) kills the
   scanner (exit 139) before the JSON is written → host logs "Pipe closed before
   result" → ZERO VST3 plugins available, though 2 good bundles exist.
2. A bundle whose .so hangs on load (constructor `for(;;) pause();`) → scanner
   never exits; no timeout anywhere.
3. With (2), OBS shutdown blocks forever in freeVst3List() join (it exited ~1 s
   after the scanner was killed manually).

## Proposed design (critique it; propose better if you see one)
A. Scanner becomes a coordinator that never loads plugin code itself. For each
   bundle that needs a code load, it spawns ITSELF in a child mode
   `obs-vst3-scanner --probe <bundle>` (via os_process_pipe), which loads that one
   module and prints its classes (e.g. one JSON object on stdout, then a marker).
   moduleinfo.json-only bundles stay parsed in the coordinator (no code load).
B. Per-probe timeout enforced INSIDE the child: a watchdog thread started before
   loading calls a hard exit after N s (POSIX `_exit`; Windows
   `TerminateProcess(GetCurrentProcess(),…)`, NOT ExitProcess because of the loader
   lock). The coordinator's blocking read then returns when the child dies.
   A crash/timeout/garbage output marks only that bundle as failed (logged with
   path and reason), the others are kept.
C. Overall scan deadline in the coordinator, and a bounded host shutdown: since
   libobs can't kill the scanner, options are (i) add `os_process_pipe_kill()`
   (or pid accessor) to libobs (POSIX kill(SIGKILL), Win TerminateProcess) and
   kill on unload; (ii) a cancel-file the host creates on unload that the
   coordinator polls between probes; (iii) accept the bounded wait of one probe
   timeout. Pick one and justify.
D. Keep the JSON cache format compatible (version 1) unless there's a reason;
   optionally remember failed bundles so a crashing bundle isn't re-probed every
   launch (with invalidation by mtime) — say whether that's worth it now.

## Constraints
- Small, reviewable diff; follow the existing code style (tabs, clang-format
  OBS style, 120 cols). No new third-party deps. Must still build with
  ENABLE_VST3=OFF (untouched) and compile on Windows/macOS (no Linux-only APIs
  without #ifdef).
- Don't change DSP/audio paths. Don't load plugin code in the OBS process during
  scanning.
- Evidence will be the Linux runtime check: with crash.vst3 + hang.vst3 + 2 good
  bundles present, the 3 good classes are listed, the scan finishes within the
  timeout, and OBS exits cleanly within a few seconds.

## Also open (separate, investigate only)
- `again-sample-accurate` (Steinberg SDK 3.8.0 sample) outputs silence through
  the host filter; ADelay works. Suspect the processor's gain parameter starts at 0
  unless the host applies controller defaults/state to the processor. See
  plugins/obs-vst3/VST3Plugin.cpp and the SDK sample at
  ~/oss/jeiobs-deps/vst3-full/vst3sdk/public.sdk/samples/vst/again_sampleaccurate.
- ADelay measured 1.078 s delay vs 1.000 s configured (host processes fixed 10 ms
  blocks, obs-vst3.h:34).
````

## Answer

## Verdict

A/B are the right direction, but the design needs stronger framing, ownership, and shutdown semantics. C should use option (i): add `os_process_pipe_kill()`, with explicit synchronization in the host. A cancel file is ineffective while either side is blocked in `fread`/`ReadFile`; accepting one probe timeout leaves other coordinator hangs unbounded.

## Main hazards

- `list` is replaced and deleted while `vst3Properties()` or `vst3Update()` may still read it. `vst3ScanDone` does not protect the pointer lifetime. Publish an immutable `std::shared_ptr<const VST3Scanner>` snapshot only after a complete result is assembled; readers atomically load the snapshot. Never mutate the published list.

- Rescan, unload, and the scan worker need one lifecycle mutex. Protect `vst3ScanPipe` while killing/destroying it, and serialize “start scan” against rescan/unload. `freeVst3List()` must kill the pipe before joining if a scan is active.

- `VST3Scanner::getVST3Paths()` returns an unordered set. With an overall deadline, a hang encountered first can prevent good bundles from being reached. Sort paths, parse all safe `moduleinfo.json` bundles first, then probe code-loading bundles deterministically.

- `tryReadModuleInfo()` currently conflates “valid moduleinfo with zero audio classes” and “could not parse moduleinfo”; both lead to `addModuleClasses()`. Refactor it to a tri-state result. A valid, non-discardable moduleinfo file must never cause a code load.

- The coordinator must not call `addModuleClasses()` anywhere. Cache loading currently does so for discardable and new bundles; those paths need to become probe jobs.

- The existing host protocol can deadlock: it reads scanner stdout until the marker and only then drains stderr. A scanner or probe can fill stderr first. Drain stdout and stderr concurrently, both in the host and coordinator, with bounded diagnostic storage. The drain must continue after the cap rather than stop reading.

- Do not use substring markers in unbounded strings. A plugin can write arbitrary stdout, and the current code has no output limit.

## Recommended protocol

Use one probe per child, sequentially:

1. Coordinator starts `obs-vst3-scanner --probe <bundle> <result-file> <nonce>`.
2. Child starts its watchdog before `Module::create()`. The watchdog covers module destruction too.
3. Child writes a version-1 JSON result to a unique temporary result file, atomically renames it, then prints:

   `VST3_PROBE_DONE/1 <nonce>\n`

4. Parent accepts success only if the exact nonce marker was observed, the result file parses, and the child exits with status zero.
5. EOF, nonzero exit, timeout, malformed JSON, missing result file, or missing marker excludes only that bundle.

A result file avoids corruption from plugin code writing to stdout. If stdout-only output is retained, use a bounded length-prefixed frame plus nonce, but it remains vulnerable to arbitrary plugin stdout noise.

The coordinator should always emit a terminal `VST3_SCAN_DONE` after a best-effort scan, even if every code probe fails. Reserve `NO_VST3` for no discovered bundles. Save the aggregate of successful moduleinfo and probe results; do not abort the whole scan on one failure.

Suggested limits: 3 seconds per probe and 30 seconds overall, using a monotonic clock. The watchdog should hard-terminate: `_exit()` on POSIX and `TerminateProcess(GetCurrentProcess(), ...)` on Windows. Do not use `ExitProcess()` from the watchdog.

## Shutdown and orphan handling

Add:

```c
EXPORT bool os_process_pipe_kill(os_process_pipe_t *pp);
```

Implement it using `kill(pid, SIGKILL)` on POSIX and `TerminateProcess()` on Windows. It must only signal; the scan thread remains responsible for closing and reaping through `os_process_pipe_destroy()`.

To avoid orphaned probe children if the coordinator crashes, add a parent-death check in probe mode:

- POSIX/macOS: watchdog detects that `getppid()` changed.
- Windows: coordinator passes its PID; the probe opens a process synchronization handle and exits when that handle is signaled.

A future improvement could use process groups/jobs, but the parent-death check is smaller and sufficient for direct probe children.

On macOS, verify the scanner target itself links Cocoa/Foundation and compiles the SDK’s Objective-C++ loader source. On Windows, preserve package paths exactly: the SDK accepts either the `.vst3` package or its package executable depending on the discovery path.

Do not add failed-bundle caching yet. It requires per-bundle fingerprints and schema work; retrying failures each launch is safer for version 1.

## Tests

- Crash, hang, malformed-output, nonzero-exit, and large-stderr probes.
- Two good bundles plus crash/hang bundles: all good classes listed; scan completes; cache is valid.
- Probe timeout during constructor and during module destruction.
- Host unload during initial scan and during an active hang; verify exit under five seconds and no child remains.
- Rescan/property creation/update concurrently with scan completion.
- Valid moduleinfo with zero audio classes must not load code.
- Discardable modules must always probe.
- UTF-8/space-containing paths, Windows package paths, and macOS bundles.
- Corrupt/partial cache and cache reload.
- `ENABLE_VST3=OFF`.

The referenced `tools/jeiobs/vst3-check` directory is not present in this worktree, so the stated runtime evidence remains pending.

Separately, the Again silence hypothesis is confirmed: `SampleAccurate::Parameter` defaults to zero, and the processor only assigns IDs. `PlugProvider::setupPlugin()` does not transfer controller defaults to the processor. Initialize the processor gain explicitly to `1.0`, or enqueue initial controller values from the host.

The ADelay result is also consistent with host buffering: the host processes fixed 10 ms chunks and subtracts a fixed 10 ms timestamp offset, while queueing can add another block/callback interval. The host also has the unrelated shadowing bug `sampleRate = sampleRate;` in `VST3Plugin::init()`.
