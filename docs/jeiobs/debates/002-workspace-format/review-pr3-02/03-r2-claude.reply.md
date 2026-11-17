**Verdict: request changes.** The head hasn't changed since round 1. I re-checked my open findings at 5827f23dc and they all still hold. claude-12 (Major: the Windows build breaks) still rules out a self-merge. I also confirmed the defect behind Codex's reopened codex-2. I have no new findings.

**Codex's findings**
- **codex-2: I agree on the defect.**
  - `AddUnrepresentable` drops `summary.error` (`CapturePlanner.cpp:311-312`). It is the only `Summarize` caller that does: `AddSpecial` (`:360-363`) and `UnknownAccumulator::Add` (`:479-482`) both turn the error into an `unreadable` entry plus a `listing-error` warning.
  - So an unrepresentable folder that can't be listed, or fails part way, adds 0 or a partial size to its `names` entry, with no warning. No tree covers this; the folders in `names-reserved.tree:25-27` are all readable.
  - I'd rate it Minor: no stored entry is lost and the `names` entry is still there. Only its `size` is understated, and the warning is missing. The severity is Codex's call.
  - A fix in line with r2.4: keep the `names` entry and add a `listing-error` warning for the folder that contains it. Fixing claude-15 also removes the walks at the app dir and `basic/`, which run up to five times in the default layout and can each hit this error.
- **codex-3:** I agree with Codex's acceptance. `:744-803` is linear and capped at 100,000.
- **codex-1, codex-4:** both withdrawn; nothing to add.

**My open findings (code unchanged)**
- **claude-12 (Major): still holds.**
  - `Fs.cpp:22-24` includes `<windows.h>` without `NOMINMAX`. The library's only definition of it is `jw-check.cpp:593`.
  - `numeric_limits<…>::max()` at `Fs.cpp:84` and `:317` then hits the `max` macro. MSVC reports C4003, which `/W4 /WX` (`CMakeLists.txt:39`) turns into an error, then C2589.
  - CI configures the library on its own (`jeiobs-workspace.yaml:40`), so the `NOMINMAX` in `cmake/windows/compilerconfig.cmake:80` doesn't apply.
  - Fix: define `NOMINMAX` before the include, or write `(std::numeric_limits<…>::max)()`. Then get a green windows-2022 run.
- **claude-7:** the sample is capped at 101 (`:487`) but the collapse counts only regular files (`:296`). So 150 empty subfolders in `basic/scenes` produce only 101 entries.
- **claude-9:** `WORKSPACE_FORMAT.md` is unchanged at §4 (`:90-96`), the export drop list (`:172-174`), the §7 omitted table (`:300-307`) and §13 (`:610`). §8 still lacks the rule for adding a type. Contract line `:103` still contradicts `cases.txt:1341`.
- **claude-13:** `:962-973` still requires `basic.ini` to be a regular file, but OBS opens it through a link (`OBSBasic_Profiles.cpp:356-360`).
- **claude-14:** `Fs.cpp:257-258` still treats every lstat error as `missing`, and `:337`/`:580` drop those silently. `:275-278` still fails the whole folder when one entry vanishes.
- **claude-15:** `:571-574` and `:583` still list unrepresentable names under the app dir and `basic/`.
- **claude-16:** `:245-266` still scan every omitted entry on each call. Only `:841` sorts, so `:589`, `:620`, `:911` and `:948` use up the 256 MiB budget in directory order.

FINDING claude-1 | Major | accepted-fix | two-pass identity (CapturePlanner.cpp:1134-1173); pass 2 sees only unmatched collections; order cases pass
FINDING claude-2 | Major | accepted-fix | themes skip removed from ProcessDirectory; only the config root ignores unowned names (CapturePlanner.cpp:642-663)
FINDING claude-3 | Major | accepted-fix | linked app dir and owned subpaths give a symlink entry and root-symlink warning; reparse points map to Symlink (Fs.cpp:52-75); Windows build break tracked as claude-12
FINDING claude-4 | Major | accepted-fix | open, entry and increment errors fail the folder as unreadable with RestoreState; per-name conversion (Fs.cpp:267-294); residuals in claude-14
FINDING claude-5 | Major | accepted-fix | count validated (Manifest.cpp:853-857) and known to export and CarryOver (:1131, :1259); doc gap in claude-9
FINDING claude-6 | Minor | accepted-fix | ReviewRules linear with a discriminating test; collections and profiles counted; maps replace find_if and per-pair strings
FINDING claude-7 | Minor | open | sample capped at 101 (CapturePlanner.cpp:487) while the collapse counts regular files only (:296): over 101 empty subfolders or FIFOs drop silently, against r2.7
FINDING claude-8 | Minor | accepted-fix | basic/profiles folders without basic.ini are one unknown dir entry with no UID (CapturePlanner.cpp:954-973)
FINDING claude-9 | Minor | open | WORKSPACE_FORMAT.md sections 4, 7, 8, 13 still stale (scene_uuids, count, names, unrepresentable, unreadable, too-large); contract r1.2 (:103) still denies the profile UID on a lone .bak
FINDING claude-10 | Minor | accepted-fix | cases.txt:1342 accepts only bak-alone.json
FINDING claude-11 | Minor | accepted-fix | CanStore runs before any read (CapturePlanner.cpp:378,684)
FINDING claude-12 | Major | open | Fs.cpp:23 includes windows.h without NOMINMAX, so numeric_limits max() at Fs.cpp:84,317 fails under MSVC /W4 /WX in the standalone Windows CI
FINDING claude-13 | Minor | open | a symlinked basic.ini turns the profile into one unknown dir entry with the link unreported (CapturePlanner.cpp:962-973); OBS follows the link
FINDING claude-14 | Minor | open | every lstat error on an app dir counts as missing and is dropped silently (Fs.cpp:257-258, CapturePlanner.cpp:337,580); an entry vanishing mid-listing fails the whole folder (Fs.cpp:275-278)
FINDING claude-15 | Minor | open | ProcessOwnedRoot lists unrepresentable names under the app dir and basic/ (CapturePlanner.cpp:571-583), against r1.4; up to five entries with path obs-studio in the default layout
FINDING claude-16 | Minor | open | AddOmitted merge scans are O(n^2) (CapturePlanner.cpp:245-266, r2.6); only basic/scenes is sorted (:841), so the 256 MiB budget follows directory order for profiles and modules (r3)
VERDICT: request changes
