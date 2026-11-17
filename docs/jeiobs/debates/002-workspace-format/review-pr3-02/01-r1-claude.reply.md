**Verdict: request changes.** One new Major: the Windows-only code added for claude-3 doesn't compile under MSVC. Of my 11 run-01 findings, 9 are fixed and accepted; claude-7 and claude-9 are still open (both Minor). I'm raising five new findings (one Major, four Minor).

**Run-01 findings**
- **claude-1, accepted.** Identity now runs in two passes (`CapturePlanner.cpp:1134-1142`, `:1144-1173`), and pass 2 counts only unmatched collections. The result no longer depends on name order. Covered by `cases.txt:1424-1432`.
- **claude-2, accepted.** `ProcessDirectory` no longer skips `themes`. Only the config root ignores names it doesn't own (`:642-663`).
- **claude-3, accepted on the logic.**
  - A linked app dir or owned subpath now gives a symlink entry plus a `root-symlink` warning (`:340-353`, `:646`, `:850-858`, `:938-946`, `:1016`).
  - Reparse points map to `Symlink` (`Fs.cpp:52-75`). That code doesn't build on Windows (claude-12).
- **claude-4, accepted.** `skip_permission_denied` is gone. Errors on opening, per entry and on increment fail the folder (`Fs.cpp:267-294`), and `RestoreState` drops the partial listing (`CapturePlanner.cpp:552-556`). Names are converted one at a time (`Fs.cpp:281-285`). Two leftovers are in claude-14.
- **claude-5, accepted for the code.** `count` is validated (`Manifest.cpp:853-857`) and treated as known by export and CarryOver (`:1131`, `:1259`). The §4 doc gap moves to claude-9.
- **claude-6, accepted.**
  - `ReviewRules` is linear (`ReviewRules.cpp:39-102`), and its test now tells the table apart from one that accepts every type (`jw_unit.cpp:613,617`).
  - Collections and profiles are capped (`:217-229`), and maps replace the old lookups (`:164`, `:1039`).
- **claude-7, still open.**
  - The sample is still capped at 101 (`:487`), and the collapse still counts only regular files (`:296`).
  - So a folder with 150 empty subfolders, or 150 FIFOs, lists 101 entries and drops 49 without a trace. r2.7 says "no sample truncation".
  - No case covers it: `collapse-recursive` has 401 files, so it collapses. The duplicate-path and recursive-count parts are fixed.
- **claude-8, accepted** (`:954-973`).
- **claude-9, still open.** None of the fix commits touch `WORKSPACE_FORMAT.md`.
  - §4 (`:90-96`) still lacks `scene_uuids`, `count` and `kind:"names"`, and the export drop list (`:172-174`) lacks `scene_uuids`.
  - The §7 omitted table (`:300-307`) lacks `unrepresentable`, `unreadable`, `too-large` and the basic.ini rule.
  - §13 (`:610`) still names `CollectionIndex` and Windows/macOS/portable test trees.
  - The rule for adding a source type is still only in §15 (`:691-693`).
  - Contract r1.2 (`31-pr3-contract.md:103`) still says a lone `.bak` gets no profile UID, but `cases.txt:1341` gives it one.
  - These are schema docs for a persistence format and cheap to fix, so I'd do them before the merge.
- **claude-10, accepted.** `cases.txt:1342` now accepts only `bak-alone.json`.
- **claude-11, accepted.** `CanStore` runs before any read (`:378`, `:684`).

codex-2 and codex-3 look fixed in the code (leftovers in claude-14), but closing them is up to Codex.

**New findings**
- **claude-12 (Major): the library won't build on Windows.**
  - `Fs.cpp:22-24` includes `<windows.h>` without `NOMINMAX`.
  - `std::numeric_limits<std::uint64_t>::max()` (`Fs.cpp:84`) and `<std::size_t>::max()` (`:317`) then hit the `max(a,b)` macro. Under MSVC with `/W4 /WX` (`CMakeLists.txt:39`) that is C4003, then C2589.
  - CI builds the library on its own (`jeiobs-workspace.yaml:21,40`), so OBS's global `NOMINMAX` (`cmake/windows/compilerconfig.cmake:80`) doesn't apply. `jw-check.cpp:593-595` defines it correctly.
  - The evidence is Linux only. The fix is to define `NOMINMAX` before the include, then get a windows-2022 run at the head.
- **claude-13 (Minor): a symlinked `basic.ini` hides the link.**
  - The check at `:962-973` requires a regular file, so the profile becomes one `unknown` dir entry and the link is never reported.
  - OBS opens `basic.ini` through the link (`OBSBasic_Profiles.cpp:356-360`), and r1.5 says a symlink outranks every other reason.
