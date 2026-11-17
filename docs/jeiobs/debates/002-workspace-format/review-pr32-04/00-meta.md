# Review run

- Started: 2026-11-17T13:58:30Z
- Reviewed commit: `67cc0488c1d5a6439e11be780ae13917ad0bc929` (peers ran in a detached worktree at this commit)
- Author: codex; rounds: 2 (round 1 independent first pass, both at once, not isolated); cross-talk order: claude, codex; timeout per turn: 1800s
- Claude: claude 2.1.326.1734 (ASBX Claude Code, channel stable), model opus, effort max; tools Read, Glob, Grep; no MCP
- Codex: codex-cli 0.169.0 (engine started directly; the toolbox launcher runs only as Bedrock credential helper), effort xhigh; CODEX_HOME config without MCP/plugin/project sections, shell env "core", no project instructions (project_doc_max_bytes=0); --sandbox read-only, sandbox available
- Round 1 is an independent first pass, not isolated (see the script header)
- Script: `tools/jeiobs/peer-review/review.sh`, sha256 `63e7012b6f02f3151f72ee2ad2c1a6d849725472001d706e960af791c4eb99a4`
- Codex home: `~/.config/jeiobs-agent/codex-review`; config.toml sha256 `e3e76dcf155ea4674ca7ba34028a06eedba3e87ed3812d50da13ede1a7efbd44`; skills sha256 `253397a7192e7cf5c8088186e9a468f0696218ee6d62efe83d06c761e627bfeb`
- Context sha256 `ce87f570a01125e08f1e8775fca93f991cd54b65307e534ff285f69c693ba232`; ledger sha256 `2a2e8db8d355615ddda0cdafc7b1988ce5dcc7dbaa9bb8c6066e9bad4fb89294`
- stderr logs: kept locally (not committed), `04-run-20261117T135829Z`
- Topic: Review run 04 of pr32.

## CI state at the reviewed commit

```text
CI queued at review start for 67cc0488c1d5a6439e11be780ae13917ad0bc929
local: jw-unit, jw-fuzz-smoke, jw-cases (1134 lines) PASS, also under ASan+UBSan
```
- Finished: 2026-11-17T14:06:02Z

## Files written by this run

The hash given for `00-meta.md` is of its header (the part before "Finished");
the runner prints the final hash of every file, this one included, when it exits.

- `00-context.md` sha256 `ce87f570a01125e08f1e8775fca93f991cd54b65307e534ff285f69c693ba232`
- `00-ledger-in.md` sha256 `2a2e8db8d355615ddda0cdafc7b1988ce5dcc7dbaa9bb8c6066e9bad4fb89294`
- `00-meta.md` sha256 `04e3dc9f99de9b052da0fdcccadd1dd6657c87290db431c78f87df45c89e3414`
- `01-r1-claude.prompt.md` sha256 `3be1a999db5d6ead1cf2163322baa499edc66b2106c7ce503aa4b52409faed77`
- `01-r1-claude.reply.md` sha256 `cdb90d96817d5da02684d85ea3494971594e3b12f89941278cde73cb2b8872c4`
- `02-r1-codex.prompt.md` sha256 `3be1a999db5d6ead1cf2163322baa499edc66b2106c7ce503aa4b52409faed77`
- `02-r1-codex.reply.md` sha256 `981e555ccd98943eea714c5a635fe8a0b45138e3accbb5cb15f2766cb4315d23`
- `03-r2-claude.prompt.md` sha256 `5c70ea2eee638b3030efc828822d1f2409acfa958a798d03057b94e3c6d74fd0`
- `03-r2-claude.reply.md` sha256 `41d00e9f44b6e430a00d85795abd245a80ecac1d7f56f944fc17bc8622b203f3`
- `04-r2-codex.prompt.md` sha256 `1931037996ebb93db3f673d6a4b364b72ab603ce34db5e453c983bf8ed61b421`
- `04-r2-codex.reply.md` sha256 `080d95ff39f3ec1630a590d3a0796d4580bad1a6875769df7840b8efbb60ba00`

## Final dispositions

```text
FINDING claude-1 | Major | accepted-fix | two-pass identity; unchanged since acceptance
FINDING claude-2 | Major | accepted-fix | themes skip only under the config root
FINDING claude-3 | Major | accepted-fix | symlinked roots and owned subpaths give a symlink entry plus root-symlink
FINDING claude-4 | Major | accepted-fix | listing errors make the folder unreadable, with RestoreState (CapturePlanner.cpp:564-566)
FINDING claude-5 | Major | accepted-fix | count validated, exported and carried over
FINDING claude-6 | Minor | accepted-fix | ReviewRules linear; maps replace scans; scan bounded by the metadata cap (:222-234)
FINDING claude-7 | Minor | accepted-fix | sample uncapped (CapturePlanner.cpp:496-504); jw_unit.cpp:826-842
FINDING claude-8 | Minor | accepted-fix | folder without basic.ini is one unknown dir entry
FINDING claude-9 | Minor | accepted-fix | WORKSPACE_FORMAT.md:94-103, :617, :698-700; cases.txt:1340-1341
FINDING claude-10 | Minor | accepted-fix | cases.txt accepts only bak-alone.json
FINDING claude-11 | Minor | accepted-fix | CanStore runs before any read
FINDING claude-12 | Major | accepted-fix | NOMINMAX/WIN32_LEAN_AND_MEAN before windows.h (Fs.cpp:22-30); merge needs green Windows/macOS CI at 67cc0488c
FINDING claude-13 | Minor | open | carried to S1-PR4, not fixed: symlinked basic.ini still makes the profile an unknown dir with the link unreported (CapturePlanner.cpp:969-979)
FINDING claude-14 | Minor | open | carried to S1-PR4, not fixed: every lstat error is still "missing" (Fs.cpp:263-264), dropped silently for app dirs and basic/ (CapturePlanner.cpp:347,589)
FINDING claude-15 | Minor | accepted-fix | owned roots ignore unrepresentable names (CapturePlanner.cpp:581-582)
FINDING claude-16 | Minor | accepted-fix | indexed merges (:246-270); budget after the plan-order sort (:1361-1381)
FINDING claude-17 | Major | accepted-fix | only a listing-error at the always-valid container, names entry kept, no own-name entry or warning (CapturePlanner.cpp:315-323); pinned by jw_unit.cpp:796-811
VERDICT: approve
FINDING codex-1 | Major | withdrawn | Lone `.bak` correctly has no UID (31-pr3-contract.md:103-105).
FINDING codex-2 | Major | accepted-fix | Listing failures emit unreadable entries and warnings (CapturePlanner.cpp:344-356).
FINDING codex-3 | Major | accepted-fix | Scene-UUID extraction is indexed and capped (CapturePlanner.cpp:750-809).
FINDING codex-4 | Minor | withdrawn | Invalid-name symlink ambiguity is resolved by the contract (31-pr3-contract.md:127-130).
FINDING codex-5 | Major | withdrawn | Metadata scanning is separately bounded while entries remain capped (CapturePlanner.cpp:222-234,1355-1358).
FINDING codex-6 | Minor | open | Contract requires “unreadable handling,” but the fix emits only a container warning and names entry; this exception is undocumented (31-pr3-contract.md:156-159; CapturePlanner.cpp:315-335).
VERDICT: approve
```
