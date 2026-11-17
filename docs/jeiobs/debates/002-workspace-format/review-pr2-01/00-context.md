# PR #28 review run 01 — S1-PR2 (scanners and redactor), head 5bdc30e12

Your working directory is a checkout of this commit. Change: `git diff b4589ee86 HEAD`. Read docs/jeiobs/debates/002-workspace-format/14-pr2-contract.md (the contract), docs/jeiobs/WORKSPACE_FORMAT.md (§6-8, §12, §13), shared/jeiobs-workspace/ (src: JsonScan, IniScan, Redactor, Manifest, jw-check; tests incl. jw_fuzz_smoke and tests/cases with 882 case lines).
Authorship: Codex worker wrote the library; Claude worker wrote the black-box cases from the contract without reading the library; the lead fixed 26 case failures (URL userinfo range keeps the "@", JSON trailing commas, repeated FFURL counted once) and applied spec edits.
Lead's evidence (Linux, GCC, -Wall -Wextra -Werror): jw-unit, jw-fuzz-smoke, jw-cases pass; the same under ASan+UBSan.
Review for: does the redactor leave any registry secret byte in OUTPUT (try escapes, duplicates, odd encodings, huge/deep input, CRLF/BOM); are limits enforced before allocation and without recursion; offsets/UTF-8 correctness; contract fidelity; INI/JSON edge cases; test validity; C++ correctness and portability (MSVC/clang); doc accuracy. This is a persistence-format slice: review rounds repeat until a run has no open Blocker or Major. Owner (verbatim): "if you peers are more or less in agreement, no problem self merging".

Severity: Blocker = must not merge; Major = must be fixed before a self-merge; Minor = follow-up.
Other agent work runs on this host in separate worktrees; it does not touch this checkout.
