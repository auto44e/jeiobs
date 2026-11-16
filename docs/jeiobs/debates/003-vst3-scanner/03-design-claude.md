# Design review — Claude (implementation-minded)

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

You are the IMPLEMENTATION-MINDED DESIGN REVIEWER for sections A-D. Read the actual code. Propose the concrete design you would implement: child protocol, where the watchdog lives, how the coordinator builds the final list (including the cache path vst3CacheLoad/updateModulesList and discardable modules), which option for C and exactly what code changes (files/functions), and the risks on Windows/macOS that can't be tested here. Max ~900 words. Do not edit files.````

## Answer

A and B hold up once the fixes below are in. For C, choose (i), a new `os_process_pipe_kill`. I would drop the coordinator-wide deadline. D is worth doing, but as a follow-up PR that only adds a key to the JSON.

## A. Probe protocol
- **Command:** `obs-vst3-scanner --probe <bundle> <timeoutMs>`, checked at the top of `main` before the `os_mkdirs(argv[1])` code.
  - Find its own path with `os_get_executable_path_ptr(VST3_SCANNER)`, the same way `start_pipe` does.
  - On macOS the child keeps the scanner's library-validation entitlement, so no new signed helper is needed.
- **stdout carries exactly one message:**
  - `VST3_PROBE_BEGIN\n<vst3list.json for one bundle>\nVST3_PROBE_END\n` — success, even with 0 classes.
  - `VST3_PROBE_FAIL <sdk error>\n` — the module failed to load.
  - `VST3_PROBE_TIMEOUT\n` — written by the watchdog.
- **Shared serializer:** move the per-class read/write out of `vst3CacheSave`/`vst3CacheLoad` (vst3-scanner.cpp:53-61, 100-110) into two helpers used by both child and coordinator.
- **Judge the outcome by these markers, not the exit status.** On POSIX, `os_process_pipe_destroy` returns the pid when the child was killed by a signal (pipe-posix.c:156-161). Windows returns NTSTATUS codes.
- **Coordinator parsing:** keep at most ~1 MiB of stdout but keep reading to the end. Use the last complete message, overwrite `path`/`pluginName` with the bundle it asked for, and drop entries with an empty id.
- **POSIX child, before loading the plugin:**
  - Copy stdout to a private close-on-exec descriptor (`fcntl(1, F_DUPFD_CLOEXEC, 3)`), then point fds 1 and 2 at `/dev/null`.
  - Plugin output then can't corrupt the result.
  - More importantly, a helper program the plugin starts can't keep the pipe open. The coordinator only knows a child died when its pipe closes, so a leaked pipe would defeat every timeout.

## B. Watchdog in the child
- **Thread:** start a detached `std::thread` before `Module::create`, checking every ~200 ms.
  - When the deadline passes, it writes the TIMEOUT marker and hard-exits.
  - On POSIX it also hard-exits when `getppid()` changes, so the probe dies soon after the host kills the coordinator. Unlike `PR_SET_PDEATHSIG`, this works on macOS too.
- **Hard exit:** `_exit` on POSIX, `TerminateProcess(GetCurrentProcess(), …)` on Windows.
- **Success path:** hard-exit after writing the result as well. That skips the SDK's `ModuleExit` + `dlclose` and static destructors, so a plugin that crashes on unload isn't reported as failed.
- **Timeout value:** default 20 s, since antivirus scans and shell plugins can legitimately take seconds. For run.sh, add an `OBS_VST3_PROBE_TIMEOUT_MS` override that only the coordinator reads.

## How the coordinator builds the list
- **One hook for all loads:** give `VST3Scanner` a `probe` function pointer.
  - `addModuleClasses` appends the probe's results, and refuses to load anything if no hook is set. The coordinator then cannot load plugin code by construction.
  - The current in-process code becomes `loadModuleClasses()`, used only in probe mode.
  - All four load sites then run out of process without other changes: VST3Scanner.cpp:187, :209, :258 and vst3-scanner.cpp:141 (cached discardable modules).
- **Bug — double probe:** `tryReadModuleInfo` should return true whenever it parsed moduleinfo.json. Today, when a discardable bundle fails, the caller loads it a second time (two timeouts). A moduleinfo bundle with 0 audio classes also gets loaded for nothing.
- **Bug — needless rescan:** `vst3CacheLoad` should return "cache parsed", not `!pluginList.empty()` (:152). Otherwise a cache where every probe failed triggers a full rescan that probes the same bad bundles again.
- **Bug — unreadable folder:** `getVST3Paths` (VST3Scanner.cpp:81) needs `skip_permission_denied` and error-code overloads. One unreadable folder throws, aborts the scan and hides every plugin — the same symptom as failure 1.
- **Report skipped bundles on stdout:** use `blog(LOG_WARNING)`, which goes to stdout (base.c:44), and have the host log what it captures.
  - Not stderr: the host only reads stderr after DONE (obs-vst3.cpp:299). Enough failure lines would fill the pipe and deadlock, and Windows pipe buffers are small.
