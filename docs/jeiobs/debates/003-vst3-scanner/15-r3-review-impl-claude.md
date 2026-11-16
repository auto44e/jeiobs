# Round 3 review of implementation — Claude

_Raw peer correspondence, task 003 (2026-11-15/16). Codex tool-call output trimmed; prompt and final answer kept verbatim. AI-assisted, not human review._

## Prompt

````text
You are the ADVERSARIAL REVIEWER (Claude lineage) of round 3 of a change written by Codex, in worktree ~/oss/jeiobs-wt/scanner-impl. Round-3 diff: /tmp/jeiobs-scan/impl-r3.diff (commit 8edfb74f6). It answers your own round-2 review (/tmp/jeiobs-scan/claude-review-impl-r2.md): (1) bounded non-reaping process wait os_process_pipe_wait + coordinator kill after drains; (2) promise-based watchdog handshake; (3) Windows probe stdio redirected to NUL and original handles closed; (4) host forwards all [VST3 Scanner] lines; (5) exception-safe thread starts; (6) skip cleanup without a directory. The suite now has 16 checks incl. a SIGSTOP-frozen probe (passes: killed at 7.0 s). Verify each fix is correct and complete, and look for new bugs: PID reuse (kill after reap), waitid/WNOWAIT portability on macOS and FreeBSD, timespec arithmetic, Windows CRT/handle ordering in the NUL redirect (closing a handle the CRT still owns, _dup2 failures), duplicated or noisy host log lines, and any regression for the normal case. You may run the scanner yourself with throwaway HOMEs as in your earlier check (/tmp/rv-r2-scripts/). Report findings with file:line, severity, fix; 'no finding' per item if none. Max 500 words. Do not edit repo files.
````

## Answer

**Round-3 review of 8edfb74f6:** fix 3 won't compile on MSVC, and fix 1 roughly doubles scan time for fast plugins. The other fixes hold on Linux, where I re-ran the frozen-probe tests. I found no PID-reuse or timespec bug.

**Findings**

1. **High: the Windows build breaks** (`vst3-scanner.cpp:264-265`).
   - `STDOUT_FILENO`/`STDERR_FILENO` are used inside `_WIN32` code. MSVC doesn't define them, and nothing in the repo or SDK does either; upstream uses `_fileno(stdin)` (`ffmpeg-mux.c:1166`).
   - **Fix:** use `_fileno(stdout)` and `_fileno(stderr)`.

2. **Low-medium: normal scans get slower** (`pipe-posix.c:186-193`).
   - The drains end when the probe redirects its output, before it loads the plugin. So the first poll always comes before the exit, and the coordinator then sleeps the full 10 ms.
   - **Measured:** each probe's exit is noticed 9.4–10.1 ms late. 50 bundles took 1.0–1.1 s as built, against 0.47–0.53 s with a blocking-wait shim.
   - **Fix:** poll at 1, 2, 4, 8, then 10 ms, or block on a pidfd (Linux) or kqueue (macOS/FreeBSD).

3. **Low: Windows can close the same handle twice** (`vst3-scanner.cpp:258-260`).
   - `_dup2` already closes fd 1's handle, which is `originalHandle`. The explicit `CloseHandle` runs again unless Windows happened to reuse that handle value. The `!= redirectedHandle` check only prevents closing the new handle.
   - This normally fails silently. It raises an exception under a debugger or App Verifier, and can close another thread's handle that took the value.
   - **Fix:** remove that close, or close only if `_get_osfhandle(fd)`, read before `_dup2`, differs from `originalHandle`.

4. **Low, unverified: macOS `waitid`** (`pipe-posix.c:169-171`).
   - The macOS kernel source looks safe: it ignores stopped children without `WSTOPPED`, and the zeroed `info` covers a `WNOHANG` miss.
   - But golang/go#19314 saw macOS `waitid(WEXITED|WNOWAIT)` return for a SIGSTOPped child, and Go stopped using it there. If that happens here, nothing is killed and `destroy`'s `waitpid` hangs — the exact frozen-probe case.
   - **Fix:** also require `si_code` to be `CLD_EXITED`, `CLD_KILLED` or `CLD_DUMPED`. Run the freeze check on macOS.
   - FreeBSD: no finding.

5. **Nit** (`obs-vst3.cpp:313`): the host filters lines the scanner never prints ("Available plugins:", two-space indent). `debug: ` lines keep their prefix and log at INFO.

**Per item**

1. Findings 2 and 4.
   - PID reuse: no finding. Every kill happens before `destroy`, the only reap: the timer, the post-wait kill, the thread-failure path, and the host's cancel (which shares a mutex with destroy).
   - Timespec: no finding. Worst case is a 10 ms overshoot.
   - Remaining limit: a probe stuck in an uninterruptible kernel wait ignores SIGKILL, so `destroy` still blocks on it.
2. No finding. ThreadSanitizer was clean on a standalone copy of the handshake.
3. Findings 1 and 3. Otherwise it does its job: the pipe handles are closed before plugin code runs, and a failed redirect ends the probe.
4. Finding 5 only. No duplicate or extra lines; a normal scan adds nothing to the OBS log.
5. No finding.
6. No finding. Result files still go to the current directory, as before.

**Tests (Linux only; no Windows or macOS build):**
- Probe frozen before its watchdog starts (via a preload library): killed at 7.0 s.
- Probe frozen inside the plugin: killed at 7.0 s.
- Speed comparison: numbers above.
- `run.sh`: 16/16 passed.

My scripts are in `/tmp/rv-r3-scripts/`. I edited no repo files.
