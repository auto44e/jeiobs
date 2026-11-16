# Review run

- Started: 2026-11-16T06:54:36Z
- Reviewed commit: `bde9219b615253bfd7f079cd9fb9e273497ab9e9` (peers ran in a detached worktree at this commit)
- Author: lead; rounds: 2 (round 1 blind, both at once); cross-talk order: claude, codex; timeout per turn: 1500s
- Claude: claude 2.1.325.1734 (ASBX Claude Code, channel stable), model opus, effort max; tools Read, Glob, Grep; no MCP
- Codex: codex-cli 0.169.0 (engine, not the toolbox launcher), effort xhigh; CODEX_HOME config without MCP/plugin/project sections, shell env "core"; --sandbox read-only, sandbox UNAVAILABLE on this host (bubblewrap cannot create user namespaces); tamper check enforced
- Round 1 is an independent first pass, not isolated (see the script header)
- Script: `tools/jeiobs/peer-review/review.sh`, sha256 `37e18fb20339628e28c454ada4160690ea60f0004ecaac0361beb8cc0e3e20bf`
- Context sha256 `f9506a0a9d8365b2056c074c696cd9494cabd7979d746b0bc9efd5d527620cf6`; ledger sha256 `96778ba9fa6e995a55e6f9e7c1b3d4a287238a99f85607fe57bd38b42cde0247`
- stderr logs: kept locally (not committed), `09-run-20261116T065435Z`
- Topic: Review run 09 of PR #18: dispose of your findings; review the run-08 fixes.

## CI state at the reviewed commit

```text
run 43819883607 jeiobs quick checks event=push completed success
head bde9219b615253bfd7f079cd9fb9e273497ab9e9
local: check_scaffold.py PASS; git diff --check clean; review.sh stub cases: clean pass; evidence edit and late checkout write fail as expected (plus earlier cases)
```
- Finished: 2026-11-16T07:17:36Z

## Final dispositions

