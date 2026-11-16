Read-only checks pass (`bash -n`, `git diff --check`). Round-1 wording remains materially false, and a new fail-open evidence path exists.

FINDING c1 | Blocker | withdrawn | Missing-record rule is explicit (DECISIONS.md:63-64).
FINDING c2 | Major | withdrawn | Two review rounds are required (DECISIONS.md:100-102).
FINDING c3 | Major | withdrawn | Finder-only closure is explicit (DECISIONS.md:151-160).
FINDING c4 | Major | withdrawn | Owner-only classes are explicit (DECISIONS.md:70-94).
FINDING c5 | Minor | accepted-fix | Owner merge authority is corrected (CONTRIBUTING.md:14-16).
FINDING c6 | Major | withdrawn | Transcript path rules are explicit (DECISIONS.md:53-61).
FINDING c7 | Minor | accepted-fix | Public identity exception is recorded (DECISIONS.md:58-60).
FINDING c8 | Minor | withdrawn | D2 wording is corrected (tasks/003-vst3-scanner-robustness.md:128-136).
FINDING M1 | Major | accepted-fix | Third-party changes are owner-only (DECISIONS.md:78-83).
FINDING M2 | Major | accepted-fix | Ledger scale and finder-line sourcing are defined (DECISIONS.md:151-155).
FINDING M3 | Major | accepted-fix | Prior correspondence is committed and documented (debates/018-decision-records/README.md:3-10).
FINDING N1 | Major | accepted-fix | Rewrite authorization is recorded (DECISIONS.md:18-28).
FINDING N2 | Major | accepted-fix | Engine, config, environment, and sandbox limitations are documented (review.sh:68-75,135-149).
FINDING N3 | Major | accepted-fix | Repository administration is owner-only (DECISIONS.md:85-88).
FINDING N4 | Major | accepted-fix | Persistence and ABI breaks require owner decisions (DECISIONS.md:90-94).
FINDING N5 | Minor | accepted-fix | Owner holds are defined (DECISIONS.md:70-71).
FINDING N6 | Major | accepted-fix | Prompts and replies are saved before validation (review.sh:218-250).
FINDING N7 | Major | accepted-fix | Peer failures and malformed outputs fail closed (review.sh:204-217,229-250).
FINDING codex-1 | Major | accepted-fix | Peers use the detached reviewed-SHA worktree (review.sh:98,143).
FINDING codex-2 | Major | accepted-fix | Transcript exemption is narrowly scoped (DECISIONS.md:140-145).
FINDING codex-3 | Major | accepted-fix | Author selection controls cross-talk order (review.sh:64-66,242-251).
FINDING codex-4 | Minor | accepted-fix | Prompt-retention limits are documented (debates/018-decision-records/README.md:3-10).
FINDING codex-5 | Major | accepted-fix | Peer-owned IDs are enforced across rounds (review.sh:173-216).
FINDING codex-6 | Major | open | The runner still says “round 1 blind” while peers share live reply staging (review.sh:144,191,229-237), so the gate evidence is false and first-pass independence is not enforced.
FINDING codex-7 | Major | accepted-fix | Earlier transcripts cannot change through the exemption (DECISIONS.md:140-145).
FINDING codex-8 | Minor | accepted-fix | Exempt transcript commits are included in merge matching (DECISIONS.md:148-150).
FINDING codex-9 | Major | accepted-fix | Prompts use start-of-run snapshots (review.sh:100-111,181-193).
FINDING codex-10 | Major | accepted-fix | Finding syntax and statuses are validated (review.sh:204-217).
FINDING codex-11 | Major | accepted-fix | Saved inputs and prompt/reply evidence are hashed and rechecked (review.sh:103-111,128-132,224-227).
FINDING codex-12 | Minor | accepted-fix | Round 1 waits for both peers before tamper checking (review.sh:229-241).
FINDING codex-13 | Major | accepted-fix | Cleanup now exits nonzero on checkout tampering (review.sh:87-95).
FINDING codex-14 | Major | open | `cp` failures are ignored and missing prompts are never validated, so incomplete evidence can pass (review.sh:204-214,218-227).
VERDICT: request changes