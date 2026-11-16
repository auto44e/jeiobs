# Round 1 review of implementation — Claude

_Raw peer correspondence, task 003 (2026-11-15/16). Codex tool-call output trimmed; prompt and final answer kept verbatim. AI-assisted, not human review._

## Prompt

````text
# Worker brief: JeiOBS task 003 — VST3 scanner robustness (IMPLEMENTATION)

You are the implementation worker. Work ONLY inside this git worktree:
`~/oss/jeiobs-wt/scanner-impl` (branch
`wip/vst3-scanner-impl`). A configured build directory exists at `_build/`
(Ninja, warnings are errors). Build with: `cmake --build _build -j10`. Do not
reconfigure, do not touch files outside the worktree, no network, no push. You may
`git commit` on this branch when done (author identity: use
`git -c user.name=auto44e -c user.email=auto44e@gmail.com commit`).
Do NOT edit `tools/jeiobs/vst3-check/*` (another worker owns the tests).

Context: JeiOBS is a fork of OBS Studio 33.0.1. The upstream VST3 host was
backported to `plugins/obs-vst3`. Read `docs/jeiobs/tasks/003-vst3-scanner-robustness.md`
and `docs/jeiobs/tasks/001-vst3.md` (runtime evidence section) first, then the code:
`plugins/obs-vst3/{obs-vst3.cpp,plugin-main.cpp,VST3Scanner.h,VST3Scanner.cpp,vst3-scan/vst3-scanner.cpp}`
and `libobs/util/{pipe.h,pipe-posix.c,pipe-windows.c}`.

## Observed failures to fix (Linux, reproducible)
1. One bundle that segfaults on load kills the single-process scanner before it
   writes the list -> OBS has zero VST3 plugins.
2. One bundle that hangs on load -> scanner never exits (no timeout).
3. With (2), OBS shutdown blocks forever joining the scan thread.

## Agreed design (from two independent design reviews; follow it)

### libobs: `os_process_pipe_kill`
Add `EXPORT bool os_process_pipe_kill(os_process_pipe_t *pp);` to `libobs/util/pipe.h`.
POSIX (`pipe-posix.c`): `kill(pp->pid, SIGKILL)`; Windows (`pipe-windows.c`):
`TerminateProcess(pp->process, 1)`. Signal only; reaping stays in
`os_process_pipe_destroy`. Purely additive; null-safe; document it in the header.
Check that pipe-posix keeps the pid (it does, for waitpid).

### Scanner = coordinator that never loads plugin code
- New child mode: `obs-vst3-scanner --probe <bundle-path> <result-file> <nonce> <timeout-ms>`,
  handled at the top of `main` before the existing argv[1] handling.
- Coordinator spawns itself (`os_get_executable_path_ptr` like host `start_pipe`, or
  argv[0] resolution — be careful; prefer the same helper the host uses with the
  scanner binary name `obs-vst3-scanner` / `obs-vst3-scanner.exe`) once per bundle
  that needs a code load, **sequentially**, via `os_process_pipe_create2(args, "r")`.
- Child: start a watchdog `std::thread` BEFORE `VST3::Hosting::Module::create`. It
  checks every ~100-200 ms; when the deadline passes OR (POSIX) `getppid()` changed,
  it hard-exits: POSIX `_exit(3)`; Windows `TerminateProcess(GetCurrentProcess(), 3)`
  (NOT ExitProcess/exit: loader lock / static destructors).
- Child output hygiene, POSIX: before loading plugin code, point fds 1 and 2 at
  `/dev/null` (open O_RDWR|O_CLOEXEC then dup2) so plugin output and grandchildren
  cannot corrupt or hold the coordinator's pipe. Windows: before loading, make the
  inherited std handles non-inheritable (`SetHandleInformation(h, HANDLE_FLAG_INHERIT, 0)`)
  and in the COORDINATOR at startup also mark its own stdout/stderr handles
  non-inheritable is NOT possible since it must pass them... Decide carefully:
  libobs pipe-windows creates the child with inheritance on; the child should clear
  inheritance of its std handles before loading the plugin. Also set
  `SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX)` in probe mode.
- Child result: serialize the bundle's classes with the SAME per-class fields the
  cache uses (name, id, path, pluginName, discardable, isFX; factor a shared helper
  used by both the cache save/load and the probe), write JSON to
  `<result-file>.tmp`, then atomically rename to `<result-file>` (libobs
  `os_rename` or `obs_data_save_json_safe`), and include the nonce inside the JSON.
  Then hard-exit with 0 on the success path too (skip ModuleExit/dlclose/static dtors).