- **Drain each child's stderr on a helper thread** while reading stdout, for the same reason.
- **Run probes one at a time.**
- **No overall deadline:**
  - It could only act between probes, so the bound would still be deadline + one timeout.
  - It would silently cut off large first scans.
  - It isn't needed: filters fall back to the saved `vst3_path` while scanning (obs-vst3.cpp:736-742), and C bounds shutdown.

## C. Option (i)
- **libobs:** add `EXPORT bool os_process_pipe_kill(os_process_pipe_t *pp)`.
  - POSIX: `kill(pid, SIGKILL)`. Windows: `TerminateProcess(pp->process, 1)`.
  - About 15 lines, purely additive. The pid can't be reused, because the child isn't reaped until `destroy`.
- **Why not the others:**
  - (ii) adds files, stale-file cleanup and races between OBS instances. It still waits for the current probe unless the child also polls the file.
  - (iii) can block quitting for the full 20 s timeout.
- **Host:**
  - Add `vst3ScanPipeMutex` around the pipe assignment in `retrieveVst3List` and the destroy-and-null in `getScanResult` (:309-310).
  - New `cancelVst3Scan()`: set a cancelled flag, kill under the lock, then join.
  - `obs_module_unload` calls it before `freeVst3List`. The Rescan path stays as is, because it only runs after the scan has finished.
  - When cancelled, `getScanResult` skips loading the JSON, the error log and `obs_source_update_properties`.

## D. Remember failed bundles — next PR
- **Why:** without it, every launch spends a full timeout on each hanging bundle and writes an OS crash report for each crashing one.
- **Format:** add an optional `"failed":[{path,stamp,reason}]` and keep `version: 1`. The host's `vst3JSONLoad` and the upstream scanner ignore unknown keys; if upstream rewrites the file, the list is dropped and the bundle is simply probed again.
- **Stamp:** binary mtime + size on Windows; elsewhere, the newest mtime over a bounded walk of the bundle.
- **Rules:** record only crashes, timeouts and load errors, never a failure to start the probe. The Rescan button passes `--retry-failed`.

## Files
- `libobs/util/pipe.h`, `pipe-posix.c`, `pipe-windows.c`
- `plugins/obs-vst3/`: `obs-vst3.cpp`, `plugin-main.cpp`, `VST3Scanner.h`/`.cpp`, `vst3-scan/vst3-scanner.cpp`
- `run.sh` expectations: 3 s timeout via the env variable; 3 classes plus 2 "skipped" lines; OBS exits in under 5 s when quit mid-scan; one run that waits for the scan to finish.

## Risks I can't test here
**Windows**
- **Inherited handles:** libobs starts processes with handle inheritance on (pipe-windows.c:67).
  - The coordinator must mark its own stdout/stderr handles non-inheritable at startup. Otherwise the probe child holds the host's pipe, and after the kill the host's read doesn't finish until the probe dies (up to 20 s).
  - The probe child should do the same before loading, so helpers the plugin starts don't inherit its pipe.
  - Probes running at the same time would leak pipes into each other, which is another reason to keep them sequential.
- **Crash dialogs:** in probe mode set `SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX`. `main` currently sets only the first (vst3-scanner.cpp:214), and a Windows Error Reporting dialog can keep a crashed child alive indefinitely.
- **Leftover probes:** after a kill, a probe can live up to the timeout because there's no `getppid` check. Optional fix: the coordinator puts itself in a job object that kills its children when it closes.
- **Other-architecture binaries:** `getVST3Paths` returns every `*.vst3` file, including `Contents/x86-win` and `arm64-win` copies. Each would fail its probe and log a warning on every launch, so filter to the build's architecture folder.
- **Path mismatch:** moduleinfo entries store the bundle root (VST3Scanner.cpp:227/277), while the filesystem scan returns the binary, so they never match the cache. Harmless now; D must key on the scanned path.
- **Speed:** process start plus antivirus per probe makes first scans slower, which is why the timeout should stay generous.

**macOS**
- Each crash adds a crash-report delay and one report per launch, which is another argument for D. `_exit` and the `getppid` check (orphans are re-parented to launchd) should behave the same as on Linux.
