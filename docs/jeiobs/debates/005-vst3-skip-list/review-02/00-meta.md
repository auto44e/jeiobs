# Review run

- Started: 2026-11-16T09:16:26Z
- Reviewed commit: `2ab44397be87d3ecbeb20e85e242fff72f77e035` (peers ran in a detached worktree at this commit)
- Author: codex; rounds: 2 (round 1 independent first pass, both at once, not isolated); cross-talk order: claude, codex; timeout per turn: 1800s
- Claude: claude 2.1.325.1734 (ASBX Claude Code, channel stable), model opus, effort max; tools Read, Glob, Grep; no MCP
- Codex: codex-cli 0.169.0 (engine started directly; the toolbox launcher runs only as Bedrock credential helper), effort xhigh; CODEX_HOME config without MCP/plugin/project sections, shell env "core", no project instructions (project_doc_max_bytes=0); --sandbox read-only, sandbox UNAVAILABLE on this host (bubblewrap cannot create user namespaces); Codex commands ran unsandboxed, tripwires only
- Round 1 is an independent first pass, not isolated (see the script header)
- Script: `tools/jeiobs/peer-review/review.sh`, sha256 `598e463168267d25240404cc43ef709c60b50d2fb158c14e14b72da163741068`
- Context sha256 `c8f9e44b5ec348a210e2245d2abd420c5b504ad01925c8163c3ac2e58526e7eb`; ledger sha256 `7f9097817f8750ad745dfc7711c0e99ddc9138c2b8b2389855c0f3097ea0c88b`
- stderr logs: kept locally (not committed), `02-run-20261116T091625Z`
- Topic: Review run 02 of PR #20 (task 005, D6): dispose of your findings; review the fixes.

## CI state at the reviewed commit

```text
run 43838047847 jeiobs quick checks completed success (head 2ab44397b)
run 43838058617 jeiobs full build in_progress at review start (head 2ab44397b)
run 43829533572 jeiobs full build success at 6fb0cd502 (previous head)
local: tools/jeiobs/vst3-check/run.sh 40 PASS 0 FAIL at 2ab44397b; check_scaffold.py PASS
```
- Finished: 2026-11-16T09:35:26Z

## Files written by this run

The hash given for `00-meta.md` is of its header (the part before "Finished");
the runner prints the final hash of every file, this one included, when it exits.

- `00-context.md` sha256 `c8f9e44b5ec348a210e2245d2abd420c5b504ad01925c8163c3ac2e58526e7eb`
- `00-ledger-in.md` sha256 `7f9097817f8750ad745dfc7711c0e99ddc9138c2b8b2389855c0f3097ea0c88b`
- `00-meta.md` sha256 `fc46a51cf94f384c92aa25bcd5a4496640bba1340845d6a12d86653b24f78047`
- `01-r1-claude.prompt.md` sha256 `9e8442b4dd95952fee0a6b6fd991149e99851eb093282e89518b19d33ad11c8d`
- `01-r1-claude.reply.md` sha256 `cc462f14d96097a9ca34a3b56c1fa64178c5682accdea07ee3fbfc09f30a6307`
- `02-r1-codex.prompt.md` sha256 `9e8442b4dd95952fee0a6b6fd991149e99851eb093282e89518b19d33ad11c8d`
- `02-r1-codex.reply.md` sha256 `1ef7e8208c3f9027d742b30dfbf6a4cc0eae4874d190eec2efb36129d02a7ff7`
- `03-r2-claude.prompt.md` sha256 `3918ce495e2af20404a99370c7213a4ebc506fcf9e6dc433a7ddcdb063c9ed3b`
- `03-r2-claude.reply.md` sha256 `8b9cc9fb07597337e87faf36b1268db0fff3f2ddf83a00e44741cda6bd000e9c`
- `04-r2-codex.prompt.md` sha256 `5cafe11ba714739b7fc1053d0c513b9db58dda7ddb62d3403682541bfe145cdb`
- `04-r2-codex.reply.md` sha256 `f7a2adf6e4964656badcba541a3da074ca0712c81b9ebb5b4c1db4cf97d0e9e0`

## Final dispositions

```text
FINDING claude-1 | Minor | accepted-fix | valid matching-nonce result accepted before waitError/markers (vst3-scanner.cpp:1382-1387); check #17
FINDING claude-2 | Minor | accepted-fix | exit 90 → unchanged (:1157-1161, :1421-1423), exit 91 → strike (:1069-1078, :1424-1426); check #15
FINDING claude-3 | Minor | accepted-fix | Linux binary is `uname().machine + "-linux"` + `<stem>.so` (:587-595), as in the SDK's module_linux.cpp
FINDING claude-4 | Minor | accepted-fix | checks #12–#14 added (skip-list.sh:330-349); remaining gaps listed (task doc :188-195)
FINDING claude-5 | Minor | open | after-setup kills happen at :1323-1325 (pipe reads end at prepareProbeOutput, :1141/:1303); deadlinePassed (:1324) uses steady_clock, but the wait uses the tick clock (pipe-windows.c:209) or CLOCK_MONOTONIC (pipe-posix.c:157,183), so on Windows/macOS a kill can be logged "probe wait failed" and leave the entry unchanged (:1389-1391) instead of a strike (spec :68)
VERDICT: approve
FINDING codex-1 | Major | accepted-fix | Wait errors remain unchanged; deadline strikes require passed deadline and successful kill (vst3-scanner.cpp:1322-1329,1389-1391).
FINDING codex-2 | Major | accepted-fix | Exit 90 remains unchanged and exit 91 records timeout (vst3-scanner.cpp:1157-1160,1420-1425).
FINDING codex-3 | Minor | accepted-fix | Skip-list numeric fields and version require JSON integers (vst3-scanner.cpp:248-265,367-376).
FINDING codex-4 | Minor | accepted-fix | Setup publication and watchdog observation remain mutex-atomic (vst3-scanner.cpp:1067-1078,1145-1149).
FINDING codex-5 | Minor | open | Documentation permits non-empty failure kinds, but the loader accepts only `failure` or `timeout` (005-vst3-skip-list.md:164-166; vst3-scanner.cpp:280-281).
VERDICT: approve
```
