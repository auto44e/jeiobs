# Review run

- Started: 2026-11-16T04:58:48Z
- Reviewed commit: `ac008df4c`
- Rounds: 2, timeout per turn: 1200s
- Script: `tools/jeiobs/peer-review/review.sh`, sha256 `37e1e097202628997bd93546b7899371a91e6d09476fe60e0a63fff0552702e6`
- Context: `00-context.md`, sha256 `867b055b01db2a1bf6bef2aa08e525f5bc478d4108812bdcac9e4a21bb0ed690`
- Topic: Final review of PR #18 at ac008df4c: dispose of your findings and review the new changes.
- Finished: 2026-11-16T05:20:41Z

## Final dispositions

```text
FINDING k1 | Major | withdrawn | finder-only closure holds (DECISIONS:117-124)
FINDING k2 | Major | withdrawn | tests must pass or the owner accepts (:95-98)
FINDING k3 | Major | withdrawn | owner-only classes and owner-only FLEX (:70-88, :169-171)
FINDING k4 | Minor | accepted-fix | merge pinned by --match-head-commit (:112); review side is claude-1
FINDING k5 | Minor | accepted-fix | verbatim quotes, terms, merge marker (:10-41, :128-130); PR body under claude-2
FINDING k6 | Minor | accepted-fix | CONTRIBUTING:29 says owner; lead's own PR defined (:40-41)
FINDING k7 | Minor | withdrawn | withdrawn in r2 (04-r2-crosstalk.md:118)
FINDING k8 | Minor | withdrawn | memory dir empty; remaining :34-36 untruths tracked as claude-1 and N2
FINDING k9 | Minor | accepted-fix | READMEs in 003, 004, 018 point to the SHA table
FINDING k10 | Minor | open | aqtinstall still unpinned (linux-deps.sh:64) and absent from HANDOFF; non-blocking; needs an issue or its own owner-merged PR
FINDING M1 | Major | accepted-fix | pins, submodules and deps/ are owner-only (:78-82)
FINDING M2 | Major | accepted-fix | rule text fixed (:113-121); ledger leftovers in claude-5; Codex keeps M2 open as co-finder
FINDING M3 | Major | accepted-fix | 01-04 committed and match their sources; final round is claude-2
FINDING N1 | Major | accepted-fix | owner instruction recorded (:23-24); trees identical; this run reviews ac008df4c
FINDING N2 | Major | open | this turn again had Bash (--allowedTools is not --tools; auto mode); Codex MCP outside --sandbox; live D6/S1 debates still use peer-debate.sh
FINDING N3 | Major | accepted-fix | admin actions owner-only with verbatim instruction (:85-88; AGENTS:10-11)
FINDING N4 | Major | accepted-fix | owner decides first, then self-mergeable (:90-94)
FINDING N5 | Minor | accepted-fix | owner-hold rule (:70-71)
FINDING claude-1 | Major | open | review.sh never checks that the peers' tree is SHA (-s only printed, :13/:49/:79); untracked file present during this run
FINDING claude-2 | Major | open | committing 05-final and the ledger changes the head (:109/:112), omitting them blocks (:63-64); PR body stale ("Docs only", unowned Flex, "rejected with reasons")
FINDING claude-3 | Major | open | Claude-first order and round-1 relay (review.sh:97-100; 02-r1-codex.prompt.md:62-158) break :105 and the blind first look
FINDING claude-4 | Minor | open | failure logs deleted (:70); meta lacks CLI versions, models, full SHA; :81 raw-replies pointer incomplete
FINDING claude-5 | Minor | open | the "error" behind the owner's condition (runs 43805282657, 43805282753) not recorded in the repo; round-2 per-turn prompts not disclosed (018 README:3); ledger not verbatim, off-scale, k6 misstated
FINDING claude-6 | Minor | open | "two cross-talk rounds then inconclusive" (:122-124) undefined for single-SHA runs: either forces escalation or allows endless reruns
VERDICT: request changes
FINDING c1 | Blocker | withdrawn | Missing records now block self-merge (DECISIONS.md:63-64).
FINDING c2 | Major | withdrawn | Two cross-talk rounds are required (DECISIONS.md:99-104).
FINDING c3 | Major | withdrawn | Finder-only closure is required (DECISIONS.md:117-121).
FINDING c4 | Major | withdrawn | Owner-only release/signing/installer classes are explicit (DECISIONS.md:70-88).
FINDING c5 | Minor | accepted-fix | Owner authority wording is corrected (CONTRIBUTING.md:29-37; ROADMAP.md:227).
FINDING c6 | Major | withdrawn | The policy permits `~` and `/tmp` transcript paths (DECISIONS.md:56-60).
FINDING c7 | Minor | accepted-fix | The accepted policy records that the public fork identity may remain (DECISIONS.md:58-60).
FINDING c8 | Minor | withdrawn | D2 wording was corrected (tasks/003-vst3-scanner-robustness.md:128-136).
FINDING M1 | Major | accepted-fix | Third-party pins, submodules, and vendored sources are owner-only (DECISIONS.md:78-83).
FINDING M2 | Major | open | The ledger still uses nonconforming severities and “verbatim or close quote” (findings.md:8,18-26,38).
FINDING M3 | Major | accepted-fix | PR #18 correspondence is committed and linked (DECISIONS.md:161-165).
FINDING N1 | Major | accepted-fix | The rewritten head and owner instruction are recorded and reviewed (DECISIONS.md:23-28,109-112).
FINDING N2 | Major | open | Claude is not reliably constrained read-only by the script’s flags (review.sh:86-90; DECISIONS.md:99-105).
FINDING N3 | Major | accepted-fix | Repository administration is owner-only with recorded authorization (DECISIONS.md:85-88).
FINDING N4 | Major | accepted-fix | Persistence, migration, ABI, and module-ID breaks require owner decisions (DECISIONS.md:90-94).
FINDING N5 | Minor | accepted-fix | Owner holds are defined (DECISIONS.md:70-71).
FINDING N6 | Major | accepted-fix | Prompts are saved before each peer call (review.sh:77-84).
FINDING N7 | Major | accepted-fix | Peer failures and empty replies fail closed (review.sh:71,85-94).
FINDING codex-1 | Major | open | `-s SHA` is only recorded; HEAD and worktree contents are never verified or isolated (review.sh:13,49,79-80).
FINDING codex-2 | Major | open | Final-round correspondence remains outside the committed tree (DECISIONS.md:53-64; debates/018-decision-records/README.md:7-8).
FINDING codex-3 | Major | open | The script hard-codes Claude-first ordering and has no author selector (review.sh:97-100; DECISIONS.md:105).
FINDING codex-4 | Minor | open | The README claims verbatim prompts, but the round-2 record is helper output rather than per-turn prompts (README.md:3-8; 04-r2-crosstalk.md:3-10).
FINDING codex-5 | Major | open | The script checks only `VERDICT`, not the required per-finding disposition lines (review.sh:65-68,92-94; DECISIONS.md:117-121).
VERDICT: request changes
```
