You are one of two independent reviewers (Claude and Codex, different model lineages) in a
cross-talk review for the JeiOBS repository. Your working directory is a checkout of exactly the
commit under review. The lead agent wrote or integrated the change and must NOT decide on its own
whether your findings are resolved: under docs/jeiobs/DECISIONS.md a finding is closed only when the
reviewer who raised it accepts the fix or withdraws it, in writing, in a later round. Use one
severity scale: Blocker, Major, Minor. Only the finder may lower a finding's severity. You are
read-only: never modify, create or delete files, and never run commands that change state.
Verify claims in the files rather than trusting the context. Be terse; cite file:line.

End your reply with one line per finding YOU raised (every ledger row with you as finder, every
finding you raised in an earlier round, plus new ones), then the verdict. If you raised none, write
FINDING none. Use exactly this syntax:
FINDING <ID> | <Blocker|Major|Minor> | <open|accepted-fix|withdrawn> | <one-line reason>
VERDICT: approve
or
VERDICT: request changes
For a new finding, use ID <your-name>-<n> (for example claude-1) and quote the evidence.

REVIEWED COMMIT: 804e22766a42192f2d5e8a29185504080fc8e6ae

TOPIC: Review run 01 of pr36.

SHARED CONTEXT:
# PR #36 review run 01 — Unicode collision key from generated tables (owner decision 3), head 804e22766

Your working directory is a checkout of this commit. Change: `git diff f22e6274d HEAD`. Read docs/jeiobs/WORKSPACE_FORMAT.md section 12 (path rules, collision key) and section 14 decision 3, docs/jeiobs/debates/002-workspace-format/44-unicode-contract.md, shared/jeiobs-workspace/src/Unicode.cpp and include/jeiobs/workspace/Unicode.hpp, the generator tools/jeiobs/unicode/{gen_tables.py,fetch-ucd.sh}, the conformance checker tests/jw_unicode_conformance.cpp, and the wiring in PathRules.cpp, Manifest.cpp, CapturePlanner.cpp, jw-check.cpp. The generated src/UnicodeTables.inc (5141 lines) is data: check it by regeneration (fetch-ucd.sh needs network; if you have none, reason from the generator), not by eye.
Authorship: the lead wrote the library, generator and conformance checker (about 200 lines of logic over generated tables). The Claude worker wrote the 251 new black-box case lines blind from the contract. Both peers review now.
Lead's evidence (Linux, GCC, -Wall -Wextra -Werror): jw-unit, jw-fuzz-smoke, jw-cases (1385 case lines) and jw-unicode-conformance pass, also under ASan+UBSan. The conformance checker passes NormalizationTest.txt (20171 lines, UCD 18.0.0), all code points against CaseFolding.txt C+F, Hangul, and "unlisted code points are NFD-invariant"; two mutations (no canonical ordering, no ASCII fold) make it fail (3501 failures; fold fails). The key of every code point also equals Python 3.12 unicodedata (Unicode 15.0) NFD(casefold(NFD(c))) for all 1,112,064 scalar values except 96 code points unassigned in 15.0. CI on Ubuntu, macOS and Windows runs on the PR (Ubuntu also runs the conformance test).
Review for: correctness of NFD, canonical ordering, Hangul and full case folding (D145), strict UTF-8 handling, the generator's determinism and pins (sha256, version), whether removing `PathStatus::Unchecked`, `Manifest::uncheckedPaths` and `AsciiFold` from the public header breaks any caller or documented behaviour, the planner and manifest collision keys (root + NUL + key; non-UTF-8 fallback 0xFF), limits (key expansion vs path limits, no quadratic behaviour), license compliance (Unicode License v3 text, NOTICE), the workflow change, test validity (are the blind cases independent?), and doc accuracy (section 12, 14, 15, task 002). Owner-merged PR: the owner decides the merge; your verdict says whether it is ready for him.

Severity: Blocker = must not merge; Major = must be fixed before the owner merges; Minor = follow-up.
Other agent work runs on this host in separate worktrees; it does not touch this checkout.

FINDINGS LEDGER (kept by the lead):
# Findings ledger

ID | severity | status | finder | source | lead's note
--- | --- | --- | --- | --- | ---
quick checks pending at start
local: jw-unit, jw-fuzz-smoke, jw-cases (1385 lines), jw-unicode-conformance PASS on Linux, also under ASan+UBSan

