Committed `1d439c40b` on the detached HEAD (no branch, no push). It touches only `tests/cases/` (130 files) and adds 101 cases, for 1,095 in total.

**Tests run (Linux only):**
- **Main's `jw-check`** (built from `5ffe074f2`): 1,039 of 1,095 pass, and all 994 existing cases still pass with the changed runner. The 56 failures are all new PR3 behaviour main doesn't have: `plan`, the `scene_uuids` validator and export drop, and the 4,096-byte number limit.
- **A reference planner in Python** (my reading of the contract, run as a fake `jw-check` on the trees the runner built): every plan case passes. Each alternative expected file matches when the fake switches to that reading, and so does shuffled output. Outputs with a corrupted size or id fail with a JSON pointer.
- **`jw-fuzz-smoke`** over the new fixtures: passes in 2 s.
- **Not run:** Windows, macOS, or the implementation worker's build.

## How the plan cases work
A plan case names a tree file with `tree=NAME`; the runner builds that tree once per run under `${WORK}/plan/NAME`, from scratch. Trees use `file`, `copy`, `link`, `many` and `profiles` lines, with `%XX` escapes and `{N*C}` repeats. Names are created by write-then-rename, so backslashes and other odd bytes survive.

Each tree probes the filesystem before it is built, on top of the `@posix` tag. Where symlinks, case sensitivity, non-UTF-8 names, reserved names or 255-byte names aren't supported, its cases are skipped rather than failed (APFS on macOS, for example).

Output is compared as JSON after a canonical step that the contract leaves open: the order of files inside one profile or among module configs, `dir` given as `<d>` or `basic/profiles/<d>`, and warning order. Before that step, the ids must still be e1, e2, … in order and every `bak` must name an entry. There is also `entries=N` for the 10,000-entry output, and `uids=random`, which checks the UIDs are UUIDv4 and differ on a second run.

## Case list
- **Planner trees:**
  - the full realistic tree from the brief;
  - near misses for every allowlist and omit rule;
  - the collapse at 99, 100 and 101 files, and in folders that also hold stored files;
  - a `.bak` without its main;
  - symlinks: to a file, to a directory, dangling, a loop, and as module, `logs`, `user.ini` and `plugin_manager` folders;
  - unrepresentable names: reserved characters, non-UTF-8, and 512/513-byte paths;
  - case collisions;
  - roots: custom, all five in one directory, files placed under the wrong root, missing roots, `--app-dir`;
  - `scene_uuids` edge cases;
  - identity: same path, rename, tie, a parent claimed twice, empty set, profile rename, unparseable collection, and a parent without `scene_uuids`;
  - 10,000 and 10,001 entries;
  - parent, `--roots` and usage errors.
- **`manifest` and `export`:** `scene_uuids` accepted, unsorted, duplicate, non-string, 256/257 bytes, 100,000/100,001 items, on non-collection entries, and dropped by export.
- **`redact collection`:** ReviewRules counts by type.
- **Part B numbers:** a 4,096/4,097-byte token, 5,000 digits, both claude-17 reproducers, 7-digit and 2^64 exponents, values around `DBL_MAX`, `1.5e300`, and underflows.
- **Part B INI:** a section name over 1,024 bytes alone triggers the fallback; one of exactly 1,024 bytes does not.

## Ambiguities
**Either reading accepted (one expected file per reading):**
- The size and count of a collapse in a folder with stored files: unknown files only, or all of the folder's files.
- A lone collection `.bak`: no collection UID, or a UID whose path is the `.bak` path or the main path.
- A symlink inside a folder excluded as a whole: just left out of the size, or also listed.
- A folder with an unrepresentable name: its files' size counted, or 0.
- The profile collision warning: names the two `basic.ini` paths or the two folders.
- `1e0000001` and `0e1000000`: finite values, or overflow under a literal reading of the clamp.
- The long-section count: 2 or 3.
- A missing TREE_DIR: an empty plan or `ERR E_IO`.
- `--roots` file errors: any ERR code.

**Pinned by me where the contract is silent — please confirm or overrule:**
- `cache` is the three module folders; `runtime` is `logs`, `crashes`, `profiler_data`, `updates` and `.sentinel`.
- Only a collapse has `count`, not other whole-folder entries.
- A symlink entry has size 0, and a symlink beats every other reason.
- `load_rule` appears only when the `.bak` exists.
- Only new UIDs use up the `--uid-seed` counter, collections before profiles.
- Omitted entries are sorted by path bytes across all roots.
- `global.ini`, `themes/` and files under the wrong root are not listed.
- Name matching is exact and case-sensitive.
- `.json` and `Trail..json` are unknown files, not unrepresentable ones.
- Planned entries carry no `scene_uuids`, and `collision` appears only when true.
- `scene_uuids` uses `id` literally, as the contract says.
- The minus sign counts toward the 4,096 bytes.

**Spec issues:**
- §15 says `notes .txt` fails PathRules, but it passes because the name ends in "t". I test it as a normal file and use `notes.txt ` (trailing space) for the rejected case.
- Main already counts collection sources by type (`versioned_id`, else `id`), so the ReviewRules cases pass on main. They are regression cases, not new behaviour.

## Not covered
- Real runs on Windows and macOS. A tree on HFS+ would also break the `Café.json` expectations, because HFS+ rewrites names to NFD.
- Inputs the contract doesn't define: FIFOs and devices, stray files directly in `basic/profiles/`, `plugin_config/` or `obs-studio/`, subfolders in profile and scene folders, a profile folder without `basic.ini`, a collection over 64 MiB, a custom root that doesn't exist, unknown `--roots` keys, three-way collisions, and a parent claimed by path and tied at once.
- Names containing `;` or `[` (CMake limits), and backslashes in folder names.
- `child` carrying `scene_uuids` over from a parent.

The authoring tools (generator, reference planner, fake `jw-check`) are in `/tmp/jw-plan-tools/` and are not committed.
