Verified the fix: the unrepresentable directory is no longer emitted under its invalid path; aggregation and warnings use the containing folder (`CapturePlanner.cpp:311-335`, `jw_unit.cpp:796-811`). No new findings.

FINDING codex-1 | Major | withdrawn | Lone `.bak` has no UID (`31-pr3-contract.md:103-105`).
FINDING codex-2 | Major | accepted-fix | Listing failures emit `unreadable` plus `listing-error` (`CapturePlanner.cpp:344-356`).
FINDING codex-3 | Major | accepted-fix | Scene-UUID extraction is indexed and capped (`CapturePlanner.cpp:750-809`).
FINDING codex-4 | Minor | withdrawn | Invalid-name symlink ambiguity is resolved by the contract.
FINDING codex-5 | Major | withdrawn | Metadata scanning is separately bounded while manifest entries remain capped (`CapturePlanner.cpp:222-234,1355-1358`).
VERDICT: approve