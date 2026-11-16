# VST3 scanner robustness — 2026-11-16

Task 003, branch `jeiobs/vst3-scanner-robustness`. Design, evidence and gaps:
[task 003](../tasks/003-vst3-scanner-robustness.md).

How it was built (worker mode, AI-assisted; not human review):

1. Design: Codex and Claude reviewed one brief independently (read-only). Both
   chose a libobs kill API and per-bundle probe children; they differed on
   stdout markers vs result files (result files chosen) and on an overall
   deadline (none).
2. Round 1: Codex wrote the implementation, Claude the tests, in separate
   worktrees. The tests failed 9/9 new checks on the old code and passed 15/15
   on the new one. The lead added logging of skipped bundles in the host.
3. Cross-review: Claude reviewed the implementation (found a Windows timeout
   blocker, inherited pipe handles, single-file plugins dropped by the
   architecture filter, and smaller issues). Codex reviewed the tests (counts
   instead of exact classes, unproven fixtures, loose timeouts, fail-open
   process checks).
4. Round 2: both workers fixed the adjudicated findings. The lead fixed a
   missing-folder warning the test worker found. Round-2 diffs were reviewed
   again by the other lineage.
