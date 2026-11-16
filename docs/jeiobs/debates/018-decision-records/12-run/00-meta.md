# Review run

- Started: 2026-11-16T08:15:17Z
- Reviewed commit: `ce0e3c26bddd5398cf7f7fd8136ff8fdcff24a73` (peers ran in a detached worktree at this commit)
- Author: lead; rounds: 2 (round 1 independent first pass, both at once, not isolated); cross-talk order: claude, codex; timeout per turn: 1500s
- Claude: claude 2.1.325.1734 (ASBX Claude Code, channel stable), model opus, effort max; tools Read, Glob, Grep; no MCP
- Codex: codex-cli 0.169.0 (engine started directly; the toolbox launcher runs only as Bedrock credential helper), effort xhigh; CODEX_HOME config without MCP/plugin/project sections, shell env "core", no project instructions (project_doc_max_bytes=0); --sandbox read-only, sandbox UNAVAILABLE on this host (bubblewrap cannot create user namespaces); Codex commands ran unsandboxed, tripwires only
- Round 1 is an independent first pass, not isolated (see the script header)
- Script: `tools/jeiobs/peer-review/review.sh`, sha256 `f14c7ca0f637243fca49afcc5d55f5b74d3172ad038bd73aaaab1419d6687faa`
- Context sha256 `6725ca6e3911d9822139d16a72532ac5a6d5bb0673e0cb229720b3705ddb907c`; ledger sha256 `dd9ffa880b215ee04f5790b94357d24815c26fe6ed10b02e16f2cddf645aefcc`
- stderr logs: kept locally (not committed), `12-run-20261116T081516Z`
- Topic: Review run 12 of PR #19: dispose of your findings; review the run-10 and run-11 fixes.

## CI state at the reviewed commit

```text
run 43829437286 jeiobs quick checks event=push completed success
head ce0e3c26bddd5398cf7f7fd8136ff8fdcff24a73
local: check_scaffold.py PASS; git diff --check clean; review.sh stub cases as listed in the context
```
- Finished: 2026-11-16T08:42:25Z

## Files written by this run

The hash given for `00-meta.md` is of its header (the part before "Finished");
the runner prints the final hash of every file, this one included, when it exits.

- `00-context.md` sha256 `6725ca6e3911d9822139d16a72532ac5a6d5bb0673e0cb229720b3705ddb907c`
- `00-ledger-in.md` sha256 `dd9ffa880b215ee04f5790b94357d24815c26fe6ed10b02e16f2cddf645aefcc`
- `00-meta.md` sha256 `b80541f492ab0f2c743698aaa545c42fccab1f602bcc97a37cc2f13ed7e6de52`
- `01-r1-claude.prompt.md` sha256 `86f18100e834d399b66d4b91a6e774a0a0bd8f59bb9de8289063c8a3295f5ac8`
- `01-r1-claude.reply.md` sha256 `dcd3e99bf3623689d50053f38ef20d055837a2a2dc9e4b30f7be079037c1b471`
- `02-r1-codex.prompt.md` sha256 `86f18100e834d399b66d4b91a6e774a0a0bd8f59bb9de8289063c8a3295f5ac8`
- `02-r1-codex.reply.md` sha256 `da9e68affa45cdb0f7edeed7067750d65dd8087be628fde54e2e0eb2b5b8a1e4`
- `03-r2-claude.prompt.md` sha256 `a81ce03b9b8ca1456e0167c24cbd518328c54501e7bd8d1d592d91529b21c192`
- `03-r2-claude.reply.md` sha256 `dc6b9dd2313066e8f36309a350284d18b2c48be19dd1f14c7f2aa64252e8da8f`
- `04-r2-codex.prompt.md` sha256 `b62f13ff182f4a018d1f533ad919a32370828452fdfada41a45c84701095635b`
- `04-r2-codex.reply.md` sha256 `989a149a7f1c933d92444c359b2bc65bfa7919c0bf78a91e6f4782457a880b81`

## Final dispositions

