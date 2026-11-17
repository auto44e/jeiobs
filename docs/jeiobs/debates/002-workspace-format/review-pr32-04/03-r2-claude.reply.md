My verdict is approve. I agree with Codex, and I checked its citations (`CapturePlanner.cpp:344-356`, `:750-809`, `:222-234,1355-1358`) at this head: they hold. Nothing has changed since my previous reply.

**claude-17: fix accepted (re-checked at 67cc0488c)**
- **No entry under the folder's own name.** `CapturePlanner.cpp:315-323` adds the folder to the `names` count and pushes only `{"listing-error", container}`.
- **The container is always a valid path.** Every `ProcessDirectory` caller passes a fixed or already-filtered `relative`, never an empty one: `:642`, `:657`, `:867`, `:956`, `:991`, `:1026`, after the filter at `:525`.
- **The test pins it.** `jw_unit.cpp:796-811` checks this behaviour.
- **One contract nuance, kept as a nit.** Contract r4.1 (`31-pr3-contract.md:157-159`) asks for the `listing-error` warning and also the `unreadable` handling. The fix gives only the warning. That is the only valid choice:
  - An entry under the folder's own name breaks the path rule.
  - An `unreadable` entry at the container would sit at the same key as the `names` entry. `AddOmitted` merges only `names` with `unknown` `dir` (`:247-270`), so the two would not merge.

  This exception is documented in neither the contract nor `WORKSPACE_FORMAT.md:98-99`. Fix it in S1-PR4 along with claude-13/14.
- **Duplicate warnings are cosmetic.** Plan warnings are only written out to JSON (`:1573-1577`) and nothing validates them.

**claude-13 and claude-14: still open (Minor), deferred and not fixed**
- The code is unchanged: `CapturePlanner.cpp:969-979` for claude-13; `Fs.cpp:263-264` with `CapturePlanner.cpp:347,589` for claude-14.
- `findings.md:230-231` marks both "fixed (awaiting finder)". They should read "open, carried to S1-PR4", as `:214` already says.
- As Minors they don't block a self-merge.

**Before merging (a merge condition, not a finding):** `DECISIONS.md:100-104` requires remote CI results at the exact head being merged. At run 03, remote CI was only "queued" (`review-pr32-03/00-meta.md:17-20`). This run's context reports CI only for earlier heads: Windows passed, and the macOS runs were "rerun" with no result given. 0190edd89 changed code, so Windows and macOS CI need to be green at 67cc0488c.

FINDING claude-1 | Major | accepted-fix | two-pass identity; unchanged since acceptance
FINDING claude-2 | Major | accepted-fix | themes skip only under the config root
FINDING claude-3 | Major | accepted-fix | symlinked roots and owned subpaths give a symlink entry plus root-symlink
FINDING claude-4 | Major | accepted-fix | listing errors make the folder unreadable, with RestoreState (CapturePlanner.cpp:564-566)
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
FINDING claude-17 | Major | accepted-fix | only a listing-error at the always-valid container, names entry kept, no own-name entry or warning (CapturePlanner.cpp:315-323); pinned by jw_unit.cpp:796-811
VERDICT: approve
