# S1-PR3 contract: CapturePlanner, identity and review rules (shared by both workers)

Spec: `docs/jeiobs/WORKSPACE_FORMAT.md` §6 (fidelity, omitted files), §7 (allowlist, `.bak` pairs),
§9 (identity), §12 (limits, path rules), §15 (PR3 open items). Earlier contracts still hold:
`debates/002-workspace-format/04-pr1-contract.md`, `09-pr1-contract-r2.md`, `14-pr2-contract.md`,
`19-pr2-contract-r3.md`. This file wins for PR3.

## Library additions (shared/jeiobs-workspace, C++17, no Qt, no libobs)
- `Fs` interface (abstract): `List(dir) -> entries {name (UTF-8 bytes), kind: file|dir|symlink|other, size}`
  (never follows symlinks), `ReadBounded(path, max) -> bytes|error`. Implementations: `MemFs` (in-memory
  tree, used by unit tests) and `RealFs` (std::filesystem with `symlink_status`; UTF-8 via `u8path`;
  the CLI uses it).
- `CapturePlanner::Plan(fs, roots, parent) -> Plan` and the CLI below. It reads NO file content except
  collection `.json` files (to extract scene UUIDs, bounded by §12 limits) and never writes.
- `UidSource` interface (`NextUid()`); the CLI's `--uid-seed N` gives deterministic UIDs
  `uid-<seed>-0001`, `uid-<seed>-0002`, ... in plan order; the default source is random UUIDv4.
- `Roots`: five roots (`config`, `user`, `scenes`, `profiles`, `plugin_manager`), each a directory
  path in the Fs, plus the `app_dir` name (default `obs-studio`). A root may be `default` or `custom`
  (recorded as such, never as a path). Entry paths are relative to `<root>/<app_dir>/` as in §7.

## CLI `jw-check plan`
`jw-check plan TREE_DIR [--roots ROOTS.json] [--parent MANIFEST] [--uid-seed N] [--app-dir NAME]`
- TREE_DIR is a synthetic config tree. Without `--roots`: every root is `TREE_DIR/<root-dir>` where the
  five root dirs are `config`, `user`, `scenes`, `profiles`, `plugin_manager` (a missing one is
  skipped, not an error), each containing `<app_dir>/`. With `--roots` (JSON object `{"config":"rel/dir",
  ...}`, each value relative to TREE_DIR) the listed roots use those directories and are recorded
  `custom`; others default.
- stdout: one JSON document (compact, keys sorted), exit 0; `ERR <CODE> [detail]` exit 1; usage exit 2:
```
{"roots":{"config":"default","user":"default","scenes":"custom","profiles":"default","plugin_manager":"default"},
 "entries":[PlannedEntry...], "collections":[{"uid":...,"path":...,"scene_uuids":[...]}],
 "profiles":[{"uid":...,"dir":...}], "warnings":[{"code":"case-collision","path":...} ...]}
```
  `PlannedEntry` = a manifest entry without `hash`: `{"id":"e1","status":"stored","root","path","role",
  "size","fidelity":null,"collection"|"profile":UID,"bak":id,"load_rule":...}` or `{"id","status":"omitted",
  "root","path","size","reason",["kind":"dir"],["count":N]}`. `fidelity` is null in a plan (decided at capture).
  ids are `e1`, `e2`, ... in plan order. Plan order: user.ini, plugin_manager/modules.json, then
  collections (sorted by name bytes), profiles (sorted by dir name), module configs, then omitted
  entries in path order.

## What the planner does (§7)
1. **Allowlist.** Stored entries only for the exact (root, role, path) shapes of the §7 table and the
   PR1 stored-entry allowlist (09-pr1-contract-r2.md item 1). `global.ini` is never stored.
2. **Omitted, with reasons** (`reason`): `unknown` (other files in `basic/scenes/`, a profile folder or a
   listed module folder; other module folders are ONE `kind:"dir"` entry); `out-of-process-writer`
   (`obs-vst3/vst3list.json` and `.bak`); `secret-store` (`obs-browser/`); `cache`/`runtime`
   (`rtmp-services`, `text-freetype2`, `win-capture` module folders; `logs/`, `crashes/`,
   `profiler_data/`, `updates/`, `.sentinel/`); `not-loaded` (`*.json.v1`, `*.json.invalid`, decklink/AJA
   `.bak`); `transient` (`*.tmp`); `symlink` (any symlink anywhere below a root, never followed, never
   listed through); `themes/` is not captured and not listed. A directory with more than 100 unknown
   files collapses into one `kind:"dir"` entry whose `size` is the sum of its regular files (symlinks
   not followed) and `count` the number of files.
