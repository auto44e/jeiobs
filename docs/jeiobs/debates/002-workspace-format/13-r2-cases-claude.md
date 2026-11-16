I committed the r2 case updates as `96696ce59` on the detached HEAD (154 files, all under `shared/jeiobs-workspace/tests/cases/`). There are now 509 cases, 181 of them new, and all pass against my r2 fake. They have not been run against the r2 library or on Windows/macOS.

## What changed
- **Runner (`run-cases.cmake`):**
  - The `?` prefix is gone, and a case that still uses it is a format error.
  - Every TEXT or `~REGEX` expectation now also requires stdout to be exactly one line, so an ERR line can never come with JSON.
  - New generated inputs:
    - revision files of 4 MiB and 4 MiB + 1. CMake can't compute BLAKE2b, so their names are fixed in the runner and the bytes are checked with SHA-256.
    - a directory and a link to `/dev/zero`, each with a revision file name.
    - DECISIONS files with a BOM, bad UTF-8, a raw NUL, sizes of 4 MiB ± 1 and ids of 65,536 / 65,537 bytes.
    - parent/draft pairs that are each valid but give a child over the values limit or over 4 MiB.
  - The scratch directory now grows to about 36 MB.
- **Old fixtures brought into line with r2, each still testing the same thing:**
  - The child parent's `modules.json` path is now `plugin_manager/modules.json`.
  - The root/path schema faults moved to the omitted entry, so they don't also break the allowlist.
  - `export-schema-opaque-unreviewed` now carries `unreviewed`.
  - The `parse:"invalid"` fixtures use roles without registry secrets (encoder JSON, `user.ini`). §6 doesn't allow a collection there.
  - The parse-invalid decisions now use `exact`.
- **Old expectations that changed:**
  - The 11 `?` export cases now expect the exact ERR line.
  - `include-without-fidelity` is now a success case, producing the spec example output.
  - `opaque-fidelity` and `unknown-verb` now expect exactly `ERR E_SCHEMA decisions`.

## Testing
- **r2 fake:** 509/509 pass. A fault audit confirms every ERR fixture has exactly one fault, except the two cases that test the "first in that order" rule on purpose.
- **Broken fakes:** each injected defect is caught. That covers JSON after an ERR line, a missing allowlist, objects copied whole, no child check, decisions read without limits, no `--now` check, and the alternative device-name order.
- **The lead's pre-r2 `jw-check` build, run as a black box:** 393 pass and 116 fail, and every failure is an r2 behaviour.
  - The failures include the crashes the reviewers predicted: aborts for opaque without `unreviewed`, a directory as DECISIONS, and a non-UTF-8 `--now`.
  - The `/dev/zero` revision hangs until the timeout.
  - All the new OK fixtures pass on that build, and so do the hard-coded BLAKE2b names.

