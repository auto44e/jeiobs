Read-only checks pass: `bash -n` and `git diff --check`. N2 and the other fixed findings are accepted. Round-1 blindness and `$OUT` tampering remain Major; cleanup also fails open on late tampering.

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