- Coordinator accepts a probe only if: the result file exists, parses, carries the
  exact nonce, and every entry has a non-empty id. It overwrites `path` and
  `pluginName` with the bundle it asked for. Anything else (crash, timeout, missing
  or garbage file) = that bundle is skipped and logged ONCE with path and reason
  (`blog(LOG_WARNING, "[VST3 Scanner] Skipped %s: %s", ...)`), others kept. The
  coordinator reads the child's pipe until EOF (draining, bounded storage) then
  destroys it. Remove the result file afterwards. Result files go in a private temp
  directory next to the cache JSON (e.g. `<cache dir>/probe-<pid>-<n>.json`).
- Default per-probe timeout 10000 ms; env override `OBS_VST3_PROBE_TIMEOUT_MS`
  (read only by the coordinator; clamp to 500..120000).
- **No** overall scan deadline (shutdown is bounded by the kill below).
- Make it impossible for the coordinator to load code: route ALL code-load sites
  through one hook. `VST3Scanner::addModuleClasses` must call a probe function
  (set by the coordinator) and must refuse to load in-process if no hook is set.
  The in-process loader becomes a separate function used only by `--probe`.
  Load sites today: VST3Scanner.cpp scanForVST3Plugins / updateModulesList /
  tryReadModuleInfo (discardable), and vst3-scanner.cpp vst3CacheLoad (discardable
  cached modules). Grep to confirm.
- Fix the related bugs found in review:
  - `tryReadModuleInfo` must report "parsed" separately from "has classes": a valid,
    non-discardable moduleinfo.json with zero audio classes must NOT cause a code
    load, and a discardable bundle must not be probed twice when it fails.
  - `vst3CacheLoad` should return "cache was parsed", not `!pluginList.empty()`,
    so a cache with zero usable plugins does not trigger a full re-probe.
  - `getVST3Paths()` / `hasVST3()` directory iteration: use
    `std::filesystem::directory_options::skip_permission_denied` and error_code
    overloads so one unreadable folder doesn't abort the scan.
  - Iterate bundles in sorted order (deterministic).
  - Coordinator writes diagnostics to stdout via blog (blog goes to stdout), so the
    host's stdout read collects them; keep stderr small. The host reads stdout until
    the marker then stderr — do not make the coordinator write large stderr.
  - The coordinator always prints `VST3_SCAN_DONE` after a best-effort scan, even
    if every probe failed; `NO_VST3` only when no bundle exists.
- Windows: only probe binaries for the build's architecture folder
  (`Contents/x86_64-win` for x64, `Contents/arm64-win` / `arm64ec-win` for ARM64);
  look at how getVST3Paths discovers Windows bundles and filter accordingly. Don't
  over-engineer; keep it compiling on Windows/macOS (you cannot compile those here:
  be conservative, use only APIs available there, guard with `#ifdef _WIN32` /
  `__APPLE__`).

### Host (`obs-vst3.cpp`)
- Add a mutex protecting `vst3ScanPipe` (assignment in `retrieveVst3List`,
  destroy+null in `getScanResult`).
- New `cancelVst3Scan()`: set an atomic cancelled flag, kill the pipe under the lock
  (`os_process_pipe_kill`), then join the thread. Call it from `obs_module_unload`
  (plugin-main.cpp) before `freeVst3List()`; `freeVst3List` itself may call it.
- When cancelled, `getScanResult` skips JSON loading, the error log and
  `obs_source_update_properties`, but still destroys the pipe and releases `src`.
- Keep the rescan path working (it only runs when the scan is done).
- Do not change DSP/audio code. Do not change the cache JSON format (version 1).

## Done criteria
- `cmake --build _build -j10` succeeds with no new warnings.
- Quick self-check (Linux) using the test bundles: you may build throwaway bundles
  under `/tmp` from `tools/jeiobs/vst3-check/bad-crash.c` and `bad-hang.c` exactly as
  `tools/jeiobs/vst3-check/run.sh` does, plus the good sample bundles already built at
  `~/oss/jeiobs-deps/vst3-full/vst3sdk/build/VST3/Release/{adelay,again-sample-accurate}.vst3`.
  Run the scanner directly with a throwaway HOME:
  `cd _build/rundir/Debug/bin && HOME=$T LD_LIBRARY_PATH=<see run.sh> OBS_VST3_PROBE_TIMEOUT_MS=2000 ./obs-vst3-scanner $T/list.json`
  with `$T/.vst3/` containing good + crash + hang bundles. Expect: exit 0, 3 classes
  in list.json, two "Skipped" lines, finishes in ~2-3 s. LD_LIBRARY_PATH:
  `$D/ffmpeg-8.0.3/lib:$D/mbedtls-3.6.7/lib:$D/Qt/6.11.1/gcc_64/lib` with
  `D=~/oss/jeiobs-deps`. Never use the real HOME.
