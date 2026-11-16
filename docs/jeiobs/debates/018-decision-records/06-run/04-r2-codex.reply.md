FINDING c1 | Blocker | withdrawn | Missing records block self-merge (DECISIONS.md:63-64).
FINDING c2 | Major | withdrawn | Two review rounds are required (DECISIONS.md:99-106).
FINDING c3 | Major | withdrawn | Finder-only closure is explicit (DECISIONS.md:133-137).
FINDING c4 | Major | withdrawn | Owner-only classes are explicit (DECISIONS.md:70-88).
FINDING c5 | Minor | accepted-fix | Owner authority wording is corrected (CONTRIBUTING.md:14-16).
FINDING c6 | Major | withdrawn | Transcript path rules are explicit (DECISIONS.md:56-60).
FINDING c7 | Minor | accepted-fix | Public fork identity exception is recorded (DECISIONS.md:58-60).
FINDING c8 | Minor | withdrawn | D2 wording is corrected (tasks/003-vst3-scanner-robustness.md:128-136).
FINDING M1 | Major | accepted-fix | Third-party changes are owner-only (DECISIONS.md:78-83).
FINDING M2 | Major | accepted-fix | Ledger scale and finder-line sourcing are corrected (DECISIONS.md:128-137).
FINDING M3 | Major | accepted-fix | Prior correspondence is committed and linked (DECISIONS.md:53-61,178-182).
FINDING N1 | Major | accepted-fix | Rewrite authorization and SHA mapping are recorded (DECISIONS.md:23-28).
FINDING N2 | Major | open | Codex sandbox failure escalates instead of failing closed (review.sh:114-115; 06-run/02-r1-codex.stderr.txt:6,111-117).
FINDING N3 | Major | accepted-fix | Repository administration is owner-only (DECISIONS.md:85-88).
FINDING N4 | Major | accepted-fix | Persistence and ABI breaks require owner decisions (DECISIONS.md:90-94).
FINDING N5 | Minor | accepted-fix | Owner holds are defined (DECISIONS.md:70-71).
FINDING N6 | Major | accepted-fix | Prompts are saved before peer execution (review.sh:100-108).
FINDING N7 | Major | accepted-fix | Peer failures and missing outputs fail closed (review.sh:88,118-123).
FINDING codex-1 | Major | accepted-fix | Peers now use a detached reviewed-SHA worktree (review.sh:47-56).
FINDING codex-2 | Major | accepted-fix | Transcript-only head changes are permitted (DECISIONS.md:119-124).
FINDING codex-3 | Major | open | `-f` does not verify the selected speaker is the non-author lineage (review.sh:31-45; DECISIONS.md:104-106).
FINDING codex-4 | Minor | accepted-fix | README discloses helper prompt limitations (debates/018-decision-records/README.md:3-8).
FINDING codex-5 | Major | accepted-fix | Ledger-owned IDs are checked (review.sh:90-123).
FINDING codex-6 | Major | open | Round 1 exposes `$OUT` and the first reply to the second peer (review.sh:103-107,111,127-129).
FINDING codex-7 | Major | open | `git diff --name-only` cannot prove add-only transcript changes (DECISIONS.md:119-124).
FINDING codex-8 | Minor | open | Merge pins the old reviewed SHA despite the transcript exemption (DECISIONS.md:123-127).
FINDING codex-9 | Major | open | Prompts reread live context and ledger files instead of snapshots (review.sh:71-72,103-104).
FINDING codex-10 | Major | open | Validation checks only finding-ID prefixes, not disposition syntax/status (review.sh:118-123; DECISIONS.md:136-137).
VERDICT: request changes