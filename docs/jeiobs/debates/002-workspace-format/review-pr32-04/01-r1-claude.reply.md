**Verdict: approve.** The claude-17 fix is correct, and I have no open Blocker or Major left.

**claude-17: fix accepted**
- **The name is never recorded now.** When the folder can't be listed, `CapturePlanner.cpp:317-320` adds only a `listing-error` warning at `container`. The `names` entry stays, with the folder in its `count` but not in its `size`, so no partial size goes in.
- **`container` is always representable.** It is the caller's `relative`, and that has already passed the name check: `:525` filters children before any handler runs, and `ProcessOwnedRoot` ignores bad names (`:581-582`).
- **No collision.** No `unreadable` entry is added at `container`, so it can't collide with the `names` entry.
- **A later failure on the container is handled.** If the container's own listing then fails, `RestoreState` (`:565`) drops this warning and `:566` records the container as `unreadable`.
- **Non-UTF-8 names on POSIX are covered too.** They take the same branch at `:525`, so the raw bytes never reach `dump()`. The extra non-UTF-8 test I asked for isn't needed.
- **The test pins the new behaviour.** `jw_unit.cpp:796-811` expects `names` at `basic/scenes` with count 1 and size 0, a `listing-error` at `basic/scenes`, and no entry or warning at `basic/scenes/bad?`.
- **Two nits, not findings:**
  - `WORKSPACE_FORMAT.md:98-99` ("A folder that cannot be listed is an entry with `reason: "unreadable"`") doesn't mention this exception for unrepresentable names.
  - Two such folders in one container give two identical warnings, because warnings are sorted but not de-duplicated (`:1326-1328`).

**claude-13 and claude-14: still open (Minor)**
- The code is unchanged: `CapturePlanner.cpp:969-979` for claude-13, and `Fs.cpp:263-264` with `CapturePlanner.cpp:347,589` for claude-14.
- The ledger marks both "open → fixed (awaiting finder)" (`findings.md:230-231`), but `:214` says they are carried to S1-PR4. They are deferred, not fixed, so I keep them open. As Minors they don't block a self-merge.

**Before merging:** 0190edd89 changed code. Under `DECISIONS.md:100-104`, the Windows and macOS CI runs need to be green at the head being merged, 67cc0488c. The passes reported in the context are for earlier heads.

FINDING claude-1 | Major | accepted-fix | two-pass identity; unchanged since acceptance
FINDING claude-2 | Major | accepted-fix | themes skip only under the config root
FINDING claude-3 | Major | accepted-fix | symlinked roots and owned subpaths give a symlink entry plus root-symlink
FINDING claude-4 | Major | accepted-fix | listing errors make the folder unreadable, with RestoreState
FINDING claude-5 | Major | accepted-fix | count validated, exported and carried over
FINDING claude-6 | Minor | accepted-fix | ReviewRules linear; maps replace scans; scan bounded by the metadata cap (:222-234)
FINDING claude-7 | Minor | accepted-fix | sample uncapped (CapturePlanner.cpp:496-504); jw_unit.cpp:826-842
FINDING claude-8 | Minor | accepted-fix | folder without basic.ini is one unknown dir entry
FINDING claude-9 | Minor | accepted-fix | WORKSPACE_FORMAT.md:94-103, :617, :698-700; cases.txt:1340-1341
FINDING claude-10 | Minor | accepted-fix | cases.txt accepts only bak-alone.json
FINDING claude-11 | Minor | accepted-fix | CanStore runs before any read
FINDING claude-12 | Major | accepted-fix | NOMINMAX/WIN32_LEAN_AND_MEAN before windows.h (Fs.cpp:22-30); merge needs green Windows/macOS CI at 67cc0488c
FINDING claude-13 | Minor | open | carried to S1-PR4, not fixed: symlinked basic.ini still makes the profile an unknown dir with the link unreported (CapturePlanner.cpp:969-979)
FINDING claude-14 | Minor | open | carried to S1-PR4, not fixed: every lstat error is still "missing" (Fs.cpp:263-264), dropped silently for app dirs and basic/ (CapturePlanner.cpp:347,589)
FINDING claude-15 | Minor | accepted-fix | owned roots ignore unrepresentable names (CapturePlanner.cpp:581-582)
FINDING claude-16 | Minor | accepted-fix | indexed merges (:246-270); budget after the plan-order sort (:1361-1381)
FINDING claude-17 | Major | accepted-fix | only a listing-error at the container, names entry kept, no own-name entry or warning (CapturePlanner.cpp:317-320); pinned by jw_unit.cpp:796-811
VERDICT: approve
