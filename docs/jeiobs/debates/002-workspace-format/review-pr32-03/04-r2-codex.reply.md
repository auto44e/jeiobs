`codex-5` is withdrawn: metadata is bounded separately, while emitted manifest entries are capped at 10,000; each admitted collection/profile produces an entry (`CapturePlanner.cpp:212-234,687-713,875-991,1355-1357`). Claude’s `claude-17` remains an open Major.

FINDING codex-1 | Major | withdrawn | Lone `.bak` correctly has no UID (`31-pr3-contract.md:103-105`).
FINDING codex-2 | Major | accepted-fix | Listing errors now emit unreadable entries and warnings (`CapturePlanner.cpp:315-321`).
FINDING codex-3 | Major | accepted-fix | Scene-UUID extraction is indexed and capped (`CapturePlanner.cpp:750-809`).
FINDING codex-4 | Minor | withdrawn | Invalid-name symlink ambiguity withdrawn under the contract.
FINDING codex-5 | Major | withdrawn | Separate metadata scan bound is equivalent to the manifest-entry cap (`CapturePlanner.cpp:212-234,1355-1357`).
VERDICT: request changes