YOUR OWN PREVIOUS REPLY:
FINDING codex-1 | Minor | open | Task says “OK unchecked” despite current CLI emitting plain `OK` (docs/jeiobs/tasks/002-workspace-saving.md:61-64; shared/jeiobs-workspace/src/jw-check.cpp:123-125).
FINDING codex-2 | Minor | open | Unlisted-codepoint conformance checks test each scalar alone, so they cannot detect incorrect combining-class ordering (shared/jeiobs-workspace/tests/jw_unicode_conformance.cpp:172-179; :108-155).
VERDICT: approve
THE OTHER REVIEWER'S LATEST REPLY:
**Verdict: approve.** The commit is the same as in round 1 and nothing has been fixed. My four Minors stay open, and I found no Blocker or Major.

**On Codex's findings**
- **codex-1 looks like a misreading.** `tasks/002-workspace-saving.md:63-64` says "`OK unchecked` and the `unchecked-paths` flag are gone". That matches `jw-check.cpp:123-125` and `WORKSPACE_FORMAT.md:691-692`. The only other mentions of `OK unchecked` are in historical debate records (`04-pr1-contract.md:22-26`, `08-pr1-tests-claude.md:10-11`). Only Codex can withdraw it.
- **codex-2: I agree, Minor.**
  - `CheckUnlisted` (`jw_unicode_conformance.cpp:172-179`) tests one code point at a time, so no class can reorder anything there. Combining classes are only exercised by the orderings in `NormalizationTest.txt`.
  - As I recall Part 2's layout, each mark is placed only against classes 228, 230 and 232. So a wrong class that stays on the same side of those, for example 220 stored as 202, would pass. The file isn't in this checkout, so I couldn't confirm the layout here.
  - This is the same gap as claude-1. Either fix closes both: regenerate the tables in CI and `cmp` them, or compare `kCombiningClassRows` with field 3 of `UnicodeData.txt` in the conformance test.

**My findings, re-checked against this commit**
- **claude-1:** `jeiobs-workspace.yaml:37-56` fetches the data and runs the conformance test, but never regenerates the tables or compares them. The rule in `README.md:18` is still not enforced.
- **claude-2:** `WORKSPACE_FORMAT.md:581-583` (a newer Unicode pin can reject an older manifest) and `:511-521` (recovery quarantines a manifest that fails validation) are unchanged. No reader rule is stated.
- **claude-3:** `:575` says "one manifest", but the key is per root (`PathRules.cpp:151-163`, `Manifest.cpp:790`). Section 15 (`:714-718`) still says only case clashes are planned with `collision: true`, but normalization-only clashes now are too, under the warning code `case-collision` (`CapturePlanner.cpp:1324`).
- **claude-4, corrected wording:** the license allows the notice "with all copies" or "in associated Documentation" (`LICENSE:19-22`), not only with every copy. Either way, the binaries need a packaging or documentation step, and nothing records it yet (`NOTICE.md:33-38`).

**Also checked, no new findings**
- The planner's Unicode key is tested: `plan-case-collision` expects `CAFÉ.json|Café.json` (`cases.txt:1367-1370`, `expected/plan/case-collision.json`). A planner that went back to ASCII folding would fail it.
- `AsciiFold` is now private to `PathRules.cpp` (`:92`) and only used for device names.
- The planner/manifest conflict over `collision: true` entries was already handed to PR4 (`31-pr3-contract.md:62-64`). This PR doesn't add it.

**For the owner:** merge only with CI green on all three operating systems.

FINDING claude-1 | Minor | open | CI does not regenerate UnicodeTables.inc and cmp it (README.md:18 unenforced; jeiobs-workspace.yaml:37-56); ccc rows only partly covered by jw-unicode-conformance (same gap as codex-2)
FINDING claude-2 | Minor | open | Version-dependent E_COLLISION (WORKSPACE_FORMAT:581-583) plus section 11 recovery (:511-521) can quarantine a valid stored revision after a Unicode pin bump; reader rule not stated
FINDING claude-3 | Minor | open | WORKSPACE_FORMAT:575 says "one manifest" but the rule is per root; section 15 (:714-718) omits normalization-only clashes and the case-collision code covering them
FINDING claude-4 | Minor | open | Unicode License v3 needs the notice with copies or in associated documentation (LICENSE:19-22); nothing records this for binaries once PR4 links the library (NOTICE.md:33-38)
VERDICT: approve
