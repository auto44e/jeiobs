**Verdict: request changes.** Most of the r4 fixes hold, but the codex-2 fix adds a new Major (claude-17): an unreadable folder with an unrepresentable name now puts that name into the plan.

**New: claude-17 (Major)**
- **Where:** `AddUnrepresentable` calls `AddListingFailure(root, child.relative_path, …)` (`CapturePlanner.cpp:318`). `relative_path` is `JoinPath(relative, entry.name)` (`:518`), the very name that failed PathRules at `:524`.
- **The test pins it:** `jw_unit.cpp:789-812` expects an `unreadable` entry and a `listing-error` warning at `"basic/scenes/bad?"`.
- **What it breaks:**
  - Contract rule 4 says "no names recorded" (`31-pr3-contract.md:56-59`; `WORKSPACE_FORMAT.md:97-98`).
  - A planned entry is a manifest entry without `hash` (contract `:34`). The validator runs `ValidatePath` on every entry, omitted ones included (`Manifest.cpp:785-790`), so this plan can never become a valid manifest.
  - With a non-UTF-8 folder name on POSIX, `PlanToJson(...).dump()` (`jw-check.cpp:529`, strict UTF-8) throws. The catch at `:586-587` then turns the whole plan into `ERR E_JSON`.
- **When:** the folder sits anywhere the planner walks and any listing under it fails; `Summarize` recurses. No black-box case covers this; `names-*` (`cases.txt:1358-1364`) only use readable folders.
- **Fix:**
  - Keep the `names` entry and add one `listing-error` warning at `container`, which is representable and currently unused (`:326`).
  - Don't add an `unreadable` entry at `container` either: it would share a path with the `names` entry and fail with `E_COLLISION` (`Manifest.cpp:794-797`).
  - Update `jw_unit.cpp:797-812`, and add a non-UTF-8 case that asserts `dump()` succeeds.

**My earlier findings**
- **claude-7: accepted.** The sample is no longer capped (`:496-504`); 150 empty folders give 150 entries (`jw_unit.cpp:828-844`).
- **claude-9: accepted.**
  - §4 now covers the new fields (`WORKSPACE_FORMAT.md:94-103`); §13 (`:617`) and §15 (`:698-700`) cover the table and the rule for adding a type.
  - The lone-`.bak` profile reading is in `cases.txt:1340-1341`.
  - Nit: the §7 omitted table (`:307-314`) and the export drop list (`:179-181`) don't repeat these.
- **claude-12: accepted in code.** `Fs.cpp:22-30` defines both macros before `windows.h`, and it is the library's only `windows.h` include. Merge still needs green windows-2022 and macOS runs on 2c3cb7790.
- **claude-13: still open (Minor).** `:968-979` is unchanged: a symlinked `basic.ini` still turns the profile into an `unknown` dir entry, and the link isn't reported.
- **claude-14: still open (Minor).**
  - `Fs.cpp:263-264` still maps every lstat error to `missing`, and `:346`/`:588` drop it for app dirs and `basic/`. For example, a root whose parent folder can't be searched is skipped with no entry and no warning.
  - I drop the half about an entry vanishing mid-listing: that is now reported.
  - Fix: return `missing` only when the type is `file_type::not_found`, and treat every other error as a listing error.
- **claude-15: accepted.** `:580-582` ignores these names (`jw_unit.cpp:816-826`).
- **claude-16: accepted.** Merges go through `omitted_indices_` (`:246-270`), and the budget runs after the plan-order sort (`:1360-1380`; `jw_unit.cpp:846-880`).

**Codex's findings:** whether codex-2 closes is Codex's call; its fix is the one that introduces claude-17. Nothing to add on codex-1, codex-3 or codex-4.

FINDING claude-1 | Major | accepted-fix | two-pass identity; unchanged since acceptance
FINDING claude-2 | Major | accepted-fix | themes skip only under the config root
FINDING claude-3 | Major | accepted-fix | symlinked roots and owned subpaths give a symlink entry plus root-symlink
FINDING claude-4 | Major | accepted-fix | listing errors make the folder unreadable, with RestoreState
FINDING claude-5 | Major | accepted-fix | count validated, exported and carried over
FINDING claude-6 | Minor | accepted-fix | ReviewRules linear; maps replace scans
FINDING claude-7 | Minor | accepted-fix | sample uncapped (CapturePlanner.cpp:496-504); jw_unit.cpp:828-844
FINDING claude-8 | Minor | accepted-fix | folder without basic.ini is one unknown dir entry
FINDING claude-9 | Minor | accepted-fix | WORKSPACE_FORMAT.md:94-103, :617, :698-700; cases.txt:1340-1341
FINDING claude-10 | Minor | accepted-fix | cases.txt accepts only bak-alone.json
FINDING claude-11 | Minor | accepted-fix | CanStore runs before any read
FINDING claude-12 | Major | accepted-fix | NOMINMAX/WIN32_LEAN_AND_MEAN before windows.h (Fs.cpp:22-30); merge still needs green Windows/macOS CI
FINDING claude-13 | Minor | open | symlinked basic.ini still makes the profile an unknown dir with the link unreported (CapturePlanner.cpp:968-979)
FINDING claude-14 | Minor | open | every lstat error is still "missing" (Fs.cpp:263-264) and dropped silently for app dirs and basic/ (CapturePlanner.cpp:346,588)
FINDING claude-15 | Minor | accepted-fix | owned roots ignore unrepresentable names (CapturePlanner.cpp:580-582)
FINDING claude-16 | Minor | accepted-fix | indexed merges (:246-270); budget after the plan-order sort (:1360-1380)
FINDING claude-17 | Major | open | unreadable unrepresentable folder is recorded under its own name (CapturePlanner.cpp:318; pinned by jw_unit.cpp:790,812 "basic/scenes/bad?"): breaks rule 4 and PathRules (Manifest.cpp:785-790); non-UTF-8 name makes plan ERR E_JSON (jw-check.cpp:529,586)
VERDICT: request changes
