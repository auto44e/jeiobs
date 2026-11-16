# Review run

- Started: 2026-11-16T09:44:59Z
- Reviewed commit: `db2b88d342491ea758919d03783916ada7a80d1d` (peers ran in a detached worktree at this commit)
- Author: lead; rounds: 2 (round 1 independent first pass, both at once, not isolated); cross-talk order: claude, codex; timeout per turn: 1500s
- Claude: claude 2.1.325.1734 (ASBX Claude Code, channel stable), model opus, effort max; tools Read, Glob, Grep; no MCP
- Codex: codex-cli 0.169.0 (engine started directly; the toolbox launcher runs only as Bedrock credential helper), effort xhigh; CODEX_HOME config without MCP/plugin/project sections, shell env "core", no project instructions (project_doc_max_bytes=0); --sandbox read-only, sandbox UNAVAILABLE on this host (bubblewrap cannot create user namespaces); Codex commands ran unsandboxed, tripwires only
- Round 1 is an independent first pass, not isolated (see the script header)
- Script: `tools/jeiobs/peer-review/review.sh`, sha256 `1eb632118318869417904a02ac69a17ac86ff72f82abeda2aacde256b1119bcf`
- Codex home: `~/.config/jeiobs-agent/codex-review`; config.toml sha256 `e3e76dcf155ea4674ca7ba34028a06eedba3e87ed3812d50da13ede1a7efbd44`; skills sha256 `253397a7192e7cf5c8088186e9a468f0696218ee6d62efe83d06c761e627bfeb`
- Context sha256 `39318050fd0d319687bd50bab560b038315102c261c07c2b26516d50dccd012e`; ledger sha256 `b8d1b410821ae4de5359e82a5585cf6b5f80bb80c84c95cdcfca771a3b65a4a6`
- stderr logs: kept locally (not committed), `14-run-20261116T094458Z`
- Topic: Review run 14 of PR #19: dispose of your findings; review the run-13 fixes.

## CI state at the reviewed commit

```text
quick checks for db2b88d342491ea758919d03783916ada7a80d1d: success
local: check_scaffold.py PASS; git diff --check clean; review.sh stub cases as listed in the context
```
- Finished: 2026-11-16T10:08:08Z

## Files written by this run

The hash given for `00-meta.md` is of its header (the part before "Finished");
the runner prints the final hash of every file, this one included, when it exits.

- `00-context.md` sha256 `39318050fd0d319687bd50bab560b038315102c261c07c2b26516d50dccd012e`
- `00-ledger-in.md` sha256 `b8d1b410821ae4de5359e82a5585cf6b5f80bb80c84c95cdcfca771a3b65a4a6`
- `00-meta.md` sha256 `94e0093eb59bd59c1c124ef75189983f73f8817639e0d508381316d810eea800`
- `01-r1-claude.prompt.md` sha256 `f3ca5a7ecd816c68924b6f4303f5a470cd554c4deaf87cad34017777bf9d8322`
- `01-r1-claude.reply.md` sha256 `8ce01c90e830da52b1690f74c0c16e0144b80389ec74d8c55dcca75ad1df1553`
- `02-r1-codex.prompt.md` sha256 `f3ca5a7ecd816c68924b6f4303f5a470cd554c4deaf87cad34017777bf9d8322`
- `02-r1-codex.reply.md` sha256 `538f0f4c566b8d08dd0dd36f52a98a4bf8f29eb3e21f6cba77c4390becefca1f`
- `03-r2-claude.prompt.md` sha256 `a58604e43f2436ca9dded45ea53e2f9c2883c168ad7b2e1351afd1fdf14578a3`
- `03-r2-claude.reply.md` sha256 `1f949428fc58aceb95f1a120556269ba16ffb002e8291f1d0f4bc7eaba1e7060`
- `04-r2-codex.prompt.md` sha256 `dafbb3de196864fdcd4622e9e9cd577eaa8ba16d0dd174de186d5daa770fc058`
- `04-r2-codex.reply.md` sha256 `1ea7a32e44b22bea71c3e6e567ecb14bb76258c90b2433a65c542f61cedede81`

## Final dispositions

