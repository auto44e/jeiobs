# Review run

- Started: 2026-11-16T05:57:13Z
- Reviewed commit: `d9f946609664a6c9630d8d86776307610edbaae5` (peers ran in a detached worktree at this commit)
- Author: lead; rounds: 2 (round 1 blind, both at once); cross-talk order: claude, codex; timeout per turn: 1500s
- Claude: claude 2.1.325.1734 (ASBX Claude Code, channel stable), model opus, effort max; tools Read, Glob, Grep; no MCP
- Codex: 1.0.7804.0 (stable), effort xhigh; CODEX_HOME without MCP, plugins or skills; --sandbox read-only, sandbox UNAVAILABLE on this host (bubblewrap cannot create user namespaces); tamper check enforced
- Script: `tools/jeiobs/peer-review/review.sh`, sha256 `54b05fb555b3a7fa9742e6550d83962389cff173107a6cadbcc7a53d6bc8214f`
- Context sha256 `4f4b092277948f72eace1d537a72896fa6db39f1e486d9e6fe553ad79d13a035`; ledger sha256 `5c9fa940d619e7b749f244b054ce71cd841197a9e417097d11ebad4ae7292df9`
- stderr logs: kept locally (not committed), `07-run-20261116T055712Z`
- Topic: Review run 07 of PR #18: dispose of your findings; review review.sh v3 and the gate text.

## CI state at the reviewed commit

```text
run 43813354850 jeiobs quick checks event=push completed success
head d9f946609664a6c9630d8d86776307610edbaae5
local: check_scaffold.py PASS; git diff --check clean; review.sh stub tests (clean/tamper/repo-tamper/malformed/dropped-finding) as expected
```
- Finished: 2026-11-16T06:23:04Z

## Final dispositions

