# PR #21 review run 01 — task 002 S1-PR1 (workspace format v1 + inert manifest library), head 6c8269596

Your working directory is a checkout of this commit. The change is `git diff 270f9d6c9 HEAD`
(main is 270f9d6c9). Read: docs/jeiobs/WORKSPACE_FORMAT.md (the spec), the PR1 contract at
docs/jeiobs/debates/002-workspace-format/04-pr1-contract.md (wins over the spec for PR1),
docs/jeiobs/s1-persistence-audit.md, shared/jeiobs-workspace/ (library, jw-check CLI, jw-unit,
tests/cases with 328 black-box cases), docs/jeiobs/tasks/002-workspace-saving.md (Progress).

Authorship: the Codex worker wrote the library; the Claude worker wrote the black-box cases from
the spec and contract without reading the library; the lead wrote the contract, fixed 14
contract mismatches the cases found (412d28adb), and the docs. The format doc was drafted by
Claude from the lead's resolutions of the design debate.

Lead's evidence (Linux, Ubuntu 24.04, GCC 13, CMake 3.28, -Wall -Wextra -Werror):
`cmake -S shared/jeiobs-workspace -B build && cmake --build build && ctest --test-dir build`:
jw-unit PASS, jw-cases 328/328 PASS. Windows/macOS not compiled (CI workflow is a separate,
owner-merged .github PR).

This is a new persistence format (owner rule: review rounds repeat until one finds no Blocker or
Major). Review for: spec correctness against the OBS code it cites (verify file:line), security of
the secret/placeholder/export rules, bounded parsing (can any input exceed a limit before it is
checked, crash, or hang?), path-rule completeness (Windows/macOS/Linux), compatibility rules,
whether the cases prove what they claim, C++ correctness and portability, and doc accuracy.

Severity: Blocker = must not merge; Major = must be fixed before a self-merge; Minor = follow-up.
Other agent work runs on this host in separate worktrees; it does not touch this checkout.