3. **`.bak` pairs.** A stored collection or `service.json` and its `.bak` are two entries; the main
   entry has `bak` = the `.bak` entry's id and `load_rule:"bak-fallback"`; encoder JSON mains have
   `load_rule:"mixed"`. A `.bak` without its main is stored alone (no `bak` link).
4. **Unrepresentable names** (§15): a file or folder name that is not valid UTF-8, fails a PathRules rule
   (reserved characters, trailing dot/space, device names, control characters, over-long), or is a
   Windows-reserved name, is NOT an error: it is omitted as ONE entry per containing folder
   `{"reason":"unrepresentable","kind":"names","count":N,"size":S}` with no names recorded.
5. **Case collisions** (same root, ASCII-case-equal paths, e.g. profiles `Main` and `main` on Linux):
   both are planned normally, and a warning `{"code":"case-collision","path":<both paths joined by "|">}` is
   added (restore onto a case-insensitive filesystem must block; PR1 validation of a manifest with such
   paths still rejects, so the planner marks both entries `"collision":true` and the capture step
   (PR4) decides). Entries with `collision:true` are NOT rejected by the planner.
6. **Limits.** More than 10,000 planned entries → `ERR E_LIMIT entries`; any path over the §12 path
   limits is "unrepresentable" (rule 4), not an error.

## Identity (§9)
- Each collection and profile gets a UID. Matching against `--parent MANIFEST` (a local manifest, validated):
  1. same `root` and `path` as a parent entry that has a `collection`/`profile` UID: reuse that UID;
  2. collections only: the same NON-EMPTY set of scene UUIDs as exactly ONE parent collection (the parent
     entry's `scene_uuids`), and not claimed by another current collection: reuse that UID;
  3. otherwise a new UID. Only a unique match counts (a tie, or a parent claimed twice, gives new UIDs).
  Profiles have no UUIDs: only rule 1 applies.
- `scene_uuids`: for a collection, the sorted unique `uuid` strings of top-level `sources` items whose
  `id` is `scene` (use JsonScan; not a DOM). A collection that does not parse has `scene_uuids: []`
  and a warning `{"code":"collection-unparseable","path":...}`; it is still planned (stored, fidelity
  decided at capture).
- The manifest schema gains an OPTIONAL entry field `scene_uuids` (array of strings, sorted, unique,
  each <= 256 bytes, at most 100,000 items): only allowed on entries with role `collection`; validator
  errors `E_SCHEMA scene_uuids`; export projection drops it (it is local identity data).

## Review rules u1: first-party source list (§8, §15)
- `ReviewRules` table: the source type ids (the `versioned_id` or `id`) whose collection items need no
  unreviewed count: initially ONLY `scene` and `group` (no settings). Every other source type counts as
  unreviewed as in PR2 (count = max(1, number of sources whose type is not in the table)), so real
  collections stay `opaque-unreviewed`. The table is data (`ReviewRules.cpp`), documented in §8 with the
  rule for adding a type: its settings keys are enumerated, none is a secret, URL, path or user text.

## Part B: PR2 Minors (library)
1. JSON numbers: a number token longer than 4,096 bytes makes the document invalid (parse invalid);
   an exponent with more than 6 digits is clamped (treated as overflow for a positive exponent and
   underflow for a negative one); cover with unit tests (the reproducers are in
   `debates/002-workspace-format/review-pr2-04/03-r2-claude.reply.md`).
2. Tests: the locale test must exercise real conversion (a number near a double limit, e.g.
   `1.5e300`) and one underflow case; an isolated test that a >1,024-byte section name alone (nothing
   else malformed) triggers the INI fallback.
3. Docs (lead does these; do not edit docs).

## r1 (integration: the lead's decisions on the cases' readings; wins over the text above)
1. Planned entries carry NO `scene_uuids` (it is in `collections[]`; capture copies it into the manifest
   entry). `collision` appears only when true.
2. A `.bak` without its main is stored alone with NO collection/profile UID and is NOT listed in
   `collections[]`/`profiles[]` (OBS lists collections by their `.json` files, so a lone `.bak` is not a
   collection).
3. `<f>` and `<d>` must themselves pass PathRules; a name that matches the shape but whose `<f>`/`<d>`
   fails (for example `Trail..json`, `<f>`=`Trail.`, or `.json`, empty `<f>`) is an `unknown` file,
   not unrepresentable (the whole file name passes PathRules).
4. Each root scans ONLY the subpaths it owns (`user`: `user.ini`; `scenes`: `basic/scenes/`; `profiles`:
   `basic/profiles/`; `plugin_manager`: `plugin_manager/`; `config`: `plugin_config/`, `logs/`,
   `crashes/`, `profiler_data/`, `updates/`, `.sentinel/`, `global.ini` (never stored), `themes/` (never
   listed)). Anything else under a root's app dir is ignored and NOT listed (in the default OBS layout all
   five roots are the same directory; listing would duplicate entries).
