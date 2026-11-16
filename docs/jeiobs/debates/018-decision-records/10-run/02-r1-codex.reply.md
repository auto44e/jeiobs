Run-09 fixes for codex-6 and codex-14 are accepted. Two issues remain:

- Major: after the final evidence check, an unsandboxed surviving process can alter saved evidence; cleanup checks only the review checkout, not evidence files (`review.sh:271-284`, `:95-103`; `DECISIONS.md:120-132`).
- Minor: `00-meta.md` records its pre-append hash, so its manifest hash is necessarily stale (`review.sh:166`, `:271-284`).

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
FINDING codex-2 | Major | accepted-fix | transcript exemption narrowly scoped
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
FINDING codex-14 | Major | accepted-fix | missing evidence and copy failures now fail closed
FINDING codex-15 | Major | open | final evidence is unchecked after the last verification and can be altered
FINDING codex-16 | Minor | open | 00-meta records its pre-append hash
VERDICT: request changes