- Commit with a clear message. Then print a summary: files changed, design choices
  where you deviated from this brief and why, what you verified, and every
  Windows/macOS risk you could not test.

---
You are now the ADVERSARIAL REVIEWER (different model lineage from the author). The implementation is committed in the worktree ~/oss/jeiobs-wt/scanner-impl (branch wip/vst3-scanner-impl); the diff vs base is in /tmp/jeiobs-scan/impl.diff. Read the full changed files for context. On Linux it builds and passes the runtime suite (15/15: crash, hang, noisy-with-fd-holding-grandchild, clean-exit, moduleinfo-only bundles; OBS quits mid-scan in 3.3 s with 0 leaks). Try to BREAK it. Focus: (1) Windows and macOS correctness that can't be compiled here: process spawn via libobs pipes, handle inheritance, TerminateProcess semantics, SetErrorMode, _getpid include, the architecture-folder filter vs how Windows bundles are laid out (single-file legacy .vst3 vs Contents/x86_64-win/*.vst3), macOS bundle paths and whether the scanner can find itself with os_get_executable_path_ptr inside an .app; any symbol not available there; -Werror risks (unused variables in #ifdef branches, sign-compare). (2) Races/lifetime in obs-vst3.cpp scan thread vs cancel vs rescan vs vst3Update. (3) Watchdog/hard-exit correctness, result file atomicity, stale files left in the config dir after kill/crash. (4) Cache path: discardable modules, updateModulesList, a cache with zero plugins, behaviour change vs upstream. (5) Anything that changes behaviour for the normal case (all good plugins, moduleinfo present). Report concrete findings: file:line, severity (blocker/major/minor), and a suggested fix. Say 'no finding' per area if none. Max 900 words. Do not edit files.
````

## Answer

Linux holds up. Windows does not yet: the timeout there can fail completely, probe children keep OBS's pipes open, and plugins packaged as a single `.vst3` DLL disappear from the list. I reviewed by reading the code and the pinned SDK. Nothing was compiled or run on Windows or macOS.

## 1. Windows / macOS

**BLOCKER (Windows): the coordinator has no timeout of its own.** `vst3-scanner.cpp:295-298` waits for EOF, then calls `os_process_pipe_destroy`, which waits forever. The child's own watchdog is the only limit.
- On Windows, MSVC's `std::thread` returns before the new thread has started running.
- A new thread must take the loader lock (for `DLL_THREAD_ATTACH`) before its code runs.
- If the main thread reaches `LoadLibrary` first and the plugin hangs in `DllMain` or a static constructor (the Windows version of `bad-hang.c`), the watchdog never runs. The probe and the coordinator then hang forever.
- The same happens if Windows Error Reporting freezes the process on a crash, or if the probe is stopped (SIGSTOP).
- **Fix:** in `probeVst3Module`, run a timer (condition variable `wait_for`) that calls `os_process_pipe_kill(pipe)` after the timeout plus a grace period, and join it after draining. Also make the main thread wait until the watchdog thread has signalled that it started, before loading the plugin. This matches task 003's original idea of having the parent kill the scanner.

**MAJOR (Windows): probes inherit OBS's pipe handles.**
- OBS creates the coordinator's stdout/stderr pipes as inheritable (`pipe-windows.c:37`). libobs then starts probes with handle inheritance on (`pipe-windows.c:67`).
- So every probe, and every helper process a plugin starts with inheritance, holds OBS's stdout/stderr pipe. `prepareProbeOutput` (`:138`) only clears the probe's own std handles.
- **Quitting OBS mid-probe:** `TerminateProcess` kills only the coordinator. Windows has no parent-death check, so `getScanResult`'s `ReadFile` waits until the probe's deadline (10 s, up to 120 s). The orphaned probe also leaves a stale `probe-*.json`.
- **A plugin helper that stays alive** keeps OBS's stderr pipe open. The stderr read loop never ends, `vst3ScanDone` is never set, and the join at shutdown hangs forever. This is the Windows version of the Linux noisy-grandchild test.
- **Fix:** in coordinator mode, clear `HANDLE_FLAG_INHERIT` on its own `STD_INPUT/OUTPUT/ERROR` handles at startup. libobs passes fresh pipe handles to each probe explicitly, so probes don't need these. Then either put the coordinator in a job object with `KILL_ON_JOB_CLOSE` (plus `DIE_ON_UNHANDLED_EXCEPTION`), or pass the coordinator's PID to the probe and have the watchdog wait on it.