5. The tester's pinned readings are accepted: `cache` = the three module folders, `runtime` = logs,
   crashes, profiler_data, updates, .sentinel; only a collapse has `count`; a symlink entry has size 0 and
   beats every other reason; `load_rule` only when the `.bak` exists; only new UIDs consume the
   `--uid-seed` counter, collections before profiles; omitted entries sorted by path bytes across all roots;
   name matching exact and case-sensitive; `scene_uuids` uses `id` literally; the minus sign counts
   toward the 4,096 number bytes.

## r2 (review run 01 of PR #32; read ~/.local/share/jeiobs-work/reviews/pr32/01-run/*.reply.md)
1. Identity (claude-1): two passes. Pass 1 applies rule 1 (same root and path) to ALL collections and
   profiles and claims those parent UIDs. Pass 2 applies rule 2 (unique scene-UUID set) only to the
   unmatched collections, against unclaimed parent collections. The result must not depend on name order.
2. `themes/` is skipped only directly under the config root's app dir (claude-2); a profile or collection
   folder named `themes` is planned normally.
3. Symlinked roots (claude-3): if a root's app dir, or a subpath a root owns (`basic/`, `basic/scenes`,
   `basic/profiles`, `plugin_manager`, `plugin_config`), is a symlink or a Windows junction/reparse point,
   it is NOT followed: it becomes one omitted entry `{"reason":"symlink"}` and a warning
   `{"code":"root-symlink","path":...}`.
4. Listing errors (claude-4, codex-2): a directory that cannot be listed (permission, an error part way
   through) becomes one omitted entry `{"reason":"unreadable","kind":"dir"}` plus a warning
   `{"code":"listing-error","path":...}`; the plan is never silently partial. On Windows a name with an
   unpaired UTF-16 surrogate is `unrepresentable`.
5. `count` (claude-5): the manifest v1 schema gains an OPTIONAL field `count` (integer >= 1) allowed only
   on omitted entries with `kind` "dir" or "names"; elsewhere `E_SCHEMA count`. Export keeps it; CarryOver
   treats it as known.
6. Complexity (claude-6, codex-3): scene-UUID extraction is linear and stops at 100,000 UUIDs (then the
   collection gets `scene_uuids: []` and a warning `scene-uuids-limit`); collections and profiles count
   toward the 10,000 entry limit; no O(n^2) lookups (use maps/sets).
7. Collapse accounting (claude-7): a collapsed folder's `size` and `count` cover its unknown regular files,
   recursively, without following symlinks; one entry per folder path; no sample truncation.
8. A folder in `basic/profiles/` without a `basic.ini` is not a profile (OBS lists profiles by
   `basic.ini`): no UID; its contents are one omitted `unknown` `kind:"dir"` entry.
9. A stored-shape file larger than 64 MiB is omitted `{"reason":"too-large"}` (no warning about parsing);
   the planner also stops adding stored entries when their total would exceed 256 MiB (further ones
   omitted `too-large`).
10. Lone `.bak`: the only valid reading is "no UID" (r1.2): the black-box cases drop the alternatives.

## r3 (integration)
`plan-too-large-total` (tests/cases) fails: with four 64 MiB collections A..D plus E.json and a 1-byte module config,
the stored total is inclusive of 256 MiB counted in plan order, and the collections list must contain A, B, C, D
in name order. The implementation lists A and C (check how collections are indexed when a file is the 64 MiB
limit, and whether B is dropped from `collections[]` while kept in `entries[]`). Fix so the case passes; do not edit it.

## r4 (review run 02 of PR #32)
1. (codex-2, Major) A directory whose listing failed part way (`Summary.error`) must never produce a partial
   `unrepresentable/names` entry without the `listing-error` warning and `unreadable` handling of r2.4; every code
   path that lists a directory checks the error.
2. (claude-7) The collapse sample list is not capped at 101 entries while it counts only regular files:
   subfolders and non-regular files never vanish silently (count them under the folder, `size` 0).
3. (claude-15) `ProcessOwnedRoot` lists nothing under the app dir or `basic/` except what each root owns
   (r1.4): an unrepresentable name there is ignored, not planned (no `obs-studio` path entries).
4. (claude-16) AddOmitted merge scans are linear/map-based; the 256 MiB stored budget is applied in the plan
   order of r1 (user.ini, modules, collections, profiles, module configs), sorted by name bytes within each
   group, not directory order.
5. (claude-12) `windows.h` is included only with NOMINMAX/WIN32_LEAN_AND_MEAN (lead did it).
