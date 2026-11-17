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
