FINDING codex-1 | Minor | withdrawn | The task says the old `OK unchecked` output and flag “are gone,” matching `docs/jeiobs/tasks/002-workspace-saving.md:63-65` and `shared/jeiobs-workspace/src/jw-check.cpp:123-125`.
FINDING codex-2 | Minor | open | Unlisted-codepoint checks use one scalar at a time, so they cannot detect incorrect combining-class ordering (`shared/jeiobs-workspace/tests/jw_unicode_conformance.cpp:152-179`).
VERDICT: approve
