# Review run

- Started: 2026-11-16T06:28:48Z
- Reviewed commit: `75c0ea4352a53c6974e829b4007e7a3ed2f079fd` (peers ran in a detached worktree at this commit)
- Author: lead; rounds: 2 (round 1 blind, both at once); cross-talk order: claude, codex; timeout per turn: 1500s
- Claude: claude 2.1.325.1734 (ASBX Claude Code, channel stable), model opus, effort max; tools Read, Glob, Grep; no MCP
- Codex: codex-cli 0.169.0 (engine, not the toolbox launcher), effort xhigh; CODEX_HOME config without MCP/plugin/project sections, shell env "core"; --sandbox read-only, sandbox UNAVAILABLE on this host (bubblewrap cannot create user namespaces); tamper check enforced
- Round 1 is blind by protocol and timing, not enforced (see the script header)
- Script: `tools/jeiobs/peer-review/review.sh`, sha256 `3e678dbcac735a6bc069275e13b79bb6d5e1e0c05311abd63c443e8f43651f73`
- Context sha256 `5dc151e01717a437d3f46b84f0abdb86cc4310537c02e779f768f8fb751d742d`; ledger sha256 `163671b4a6756b4f5379bc642d65f7548967546f260385af615f547d2221e9fb`
- stderr logs: kept locally (not committed), `08-run-20261116T062848Z`
- Topic: Review run 08 of PR #18: dispose of your findings; review the run-07 fixes.

## CI state at the reviewed commit

```text
run 43816884521 jeiobs quick checks event=push completed success
head 75c0ea4352a53c6974e829b4007e7a3ed2f079fd
local: check_scaffold.py PASS; git diff --check clean; review.sh stub cases: clean pass; checkout tamper, hooks tamper, peer failure (files kept), launcher binary all fail as expected
```
- Finished: 2026-11-16T06:51:22Z

## Final dispositions

