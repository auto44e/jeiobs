No change to my position: claude-1 to claude-4 stay accepted, claude-5 stays open at Minor, and I approve. I agree with codex-5, but only as a Minor wording issue; the code is correct. I read the code again; I didn't build anything or run `run.sh`.

**codex-5 (Codex's finding)**
- Task doc :164-165 records the test author's reading of what the scanner *writes*, not a rule for what the loader accepts. The checks only read `kind` and never write one (`skiplist.py:7-8, :116-119, :140-141`).
- The code agrees with itself on two values:
  - the scanner writes only `failure` or `timeout` (`vst3-scanner.cpp:1411-1430`);
  - the loader accepts only those (:280-281);
  - only those can cause a skip (:530, :535).
- An entry with any other `kind` would never be skipped even if it were loaded. Ignoring it costs one extra probe, which is the documented behaviour for malformed entries (doc :36-37).
- The real gap is that Storage (:34-38) never lists the allowed `kind` values. Suggested wording:
  - in Storage, "`kind` is `failure` or `timeout`; any other value makes the entry ignored";
  - at :164, "the checks accept any non-empty `kind` other than `timeout` as a remembered failure; the scanner writes `failure`".

**claude-5: re-checked, it holds, and it matters more than I said in round 1.**
- `prepareProbeOutput()` points the probe's stdout and stderr at /dev/null or NUL (:972-982, :968-970). This happens before `setup` is written (:1141, :1148), so the coordinator's pipe reads end right away.
- As a result, `finishThreads()` (:1303) stops the coordinator's timeout thread long before the deadline. In practice every coordinator kill after `setup` happens at :1323-1325, the path check #14 exercises (`want line mhang 1 killed`, `skip-list.sh:345-347`).
- Whether that kill counts as a timeout strike depends on `steady_clock::now() >= deadline` (:1324). That is measured after a wait that runs on a different clock:
  - Windows: `WaitForSingleObject` (`pipe-windows.c:209`), which runs on the system tick clock and can return up to a tick early.
  - macOS: `CLOCK_MONOTONIC` (`pipe-posix.c:157, :183`). Apple's clock_gettime(3) says only `CLOCK_MONOTONIC_RAW` is "unaffected by frequency or time adjustments", and that is the clock libc++'s `steady_clock` uses.
- Rounding the wait up by 1 ms (:1317-1319) only protects Linux. On Windows and macOS the result can be "probe wait failed" with the entry left unchanged (:1389-1391), instead of the timeout strike in spec row :68.
- It stays Minor: it only affects the backstop case (the probe's own watchdog failed to end it), and the error is in the less sticky direction.
- Fix as before: if the wait returns false while `now < deadline`, wait again for the remaining time, with a bounded number of retries.

**codex-4 leftover window:** the watchdog releases the mutex (:1076) before calling `probeExit` (:1077). Codex accepted codex-4 without changing this. I'm not raising it: it needs a stall at least as long as the probe timeout right before the `setup` write, and the worst outcome is one timeout strike.

**Evidence count is correct:** 23 `check` lines plus #6 give 24 skip-list checks (`skip-list.sh:219-387, :264-267`). With the 16 task-003 checks that makes 40, as doc :173 says.

My approval still depends on the full CI build at this head coming back green; it hasn't been reported yet.

FINDING claude-1 | Minor | accepted-fix | valid matching-nonce result accepted before waitError/markers (vst3-scanner.cpp:1382-1387); check #17
FINDING claude-2 | Minor | accepted-fix | exit 90 → unchanged (:1157-1161, :1421-1423), exit 91 → strike (:1069-1078, :1424-1426); check #15
FINDING claude-3 | Minor | accepted-fix | Linux binary is `uname().machine + "-linux"` + `<stem>.so` (:587-595), as in the SDK's module_linux.cpp
FINDING claude-4 | Minor | accepted-fix | checks #12–#14 added (skip-list.sh:330-349); remaining gaps listed (task doc :188-195)
FINDING claude-5 | Minor | open | after-setup kills happen at :1323-1325 (pipe reads end at prepareProbeOutput, :1141/:1303); deadlinePassed (:1324) uses steady_clock, but the wait uses the tick clock (pipe-windows.c:209) or CLOCK_MONOTONIC (pipe-posix.c:157,183), so on Windows/macOS a kill can be logged "probe wait failed" and leave the entry unchanged (:1389-1391) instead of a strike (spec :68)
VERDICT: approve
