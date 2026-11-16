The commit is the same as last round and nothing has been fixed, so my four Minor findings still stand and I still approve. Of Codex's findings, codex-1 and codex-3 are real; I'd rate codex-1 Minor rather than Major. codex-2 is the same defect as claude-2. The ledger is still empty: please add rows claude-1 to claude-4 and codex-1 to codex-3, and note that one fix closes both codex-2 and claude-2.

**codex-1: the problem is real, but it's very hard to trigger.**
- After a failed wait, `vst3-scanner.cpp:1278-1281` kills the probe and `:1361-1363` records a timeout strike without checking the deadline. Both reviewers asked for that check in the design debate (`06-design-r4.md:40`, `:79`).
- This wait is the main path, not a rare one. The probe sends stdout and stderr to /dev/null straight away (`:1101`, `:949-955`), so draining its output ends almost at once. The timeout thread is joined at `:1263`, so `:1278` is the only deadline kill.
- But a wait *error* almost never happens:
  - **POSIX:** the wait returns false on the deadline, on a `waitid` error other than EINTR, or if `clock_gettime` fails (`pipe-posix.c:157-189`). The probe is our own child and isn't reaped until destroy reaps it by pid (`:218`). Nothing in `plugins/obs-vst3` touches SIGCHLD.
  - **Windows:** false means only WAIT_TIMEOUT or WAIT_FAILED (`pipe-windows.c:209`).
- If the child has already exited, killing the zombie turns "failure" into "timeout", which is less sticky. It only escalates if a wait error happens while the child is still alive. So I'd call it Minor, but the severity is Codex's call.
- **Fix caution:** `remainingMs` is rounded down (`:1274-1275`), so the wait can time out just under 1 ms before `deadline`. A plain `now >= deadline` check would then turn real deadline kills into "unchanged". No check would catch that: the frozen-probe check (`run.sh:316-318`) never looks at `vst3skip.json` (see claude-4). Round up instead.

**codex-2: same defect as claude-2.**
- Probe results and `vst3skip.json` live in the same directory, `cacheDirectory()` (`:138-141`, `:797-800`).
- `rememberProbeFailure` saves the skip list (`:471`) before the probe files are removed (`:1175`, `:1183`).
- So a full disk or a permission error that blocks both probe writes also blocks the skip-list write, and nothing is saved.
- It only becomes a remembered failure if one write fails briefly and the others succeed, for example a sharing violation on `.status` from `MoveFileExW` (`platform-windows.c:743`).
- I keep claude-2 at Minor. I don't object to fixing it before merge, since the fix is local.

**codex-3: confirmed, Minor.**
- `obs-data.c:1681` truncates doubles to integers, so `"version": 1.5` passes `:345`.
- It's also broader than that: `obs_data_get_int` returns 0 for non-numbers (`:1689`), and `:256-259` only checks that a field is present. So `"size": "x"` is accepted as 0 by `:274`.
- Only a hand-edited file can contain these values. A torn file fails to parse and falls back to `.bak`. A skip also still needs an exact fingerprint match (`:238-241`, `:494`).

Approval still assumes the 3-OS build comes back green. It doesn't close codex-1 or codex-2.

FINDING claude-1 | Minor | open | valid result rejected without valid marker / with save-failed (vst3-scanner.cpp:1334-1346 before :1350); spec row 1, R1 "result supersedes all"
FINDING claude-2 | Minor | open | failed post-setup marker write (:1150-1154, watchdog :1036-1038) → setup+no result → remembered failure (:1367); ≡ codex-2 plus watchdog case
FINDING claude-3 | Minor | open | arch dir `arm64-linux`/`x86-linux` (:562-570) ≠ SDK `uname().machine+"-linux"` (module_linux.cpp:73-79,141-146); inert on aarch64, `architecture` undeclared at :571 elsewhere
FINDING claude-4 | Minor | open | no skip-list check for load-failed with fingerprint, setup+clean exit, coordinator deadline kill → timeout strike; not listed in task doc :148-150
VERDICT: approve