```text
FINDING k1 | Major | withdrawn | finder-only closure holds (DECISIONS:151-155)
FINDING k2 | Major | withdrawn | tests pass or owner accepts (:95-99)
FINDING k3 | Major | withdrawn | owner-only classes and owner FLEX (:70-89, :215-221)
FINDING k4 | Minor | accepted-fix | review pinned (:100-104), merge pinned to head (:143-145)
FINDING k5 | Minor | accepted-fix | verbatim owner quotes, terms, merge marker (:8-41, :163-165)
FINDING k6 | Minor | accepted-fix | CONTRIBUTING:14-16; lead's own PR defined (:40-41)
FINDING k7 | Minor | withdrawn | withdrawn in r2 (04-r2-crosstalk.md:118)
FINDING k8 | Minor | withdrawn | memory dir still empty; no carry-over
FINDING k9 | Minor | accepted-fix | 018 README:16-17 points to the SHA table
FINDING k10 | Minor | withdrawn | deferred to an owner-merged dependency PR (HANDOFF:110-111); findings.md:21 still says "open (deferred)", should read withdrawn
FINDING M1 | Major | accepted-fix | pins, submodules, deps/ owner-only (:78-82)
FINDING M2 | Major | accepted-fix | one scale, FINDING-line sources (:146-150)
FINDING M3 | Major | accepted-fix | 01-07 committed and linked (:207-211); my 07-run r2 reply matches my session
FINDING N1 | Major | accepted-fix | owner instruction and rewrite recorded (:18-28)
FINDING N2 | Major | accepted-fix | claims match what runs: engine (review.sh:64-65), no MCP headers and core env (:69-72), unsandboxed and tamper scope stated (DECISIONS:110-122); residue in claude-14
FINDING N3 | Major | accepted-fix | admin actions owner-only (:85-88)
FINDING N4 | Major | accepted-fix | owner decides first (:90-94)
FINDING N5 | Minor | accepted-fix | owner-hold rule (:70-71)
FINDING claude-1 | Major | accepted-fix | review worktree HEAD is 75c0ea435 (r1 check; HEAD check at review.sh:113 passed round 1)
FINDING claude-2 | Major | accepted-fix | transcript-only exemption (:135-140)
FINDING claude-3 | Minor | open | review.sh:20-21 still says "neither reply exists while the other works"; replies land in $TMP, the tree's parent (:80-81, :209-210)
FINDING claude-4 | Minor | accepted-fix | stderr local, CLI versions, full SHA (review.sh:77-78, :124-128)
FINDING claude-5 | Minor | accepted-fix | CI runs recorded (history :39-44); README :4-6
FINDING claude-6 | Minor | accepted-fix | inconclusive defined (:156-159)
FINDING claude-7 | Minor | accepted-fix | merge pins head (:143-145); exemption only adds a new run folder plus findings.md (:135-140); peers wording (:33-36)
FINDING claude-8 | Minor | accepted-fix | CI state in 08-run 00-meta :14-20; nit: -C optional (review.sh:58)
FINDING claude-9 | Minor | accepted-fix | 018 README:7-10 says stderr stays local, run 06 trimmed
FINDING claude-10 | Minor | accepted-fix | raised IDs become required (review.sh:196-197)
FINDING claude-11 | Minor | accepted-fix | prompt and reply saved before tamper check and validation (review.sh:212-213, :223)
FINDING claude-12 | Major | open | raised: $OUT holds the gate's inputs (snapshots re-read per prompt, review.sh:93-94, :168-169; hashes at :131 never re-checked) and outputs (:213-239) but is excluded (:102, :105) or outside the repo (this run); DECISIONS:117-121 omits it; TREE status-only (:114); cleanup exits 0 (:84-86); fixable without bubblewrap or owner
FINDING claude-13 | Minor | accepted-fix | engine binary, core env, HANDOFF:106-109; approval_policy=never declined for a valid reason
FINDING claude-14 | Minor | open | launcher still credential helper and writer of [otel] (config.toml:15-16, :30-32) vs DECISIONS:114 and review.sh:27-28; config checked once (review.sh:69-72); probe is not Codex's bwrap (:117); 06/07 meta uncorrected
VERDICT: request changes
FINDING c1 | Blocker | withdrawn | Missing-record rule is explicit (DECISIONS.md:63-64).
FINDING c2 | Major | withdrawn | Two review rounds are required (DECISIONS.md:100-109).
FINDING c3 | Major | withdrawn | Finder-only closure is explicit (DECISIONS.md:146-155).
FINDING c4 | Major | withdrawn | Owner-only classes are explicit (DECISIONS.md:70-94).
FINDING c5 | Minor | accepted-fix | Owner merge authority is corrected (CONTRIBUTING.md:14-16).
FINDING c6 | Major | withdrawn | Transcript path rules are explicit (DECISIONS.md:53-61).
FINDING c7 | Minor | accepted-fix | Public identity exception is recorded (DECISIONS.md:58-60).
FINDING c8 | Minor | withdrawn | D2 wording is corrected (tasks/003-vst3-scanner-robustness.md:128-136).
FINDING M1 | Major | accepted-fix | Third-party changes are owner-only (DECISIONS.md:78-83).
FINDING M2 | Major | accepted-fix | Ledger scale and finder-line sourcing are defined (DECISIONS.md:146-155).
FINDING M3 | Major | accepted-fix | Prior correspondence is committed and linked (DECISIONS.md:53-61).
FINDING N1 | Major | accepted-fix | Rewrite authorization is recorded (DECISIONS.md:18-28).
FINDING N2 | Major | accepted-fix | Engine, config, environment, and unsandboxed limitations are stated accurately (review.sh:64-72,117-131; DECISIONS.md:110-122).
FINDING N3 | Major | accepted-fix | Repository administration is owner-only (DECISIONS.md:85-88).
FINDING N4 | Major | accepted-fix | Persistence and ABI breaks require owner decisions (DECISIONS.md:90-94).
FINDING N5 | Minor | accepted-fix | Owner holds are defined (DECISIONS.md:70-71).
FINDING N6 | Major | accepted-fix | Prompts and replies are saved before validation (review.sh:206-226).
FINDING N7 | Major | accepted-fix | Peer failures and malformed outputs fail closed (review.sh:185-198,206-217).
FINDING codex-1 | Major | accepted-fix | Peers use the detached reviewed-SHA worktree (review.sh:90-94).
FINDING codex-2 | Major | accepted-fix | Transcript exemption is narrowly scoped (DECISIONS.md:135-145).
FINDING codex-3 | Major | accepted-fix | Author selection controls cross-talk order (review.sh:60-63,218-221).
FINDING codex-4 | Minor | accepted-fix | Prompt-retention limits are documented (debates/018-decision-records/README.md:3-10).
FINDING codex-5 | Major | accepted-fix | Peer-owned IDs are enforced across rounds (review.sh:156-161,193-198).
FINDING codex-6 | Major | open | “Blindness ... not enforced”; live replies are created under shared TMP while both peers run (review.sh:35-36,209-210; DECISIONS.md:105-107).
FINDING codex-7 | Major | accepted-fix | Earlier transcripts cannot be changed by the exemption (DECISIONS.md:135-140).
FINDING codex-8 | Minor | accepted-fix | Exempt transcript commits are included in merge matching (DECISIONS.md:143-145).
FINDING codex-9 | Major | accepted-fix | Prompts use start-of-run snapshots (review.sh:92-94,168-169).
FINDING codex-10 | Major | accepted-fix | Finding syntax and statuses are validated (review.sh:185-198).
FINDING codex-11 | Major | open | `$OUT` is excluded from tamper state while snapshots and evidence are reread there (review.sh:93-94,102-106,168-173,199-204,230-238).
FINDING codex-12 | Minor | accepted-fix | Round 1 waits for both peers before tamper checking (review.sh:206-217).
FINDING codex-13 | Major | open | “FAILED: the review checkout was modified” is only printed by cleanup with no nonzero exit (review.sh:83-87), so late tamper can pass.
VERDICT: request changes
```
