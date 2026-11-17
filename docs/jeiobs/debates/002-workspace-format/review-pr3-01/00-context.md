# PR #32 review run 01 — S1-PR3 (capture planner and identity), head 21489bd03

Your working directory is a checkout of this commit. Change: `git diff a0373dec8 HEAD`. Read docs/jeiobs/debates/002-workspace-format/31-pr3-contract.md (the contract with addendum r1; it lists the earlier contracts that still hold), docs/jeiobs/WORKSPACE_FORMAT.md (§6-9, §12, §15), shared/jeiobs-workspace/ (CapturePlanner, Fs, identity, ReviewRules, jw-check plan; tests incl. tests/cases with 1095 case lines).
Authorship: Codex worker wrote the library; Claude worker wrote the black-box cases from the contract without reading the library; the lead wrote the contract, decided four readings the cases raised (r1), fixed a stale unit test and the docs.
Lead's evidence (Linux, GCC, -Wall -Wextra -Werror): jw-unit, jw-fuzz-smoke and jw-cases pass, also under ASan+UBSan.
Review for: correctness against OBS's real layout (verify against frontend code: which files OBS writes where, default vs custom roots, portable mode), symlink and path safety (can a planned entry escape a root or name the store), unrepresentable names, identity matching (can it bind the wrong collection), bounded memory and time on huge trees, portability (Windows/macOS: long paths, case-insensitive filesystems, NFD names on HFS+/APFS), test validity, doc accuracy. Owner (verbatim): "if you peers are more or less in agreement, no problem self merging".

Severity: Blocker = must not merge; Major = must be fixed before a self-merge; Minor = follow-up.
Other agent work runs on this host in separate worktrees; it does not touch this checkout.
