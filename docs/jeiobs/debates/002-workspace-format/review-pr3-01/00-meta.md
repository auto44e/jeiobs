# Review run

- Started: 2026-11-17T09:51:50Z
- Reviewed commit: `21489bd03f5543a46575229e511269896fecd918` (peers ran in a detached worktree at this commit)
- Author: codex; rounds: 2 (round 1 independent first pass, both at once, not isolated); cross-talk order: claude, codex; timeout per turn: 1800s
- Claude: claude 2.1.326.1734 (ASBX Claude Code, channel stable), model opus, effort max; tools Read, Glob, Grep; no MCP
- Codex: codex-cli 0.169.0 (engine started directly; the toolbox launcher runs only as Bedrock credential helper), effort xhigh; CODEX_HOME config without MCP/plugin/project sections, shell env "core", no project instructions (project_doc_max_bytes=0); --sandbox read-only, sandbox available
- Round 1 is an independent first pass, not isolated (see the script header)
- Script: `tools/jeiobs/peer-review/review.sh`, sha256 `63e7012b6f02f3151f72ee2ad2c1a6d849725472001d706e960af791c4eb99a4`
- Codex home: `~/.config/jeiobs-agent/codex-review`; config.toml sha256 `e3e76dcf155ea4674ca7ba34028a06eedba3e87ed3812d50da13ede1a7efbd44`; skills sha256 `253397a7192e7cf5c8088186e9a468f0696218ee6d62efe83d06c761e627bfeb`
- Context sha256 `2ce9cf33880cc0da7445f6d69875290e6c3b258c4539b5a1b686c5bc20488368`; ledger sha256 `0e7944720746fcce027449df5aed0d9aae079aed74a565ae4b5f397cc6e9223a`
- stderr logs: kept locally (not committed), `01-run-20261117T095149Z`
- Topic: Review run 01 of pr32.

## CI state at the reviewed commit

```text
quick checks pending at start
local: jw-unit, jw-fuzz-smoke, jw-cases (1095 lines) PASS on Linux, also under ASan+UBSan
```
- Finished: 2026-11-17T10:19:10Z

## Files written by this run

The hash given for `00-meta.md` is of its header (the part before "Finished");
the runner prints the final hash of every file, this one included, when it exits.

- `00-context.md` sha256 `2ce9cf33880cc0da7445f6d69875290e6c3b258c4539b5a1b686c5bc20488368`
- `00-ledger-in.md` sha256 `0e7944720746fcce027449df5aed0d9aae079aed74a565ae4b5f397cc6e9223a`
- `00-meta.md` sha256 `5f7d449ed5cd16ae252c086539fa5a692dc2bc1d9e560b161f8f04c711586494`
- `01-r1-claude.prompt.md` sha256 `532f5370b815f21738386179738a3859a852366bb602bd3ec5f79bdeeedc72a0`
- `01-r1-claude.reply.md` sha256 `ab5e4a725c1b1984896f96deae803952f1670bd2f2d8e5b5b77fcf6279509af7`
- `02-r1-codex.prompt.md` sha256 `532f5370b815f21738386179738a3859a852366bb602bd3ec5f79bdeeedc72a0`
- `02-r1-codex.reply.md` sha256 `1d28fd0c3aafcb8d0a9592dec115ad415de8aff88a357f753beaa183b4b8bf8c`
- `03-r2-claude.prompt.md` sha256 `dd5f4ea638c168c05cb77861ed537a7816e459d151439c55ceb98bdaf4feea04`
- `03-r2-claude.reply.md` sha256 `1ec277ffb2d29ffc7e92181b15e4141d69ec4281e2732e42ffc8e2bb476c181b`
- `04-r2-codex.prompt.md` sha256 `da9d45f672d7541b0213ca677e18b736f889865d228e75888e8f5e15c9fbd58b`
- `04-r2-codex.reply.md` sha256 `46d65aceab5e7886caf9b4f55a2caf64b935c1498b5854d80fe1ca79cf70dac3`

## Final dispositions

```text
FINDING claude-1 | Major | open | rule-2 claim by an earlier-sorted collection steals a later collection's rule-1 path UID; binding depends on name order (CapturePlanner.cpp:898-919)
FINDING claude-2 | Major | open | `themes` skip in ProcessDirectory (CapturePlanner.cpp:383-385) silently drops a profile folder named themes
FINDING claude-3 | Major | open | symlinked or junctioned app dir or owned subdir gives an empty or partial plan with no entry or warning (Fs.cpp:47-59,233-237; CapturePlanner.cpp:408-413)
FINDING claude-4 | Major | open | listing errors are ignored: EACCES, mid-listing errors and Windows unpaired-surrogate names silently truncate the plan (CapturePlanner.cpp:401,413; Fs.cpp:240,249-256,267)
FINDING claude-5 | Major | open | planned `count` field unknown to the v1 schema and spec; export and CarryOver treat it as a dropped extension (Manifest.cpp:1121-1129,1253)
FINDING claude-6 | Minor | open | quadratic ReviewRules scans with a non-discriminating unit test; collections_/profiles_ outside the entry limit, O(n²) find_if (:514), per-pair strings (:805); scene-UUID part moved to codex-3
FINDING claude-7 | Minor | open | collapse accounting: duplicate container paths (false E_COLLISION), 101-sample truncation, recursive counting
FINDING claude-8 | Minor | open | directories without basic.ini become profiles with UIDs, unlike OBS
FINDING claude-9 | Minor | open | WORKSPACE_FORMAT.md §4/§7/§13/§15 stale for PR3 fields and slice; r1.2 wording wrongly covers profile `.bak` UIDs
FINDING claude-10 | Minor | open | cases.txt:1332-1334 still accepts bak-alone-alt2/alt3, which r1.2 forbids, so a lone-.bak UID regression passes jw-cases
FINDING claude-11 | Minor | open | planner plans stored files over the 64 MiB blob / 256 MiB revision limits (Manifest.cpp:830-835); oversize collection gets a misleading collection-unparseable warning (CapturePlanner.cpp:543-547)
VERDICT: request changes
FINDING codex-1 | Major | withdrawn | Lone `.bak` branch emits no UID (`CapturePlanner.cpp:986-993,1116-1118`).
FINDING codex-2 | Major | open | RealFs listing errors are discarded, allowing incomplete successful plans (`CapturePlanner.cpp:370-414`; `Fs.cpp:239-263`).
FINDING codex-3 | Major | open | Scene UUID extraction is quadratic and uncapped beyond the 100,000-item schema limit (`CapturePlanner.cpp:550-592`; `Manifest.cpp:856-867`).
FINDING codex-4 | Minor | withdrawn | Invalid-name symlinks require aggregate representation; this is a contract ambiguity, not a code defect (`CapturePlanner.cpp:375-381`).
VERDICT: request changes
```