```text
FINDING k1 | Major | withdrawn | finder-only closure holds (DECISIONS:141-145)
FINDING k2 | Major | withdrawn | tests pass or owner accepts (:95-99)
FINDING k3 | Major | withdrawn | owner-only classes and owner FLEX (:70-89, :204-206)
FINDING k4 | Minor | accepted-fix | merge and review pinned; :131 vs :135 tracked in claude-7
FINDING k5 | Minor | accepted-fix | verbatim owner quotes, terms, merge marker (:8-41, :153-155)
FINDING k6 | Minor | accepted-fix | CONTRIBUTING:14-16; lead's own PR defined (:40-41)
FINDING k7 | Minor | withdrawn | withdrawn in r2 (04-r2-crosstalk.md:118)
FINDING k8 | Minor | withdrawn | memory dir still empty; no carry-over
FINDING k9 | Minor | accepted-fix | 018 README:15-16 points to the SHA table
FINDING k10 | Minor | withdrawn | deferred to an owner-merged dependency PR; findings.md:21 should read withdrawn
FINDING M1 | Major | accepted-fix | pins, submodules, deps/ owner-only (:78-82)
FINDING M2 | Major | accepted-fix | one scale, FINDING-line sources (:136-145)
FINDING M3 | Major | accepted-fix | 01-06 committed and linked
FINDING N1 | Major | accepted-fix | owner instruction and rewrite recorded (:18-28)
FINDING N2 | Major | open | narrowed to false records: DECISIONS:108-114,:188-192 and review.sh:23-30,:110 claim real binary and settings-only home; actual launcher (00-meta:7), auto_review, inherit=all, skills; owner acceptance no longer required
FINDING N3 | Major | accepted-fix | admin actions owner-only (:85-88)
FINDING N4 | Major | accepted-fix | owner decides first (:90-94)
FINDING N5 | Minor | accepted-fix | owner-hold rule (:70-71)
FINDING claude-1 | Major | accepted-fix | review worktree HEAD is d9f946609 (.git/worktrees/tree/HEAD)
FINDING claude-2 | Major | accepted-fix | transcript-only exemption (:127-132); wording in claude-7
FINDING claude-3 | Minor | open | replies sit in $TMP, the tree's parent, while the other works (review.sh:73-74,:189-190); sessions persist (:159-160); DECISIONS:105-106 false
FINDING claude-4 | Minor | accepted-fix | stderr local, CLI versions, full SHA (review.sh:70-71,:107-110)
FINDING claude-5 | Minor | accepted-fix | CI runs recorded; README :4-6
FINDING claude-6 | Minor | accepted-fix | inconclusive defined (:146-149)
FINDING claude-7 | Minor | open | :131 pins new head vs :135 --match-head-commit reviewed SHA; :34-36 stale; :128-129 lets unreviewed commits edit earlier transcripts
FINDING claude-8 | Minor | accepted-fix | CI state in 00-meta:13-19; nit: -C optional (review.sh:52)
FINDING claude-9 | Minor | open | 018 README:8 still says every stderr log is kept
FINDING claude-10 | Minor | accepted-fix | raised IDs become required (review.sh:177-178)
FINDING claude-11 | Minor | open | failed turn loses prompt and reply: save after tamper/validate (:193-195,:199,:202), rm -rf $TMP (:80)
FINDING claude-12 | Minor | open | tamper check (:89-96) misses .git/config, hooks, review HEAD, untracked/ignored contents, ignored dirs, skip-worktree flags, $OUT
FINDING claude-13 | Minor | open | hardening follow-up: approval_policy=never or bwrap, minimal env (config.toml:9), engine binary (review.sh:58), owner action in HANDOFF (0 mentions)
VERDICT: request changes
FINDING c1 | Blocker | withdrawn | Missing records block self-merge (DECISIONS.md:63-64).
FINDING c2 | Major | withdrawn | Two review rounds are required (DECISIONS.md:100-107).
FINDING c3 | Major | withdrawn | Finder-only closure is explicit (DECISIONS.md:141-145).
FINDING c4 | Major | withdrawn | Owner-only classes are explicit (DECISIONS.md:70-94).
FINDING c5 | Minor | accepted-fix | Owner authority wording is corrected (CONTRIBUTING.md:14-16).
FINDING c6 | Major | withdrawn | Transcript path rules are explicit (DECISIONS.md:53-61).
FINDING c7 | Minor | accepted-fix | Public fork identity exception is recorded (DECISIONS.md:58-60).
FINDING c8 | Minor | withdrawn | D2 wording is corrected (tasks/003-vst3-scanner-robustness.md:128-136).
FINDING M1 | Major | accepted-fix | Third-party changes are owner-only (DECISIONS.md:78-83).
FINDING M2 | Major | accepted-fix | Ledger scale and finder-line sourcing are defined (DECISIONS.md:136-145).
FINDING M3 | Major | accepted-fix | Prior correspondence is committed and linked (DECISIONS.md:53-61,198-200).
FINDING N1 | Major | accepted-fix | Rewrite authorization is recorded (DECISIONS.md:23-28).
FINDING N2 | Major | open | Confinement claims remain false: launcher selection, state/plugins/skills, inherited environment, and unsandboxed execution (review.sh:58-65,99-100,162-164; DECISIONS.md:108-114,188-194).
FINDING N3 | Major | accepted-fix | Repository administration is owner-only (DECISIONS.md:85-88).
FINDING N4 | Major | accepted-fix | Persistence and ABI breaks require owner decisions (DECISIONS.md:90-94).
FINDING N5 | Minor | accepted-fix | Owner holds are defined (DECISIONS.md:70-71).
FINDING N6 | Major | accepted-fix | Completed runs save prompts and replies (review.sh:180-185,195-203).
FINDING N7 | Major | accepted-fix | Peer failures and missing outputs fail closed (review.sh:166-179,191-202).
FINDING codex-1 | Major | accepted-fix | Peers use a detached reviewed-SHA worktree (review.sh:73-83).
FINDING codex-2 | Major | accepted-fix | Transcript-only head changes are permitted (DECISIONS.md:127-135).
FINDING codex-3 | Major | accepted-fix | Author selection determines cross-talk order (review.sh:54-57,108).
FINDING codex-4 | Minor | accepted-fix | Prompt-retention limitations are documented (debates/018-decision-records/README.md:3-8).
FINDING codex-5 | Major | accepted-fix | Peer-owned ledger IDs are checked (review.sh:137-179).
FINDING codex-6 | Major | open | Round-1 reply files exist in shared temporary storage while the sibling peer runs (review.sh:73-74,187-195).
FINDING codex-7 | Major | open | The transcript exemption still permits unreviewed edits to earlier evidence (DECISIONS.md:127-132).
FINDING codex-8 | Minor | open | The exemption names the new head, but merge matching still uses the reviewed SHA (DECISIONS.md:131-135).
FINDING codex-9 | Major | accepted-fix | Prompts use start-of-run context and ledger snapshots (review.sh:85-87,149-154).
FINDING codex-10 | Major | accepted-fix | FINDING syntax and statuses are validated (review.sh:166-178).
FINDING codex-11 | Major | open | Tamper state omits ignored/untracked contents, index flags, review HEAD, and related metadata (review.sh:89-97).
FINDING codex-12 | Minor | open | A round-1 failure exits without waiting for or rechecking the sibling peer before cleanup (review.sh:75-83,187-193).
VERDICT: request changes
```
