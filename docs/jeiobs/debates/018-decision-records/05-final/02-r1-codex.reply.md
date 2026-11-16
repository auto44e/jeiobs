`check_scaffold.py` and whitespace checks pass. Open Majors remain: the review is not pinned to the SHA, Claude is not reliably read-only, final records are not yet committed, author-first ordering is not enforced, and the ledger remains nonconforming.

FINDING c1 | Blocker | withdrawn | Missing records now block self-merge (DECISIONS.md:63-64).
FINDING c2 | Major | withdrawn | Two cross-talk rounds are required (DECISIONS.md:99-104).
FINDING c3 | Major | withdrawn | Finder-only closure is now required (DECISIONS.md:117-121).
FINDING c4 | Major | withdrawn | Owner-only release/signing/installer classes are explicit (DECISIONS.md:70-88).
FINDING c5 | Minor | accepted-fix | Owner authority wording is corrected (CONTRIBUTING.md:29-37; ROADMAP.md:227).
FINDING c6 | Major | withdrawn | `~` and `/tmp` are explicitly permitted paths (DECISIONS.md:56-60).
FINDING c7 | Minor | open | The owner’s disposition on retaining the personal email remains unrecorded (findings.md:16).
FINDING c8 | Minor | withdrawn | D2 now says “not a framed stdout payload” and records nonce limits (tasks/003-vst3-scanner-robustness.md:128-136).
FINDING M1 | Major | accepted-fix | Third-party pins, submodules, and vendored sources are owner-only (DECISIONS.md:78-83).
FINDING M2 | Major | open | The ledger still says “verbatim or close quote” and uses High/Medium/Low/Error, violating DECISIONS.md:114-116 (findings.md:8,18-20,38).
FINDING M3 | Major | accepted-fix | PR #18 correspondence is committed and linked (DECISIONS.md:161-165).
FINDING N1 | Major | accepted-fix | The rewrite is recorded and this review targets ac008df4c (history/2026-11-16-history-rewrite.md:3-7; DECISIONS.md:109-112).
FINDING N2 | Major | open | `review.sh:86-87` does not establish Claude read-only under auto permissions, and the external debate helper still delegates the final decision (review.sh:86-90; ~/.claude/settings.json:7-15; ~/.kiro/bin/peer-debate.sh:94-100).
FINDING N3 | Major | accepted-fix | Repository administration is owner-only with recorded instruction (DECISIONS.md:85-88; history/2026-11-16-history-rewrite.md:3-7).
FINDING N4 | Major | accepted-fix | Persistence, migration, ABI, and module-ID breaks require owner decisions (DECISIONS.md:90-94).
FINDING N5 | Minor | accepted-fix | Owner holds are now defined (DECISIONS.md:70-71).
FINDING N6 | Major | accepted-fix | Prompts are saved before each peer call (review.sh:77-84).
FINDING N7 | Major | accepted-fix | Timeout, failure, empty, and malformed turns fail closed (review.sh:71,85-94).
FINDING codex-1 | Major | open | The script only records “Reviewed commit: `$SHA`” and never verifies HEAD, tree cleanliness, or a detached worktree (review.sh:13,49,79).
FINDING codex-2 | Major | open | The gate requires committed correspondence and blocks missing parts, but final 05-final artifacts are outside the tree and committing them changes the reviewed head (DECISIONS.md:53-64,109-112; debates/018-decision-records/README.md:7-8).
FINDING codex-3 | Major | open | The script hard-codes Claude before Codex and has no author selector, contrary to the “other first” rule (review.sh:97-100; DECISIONS.md:105).
FINDING codex-4 | Minor | open | README claims prompts are verbatim, but the round-2 transcript contains helper output rather than per-turn prompts (README.md:3-8; 04-r2-crosstalk.md:3-8).
VERDICT: request changes