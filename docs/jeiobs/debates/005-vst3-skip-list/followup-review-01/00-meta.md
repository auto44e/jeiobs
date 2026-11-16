# Review run

- Started: 2026-11-16T09:48:45Z
- Reviewed commit: `7adc69a8b8b88d2783b5841ea326e17b69927fe1` (peers ran in a detached worktree at this commit)
- Author: lead; rounds: 2 (round 1 independent first pass, both at once, not isolated); cross-talk order: claude, codex; timeout per turn: 1500s
- Claude: claude 2.1.325.1734 (ASBX Claude Code, channel stable), model opus, effort max; tools Read, Glob, Grep; no MCP
- Codex: codex-cli 0.169.0 (engine started directly; the toolbox launcher runs only as Bedrock credential helper), effort xhigh; CODEX_HOME config without MCP/plugin/project sections, shell env "core", no project instructions (project_doc_max_bytes=0); --sandbox read-only, sandbox UNAVAILABLE on this host (bubblewrap cannot create user namespaces); Codex commands ran unsandboxed, tripwires only
- Round 1 is an independent first pass, not isolated (see the script header)
- Script: `tools/jeiobs/peer-review/review.sh`, sha256 `1eb632118318869417904a02ac69a17ac86ff72f82abeda2aacde256b1119bcf`
- Codex home: `~/.config/jeiobs-agent/codex-review-b`; config.toml sha256 `e3e76dcf155ea4674ca7ba34028a06eedba3e87ed3812d50da13ede1a7efbd44`; skills sha256 `253397a7192e7cf5c8088186e9a468f0696218ee6d62efe83d06c761e627bfeb`
- Context sha256 `e0fcd983556753a570c9c3d285f145519721790ca44c8569df6d5e8583b855c9`; ledger sha256 `e93938341997ada8e1dd375582f095c334793e1c353d3202c48452e6849ce0c8`
- stderr logs: kept locally (not committed), `01-run-20261116T094844Z`
- Topic: Review run 01 of PR #22 (task 005 follow-ups).

## CI state at the reviewed commit

```text
quick checks for 7adc69a8b: see list at review start
local: tools/jeiobs/vst3-check/run.sh 40 PASS 0 FAIL at 7adc69a8b
```
- Finished: 2026-11-16T10:02:17Z

## Files written by this run

The hash given for `00-meta.md` is of its header (the part before "Finished");
the runner prints the final hash of every file, this one included, when it exits.

- `00-context.md` sha256 `e0fcd983556753a570c9c3d285f145519721790ca44c8569df6d5e8583b855c9`
- `00-ledger-in.md` sha256 `e93938341997ada8e1dd375582f095c334793e1c353d3202c48452e6849ce0c8`
- `00-meta.md` sha256 `55b45808379fccf64affe32ed6bf71becb3a75e0f0be40feb57fe3880163f69d`
- `01-r1-claude.prompt.md` sha256 `8fa8afe4bba62b0b41d984b5df5cfdd330ac57d2c08ca2686bb5ef20b8f8258c`
- `01-r1-claude.reply.md` sha256 `f5b6388cf919faa8fbc382f84ecebf9026ff28dd26661d7823f440aada42a433`
- `02-r1-codex.prompt.md` sha256 `8fa8afe4bba62b0b41d984b5df5cfdd330ac57d2c08ca2686bb5ef20b8f8258c`
- `02-r1-codex.reply.md` sha256 `0b4df5e6a7c32e8e51de53dd69d4b49b5b083cd9bc76e47059fefb1a62f2595b`
- `03-r2-claude.prompt.md` sha256 `8f21a70ac38e80903e7bd49fec815832b40690b6c5c14c0e0e159328b42c09aa`
- `03-r2-claude.reply.md` sha256 `fc6ffe2a79bab7a14ca78a609acc7c4df6a8b6a323cbdd40fc64c1a0c53a1aa6`
- `04-r2-codex.prompt.md` sha256 `42308c02a1cdcd7b37a55c32656a4319aeff8550a37a54e9da9442ccbe6c4e92`
- `04-r2-codex.reply.md` sha256 `028c0f623f6be704b8f7f0366372139b6da0ad39fa86f80de9a0e1965ac1ba67`

## Final dispositions

```text
FINDING claude-1 | Minor | accepted-fix | a valid result with the matching nonce is still accepted before waitError and the markers (vst3-scanner.cpp:1386-1394); check #17
FINDING claude-2 | Minor | accepted-fix | exit 90 → unchanged (:1157-1160, :1425-1426), exit 91 → strike (:1077-1078, :1428-1429); check #15
FINDING claude-3 | Minor | accepted-fix | Linux binary uses uname().machine + "-linux" (:593), as in the SDK's module_linux.cpp
FINDING claude-4 | Minor | accepted-fix | checks #12–#14 are in skip-list.sh (#14 at :342-348); remaining gaps are listed in task doc :189-196
FINDING claude-5 | Minor | accepted-fix | 100 ms slack (:1327-1330) covers Windows' early timeout of under one tick and macOS slews ≤~670 ppm over the 150 s maximum wait; larger slews end less sticky (entry unchanged)
FINDING claude-6 | Minor | open | task doc :68 "(its own monotonic deadline passed)" contradicts :1328 (a kill up to 100 ms before the deadline counts); the slack is not in the docs or in the no-check list :189-196
VERDICT: approve
FINDING codex-1 | Major | accepted-fix | Wait errors remain unchanged unless deadline slack and cleanup succeed (vst3-scanner.cpp:1322-1333).
FINDING codex-2 | Major | accepted-fix | Exit 90 and save failures remain unrecorded (vst3-scanner.cpp:1157-1160,1407-1409).
FINDING codex-3 | Minor | accepted-fix | Skip-list numeric fields require JSON integers (vst3-scanner.cpp:248-265).
FINDING codex-4 | Minor | accepted-fix | Setup publication and watchdog observation are mutex-atomic (vst3-scanner.cpp:1067-1078,1145-1149).
FINDING codex-5 | Minor | accepted-fix | Documentation now states the loader accepts only failure and timeout kinds (005-vst3-skip-list.md:164-166).
FINDING codex-6 | Minor | open | The 100 ms time-only slack can still classify a genuine wait error as a timeout strike (vst3-scanner.cpp:1323-1332; pipe-posix.c:157-189; pipe-windows.c:207-209).
VERDICT: approve
```
