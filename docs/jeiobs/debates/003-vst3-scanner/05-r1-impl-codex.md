# Round 1 implementation worker — Codex (summary)

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
````

## Answer

Implemented and committed as `1e0de02dd` (`jeiobs: isolate VST3 scanner probes`). Worktree is clean.

Changed:

- `libobs/util/pipe.*`: added `os_process_pipe_kill`.
- VST3 scanner/coordinator: isolated probes, watchdogs, atomic nonce results, crash/timeout skipping, deterministic discovery, cache fixes.
- Host: mutex-protected pipe lifecycle and unload cancellation.
- Handoff/history evidence updated.

Verification:

- `cmake --build _build -j10`: passed.
- Crash + hang scan: exit 0, 3 classes retained, both bundles skipped, ~2 seconds.
- Disposable OBS unload test: exit 0, zero leaks.
- Empty scan: exit 1 with `NO_VST3`.
- Test files were not modified.

No intentional functional deviations. Windows/macOS builds and runtime tests remain unverified; risks include platform-specific process spawning, handle inheritance, architecture-folder filtering, and loader behavior.