```text
FINDING k1 | Major | withdrawn | finder-only closure holds (DECISIONS:156-160)
FINDING k2 | Major | withdrawn | tests pass or owner accepts (:95-99)
FINDING k3 | Major | withdrawn | owner-only classes and owner FLEX (:70-89, :220-226)
FINDING k4 | Minor | accepted-fix | review pinned (:100-104), merge pinned to head (:148-150)
FINDING k5 | Minor | accepted-fix | verbatim owner quotes, terms, merge marker (:8-41, :168-170)
FINDING k6 | Minor | accepted-fix | CONTRIBUTING:14-16; lead's own PR defined (:40-41)
FINDING k7 | Minor | withdrawn | withdrawn in r2 (04-r2-crosstalk.md:118)
FINDING k8 | Minor | withdrawn | memory dir still empty; no carry-over
FINDING k9 | Minor | accepted-fix | 018 README:16-17 points to the SHA table
FINDING k10 | Minor | withdrawn | deferred to an owner-merged dependency PR (HANDOFF:110-111); findings.md:21 should read withdrawn (08-run/03-r2-claude.reply.md:48)
FINDING M1 | Major | accepted-fix | pins, submodules, deps/ owner-only (:78-82)
FINDING M2 | Major | accepted-fix | one scale, FINDING-line sources (:151-155)
FINDING M3 | Major | accepted-fix | 01-08 committed and linked (:212-216); my 08-run r2 lines are in session 7607702b
FINDING N1 | Major | accepted-fix | owner instruction and rewrite recorded (:18-28)
FINDING N2 | Major | accepted-fix | engine (review.sh:68-69), no MCP sections and core env (:73-76), unsandboxed stated (DECISIONS:117-118); residue in claude-14, claude-15, claude-16
FINDING N3 | Major | accepted-fix | admin actions owner-only (:85-88)
FINDING N4 | Major | accepted-fix | owner decides first (:90-94)
FINDING N5 | Minor | accepted-fix | owner-hold rule (:70-71)
FINDING claude-1 | Major | accepted-fix | review worktree HEAD is bde9219b6 (.git/worktrees/tree/HEAD)
FINDING claude-2 | Major | accepted-fix | transcript-only exemption (:140-145)
FINDING claude-3 | Minor | open | header and DECISIONS:105-110 fixed, but review.sh:144 writes "round 1 blind" (09-run meta:5 vs :8 "not isolated"); :191, :229; :229-230 order stale (:236-238)
FINDING claude-4 | Minor | accepted-fix | stderr local, CLI versions, full SHA (review.sh:81-82, :143-146)
FINDING claude-5 | Minor | accepted-fix | CI runs recorded (history :39-44); README :4-6
FINDING claude-6 | Minor | accepted-fix | inconclusive defined (:161-164)
FINDING claude-7 | Minor | accepted-fix | merge pins head (:148-150); exemption (:140-145); peers wording (:32-36)
FINDING claude-8 | Minor | accepted-fix | CI state in 09-run 00-meta:14-20; nit: -C optional (review.sh:62)
FINDING claude-9 | Minor | accepted-fix | 018 README:7-10 says stderr stays local, run 06 trimmed
FINDING claude-10 | Minor | accepted-fix | raised IDs become required (review.sh:216)
FINDING claude-11 | Minor | accepted-fix | saved and pinned before tamper check and validation (review.sh:236-241, :247-250)
FINDING claude-12 | Major | accepted-fix | in-turn writes to snapshots and saved replies now fail the run (:103-111, :129, :183, :237, :247); cleanup exits 3 (:89-95); unaddressed parts (meta, extra files, manifest, checkout flags) carried into claude-15
FINDING claude-13 | Minor | accepted-fix | engine binary, core env, HANDOFF:106-109
FINDING claude-14 | Minor | open | credential helper is the toolbox launcher (config.toml:14-16, :21-23), undisclosed in review.sh and DECISIONS:113-116 despite the lead's note; config checked once (:72-76); system-bwrap probe (:135); 06/07 point dropped (DECISIONS:207-209)
FINDING claude-15 | Major | open | "so a peer cannot alter the inputs or evidence" (review.sh:40; DECISIONS:126) is false: round-1 reply pinned only after both finish (:232-237), staged prompts pinned only after the turn (:184-192, :220, :226), verify-first and re-read after last check (:129, :260, :263; cleanup :89-90), meta unpinned (:139-153), OUTDIR extras unchecked (:119/:122; DECISIONS:141-145), checkout index flags (:131-132); closes with exact wording or the code fixes
FINDING claude-16 | Major | open | review.sh:197-198 runs claude -p in the reviewed checkout without --setting-sources and -p skips workspace trust (CLI help), so a tracked .claude/settings.json (not owner-only, DECISIONS:72-88) runs hooks and injects context absent from the committed prompt; undisclosed (review.sh:25; DECISIONS:111-112); closes with --setting-sources user or a fail on tracked .claude/CLAUDE.md, plus a line that AGENTS.md reaches the peers
VERDICT: request changes
FINDING c1 | Blocker | withdrawn | Missing-record rule is explicit (DECISIONS.md:63-64).
FINDING c2 | Major | withdrawn | Two review rounds are required (DECISIONS.md:100-102).
FINDING c3 | Major | withdrawn | Finder-only closure is explicit (DECISIONS.md:151-160).
FINDING c4 | Major | withdrawn | Owner-only classes are explicit (DECISIONS.md:70-94).
FINDING c5 | Minor | accepted-fix | Owner merge authority is corrected (CONTRIBUTING.md:14-16).
FINDING c6 | Major | withdrawn | Transcript path rules are explicit (DECISIONS.md:53-61).
FINDING c7 | Minor | accepted-fix | Public metadata exception is recorded (DECISIONS.md:58-60).
FINDING c8 | Minor | withdrawn | D2 wording is corrected (tasks/003-vst3-scanner-robustness.md:128-136).
FINDING M1 | Major | accepted-fix | Third-party changes are owner-only (DECISIONS.md:78-83).
FINDING M2 | Major | accepted-fix | Ledger scale and finder-line sourcing are defined (DECISIONS.md:151-155).
FINDING M3 | Major | accepted-fix | Prior correspondence is committed and linked (DECISIONS.md:53-61,214-216).
FINDING N1 | Major | accepted-fix | Rewrite authorization is recorded (DECISIONS.md:18-28).
FINDING N2 | Major | accepted-fix | Engine, configuration, environment, and sandbox limits are stated (review.sh:24-40; DECISIONS.md:111-127).
FINDING N3 | Major | accepted-fix | Repository administration is owner-only (DECISIONS.md:85-88).
FINDING N4 | Major | accepted-fix | Persistence and ABI breaks require owner decisions (DECISIONS.md:90-94).
FINDING N5 | Minor | accepted-fix | Owner holds are defined (DECISIONS.md:70-71).
FINDING N6 | Major | accepted-fix | Prompts and replies are saved before validation (review.sh:218-250).
FINDING N7 | Major | accepted-fix | Peer failures and malformed outputs fail closed (review.sh:204-217,239-250).
FINDING codex-1 | Major | accepted-fix | Peers use the detached reviewed-SHA worktree (review.sh:14-15,98,131-132).
FINDING codex-2 | Major | accepted-fix | Transcript exemption is narrowly scoped (DECISIONS.md:141-145).
FINDING codex-3 | Major | accepted-fix | Author selection controls cross-talk order (review.sh:64-66,242-251).
FINDING codex-4 | Minor | accepted-fix | Prompt-retention limits are documented (debates/018-decision-records/README.md:3-10).
FINDING codex-5 | Major | accepted-fix | Peer-owned IDs are enforced across rounds (review.sh:173-216).
FINDING codex-6 | Major | open | The script still calls round 1 “blind” despite shared live replies and non-isolation (review.sh:144,191,229-237).
FINDING codex-7 | Major | accepted-fix | Earlier transcripts cannot be changed by the exemption (DECISIONS.md:141-145).
FINDING codex-8 | Minor | accepted-fix | Exempt transcript commits are included in merge matching (DECISIONS.md:148-150).
FINDING codex-9 | Major | accepted-fix | Prompts use start-of-run snapshots (review.sh:100-111,181-193).
FINDING codex-10 | Major | accepted-fix | Finding syntax and statuses are validated (review.sh:204-217).
FINDING codex-11 | Major | accepted-fix | Saved inputs and prompt/reply evidence are hashed and rechecked (review.sh:103-111,128-132,224-250).
FINDING codex-12 | Minor | accepted-fix | Round 1 waits for both peers before tamper checking (review.sh:229-241).
FINDING codex-13 | Major | accepted-fix | Cleanup now exits nonzero on late checkout tampering (review.sh:87-95).
FINDING codex-14 | Major | open | Prompt-copy failures are ignored and missing evidence hashes can verify as empty (review.sh:101,104-108,218-227).
VERDICT: request changes
```
