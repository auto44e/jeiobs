The listing-error fix is incomplete: `AddUnrepresentable` ignores `Summary.error` at `CapturePlanner.cpp:311-312`, violating the no-partial-plan contract at `31-pr3-contract.md:131-134`.

FINDING codex-1 | Major | withdrawn | Lone collection `.bak` UID issue withdrawn in run 01.
FINDING codex-2 | Major | open | Unreadable invalid-name directories discard `Summary.error` and emit a partial successful plan (`CapturePlanner.cpp:311-325`).
FINDING codex-3 | Major | accepted-fix | Scene UUID extraction is now indexed, linear, and capped at 100,000 (`CapturePlanner.cpp:744-803`).
FINDING codex-4 | Minor | withdrawn | Invalid-name symlink issue withdrawn as contract ambiguity.
VERDICT: request changes