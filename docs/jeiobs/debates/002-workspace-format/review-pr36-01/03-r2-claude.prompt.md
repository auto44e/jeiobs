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
**Verdict: approve.** I found no Blocker or Major. I raise four Minors, all follow-ups. I could not regenerate `UnicodeTables.inc` because I have no network or shell here, so I checked it by reasoning from the generator and by hand-sampling rows.

**What I checked and found correct**
- **Strict UTF-8** (`Unicode.cpp:51-82`): rejects bad lead bytes (C0, C1, F5 and above), stray continuation bytes, overlong forms (via the per-length minimum), surrogates, values above U+10FFFF and truncated sequences.
- **NFD shortcuts are safe:** `<0xC0` at `Unicode.cpp:136` (the first decomposition row is 0xC0, `.inc:299`) and the class lookup `<0x300` at `Unicode.cpp:122` (the first combining-class row is 0x300, `.inc:4138`).
- **NFD logic:** Hangul arithmetic (`:138-144`) matches Unicode section 3.12. A stable sort of each run of nonzero classes is the canonical ordering algorithm.
- **Case folding and the key:** only statuses C and F are used (`gen_tables.py:34`), no T or S. The ASCII fast path matches table rows `.inc:2529-2554`. The D145 order (NFD, then fold, then NFD) is at `:223-225`.
- **Table samples:** about 60 rows and data offsets match the UCD. Examples: 212B→0041 030A (`.inc:1192`, `:180-181`), 0345→03B9 (`:2760`, `:2402`), 1E9E→ss, 0130→0069 0307, no row for 0131, combining classes of 0323, 0328, 0345, 0E38, 0F71, 1D165, 302A–302F and 3099. Row counts and offsets are consistent with each other (decomposition data is 3450 values, equal to the last offset plus its count). The generator is deterministic (sorted keys, LF output).
- **Removed APIs:** nothing still uses `PathStatus::Unchecked`, `uncheckedPaths` or a public `AsciiFold`. No target outside `shared/jeiobs-workspace` links the library.
- **Collision keys:** root + NUL + key is unambiguous. Roots are a fixed set, path rules ban U+0000, and NFD and folding never produce it. The 0xFF fallback byte never occurs in UTF-8. `ValidatePath` runs before the key (`Manifest.cpp:785-790`). A key is at most about 3× a 512-byte path, and nothing in the key is quadratic.
- **License and workflow:** the license text matches Unicode License v3. The workflow uses `pull_request`, read-only permissions and a sha256-checked fetch.
- **Blind cases:** they are independent of the library (expected values come from Python 15.0 and the worker's own UCD 18.0 code). They catch these mutations: inner NFD removed, F folding replaced by S, Turkic mappings, compatibility (NFKD) mappings, Hangul and ordering stability.

**Findings**
- **claude-1:** CI never checks that the committed tables match the generator. `tools/jeiobs/unicode/README.md:18` says "a clean regeneration must leave `git diff` empty", but nothing in `jeiobs-workspace.yaml:37-56` enforces it.
  - `jw-unicode-conformance` fully checks NFD and folding of every single code point. Combining classes are only checked through the orderings `NormalizationTest.txt` happens to contain. A wrong class that keeps the same order against those marks would pass, and so would a generator edit committed without regenerating.
  - Fix: add `gen_tables.py "$RUNNER_TEMP/ucd" X && cmp X shared/jeiobs-workspace/src/UnicodeTables.inc` to the Ubuntu job.
  - Optional: `fetch-ucd.sh:7` uses `declare -A`, which needs bash 4 or later. Stock macOS bash is 3.2, so the README steps fail there.
- **claude-2:** Validity now depends on the Unicode version. Section 12 (`WORKSPACE_FORMAT.md:581-583`) says a newer pinned version can reject a manifest an older build accepted. Section 11 (`:511-519`) then moves the newest such manifest to `quarantine/` and falls back to an older revision. The chance is very low, because only characters unassigned in the writer's tables can gain mappings. Even so, a reader rule is needed: for example, a collision found in an already-stored revision is not corruption, so restore is blocked instead of quarantining. Alternatively, record the key's Unicode version in the manifest.
- **claude-3:** Two doc inaccuracies.
  - `WORKSPACE_FORMAT.md:575` says "no two paths in one manifest". The rule is per root (`PathRules.hpp:38-41`, `Manifest.cpp:790`, contract `44:9`, the `*-other-root` cases).
  - Section 15 (`:714-718`) still says only case clashes are planned with `collision: true`. Clashes that differ only in normalization form (NFC vs NFD names, possible on ext4 and NTFS) are now planned that way too, still under the warning code `case-collision` (`CapturePlanner.cpp:1324`).
- **claude-4:** The Unicode License v3 notice must go with every copy (`LICENSE:19-22`). The repository copy covers source. Nothing records that the app's packaging must carry it once PR4 links the library (`NOTICE.md:33-38` lists source directories only). Add a note to PR4 or packaging.

**For the owner:** the lead's evidence does not include CI results. Merge only with green on all three operating systems, especially Windows, where the non-BMP and noncharacter cases go through argv conversion (`45:90-96`). Also, the option chosen for decision 3 was the lead's pick, not a written recommendation (`WORKSPACE_FORMAT.md:650-652`), so your merge is what approves it.

FINDING claude-1 | Minor | open | CI does not regenerate UnicodeTables.inc and diff it; combining-class rows are only partly covered by jw-unicode-conformance (README.md:18 not enforced)
FINDING claude-2 | Minor | open | Version-dependent E_COLLISION plus section 11 recovery can quarantine a valid stored revision after a Unicode pin bump; reader rule not stated
FINDING claude-3 | Minor | open | WORKSPACE_FORMAT:575 says "one manifest" but the rule is per root; section 15 omits normalization-only clashes and the case-collision code covering them
FINDING claude-4 | Minor | open | No recorded obligation to ship the Unicode License v3 notice with binaries once PR4 links the library
VERDICT: approve

THE OTHER REVIEWER'S LATEST REPLY:
FINDING codex-1 | Minor | open | Task says “OK unchecked” despite current CLI emitting plain `OK` (docs/jeiobs/tasks/002-workspace-saving.md:61-64; shared/jeiobs-workspace/src/jw-check.cpp:123-125).
FINDING codex-2 | Minor | open | Unlisted-codepoint conformance checks test each scalar alone, so they cannot detect incorrect combining-class ordering (shared/jeiobs-workspace/tests/jw_unicode_conformance.cpp:172-179; :108-155).
VERDICT: approve
