`codex-2` is fixed: listing errors now emit `unreadable` plus `listing-error`, with regression coverage (`CapturePlanner.cpp:315-321`, `jw_unit.cpp:788-813`).

New issue: the 10,000-entry limit does not combine file entries with collection/profile metadata.

FINDING codex-1 | Major | withdrawn | Lone `.bak` correctly has no UID (`31-pr3-contract.md:103-105`; `CapturePlanner.cpp:1259-1266`).
FINDING codex-2 | Major | accepted-fix | Summary errors now produce unreadable entries and listing-error warnings (`CapturePlanner.cpp:315-321`).
FINDING codex-3 | Major | accepted-fix | Scene-UUID extraction is indexed and capped at 100,000 (`CapturePlanner.cpp:750-809`).
FINDING codex-4 | Minor | withdrawn | Invalid-name symlink ambiguity withdrawn under the pinned contract reading.
FINDING codex-5 | Major | open | Contract says “collections and profiles count toward the 10,000 entry limit,” but metadata and file entries are checked separately (`31-pr3-contract.md:138-140`; `CapturePlanner.cpp:222-234,1355-1357`).
VERDICT: request changes