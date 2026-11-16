# Round 3 implementation worker — Codex (summary)

_Raw peer correspondence, task 003 (2026-11-15/16). Codex tool-call output trimmed; prompt and final answer kept verbatim. AI-assisted, not human review._

## Prompt

````text
# Worker brief, round 3: JeiOBS task 003 — VST3 scanner robustness (IMPLEMENTATION)

Same rules: work ONLY in `~/oss/jeiobs-wt/scanner-impl`
(branch `wip/vst3-scanner-impl`), build with `cmake --build _build -j10` (warnings
are errors; do not reconfigure), no network, no push, do NOT edit
`tools/jeiobs/vst3-check/*` or `docs/`. Commit with
`git -c user.name=auto44e -c user.email=auto44e@gmail.com commit`.

You wrote rounds 1-2 in earlier sessions (you don't remember them). Read
`git log --oneline -8` and the current code: `plugins/obs-vst3/vst3-scan/vst3-scanner.cpp`,
`plugins/obs-vst3/VST3Scanner.{h,cpp}`, `plugins/obs-vst3/obs-vst3.cpp` (scan
section), `libobs/util/pipe.h`, `pipe-posix.c`, `pipe-windows.c`. The suite
`tools/jeiobs/vst3-check/run.sh $PWD/_build ~/oss/jeiobs-deps`
passes 15/15 (~45 s) and must stay green.

A second adversarial review (Claude) found these. Fix all of them.

1. **The coordinator kill does not work on POSIX (medium).** The probe redirects
   fds 1/2 to /dev/null before loading plugin code, so the coordinator's drains hit
   EOF immediately, the kill timer is cancelled, and `os_process_pipe_destroy` →
   `waitpid` then blocks with no bound. Reproduced: SIGSTOP the probe at 0.65 s with
   a 2000 ms timeout → never killed until resumed. Fix by bounding the PROCESS WAIT:
   - Add a libobs export `EXPORT bool os_process_pipe_wait(os_process_pipe_t *pp, uint32_t timeout_ms);`
     returning true if the process has exited within the timeout, WITHOUT reaping it
     (destroy still reaps). POSIX: poll `waitid(P_PID, pid, &info, WEXITED | WNOHANG | WNOWAIT)`
     every ~10 ms (handle EINTR; `info.si_pid != 0` means exited). It must compile on
     Linux and macOS (both have waitid/WNOWAIT; include <sys/wait.h>). Windows:
     `WaitForSingleObject(pp->process, timeout_ms) == WAIT_OBJECT_0`. Document in pipe.h.
   - Coordinator per probe: one deadline = spawn time + timeout + margin. Keep a kill
     timer for the drain phase (drains can block only if something still holds the
     pipe). After both drains finish, wait with `os_process_pipe_wait(pipe, remaining)`;
     if it did not exit, `os_process_pipe_kill`, then `os_process_pipe_wait(pipe, 5000)`
     again, then destroy. Kill must never be called after destroy (no PID reuse).
     Report "probe timed out (killed)" when the coordinator had to kill.
   - Margin: `max(5000 ms, timeout / 4)` (the watchdog deadline starts at thread
     start, which can be late on a cold start).
2. **Watchdog handshake use-after-scope (medium-low).** `startProbeWatchdog` signals
   a condition variable that lives on its own stack after it may have returned.
   Use `std::promise<bool>` / `std::future`, or a `std::shared_ptr` state captured by
   value; no reference to the caller's stack from the detached thread.
3. **Windows probe stdio (low).** In probe mode on Windows, also redirect the probe's
   stdout/stderr to NUL before loading the plugin, as on POSIX: open NUL, `SetStdHandle`
   for STD_OUTPUT/STD_ERROR, `_dup2` the CRT fds 1 and 2 (via `_open_osfhandle` or
   `freopen("NUL", "w", stdout/stderr)`), and `CloseHandle` the original pipe handles so
   plugin code cannot duplicate or pass them on. Then the coordinator's drains end at
   once on Windows too, and the bounded wait from item 1 limits the probe. Be careful
   with the order and with handles that may be invalid/NULL. Keep it compiling (MSVC).
4. **Coordinator warnings are lost (low).** In the host (`obs-vst3.cpp` getScanResult),
   pass on every line from the scanner's stdout that contains `[VST3 Scanner] `
   (not only `Skipped` lines), with LOG_WARNING if the line starts with `warning:`,
   LOG_ERROR if it starts with `error:`, else LOG_INFO, stripping the `warning: ` /
   `error: ` / `info: ` prefix. Do not duplicate the "Available plugins" list the host
   prints itself.
5. **Thread start failure (low).** If starting the error-drain thread (or the timer
   thread) throws, kill the probe, join anything started, destroy the pipe, and skip
   the bundle with a reason; never let a joinable std::thread be destroyed.
6. **Cleanup directory (nit).** If the cache path has no directory part, skip the
   stale probe-file cleanup instead of scanning "./".

## Done
Build passes; suite 15/15. Also run this manual check and report it: throwaway HOME
with good bundles (`~/oss/jeiobs-deps/vst3-full/vst3sdk/build/VST3/Release/{adelay,again-sample-accurate}.vst3`)
plus a hang bundle built like run.sh builds it from `tools/jeiobs/vst3-check/bad-hang.c`;
run the scanner with `OBS_VST3_PROBE_TIMEOUT_MS=2000`, find the probe child
(`pgrep -f -- "--probe"` restricted to that HOME path) and `kill -STOP` it within the
first second; expect the coordinator to kill it and finish with exit 0, 3 classes,
reason "probe timed out (killed)", within about 2000 + 5000 + 1000 ms. Never use the
real HOME; kill any leftover processes you started. Commit, then print what changed
per item, the manual check output, and remaining Windows/macOS risks.
````

## Answer

Implemented and committed as `8edfb74f6`.

1. Added bounded, non-reaping process waits; coordinator deadline/kill cleanup now handles POSIX hangs.
2. Replaced watchdog stack-reference handshake with `std::promise`.
3. Redirected Windows probe stdout/stderr to NUL and closed inherited pipe handles.
4. Forwarded scanner warnings/errors/info while avoiding duplicate plugin lists.
5. Added exception-safe thread startup cleanup.
6. Skipped stale-result cleanup when cache paths lack directories.

Validation:

- `cmake --build _build -j10`: passed, 47/47.
- VST3 suite: 15/15 passed.
- STOP manual check: probe stopped at 480 ms; scanner exited 0 after 7028 ms, found 3 classes, reported `probe timed out (killed)`, and left 0 processes.
- Worktree clean.

Remaining risk: Windows/MSVC and macOS builds/runtime remain untested; specifically Windows CRT handle behavior and macOS `waitid(WNOWAIT)` compatibility.