## New cases (181)
| Group | n | Covers |
|---|---|---|
| allowlist (item 1) | 46 | all 17 rows one by one, plus all together, names with spaces and dots, a non-ASCII folder; wrong role, wrong path, path for another root, `<f>`/`<d>` with `/`, `<f>` empty or ending in dot/space, case, `machine-id`, `workspaces/x`, `global.ini`, decklink `.bak`, swapped encoder files, `modules.json` outside its folder, an unlisted module; omitted entries not bound |
| entry fields (item 2) | 24 | opaque without `unreviewed` or with a bad one (8 shapes), count 1 OK, `unreviewed` on exact/redacted, `parse` misuse, omitted with each forbidden field, the "first in that order" rule |
| `--export` (items 1, 2, 7) | 9 | `target.machine_id`/`arch`/unknown/`audio.explicit`, target without audio OK, a `restore:"never"` key, a non-allowlisted stored entry, `unreviewed`/`parse` |
| DECISIONS file (item 4) | 24 | duplicate key (two levels), BOM, UTF-8, NUL (escape and raw), 1e400, syntax error, empty file, depth 65, 64 KiB key both sides, 4 MiB both sides, missing file, directory, `/dev/null`, `/dev/zero`, array, bad value shapes, decision for an omitted entry |
| derived fidelity (item 5) | 7 | derived redacted, given redacted, both mismatches, `redactions: []` → exact |
| rebuild (item 6) | 16 | unknown members of creator, active, compat, app_keys and target; earlier `dropped_ext` items kept first; target without audio; every output re-checked with `--export` |
| `--now` (item 3) | 25 | 3 good (leap days, new year); bad month, day, Apr 31, Feb 29 (common year and 1900), 24:00, :60, :61, missing Z, offsets, lowercase, space, fraction, short month, trailing space, date only, empty, non-UTF-8 |
| devices (item 8) | 12 | `CON .txt`, `nul  .json`, `aux .`, COM/superscript/CONIN$ variants, inside a directory, in a manifest; `CON ` → `trailing-dot-space`; 3 OK near-misses |
| bounded reads (item 9) | 11 | revision 4 MiB ± 1, revision directory and `/dev/zero`, `hash` over 4 MiB still works, manifest/hash of `/dev/null`, directories as manifest, export input or child parent |
| child (item 10) | 6 | each input valid on its own; the child over the values limit → ERR; the child over 4 MiB → ERR |
| other export | 4 | the two inputs that crashed the old code are now refused, and the decision outputs re-validate |

## Ambiguities and how I read them
1. **`aux .`** also ends in a dot, yet r2 says `device-name`, while `CON ` stays `trailing-dot-space`. The only reading that fits both: strip spaces only when a dot follows them, and check device names before trailing dot/space. The pre-r2 build gives `trailing-dot-space` for `aux .`, so a fix that only adds the stripping fails `path-device-space-before-trailing-dot`. Please confirm r2 meant it.
2. **DECISIONS over 4 MiB** → `E_LIMIT manifest` ("same reader"; the contract has no `decisions` detail).
3. **A DECISIONS file that is an array** → either `E_JSON` or `E_SCHEMA decisions` is accepted, as for manifests.
4. **"anything else" read strictly:** all of these → `E_SCHEMA decisions`:
   - `omit` with `fidelity`
   - an extra member such as `note`
   - a missing `decision`
   - a value that isn't an object
   - a decision for an omitted entry
5. **`redactions: []`** derives `exact` and stays `[]` in the output. On an omitted entry, an empty `redactions` still counts as present.
6. **Dotted names:**
   - Only direct members get one, so there is no `target.audio.x`.
   - Known target members that are dropped (`arch`, `roots` and so on) aren't listed.
   - The dotted names and unknown top-level names form one sorted list; `comment` lands between `app_keys.x_source` and `compat.x_note`.
7. **Allowlist:**
   - The detail is always `role`, even when the path is what's wrong.
   - `<f>` must pass PathRules on its own, so `Main..json`, `Main .json` and `.json` are rejected although the whole path passes.
   - The rule also applies to export manifests.
8. **`--now ""`** → `E_SCHEMA now`, not a usage error.
9. **Malformed `unreviewed.count`** → `E_SCHEMA unreviewed` as r2 says; the pre-r2 build reported `E_SCHEMA count`.
10. **Child output** is checked against both the values limit and the 4 MiB size; each input is about 3 MiB, so the child is over the limit in any formatting.
11. **The `/dev/*` cases** reuse the `@posix` tag (skipped on Windows); the header now says so.

## Not covered
- The values limit on DECISIONS (r2's list leaves it out).
- FIFOs, which CMake can't create.
- A redaction item with no `pointer` in an included entry: the "never crash" rule can't be asserted without knowing the expected output.
- An `audio` block with members missing, and `app_keys` items with no `restore`.
- Order between a new top-level `dropped_ext` item and new per-entry items.
- An `omit` decision for a `parse:"invalid"` entry.
- The leap second `:60` and year 0000.
- Windows and macOS runs.

The generator and fake I used are in `/tmp/jwauthor2` and are not committed.
