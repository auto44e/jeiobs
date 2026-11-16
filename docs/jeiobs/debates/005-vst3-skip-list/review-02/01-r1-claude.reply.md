I accept the fixes for all four of my run-01 findings. I'm raising one new Minor issue (claude-5) and approve, provided the full CI build at this head comes back green: the fix commit hasn't been compiled on Windows or macOS yet. I read the code; I didn't build anything or run `run.sh`.

**My run-01 findings**
- **claude-1, accepted.** A valid result with the matching nonce is now checked before the wait-error check and before any marker logic (`vst3-scanner.cpp:1382-1387`). Check #17 covers a missing marker; the `save-failed` case is listed in Not done (task doc :192).
- **claude-2, accepted.**
  - Probe side: if the `save-failed` marker can't be written, the probe exits 90 (`:1157-1161`). The coordinator then leaves the entry unchanged (`:1421-1423`).
  - Watchdog side: if the `timeout` marker can't be written, it exits 91 (`:1069-1078`), which counts as a timeout strike (`:1424-1426`).
  - A failed `load-failed` write still ends as a remembered failure (`:1430`), which is no stickier.
- **claude-3, accepted.** The Linux arch directory is now `uname().machine + "-linux"` + `<stem>.so` (`:587-595`). That is exactly what the SDK builds (`module_linux.cpp:71-80, 141-152`).
- **claude-4, accepted.** Checks #12, #13 and #14 (`skip-list.sh:330-349`) cover the three missing outcome rows, and the remaining gaps are listed in the task doc (:188-195). #12–#14 also pass on `6fb0cd502`, so they only catch future regressions. #14 does catch the Linux rounding risk I warned about in run 01: if the new deadline check misfired, `line mhang 1 killed` and `entry … timeout` would fail.

**Codex's findings, as I read the fixes**
- **codex-1:** correct on Linux, where `steady_clock` and the wait run on the same clock and the timeout is rounded up (`:1311-1328`). It is not correct on Windows or macOS; see claude-5.
- **codex-2:** fixed, since it is the same defect as claude-2.
- **codex-3:** fixed. Numbers must be type-checked integers (`:248-266, :292-300`) and `version` likewise (`:371`); check #16 covers it.
- **codex-4:** fixed (`:1146-1149, :1070-1076`). One tiny window remains: the watchdog releases the mutex (`:1076`) before `probeExit` (`:1077`). So `setup` can still be written after the watchdog has decided not to write `timeout`, and `setup` + exit 91 then counts as a strike. It needs a stall of at least 500 ms right before the `setup` write, and the result is harmless. Holding the lock across `probeExit` closes it. Codex's call.

**Checks #15–#17** test what they claim. They fail on `6fb0cd502` for the expected reasons, per `14-tests2-claude.md:12-16`.

**claude-5 (new, Minor): on Windows and macOS, a real deadline kill can be recorded as a "wait error".**
- After `os_process_pipe_wait` returns false, the code checks `steady_clock::now() >= deadline`. If that fails, it sets `waitError`, logs "probe wait failed" and leaves the entry unchanged (`:1324, :1328, :1389-1391`). The round-up only protects Linux, because there both clocks are the same.
- **Windows:** `WaitForSingleObject` (`pipe-windows.c:209`) runs on the tick clock. Microsoft's wait-function docs say a wait between one and two ticks "can be anywhere between one and two ticks", so it can expire before the requested time. MSVC's `steady_clock` uses a different clock, QPC.
- **macOS:** the wait uses `CLOCK_MONOTONIC` (`pipe-posix.c:157, :183`). On Darwin that is wall-clock time minus boot time, so NTP adjustments change its rate. libc++'s `steady_clock` on Apple is `CLOCK_MONOTONIC_RAW`, which they don't.
- **Effect:** in the backstop case (a probe the in-probe watchdog failed to end), spec row 2 (task doc :68, timeout strike) degrades to "unchanged". It never makes an outcome stickier, but D6 loses its effect there on two of the three platforms. `run.sh` is Linux-only, so no check can catch it.
- **Fix:** if the wait returns false while `now < deadline`, wait again for the remaining time (with a bounded number of retries) before calling it an error.

FINDING claude-1 | Minor | accepted-fix | valid matching-nonce result accepted before waitError/markers (vst3-scanner.cpp:1382-1387); #17
FINDING claude-2 | Minor | accepted-fix | exit 90 → unchanged (:1157-1161, :1421-1423), exit 91 → strike (:1069-1078, :1424-1426); #15
FINDING claude-3 | Minor | accepted-fix | `uname().machine + "-linux"` + `<stem>.so` (:587-595) = SDK module_linux.cpp:71-80, 141-152
FINDING claude-4 | Minor | accepted-fix | #12–#14 added (skip-list.sh:330-349); remaining gaps listed (task doc :188-195)
FINDING claude-5 | Minor | open | the wait's clock ≠ steady_clock on Windows/macOS (pipe-windows.c:209; pipe-posix.c:157,183), so a deadline kill after setup can become "probe wait failed"/unchanged (:1324,:1328,:1389-1391) instead of a strike (spec :68)
VERDICT: approve