```text
FINDING k1 | Major | withdrawn | finder-only closure holds (DECISIONS:177-181)
FINDING k2 | Major | withdrawn | tests pass or owner accepts (:95-99)
FINDING k3 | Major | withdrawn | owner-only classes and owner FLEX (:70-88, :241-243)
FINDING k4 | Minor | accepted-fix | review pinned (:100-102), merge pinned to head (:169-171)
FINDING k5 | Minor | accepted-fix | verbatim owner quotes, terms, merge marker (:8-41, :189-191)
FINDING k6 | Minor | accepted-fix | CONTRIBUTING:14-16; lead's own PR defined (:40-41)
FINDING k7 | Minor | withdrawn | withdrawn in r2 (04-r2-crosstalk.md:118)
FINDING k8 | Minor | withdrawn | memory dir still empty (re-checked r2); shared-dir residue in claude-20
FINDING k9 | Minor | accepted-fix | 018 README:21-22 points to the SHA table
FINDING k10 | Minor | withdrawn | deferred to an owner-merged dependency PR (HANDOFF:110-111); findings.md reads withdrawn
FINDING M1 | Major | accepted-fix | pins, submodules, deps/ owner-only (:78-82)
FINDING M2 | Major | accepted-fix | one scale, FINDING-line sources (:172-176)
FINDING M3 | Major | accepted-fix | runs committed and linked (:233-237); committed 11-run reply matches the /tmp copy
FINDING N1 | Major | accepted-fix | owner instruction and rewrite recorded (:18-28)
FINDING N2 | Major | accepted-fix | engine (review.sh:95-96), config checks (:99-103), unsandboxed stated (DECISIONS:127-129); shared-home residue in claude-20
FINDING N3 | Major | accepted-fix | admin actions owner-only (:85-88)
FINDING N4 | Major | accepted-fix | owner decides first (:90-94)
FINDING N5 | Minor | accepted-fix | owner-hold rule (:70-71)
FINDING claude-1 | Major | accepted-fix | review worktree HEAD and ORIG_HEAD ce0e3c26b, one reflog entry (.git/worktrees/tree)
FINDING claude-2 | Major | accepted-fix | transcript-only exemption (:161-166)
FINDING claude-3 | Minor | accepted-fix | round-1 comment (review.sh:293-295) matches code (:296-314)
FINDING claude-4 | Minor | accepted-fix | stderr local, CLI versions, full SHA (review.sh:109, :193-196)
FINDING claude-5 | Minor | accepted-fix | CI runs recorded (history :39-44); README :4-6
FINDING claude-6 | Minor | accepted-fix | inconclusive defined (:182-185)
FINDING claude-7 | Minor | accepted-fix | merge pins head (:169-171); exemption (:161-166); peers wording (:33-36)
FINDING claude-8 | Minor | accepted-fix | CI state passed via -C into 00-meta (review.sh:202)
FINDING claude-9 | Minor | accepted-fix | 018 README:7-10 says stderr stays local, run 06 trimmed
FINDING claude-10 | Minor | accepted-fix | raised IDs become required (review.sh:279)
FINDING claude-11 | Minor | accepted-fix | saved and pinned before tamper check and validation (review.sh:308, :311-314, :322-325)
FINDING claude-12 | Major | accepted-fix | in-turn writes to snapshots and saved replies fail the run (:145-151, :170); cleanup exits 3 (:118-132)
FINDING claude-13 | Minor | accepted-fix | engine binary, core env, HANDOFF:106-109
FINDING claude-14 | Minor | accepted-fix | launcher as credential helper stated (review.sh:34-36, :196; DECISIONS:121-123) matching config.toml:15-16, :22-23
FINDING claude-15 | Major | accepted-fix | points 1 and 3 in code (:302-310, :126, :174); tripwire wording (:41-64; DECISIONS:127-148); final hashes printed (:349-350)
FINDING claude-16 | Major | accepted-fix | --setting-sources user (:251); refusal at any depth (:135-137)
FINDING claude-17 | Major | accepted-fix | project_doc_max_bytes=0 (:255); session-log check (:177) also catches a global AGENTS.md
FINDING claude-18 | Major | accepted-fix | hits collected in a variable, grep status 2 fails, no pipe into grep -q (:175-182)
FINDING claude-19 | Minor | accepted-fix | .agents/.codex refused at any depth (:135-137; DECISIONS:124-126); skills/ holds only .system
FINDING claude-20 | Major | open | review CODEX_HOME shared with S1-PR1 worker (rollout 08-05-59 …3cda…:1,6,8); config checked once by bypassable line regex, not pinned (:99-103); "dedicated", "lead controls", "engine adds telemetry" wrong (:29-31, :39-40; DECISIONS:116-117)
FINDING claude-21 | Minor | open | records still call runs 06-12 PR #18 and omit the owner merge at 374f842c7 (DECISIONS:3-4, :233-237; README:1; findings.md:1); DECISIONS:56-58 vs README:16-18 edits
VERDICT: request changes
FINDING c1 | Blocker | withdrawn | Missing-record rule explicit (DECISIONS.md:151-153)
FINDING c2 | Major | withdrawn | Two review rounds required (DECISIONS.md:100-110)
FINDING c3 | Major | withdrawn | Finder-only closure explicit (DECISIONS.md:172-181)
FINDING c4 | Major | withdrawn | Owner-only classes and FLEX explicit (DECISIONS.md:70-88,241-243)
FINDING c5 | Minor | accepted-fix | Owner authority recorded (DECISIONS.md:8-28)
FINDING c6 | Major | withdrawn | Transcript rules explicit (DECISIONS.md:42-63)
FINDING c7 | Minor | accepted-fix | Public metadata exception recorded (DECISIONS.md:56-61)
FINDING c8 | Major | withdrawn | Transcript exemption bounded (DECISIONS.md:161-171)
FINDING M1 | Major | accepted-fix | Third-party changes owner-only (DECISIONS.md:78-83)
FINDING M2 | Major | accepted-fix | Severity and finder sources defined (DECISIONS.md:172-176)
FINDING M3 | Major | accepted-fix | Correspondence committed and linked (DECISIONS.md:53-57,233-237)
FINDING N1 | Major | accepted-fix | Rewrite authorization recorded (DECISIONS.md:18-28)
FINDING N2 | Major | accepted-fix | Confinement limits documented (DECISIONS.md:111-148)
FINDING N3 | Major | accepted-fix | Repository administration owner-only (DECISIONS.md:85-88)
FINDING N4 | Major | accepted-fix | Owner decisions required where specified (DECISIONS.md:90-94)
FINDING N5 | Minor | accepted-fix | Owner holds defined (DECISIONS.md:70-71)
FINDING N6 | Major | accepted-fix | Prompts and replies retained (DECISIONS.md:149-153)
FINDING N7 | Major | accepted-fix | Malformed runs fail closed (DECISIONS.md:151-153)
FINDING codex-1 | Major | accepted-fix | Detached reviewed-SHA checkout enforced (review.sh:106,134)
FINDING codex-2 | Major | accepted-fix | Transcript exemption narrowly scoped (DECISIONS.md:161-171)
FINDING codex-3 | Major | accepted-fix | Cross-talk order follows author selection (review.sh:91-94)
FINDING codex-4 | Minor | accepted-fix | Transcript-retention limits documented (README.md:3-20)
FINDING codex-5 | Major | accepted-fix | Finder-owned findings required (review.sh:225-230,275-279)
FINDING codex-6 | Major | accepted-fix | Round one explicitly not isolated (review.sh:20-22,65-66)
FINDING codex-7 | Major | accepted-fix | Earlier transcripts protected (DECISIONS.md:161-168)
FINDING codex-8 | Minor | accepted-fix | Exempt transcript commits covered (DECISIONS.md:161-171)
FINDING codex-9 | Major | accepted-fix | Start-of-run snapshots feed prompts (review.sh:141-152,237-243)
FINDING codex-10 | Major | accepted-fix | Finding syntax and statuses checked (review.sh:262-279)
FINDING codex-11 | Major | accepted-fix | Evidence hashes rechecked (review.sh:145-150,329-350)
FINDING codex-12 | Minor | accepted-fix | Both round-one peers awaited (review.sh:293-314)
FINDING codex-13 | Major | accepted-fix | Cleanup fails on checkout tampering (review.sh:117-130)
FINDING codex-14 | Major | accepted-fix | Saved evidence failures fail closed (review.sh:145,283-290)
FINDING codex-15 | Major | accepted-fix | Cleanup rechecks evidence after turn-group cleanup (review.sh:117-130,329-350)
FINDING codex-16 | Minor | accepted-fix | Header and final hashes distinguished (review.sh:335-350)
FINDING codex-17 | Major | accepted-fix | Codex project docs disabled and monitored (review.sh:135-137,177-182,254-255)
FINDING codex-18 | Major | accepted-fix | Verdict uniqueness and duplicate IDs rejected (review.sh:265-279)
FINDING codex-19 | Major | open | With no `set -e` (review.sh:74), the final `00-meta.md` append is unchecked (review.sh:345-346); a stale unwritable file can be re-pinned and pass cleanup (review.sh:121-123)
VERDICT: request changes
```