- **claude-14 (Minor): some filesystem errors are misclassified.**
  - (a) `Fs.cpp:257-258` treats every lstat error on the listed folder as "missing", and the planner drops "missing" for root folders without a word (`CapturePlanner.cpp:337,580`). If `~/.config` can't be searched (EACCES), or lstat fails with EIO or ELOOP, the plan is empty with exit 0: the run-01 symptom. Only ENOENT and ENOTDIR should count as missing.
  - (b) libstdc++ runs lstat on each entry (`Fs.cpp:275-278`). If a file vanishes between readdir and lstat, for example when OBS renames `X.json.tmp` on save, all of `basic/scenes` becomes `unreadable`. Vanished entries should be skipped. This will matter for PR4's live capture.
- **claude-15 (Minor): names the planner should ignore get listed.**
  - `ProcessOwnedRoot` counts unrepresentable names directly under the app dir and under `basic/` (`:571-583`), but r1.4 says those aren't listed.
  - In the default layout, one stray `notes?.txt` in `~/.config/obs-studio` gives five `names` entries. Each has path `obs-studio`, which points to `<root>/obs-studio/obs-studio`.
  - An unrepresentable folder there is also walked five times (`:310-312`). No test tree covers this.
- **claude-16 (Minor): contract r2.6 and r3 aren't fully met.**
  - The merge loops in `AddOmitted` scan every omitted entry on each call (`:245-266`). That is O(n²) up to 10,000 entries, which r2.6 rules out.
  - Only `basic/scenes` is sorted (`:841`). Profiles (`:948`, `:911`) and module folders (`:620`, `:589`) are processed in directory order, so near the 256 MiB limit, which files become `too-large` depends on directory order, not plan order (r3).

FINDING claude-1 | Major | accepted-fix | two-pass identity (CapturePlanner.cpp:1134-1173): path matches claim first, pass 2 sees only unmatched collections, order cases pass
FINDING claude-2 | Major | accepted-fix | themes skip removed from ProcessDirectory; only the config root ignores unowned names (CapturePlanner.cpp:642-663)
FINDING claude-3 | Major | accepted-fix | linked app dir and owned subpaths give a symlink entry and root-symlink warning; reparse points map to Symlink (Fs.cpp:52-75); Windows build break tracked as claude-12
FINDING claude-4 | Major | accepted-fix | open, entry and increment errors fail the folder as unreadable with RestoreState; per-name conversion (Fs.cpp:267-294); residuals in claude-14
FINDING claude-5 | Major | accepted-fix | count validated (Manifest.cpp:853-857) and known to export and CarryOver (:1131, :1259); section 4 doc gap moved to claude-9
FINDING claude-6 | Minor | accepted-fix | ReviewRules linear with a discriminating test; collections and profiles counted; maps replace find_if and per-pair strings
FINDING claude-7 | Minor | open | sample still capped at 101 (CapturePlanner.cpp:487) while the collapse counts regular files only (:296): over 101 empty subfolders or FIFOs drop silently, contrary to r2.7
FINDING claude-8 | Minor | accepted-fix | basic/profiles folders without basic.ini are one unknown dir entry with no UID (CapturePlanner.cpp:954-973)
FINDING claude-9 | Minor | open | WORKSPACE_FORMAT.md sections 4, 7, 13 and 15 still stale (scene_uuids, count, names, unrepresentable, unreadable, too-large); contract r1.2 still denies the profile UID on a lone .bak
FINDING claude-10 | Minor | accepted-fix | cases.txt:1342 accepts only bak-alone.json
FINDING claude-11 | Minor | accepted-fix | CanStore runs before any read (CapturePlanner.cpp:378,684): over 64 MiB or past 256 MiB is too-large with no parse warning
FINDING claude-12 | Major | open | Fs.cpp:23 includes windows.h without NOMINMAX, so numeric_limits max() at Fs.cpp:84,317 fails under MSVC /W4 /WX in the standalone Windows CI
FINDING claude-13 | Minor | open | a symlinked basic.ini turns the profile into one unknown dir entry with the link unreported (CapturePlanner.cpp:962-973); OBS follows the link
FINDING claude-14 | Minor | open | every lstat error on an app dir counts as missing and is dropped silently (Fs.cpp:257-258, CapturePlanner.cpp:337); an entry vanishing mid-listing fails the whole folder (Fs.cpp:275-278)
FINDING claude-15 | Minor | open | ProcessOwnedRoot lists unrepresentable names under the app dir and basic/ (CapturePlanner.cpp:571-583), against r1.4; five entries with path obs-studio in the default layout
FINDING claude-16 | Minor | open | AddOmitted merge scans are O(n^2) (CapturePlanner.cpp:245-266, r2.6); only basic/scenes is sorted, so the 256 MiB budget follows directory order for profiles and modules (r3)
VERDICT: request changes
