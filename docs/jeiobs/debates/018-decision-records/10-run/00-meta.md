# Review run

- Started: 2026-11-16T07:20:42Z
- Reviewed commit: `02d4c28eecab2baf7ee0c2d9b0c7530b0d673b6b` (peers ran in a detached worktree at this commit)
- Author: lead; rounds: 2 (round 1 independent first pass, both at once, not isolated); cross-talk order: claude, codex; timeout per turn: 1500s
- Claude: claude 2.1.325.1734 (ASBX Claude Code, channel stable), model opus, effort max; tools Read, Glob, Grep; no MCP
- Codex: codex-cli 0.169.0 (engine, not the toolbox launcher), effort xhigh; CODEX_HOME config without MCP/plugin/project sections, shell env "core"; --sandbox read-only, sandbox UNAVAILABLE on this host (bubblewrap cannot create user namespaces); tamper check enforced
- Round 1 is an independent first pass, not isolated (see the script header)
- Script: `tools/jeiobs/peer-review/review.sh`, sha256 `ec14db7c275c75d4c63b020beb9827aa582d17935dcced0a5e15d14ab72c1903`
- Context sha256 `7c527df4de997a23e43dfe85d553249a79092bbf61dfa161c044aa800f4bfd77`; ledger sha256 `b6e6b1c07683bed688badc03a2bc4bf1c410277b9f5fc0b7b26cdbc2c8015137`
- stderr logs: kept locally (not committed), `10-run-20261116T072042Z`
- Topic: Review run 10 of PR #18: dispose of your findings; review the run-09 fixes.

## CI state at the reviewed commit

```text
run 43823008994 jeiobs quick checks event=push completed success
head 02d4c28eecab2baf7ee0c2d9b0c7530b0d673b6b
local: check_scaffold.py PASS; git diff --check clean; review.sh stub cases pass/fail as expected incl. setting-sources, reply edit, .claude refusal
```
- Finished: 2026-11-16T07:48:40Z

## Files written by this run

- `00-context.md` sha256 `7c527df4de997a23e43dfe85d553249a79092bbf61dfa161c044aa800f4bfd77`
- `00-ledger-in.md` sha256 `b6e6b1c07683bed688badc03a2bc4bf1c410277b9f5fc0b7b26cdbc2c8015137`
- `00-meta.md` sha256 `91538c1453f1b709256ad1b09f3e96113dfea9dfa0c28c46ab696f7cfd2b59e4`
- `01-r1-claude.prompt.md` sha256 `810a69c120f9c87b362d26f19d698a0848055839bcc8184405f184b82a0ca284`
- `01-r1-claude.reply.md` sha256 `120370ebbc0033ad2a3fc775964ae1f2453ea751b3765a32ee57ce2888dcebbf`
- `02-r1-codex.prompt.md` sha256 `810a69c120f9c87b362d26f19d698a0848055839bcc8184405f184b82a0ca284`
- `02-r1-codex.reply.md` sha256 `e75f3756f73a516a1c25c0a1de2c8e4f3dd0eac39e75ea31035488a547b45dc3`
- `03-r2-claude.prompt.md` sha256 `b53b33aac134f32afa7998a16704dadedd65f9cbd045dbdc5a7cd01d18aa0468`
- `03-r2-claude.reply.md` sha256 `c40b1f1ab454786bac7adf9140cbae240ddf4889ef69da3fd69fc55a0466f4e6`
- `04-r2-codex.prompt.md` sha256 `aab0d278aae168a385b39d708171c0adc7c982bca69a8ffdc3520051046592f2`
- `04-r2-codex.reply.md` sha256 `9fc3c9d386e95c44519d9b63f0aff8421cd0091cf0acefd182e9bc008f3e78f8`

## Final dispositions

