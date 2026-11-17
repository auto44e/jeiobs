**Verdict: request changes.** The head is unchanged (2c3cb7790), so claude-17 is still an open Major. I disagree with codex-5.

**claude-17 still holds (Major)**
- `CapturePlanner.cpp:318` records `child.relative_path`, which is built at `:518`. That is the name that just failed PathRules at `:524`. `jw_unit.cpp:789-812` pins this as `"basic/scenes/bad?"`.
- **How it triggers:** on Linux or macOS, any folder under an owned path whose name breaks a path rule (`a:b`, `bad?`, a trailing dot) or isn't UTF-8, when a listing at or below it fails (for example chmod 000).
- **Non-UTF-8 names:** on POSIX, `u8string()` passes the raw bytes through (`Fs.cpp:288`), and `ValidatePath` flags them `not-utf8` (`PathRules.cpp:160-161`). So `dump()` throws (`jw-check.cpp:529`) and the whole plan becomes `ERR E_JSON` (`:586-587`).
- **Windows:** a name with an unpaired surrogate has an empty `name` (`Fs.cpp:287-291`), so `:315` skips it. Only names that break a path rule hit this on Windows.
- **To Codex:** the codex-2 fix you accepted (`:315-321`) contains the line in question. Accepting codex-2 doesn't clear `:318`.
- **Fix (unchanged):** keep the `names` entry and add only a `listing-error` warning at `container`. Don't add an `unreadable` entry there, because it would share the `names` entry's path and fail with `E_COLLISION` (`Manifest.cpp:794-797`).

**codex-5: I disagree that this is a defect**
- **The limit counts manifest entries only.** §12 says "10,000 per manifest" (`WORKSPACE_FORMAT.md:538`), and the validator counts only `entries` (`Manifest.cpp:755`). `collections[]` and `profiles[]` are plan-only data (r1.1, contract `:101-102`).
- **Collection and profile files already count:**
  - Collection files become entries through `AddStored` (`:1269-1272`, check at `:212`), and the total is checked again at `:1356`.
  - Profile files count as they are added (`:383`, `:907`).
- **The separate cap can't change the outcome.**
  - Every collection or profile yields at least one entry: the main or a lone `.bak` (`:1236-1267`), and for a profile `basic.ini` stored, `too-large`, or its folder's `unreadable` entry.
  - So the cap at `:222-234` fires only when entries would pass 10,000 anyway. It just bounds the scan.
- **Summing would reject valid captures.** 6,000 collections and nothing else make 6,000 entries, a valid manifest, but count as 12,000 under the summed reading.
- **Where the clause came from:** my run-01 claude-6 (`review-pr3-01/01-r1-claude.reply.md:47`): "`collections_` and `profiles_` don't count toward the 10,000-entry limit during the scan. Every `.json` is read before the check…". That asked for a bounded scan, which `:222-234` provides. At most the contract wording needs a fix, along the lines of "their files count; the scan stops at 10,000 collections plus profiles".

**My other findings**
- claude-13 and claude-14 are unchanged (`CapturePlanner.cpp:968-979`; `Fs.cpp:263-264` with `CapturePlanner.cpp:346,588`). They are Minors, so follow-ups that don't block a self-merge.
- claude-12 is accepted in code. I can't see from this checkout whether the Windows/macOS CI runs for 2c3cb7790 are green, and the merge needs them.

FINDING claude-1 | Major | accepted-fix | two-pass identity; unchanged since acceptance
FINDING claude-2 | Major | accepted-fix | themes skip only under the config root
FINDING claude-3 | Major | accepted-fix | symlinked roots and owned subpaths give a symlink entry plus root-symlink
FINDING claude-4 | Major | accepted-fix | listing errors make the folder unreadable, with RestoreState
FINDING claude-5 | Major | accepted-fix | count validated, exported and carried over
FINDING claude-6 | Minor | accepted-fix | ReviewRules linear; maps replace scans; scan bounded by the metadata cap (:222-234)
FINDING claude-7 | Minor | accepted-fix | sample uncapped (CapturePlanner.cpp:496-504); jw_unit.cpp:828-844
FINDING claude-8 | Minor | accepted-fix | folder without basic.ini is one unknown dir entry
FINDING claude-9 | Minor | accepted-fix | WORKSPACE_FORMAT.md:94-103, :617, :698-700; cases.txt:1340-1341
FINDING claude-10 | Minor | accepted-fix | cases.txt accepts only bak-alone.json
FINDING claude-11 | Minor | accepted-fix | CanStore runs before any read
FINDING claude-12 | Major | accepted-fix | NOMINMAX/WIN32_LEAN_AND_MEAN before windows.h (Fs.cpp:22-30); merge still needs green Windows/macOS CI on 2c3cb7790
FINDING claude-13 | Minor | open | symlinked basic.ini still makes the profile an unknown dir with the link unreported (CapturePlanner.cpp:968-979)
FINDING claude-14 | Minor | open | every lstat error is still "missing" (Fs.cpp:263-264) and dropped silently for app dirs and basic/ (CapturePlanner.cpp:346,588)
FINDING claude-15 | Minor | accepted-fix | owned roots ignore unrepresentable names (CapturePlanner.cpp:580-582)
FINDING claude-16 | Minor | accepted-fix | indexed merges (:246-270); budget after the plan-order sort (:1360-1380)
FINDING claude-17 | Major | open | unreadable unrepresentable folder is recorded under its own name (CapturePlanner.cpp:318; pinned by jw_unit.cpp:789-812 "basic/scenes/bad?"): breaks rule 4 and PathRules (Manifest.cpp:785-790); raw non-UTF-8 bytes on POSIX (Fs.cpp:288) make the plan ERR E_JSON (jw-check.cpp:529,586)
VERDICT: request changes
