`bash -n` and `git diff --check` pass. Run-07 fixes are mostly effective, but shared temporary replies and excluded `$OUTDIR` remain unsafe without confinement.

FINDING c1 | Blocker | withdrawn | Missing-record rule is explicit (DECISIONS.md:63-64).
FINDING c2 | Major | withdrawn | Two review rounds remain required (DECISIONS.md:100-109).
FINDING c3 | Major | withdrawn | Finder-only closure is explicit (DECISIONS.md:146-155).
FINDING c4 | Major | withdrawn | Owner-only classes are explicit (DECISIONS.md:70-94).
FINDING c5 | Minor | accepted-fix | Owner merge authority is corrected (docs/jeiobs/CONTRIBUTING.md:14-16).
FINDING c6 | Major | withdrawn | Transcript path rules are explicit (DECISIONS.md:53-61).
FINDING c7 | Minor | accepted-fix | Public identity exception is recorded (DECISIONS.md:58-60).
FINDING c8 | Minor | withdrawn | D2 wording is corrected (docs/jeiobs/tasks/003-vst3-scanner-robustness.md:128-136).
FINDING M1 | Major | accepted-fix | Third-party changes are owner-only (DECISIONS.md:78-83).
FINDING M2 | Major | accepted-fix | Ledger scale and finder-line sourcing are defined (DECISIONS.md:146-155).
FINDING M3 | Major | accepted-fix | Prior correspondence is committed and linked (DECISIONS.md:53-61, 209-211).
FINDING N1 | Major | accepted-fix | Rewrite authorization is recorded (DECISIONS.md:18-28).
FINDING N2 | Major | accepted-fix | Engine/config and unsandboxed limitations are now stated accurately (review.sh:64-72, 117-129; DECISIONS.md:110-122).
FINDING N3 | Major | accepted-fix | Repository administration is owner-only (DECISIONS.md:85-88).
FINDING N4 | Major | accepted-fix | Persistence and ABI breaks require owner decisions (DECISIONS.md:90-94).
FINDING N5 | Minor | accepted-fix | Owner holds are defined (DECISIONS.md:70-71).
FINDING N6 | Major | accepted-fix | Prompts/replies are saved before validation (review.sh:163-173, 199-204).
FINDING N7 | Major | accepted-fix | Peer failures and malformed outputs fail closed (review.sh:185-198, 206-217).
FINDING codex-1 | Major | accepted-fix | Peers use the detached reviewed-SHA worktree (review.sh:90-94).
FINDING codex-2 | Major | accepted-fix | Transcript exemption is narrowly scoped (DECISIONS.md:135-145).
FINDING codex-3 | Major | accepted-fix | Author selection controls cross-talk order (review.sh:60-63, 218-221).
FINDING codex-4 | Minor | accepted-fix | Prompt-retention limits are documented (debates/018-decision-records/README.md:3-10).
FINDING codex-5 | Major | accepted-fix | Peer-owned IDs are enforced across rounds (review.sh:156-161, 193-198).
FINDING codex-6 | Major | open | Round-1 replies remain in shared TMP while peers run; blindness is explicitly unenforced (review.sh:35-36, 208-210).
FINDING codex-7 | Major | accepted-fix | Exemption forbids edits to earlier transcripts (DECISIONS.md:135-140).
FINDING codex-8 | Minor | accepted-fix | Merge matching now permits the exempt transcript head (DECISIONS.md:143-145).
FINDING codex-9 | Major | accepted-fix | Context and ledger are snapshotted before peer execution (review.sh:92-94, 167-173).
FINDING codex-10 | Major | accepted-fix | Finding syntax and statuses are validated (review.sh:185-198).
FINDING codex-11 | Major | open | Tamper hashing excludes OUTDIR, although later prompts read its saved evidence (review.sh:32-34, 102-106, 199-204, 220-221).
FINDING codex-12 | Minor | accepted-fix | Round 1 waits for both peers before tamper checking (review.sh:206-217).
VERDICT: request changes