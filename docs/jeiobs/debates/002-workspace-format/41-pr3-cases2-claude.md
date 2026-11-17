Committed `cffa78221` on the detached HEAD (not pushed). It touches only `tests/cases/` (77 files): 39 new cases, 1,134 in total, and three alternative expected files removed.

**Tests run (Linux only):**
- **My reference planner** (my reading of r2, run as a fake `jw-check` on the trees the runner builds): all 1,134 cases pass, and they also pass in each of the 7 alternative-reading modes. Outputs I corrupted on purpose fail, including in the `<any>` cases.
- **PR-head `jw-check`** (built from 21489bd03): 1,103 pass. Each of the 31 failures is behaviour r2 adds, so each new case catches the bug it targets. Claude-1's name-order bug reproduces (the copy that sorts before `Changed` takes `Changed`'s uid). Both scene-UUID cases time out at 120 s on the quadratic scan.
- **`jw-fuzz-smoke`** over the new fixtures: passes.
- **Not run:** Windows, macOS, or the r2 implementation.

**What changed, by r2 item**
1. **Identity:** a new parent fixture and four trees. A copy sorting before and one sorting after `Changed`; X keeps its uid by path and does not compete with Alpha for Y's uid; a tie where one of the tied parent collections is already claimed by path.
2. **themes:** one exact-match tree (a profile, a collection, a module folder and files named themes; the config `themes/` is not listed). Three pattern-match cases check only that themes subfolders are listed, because the contract doesn't say how subfolders appear.
3. **Symlinked roots:** owned subpaths across two root layouts, plus a symlinked app dir. `symlinks.json` now expects a root-symlink warning for `plugin_manager`.
4. **Unreadable folders:** directories lose read access only while each case runs. A probe skips these cases on Windows and when running as root. A profile folder, `basic/scenes`, a listed module folder and `plugin_manager` are covered, plus excluded folders, nested folders and an app dir.
5. **`count`:** 12 manifest cases, export keeps it, and `child` treats it as a known field.
6. **Scene UUIDs:** 100,000 and 100,001 generated (the expected output is generated too), and claude-6's 330,000-item reproducer with a 60 s timeout.
7. **Collapse:** recursive counting with subfolders, a symlink inside a collapsed folder's subfolder, and a rejected name in a collapsed folder. The `collapse-mixed` alternative is dropped, since r2.7 settles that reading.
8. **Profiles without `basic.ini`:** several cases, plus an empty folder.
9. **Too large:** a collection of exactly 64 MiB against 64 MiB + 1, user/profile/module files over the limit, and the 256 MiB total. These are real files, not sparse: CMake can't create sparse files portably, so WORK_DIR grows by about 130 MB. The trees hard-link them, so the trees themselves add no disk space.
10. **Lone `.bak`:** the two alternatives are removed.

The runner also gained a few features: `hardlink` and `unreadable` tree lines, `copy` from `${WORK}`, `timeout=` for plan cases, generated expected files, and `"<any>"` placeholders in expected files.

**Readings I pinned (please confirm)**
- An unreadable directory's entry has size 0.
- A folder that isn't a profile gets no `count`.
- Symlinked `logs` and `user.ini` get no root-symlink warning, because they aren't in r2.3's list.
- A too-large omitted entry carries no profile uid.
- A `.bak` next to a too-large main file is stored alone.
- The 256 MiB total is inclusive and counted in plan order.

**Ambiguities: either reading accepted**
- The tie with a claimed parent (PR head gives a new uid).
- An unreadable excluded folder: `unreadable`, or its own reason.
- A rejected name in a collapsed folder: counted in the collapse or not.
- An empty non-profile folder: listed or not.
- A too-large collection's identity: three readings.
- A symlink inside a collapsed folder's subfolder: listed or not.

**Still open — these need a decision from you**
- **App-dir entry path:** which path names a symlinked or unreadable app dir? It's `<any>` for now; an empty path would fail PathRules.
- **How a subfolder inside a walked folder is listed:** the contract doesn't say.
- **Subfolder files and the 100-file threshold:** whether they count toward it is untested.
- **r2.6 entry limit:** I read "collections and profiles count toward the 10,000 limit" as checking the limit early. If `collections[]` and `profiles[]` items count as entries themselves, `plan-limit-entries-10000` would turn into `E_LIMIT`.

**Not covered**
- Windows junctions, unpaired UTF-16 surrogates and errors part-way through a listing; CMake can't produce any of them.
- claude-7's sample truncation with empty subfolders or FIFOs. `plan-collapse-recursive` already passes on PR head, so it only guards against regressions.
- O(n²) lookups and the early limit check: not visible within 10,000 entries.
- A symlinked app dir or `basic/` when all five roots share one directory.
- An unreadable root directory itself.

The authoring tools (reference planner, fake `jw-check`, generator) are in `/tmp/jw-r2-cases/tools/` and are not committed.