```text
FINDING k1 | Major | withdrawn | finder-only closure holds (DECISIONS:188-192)
FINDING k2 | Major | withdrawn | tests pass or owner accepts (DECISIONS:100-104)
FINDING k3 | Major | withdrawn | owner-only classes and owner FLEX (DECISIONS:75-93, :253-255)
FINDING k4 | Minor | accepted-fix | review pinned (DECISIONS:105-107), merge pinned to head (:180-182)
FINDING k5 | Minor | accepted-fix | verbatim owner quotes, terms, merge marker (DECISIONS:10-43, :200-202)
FINDING k6 | Minor | accepted-fix | CONTRIBUTING:14-16; lead's own PR defined (DECISIONS:42-43)
FINDING k7 | Minor | withdrawn | withdrawn in r2 (04-r2-crosstalk.md:118)
FINDING k8 | Minor | withdrawn | memory dir still empty (re-checked); shared-dir wording tracked in claude-20
FINDING k9 | Minor | accepted-fix | 018 README:26-27 points to the SHA table
FINDING k10 | Minor | withdrawn | deferred to an owner-merged dependency PR (HANDOFF:110-111); findings.md:28 reads withdrawn
FINDING M1 | Major | accepted-fix | pins, submodules, deps/ owner-only (DECISIONS:83-87)
FINDING M2 | Major | accepted-fix | one scale, FINDING-line sources (DECISIONS:183-187)
FINDING M3 | Major | accepted-fix | runs committed and linked (DECISIONS:244-249)
FINDING N1 | Major | accepted-fix | owner instruction and rewrite recorded (DECISIONS:20-30)
FINDING N2 | Major | accepted-fix | engine (review.sh:101-102), config allowlist (:106-142), unsandboxed stated (DECISIONS:138-140); residue in claude-20, codex-21, codex-22
FINDING N3 | Major | accepted-fix | admin actions owner-only (DECISIONS:90-93)
FINDING N4 | Major | accepted-fix | owner decides first (DECISIONS:95-99)
FINDING N5 | Minor | accepted-fix | owner-hold rule (DECISIONS:75-76)
FINDING claude-1 | Major | accepted-fix | review worktree HEAD db2b88d34, one reflog entry (.git/worktrees/tree/logs/HEAD:1)
FINDING claude-2 | Major | accepted-fix | transcript-only exemption (DECISIONS:172-177)
FINDING claude-3 | Minor | accepted-fix | round-1 comment (review.sh:355-357) matches code (:358-376)
FINDING claude-4 | Minor | accepted-fix | stderr local, CLI versions, full SHA (review.sh:163-164, :254-257)
FINDING claude-5 | Minor | accepted-fix | CI runs recorded (history :41-44)
FINDING claude-6 | Minor | accepted-fix | inconclusive defined (DECISIONS:193-196)
FINDING claude-7 | Minor | accepted-fix | merge pins head (DECISIONS:180-182); exemption (:172-177); peers wording (:35-38)
FINDING claude-8 | Minor | accepted-fix | CI state passed via -C into 00-meta (review.sh:264)
FINDING claude-9 | Minor | accepted-fix | 018 README:11-12 says stderr stays local, run 06 trimmed
FINDING claude-10 | Minor | accepted-fix | raised IDs become required (review.sh:341)
FINDING claude-11 | Minor | accepted-fix | saved and pinned before tamper check and validation (review.sh:370-376, :384-387)
FINDING claude-12 | Major | accepted-fix | in-turn writes to snapshots and saved replies fail the run (:196-206, :224, :346); cleanup exits 3 (:172-186)
FINDING claude-13 | Minor | accepted-fix | engine binary (:101-102), core env (:116, :140-141), HANDOFF:106-109
FINDING claude-14 | Minor | accepted-fix | launcher as credential helper stated (review.sh:40-42, :257; DECISIONS:132-134), matching config.toml:15, :22; enforcement gap is codex-22
FINDING claude-15 | Major | accepted-fix | process groups (:308-323); tripwire wording (:47-72; DECISIONS:138-153); final hashes printed (:410-413)
FINDING claude-16 | Major | accepted-fix | --setting-sources user (:313); refusal at any depth (:189-192)
FINDING claude-17 | Major | accepted-fix | project_doc_max_bytes=0 (:317); structural match (:229-240) fits the engine's item; residual false positive in claude-22
FINDING claude-18 | Major | accepted-fix | session-log hits in a variable, grep status 2 fails (:234-236); the :153 pipe is tracked in claude-20
FINDING claude-19 | Minor | accepted-fix | .agents/.codex refused at any depth (:189-192; DECISIONS:135-137); this run's only skill root is skills/.system (df9a:3)
FINDING claude-20 | Minor | open | left: tr piped into grep -q under pipefail (:153); wording on account-writable inputs (home, ~/.claude, launcher, binaries); python3 without -I (:110) closes with codex-21
FINDING claude-21 | Minor | accepted-fix | DECISIONS:3-6, :246-249 and README:3-4 put run 06 under #18's merged head and runs 07+ under #19
FINDING claude-22 | Minor | open | :234 also matches raw command output ("aggregated_output":"…, df9a:30), not only the engine's input_text item (b820:4); a peer's grep -o can fail the run
VERDICT: request changes
FINDING c1 | Blocker | withdrawn | prior withdrawal stands (findings.md:53)
FINDING c2 | Major | withdrawn | prior withdrawal stands (findings.md:54)
FINDING c3 | Major | withdrawn | prior withdrawal stands (findings.md:55)
FINDING c4 | Major | withdrawn | prior withdrawal stands (findings.md:56)
FINDING c5 | Minor | accepted-fix | prior acceptance stands (findings.md:57)
FINDING c6 | Major | withdrawn | prior withdrawal stands (findings.md:58)
FINDING c7 | Minor | accepted-fix | prior acceptance stands (findings.md:59)
FINDING c8 | Major | withdrawn | prior withdrawal stands (findings.md:60)
FINDING M1 | Major | accepted-fix | prior acceptance stands (findings.md:61)
FINDING M2 | Major | accepted-fix | prior acceptance stands (findings.md:62)
FINDING M3 | Major | accepted-fix | prior acceptance stands (findings.md:63)
FINDING N1 | Major | accepted-fix | prior acceptance stands (findings.md:64)
FINDING N2 | Major | accepted-fix | prior acceptance stands (findings.md:65)
FINDING N3 | Major | accepted-fix | prior acceptance stands (findings.md:66)
FINDING N4 | Major | accepted-fix | prior acceptance stands (findings.md:67)
FINDING N5 | Minor | accepted-fix | prior acceptance stands (findings.md:68)
FINDING N6 | Major | accepted-fix | prior acceptance stands (findings.md:69)
FINDING N7 | Major | accepted-fix | prior acceptance stands (findings.md:70)
FINDING codex-1 | Major | accepted-fix | prior acceptance stands (findings.md:71)
FINDING codex-2 | Major | accepted-fix | prior acceptance stands (findings.md:72)
FINDING codex-3 | Major | accepted-fix | prior acceptance stands (findings.md:73)
FINDING codex-4 | Minor | accepted-fix | prior acceptance stands (findings.md:74)
FINDING codex-5 | Major | accepted-fix | prior acceptance stands (findings.md:75)
FINDING codex-6 | Major | accepted-fix | prior acceptance stands (findings.md:76)
FINDING codex-7 | Major | accepted-fix | prior acceptance stands (findings.md:77)
FINDING codex-8 | Minor | accepted-fix | prior acceptance stands (findings.md:78)
FINDING codex-9 | Major | accepted-fix | prior acceptance stands (findings.md:79)
FINDING codex-10 | Major | accepted-fix | prior acceptance stands (findings.md:80)
FINDING codex-11 | Major | accepted-fix | prior acceptance stands (findings.md:81)
FINDING codex-12 | Minor | accepted-fix | prior acceptance stands (findings.md:82)
FINDING codex-13 | Major | accepted-fix | prior acceptance stands (findings.md:83)
FINDING codex-14 | Major | accepted-fix | prior acceptance stands (findings.md:84)
FINDING codex-15 | Major | accepted-fix | prior acceptance stands (findings.md:86)
FINDING codex-16 | Minor | accepted-fix | prior acceptance stands (findings.md:87)
FINDING codex-17 | Major | accepted-fix | prior acceptance stands (findings.md:88)
FINDING codex-18 | Major | accepted-fix | prior acceptance stands (findings.md:91)
FINDING codex-19 | Major | accepted-fix | nonempty OUT and final metadata checks are enforced (review.sh:160,407-409)
FINDING codex-20 | Major | accepted-fix | TOML parsing and exact validation replace regex checks (review.sh:110-141)
FINDING codex-21 | Major | open | non-isolated Python permits reviewed-tree tomllib shadowing (review.sh:110-112)
FINDING codex-22 | Major | open | credential-export and OTEL nested values remain arbitrary (review.sh:117-119)
VERDICT: request changes
```