```text
FINDING k1 | Major | withdrawn | finder-only closure holds (DECISIONS:161-165)
FINDING k2 | Major | withdrawn | tests pass or owner accepts (:95-99)
FINDING k3 | Major | withdrawn | owner-only classes and owner FLEX (:70-89, :225-227)
FINDING k4 | Minor | accepted-fix | review pinned (:100-104), merge pinned to head (:153-155)
FINDING k5 | Minor | accepted-fix | verbatim owner quotes, terms, merge marker (:8-41, :173-175)
FINDING k6 | Minor | accepted-fix | CONTRIBUTING:14-16; lead's own PR defined (:40-41)
FINDING k7 | Minor | withdrawn | withdrawn in r2 (04-r2-crosstalk.md:118)
FINDING k8 | Minor | withdrawn | memory dir still empty; no carry-over
FINDING k9 | Minor | accepted-fix | 018 README:16-17 points to the SHA table
FINDING k10 | Minor | withdrawn | deferred to an owner-merged dependency PR (HANDOFF:110-111); findings.md:21 should read withdrawn
FINDING M1 | Major | accepted-fix | pins, submodules, deps/ owner-only (:78-82)
FINDING M2 | Major | accepted-fix | one scale, FINDING-line sources (:156-160)
FINDING M3 | Major | accepted-fix | 01-09 committed and linked (:217-221); my 10-run r1 reply matches session 2c484437 :291
FINDING N1 | Major | accepted-fix | owner instruction and rewrite recorded (:18-28)
FINDING N2 | Major | accepted-fix | engine (review.sh:76-77), config checks (:80-84), unsandboxed stated (DECISIONS:120-121); residue in claude-14, claude-15, claude-17
FINDING N3 | Major | accepted-fix | admin actions owner-only (:85-88)
FINDING N4 | Major | accepted-fix | owner decides first (:90-94)
FINDING N5 | Minor | accepted-fix | owner-hold rule (:70-71)
FINDING claude-1 | Major | accepted-fix | review worktree HEAD is 02d4c28ee (.git/worktrees/tree/HEAD)
FINDING claude-2 | Major | accepted-fix | transcript-only exemption (:145-150)
FINDING claude-3 | Minor | open | "blind" gone (meta:5), but the comment at review.sh:244-245 says tamper check before save; code saves and pins first (:252-254)
FINDING claude-4 | Minor | accepted-fix | stderr local, CLI versions, full SHA (review.sh:89, :155-158)
FINDING claude-5 | Minor | accepted-fix | CI runs recorded (history :39-44); README :4-6
FINDING claude-6 | Minor | accepted-fix | inconclusive defined (:166-169)
FINDING claude-7 | Minor | accepted-fix | merge pins head (:153-155); exemption (:145-150); peers wording (:32-36)
FINDING claude-8 | Minor | accepted-fix | CI state in 10-run 00-meta:14-20
FINDING claude-9 | Minor | accepted-fix | 018 README:7-10 says stderr stays local, run 06 trimmed
FINDING claude-10 | Minor | accepted-fix | raised IDs become required (review.sh:230)
FINDING claude-11 | Minor | accepted-fix | saved and pinned before tamper check and validation (review.sh:252-257, :264-267)
FINDING claude-12 | Major | accepted-fix | in-turn writes to snapshots and saved replies fail the run (:116-123, :141, :196, :241); cleanup exits 3 (:95-104); residue in claude-15
FINDING claude-13 | Minor | accepted-fix | engine binary, core env, HANDOFF:106-109
FINDING claude-14 | Minor | open | credential helper is the toolbox launcher (config.toml:14-16, :21-23) while meta says "engine, not the toolbox launcher" (:158); undisclosed (review.sh:26-27, DECISIONS:116-117)
FINDING claude-15 | Major | open | "detects any later change" (:41-42; DECISIONS:124-131) omits the round-1 unpinned reply (:248-253), post-run changes (as codex-15), checkout index flags (:143-144 vs :131/:136), "enforcement" (:32, :148), manifest caveats (as codex-16; :166/:272); closes with my r1 wording or code for points 1 and 3
FINDING claude-16 | Major | accepted-fix | --setting-sources user (:211) and .claude/CLAUDE.md refusal (:107-109); nit: root-only .claude check
FINDING claude-17 | Major | open | run 10 repeats it: exec and guardian_review rollouts (…c947…:4, …c967…:4) load "AGENTS.md instructions for /tmp/tmp.1UsQmTwkDD/tree"; engine agents_md.rs names AGENTS.override.md, neither owner-only (DECISIONS:73) nor refused (:107); closes with a refusal or project_doc_max_bytes=0, plus a DECISIONS line
VERDICT: request changes
FINDING c1 | Blocker | withdrawn | missing-record rule explicit
FINDING c2 | Major | withdrawn | two review rounds required
FINDING c3 | Major | withdrawn | finder-only closure explicit
FINDING c4 | Major | withdrawn | owner-only classes explicit
FINDING c5 | Minor | accepted-fix | owner merge authority corrected
FINDING c6 | Major | withdrawn | transcript path rules explicit
FINDING c7 | Minor | accepted-fix | public metadata exception recorded
FINDING c8 | Minor | withdrawn | D2 wording corrected
FINDING M1 | Major | accepted-fix | third-party changes owner-only
FINDING M2 | Major | accepted-fix | one severity scale and finder sources defined
FINDING M3 | Major | accepted-fix | prior correspondence committed and linked
FINDING N1 | Major | accepted-fix | rewrite authorization recorded
FINDING N2 | Major | accepted-fix | engine, config, environment, and sandbox limits documented
FINDING N3 | Major | accepted-fix | repository administration owner-only
FINDING N4 | Major | accepted-fix | persistence and ABI breaks require owner decision
FINDING N5 | Minor | accepted-fix | owner holds defined
FINDING N6 | Major | accepted-fix | prompts and replies saved before validation
FINDING N7 | Major | accepted-fix | failures and malformed outputs fail closed
FINDING codex-1 | Major | accepted-fix | detached reviewed-SHA worktree enforced
FINDING codex-2 | Major | accepted-fix | transcript-only exemption narrowly scoped
FINDING codex-3 | Major | accepted-fix | author controls cross-talk order
FINDING codex-4 | Minor | accepted-fix | prompt-retention limits documented
FINDING codex-5 | Major | accepted-fix | peer-owned findings enforced
FINDING codex-6 | Major | accepted-fix | round one no longer falsely called blind
FINDING codex-7 | Major | accepted-fix | earlier transcripts protected
FINDING codex-8 | Minor | accepted-fix | exempt transcript commits included in merge matching
FINDING codex-9 | Major | accepted-fix | prompts use start-of-run snapshots
FINDING codex-10 | Major | accepted-fix | finding syntax and statuses validated
FINDING codex-11 | Major | accepted-fix | evidence is hashed and rechecked during turns
FINDING codex-12 | Minor | accepted-fix | round one waits for both peers
FINDING codex-13 | Major | accepted-fix | cleanup fails on checkout tampering
FINDING codex-14 | Major | accepted-fix | missing evidence and copy failures fail closed
FINDING codex-15 | Major | open | final evidence can change after the last verification (`review.sh:271-284`)
FINDING codex-16 | Minor | open | 00-meta hash is captured before its final append (`review.sh:165-166`, `:271-284`)
FINDING codex-17 | Major | open | Codex project instructions are not disabled or rejected (`review.sh:107-108`, `:214-215`; `AGENTS.md:1`)
VERDICT: request changes
```