**MAJOR (Windows, normal case): the architecture filter drops single-file plugins.**
- `VST3Scanner.cpp:73-82,149` only accepts a `.vst3` file whose parent folder is an architecture folder.
- That drops single-file `.vst3` DLLs such as `VST3\Foo.vst3` or `VST3\Vendor\Foo.vst3`. Upstream found these, and the pinned SDK still loads them (`module_win32.cpp` `loadAsDll`; `findFilesWithExt` adds non-folder `.vst3` files).
- Side effects: `hasVST3()` returns false on systems with only such plugins, and `updateModulesList` erases them from existing caches after an upgrade.
- On ARM64 the filter accepts `arm64ec-win`, which a native ARM64 process can't load (the SDK only uses `arm64-win` there). Those bundles will always produce "Skipped" lines.
- **Fix:** apply the filter only when `parent.parent.filename() == "Contents"`. Accept `arm64-win` on ARM64, and `arm64ec`/`arm64x`/`x86_64-win` on ARM64EC.

**macOS:** no finding. `os_get_executable_path_ptr` (`_NSGetExecutablePath` + `realpath`) resolves to the scanner's own folder in `OBS.app/Contents/MacOS`. The entitlements already disable library validation, and `O_CLOEXEC`/`dup2`/`getppid` are all available. One cosmetic risk: Gatekeeper or AppKit side effects (dialogs, Dock icon) now happen once per probed bundle instead of once.

**-Werror:** no finding by inspection. MSVC `/W3` is fine given the explicit casts, `[[noreturn]]` and `_CRT_SECURE_NO_WARNINGS`. The Xcode warning settings are fine too. `_getpid` comes from `<process.h>`.

## 2. Host races (`obs-vst3.cpp`)
- **Minor:** the cancel flag is read once (`:299`). A cancel that arrives while `vst3JSONLoad` runs still triggers `obs_source_update_properties` (`:343`) during shutdown. Check the flag again just before it.
- **Minor, pre-existing:** a rescan holds a strong `src` reference (`:377`). If that is the last reference, the filter is destroyed on the scan thread inside `obs_module_unload`. Use a weak reference.
- **Pre-existing, unchanged:** `vst3Update` on another thread can read `vst3ScanDone`, then use `list` after a rescan has already deleted it (use-after-free window).
- Kill and destroy under one mutex is correct; the kill can never hit a reaped PID.

## 3. Watchdog and files
- **Minor:** stale `probe-<pid>-<n>.json` / `.tmp` files are left after the coordinator is killed: on Windows always, on Linux/macOS if the probe finishes inside the 150 ms window. The next run has a different PID, so nothing cleans them. Fix: use a per-run subfolder and clean up old ones at startup by age, not by deleting all of them, since `--multi` can run two coordinators on the same folder.
- **Minor:** the child records its parent PID itself (`:167`). If the coordinator dies before that line runs, the orphan runs until its deadline. Pass the expected PID on the command line instead.
- **Minor:** exit code 3 also comes from MSVC `abort()` or a plugin calling `exit(3)`, which then gets logged as "timed out". Use a more distinctive code.
- Result files are safe: exit status is checked first, then temp file plus rename, then the nonce.

## 4. Cache
- **Minor:** bundles that fail are never cached, so every launch probes them again, one after another. Each hanging bundle adds 10 s before the plugin list can be used, on every start. Consider a separate skip list keyed by path, mtime and size, so `vst3list.json` stays version 1.
- **Minor:** `VST3_SCAN_DONE` is printed even when `vst3CacheSave` fails or the scan threw (`:486`, `:506`). The host then quietly loads the previous `vst3list.json`.
- **Nit:** `hasClasses` (`VST3Scanner.h:73`) is never used, and is true for a probe that found zero classes.
- Discardable modules are probed once per run, not twice. A cache with zero plugins is fine, and `updateModulesList` still finds new bundles.

## 5. Normal case
- The Windows single-file regression above is the main normal-case change.
- **Minor:** `hasVST3()` (`:199`) now walks and sorts the whole plugin tree synchronously during `obs_module_load`; upstream stopped at the first match. The coordinator also walks the tree 2–3 times. Restore the early exit.
- **Minor:** the 10 s default can skip plugins that are slow but fine, for example on a cold start with antivirus scanning or license checks. Upstream had no limit. Consider 30 s, or one retry with a longer timeout.
- **Minor:** in libstdc++, MSVC STL and libc++, any increment error other than permission-denied ends that whole root folder (`:140`, `:157`), and nothing is logged. So it is not really "skip one folder".
- With `moduleinfo.json` present and nothing discardable, there are no probes and behaviour matches upstream.
