# Review run

- Started: 2026-11-17T13:16:18Z
- Reviewed commit: `2c3cb779026a001ef5a8356f25d26b742eba1149` (peers ran in a detached worktree at this commit)
- Author: codex; rounds: 2 (round 1 independent first pass, both at once, not isolated); cross-talk order: claude, codex; timeout per turn: 1800s
- Claude: claude 2.1.326.1734 (ASBX Claude Code, channel stable), model opus, effort max; tools Read, Glob, Grep; no MCP
- Codex: codex-cli 0.169.0 (engine started directly; the toolbox launcher runs only as Bedrock credential helper), effort xhigh; CODEX_HOME config without MCP/plugin/project sections, shell env "core", no project instructions (project_doc_max_bytes=0); --sandbox read-only, sandbox available
- Round 1 is an independent first pass, not isolated (see the script header)
- Script: `tools/jeiobs/peer-review/review.sh`, sha256 `63e7012b6f02f3151f72ee2ad2c1a6d849725472001d706e960af791c4eb99a4`
- Codex home: `~/.config/jeiobs-agent/codex-review`; config.toml sha256 `e3e76dcf155ea4674ca7ba34028a06eedba3e87ed3812d50da13ede1a7efbd44`; skills sha256 `253397a7192e7cf5c8088186e9a468f0696218ee6d62efe83d06c761e627bfeb`
- Context sha256 `67779bece54fd9e9f507cc248ac8300eadbbe3b0ab5e9953f138cf975a6998db`; ledger sha256 `7c638a22163755ea88ec87c5b453cd07aa250dbb486a584d927a4eb6d91f1cfa`
- stderr logs: kept locally (not committed), `03-run-20261117T131617Z`
- Topic: Review run 03 of pr32.

## CI state at the reviewed commit

```text
CI queued at review start for 2c3cb779026a001ef5a8356f25d26b742eba1149
local: jw-unit, jw-fuzz-smoke, jw-cases (1134 lines) PASS
```
- Finished: 2026-11-17T13:29:19Z

## Files written by this run

The hash given for `00-meta.md` is of its header (the part before "Finished");
the runner prints the final hash of every file, this one included, when it exits.

- `00-context.md` sha256 `67779bece54fd9e9f507cc248ac8300eadbbe3b0ab5e9953f138cf975a6998db`
- `00-ledger-in.md` sha256 `7c638a22163755ea88ec87c5b453cd07aa250dbb486a584d927a4eb6d91f1cfa`
- `00-meta.md` sha256 `6ae3f6c5a3c27e9bba5be8775c6022cdd0b51119cb7263ef8fc80b477f75538a`
- `01-r1-claude.prompt.md` sha256 `96dfc19ecefabff1640138b360c5782080cb8eafac5f427ff4607ee1ded5474c`
- `01-r1-claude.reply.md` sha256 `1086741a2a3afd8b51553111b7fc2be815952f81ced367dea27933ddd95814a0`
- `02-r1-codex.prompt.md` sha256 `96dfc19ecefabff1640138b360c5782080cb8eafac5f427ff4607ee1ded5474c`
- `02-r1-codex.reply.md` sha256 `f6134e2a27fc5892539f5f9cfc0875ffe6e28c91717177f81a4516ae47b836ce`
- `03-r2-claude.prompt.md` sha256 `716030d23d6eeaa0cb2b02bdcca6bccd0d49cc743263d04e9d0892508375dc2b`
- `03-r2-claude.reply.md` sha256 `c208fef293c13e6383a7267ee73e6e25c00f60b94450110793f53d701cde82d8`
- `04-r2-codex.prompt.md` sha256 `3eda44164139b701efb755b911e7f92e8d1c24884e213acaca08c1c20805c9e3`
- `04-r2-codex.reply.md` sha256 `b3fc7325d84fc97f3a2101602c2d239f264ddf15676b2ae4c102a380c4889cc8`

## Final dispositions

```text
FINDING claude-1 | Major | accepted-fix | two-pass identity; unchanged since acceptance
FINDING claude-2 | Major | accepted-fix | themes skip only under the config root
FINDING claude-3 | Major | accepted-fix | symlinked roots and owned subpaths give a symlink entry plus root-symlink
FINDING claude-4 | Major | accepted-fix | listing errors make the folder unreadable, with RestoreState
FINDING claude-5 | Major | accepted-fix | count validated, exported and carried over
FINDING claude-6 | Minor | accepted-fix | ReviewRules linear; maps replace scans; scan bounded by the metadata cap (:222-234)
FINDING claude-7 | Minor | accepted-fix | sample uncapped (CapturePlanner.cpp:496-504); jw_unit.cpp:828-844
FINDING claude-8 | Minor | accepted-fix | folder without basic.ini is one unknown dir entry
FINDING claude-9 | Minor | accepted-fix | WORKSPACE_FORMAT.md:94-103, :617, :698-700; cases.txt:1340-1341
FINDING claude-10 | Minor | accepted-fix | cases.txt accepts only bak-alone.json
FINDING claude-11 | Minor | accepted-fix | CanStore runs before any read
FINDING claude-12 | Major | accepted-fix | NOMINMAX/WIN32_LEAN_AND_MEAN before windows.h (Fs.cpp:22-30); merge still needs green Windows/macOS CI on 2c3cb7790
FINDING claude-13 | Minor | open | symlinked basic.ini still makes the profile an unknown dir with the link unreported (CapturePlanner.cpp:968-979)
FINDING claude-14 | Minor | open | every lstat error is still "missing" (Fs.cpp:263-264) and dropped silently for app dirs and basic/ (CapturePlanner.cpp:346,588)
FINDING claude-15 | Minor | accepted-fix | owned roots ignore unrepresentable names (CapturePlanner.cpp:580-582)
FINDING claude-16 | Minor | accepted-fix | indexed merges (:246-270); budget after the plan-order sort (:1360-1380)
FINDING claude-17 | Major | open | unreadable unrepresentable folder is recorded under its own name (CapturePlanner.cpp:318; pinned by jw_unit.cpp:789-812 "basic/scenes/bad?"): breaks rule 4 and PathRules (Manifest.cpp:785-790); raw non-UTF-8 bytes on POSIX (Fs.cpp:288) make the plan ERR E_JSON (jw-check.cpp:529,586)
VERDICT: request changes
FINDING codex-1 | Major | withdrawn | Lone `.bak` correctly has no UID (`31-pr3-contract.md:103-105`).
FINDING codex-2 | Major | accepted-fix | Listing errors now emit unreadable entries and warnings (`CapturePlanner.cpp:315-321`).
FINDING codex-3 | Major | accepted-fix | Scene-UUID extraction is indexed and capped (`CapturePlanner.cpp:750-809`).
FINDING codex-4 | Minor | withdrawn | Invalid-name symlink ambiguity withdrawn under the contract.
FINDING codex-5 | Major | withdrawn | Separate metadata scan bound is equivalent to the manifest-entry cap (`CapturePlanner.cpp:212-234,1355-1357`).
VERDICT: request changes
```
