# S1-PR1 contract (shared by the implementation and test workers)

Spec: `docs/jeiobs/WORKSPACE_FORMAT.md` (sections 3, 4, 5, 6 export gate, 12, 13). Where this
contract and the spec differ, this contract wins for PR1; report the difference.

## Layout
`shared/jeiobs-workspace/` — standalone CMake project, C++17, no Qt, no libobs:
`cmake -S shared/jeiobs-workspace -B <build> && cmake --build <build> && ctest --test-dir <build> --output-on-failure`.
- `OBS::blake2`: `if(NOT TARGET OBS::blake2) add_subdirectory(../../deps/blake2 ...)` (binary dir
  inside the build tree). Note deps/blake2 asks for CMake 3.28 minimum.
- `find_package(nlohmann_json 3.11 REQUIRED)`.
- Library target `jeiobs-workspace` (static), headers under `include/jeiobs/workspace/`.
- CLI target `jw-check` (below). Unit tests target `jw-unit` (in-tree `tests/jw_test.hpp`, no
  third-party test framework), registered with ctest as `jw-unit`.
- Black-box cases: if `tests/cases/run-cases.cmake` exists, CMakeLists registers
  `add_test(NAME jw-cases COMMAND ${CMAKE_COMMAND} -DJW_CHECK=$<TARGET_FILE:jw-check>
  -DCASES_DIR=${CMAKE_CURRENT_SOURCE_DIR}/tests/cases -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/cases-work
  -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/cases/run-cases.cmake)`.

## CLI `jw-check` (stdout one result line unless stated; exit 0 = OK, 1 = ERR, 2 = usage)
- `jw-check hash FILE` → 64 lowercase hex BLAKE2b-256 of the file bytes.
- `jw-check path PATH` → `OK`, `OK unchecked` (valid under the ASCII rules but contains non-ASCII,
  so the Unicode collision key is unchecked), or `ERR E_PATH <rule>`.
- `jw-check manifest FILE [--export]` → `OK` optionally followed by flags in this order, space
  separated: `readonly` (unknown major, but valid under v1 rules), `ro_compat` (unknown
  ro_compat feature), `unchecked-paths` (some path non-ASCII) — e.g. `OK readonly unchecked-paths`.
  Or `ERR <CODE> <detail>`. `--export` validates an export manifest instead of a local one.
- `jw-check revision FILE` → checks a stored local manifest whose file name is
  `<seq>-<revid>.json`: name shape (10 decimal digits, `-`, 64 lowercase hex, `.json`) else
  `ERR E_NAME`; BLAKE2b-256 of the bytes equals revid else `ERR E_REVID`; then manifest
  validation (as `manifest`); `seq` field equals the name's seq else `ERR E_SEQ`. Output as `manifest`.
- `jw-check child PARENT DRAFT` → prints (pretty or compact) the child manifest JSON: DRAFT (a
  local manifest a v1 writer built, knowing only v1 fields) plus, from PARENT, unknown top-level
  fields copied as JSON values (DRAFT's own value wins if both have the key), and for each DRAFT
  entry whose `root`, `path` and `hash` equal a PARENT entry's, that entry's unknown fields
  copied; unknown fields of a PARENT entry that has a same root+path DRAFT entry with a different
  hash (or no hash) are not copied and are appended to DRAFT's `dropped_ext` as
  `{"entry": <draft entry id>, "fields": [sorted names]}`. Errors: `ERR <CODE>` as `manifest`
  for either input; a PARENT with an unknown `ro_compat` feature → `ERR E_RO_COMPAT <feature>`;
  unknown major in PARENT → `ERR E_MAJOR`.
- `jw-check export MANIFEST DECISIONS --now RFC3339` → prints the export projection JSON (section
  4 "Export projection"; `exported_utc` = `--now`; `origin.revision_id` = BLAKE2b-256 of the
  MANIFEST file bytes). DECISIONS is a JSON object keyed by entry id:
  `{"decision":"include","fidelity":"exact"|"redacted"}` or `{"decision":"omit"}`. Every
  `opaque-unreviewed` entry needs one, else `ERR E_UNDECIDED <entry id>` (first in entry order)
  and nothing on stdout. A decision for an entry that is not opaque-unreviewed →
  `ERR E_SCHEMA decisions`. Include adds `"review": {"rules": <its unreviewed.rules>,
  "decision": "include"}` and removes `unreviewed`. Omit turns it into
  `{id, status:"omitted", root, path, [role], [profile|collection], size, reason:"unreviewed"}`.
  An entry with `parse:"invalid"` needs no decision and always exports as omitted with
  `reason:"unparseable"`; an `include` decision for it → `ERR E_SCHEMA decisions`. Unknown major → `ERR E_MAJOR`; unknown ro_compat →
  `ERR E_RO_COMPAT <f>`; unknown incompat → `ERR E_INCOMPAT <f>`. Unknown fields are dropped
  and listed in the output's `dropped_ext` (top level: `{"entry": null, "fields": [...]}`; per
  entry: `{"entry": id, "fields": [...]}`; names sorted).

## Error codes (first token after `ERR`; detail is one token)
`E_IO`, `E_LIMIT <manifest|entries|string|values|depth|blob|revision|path|components|component>`,
`E_JSON`, `E_DUPKEY`, `E_UTF8`, `E_NUL`, `E_NONFINITE`, `E_BOM`, `E_SCHEMA <field>`,
`E_PATH <rule>`, `E_COLLISION`, `E_INCOMPAT <feature>`, `E_MAJOR`, `E_RO_COMPAT <feature>`,
`E_UNDECIDED <id>`, `E_NAME`, `E_REVID`, `E_SEQ`.
Fixtures have one fault each; precedence among faults is not tested.

Path rules (`E_PATH <rule>`), checked on each manifest `path`: `empty`, `absolute` (leading
`/`), `dot` (`.` component or empty component, e.g. `a//b`), `dotdot`, `backslash`, `colon`,
`control` (U+0000–U+001F, U+007F), `reserved-char` (`< > " | ? *`), `trailing-dot-space`,
`device-name` (part before the first dot equals, ASCII case-insensitively, CON PRN AUX NUL
COM0–COM9 LPT0–LPT9 CONIN$ CONOUT$, or COM/LPT followed by ¹ ² ³), `not-utf8`. Length limits
give `E_LIMIT path` (> 512 bytes), `E_LIMIT components` (> 8), `E_LIMIT component` (> 255 bytes).
Collision (`E_COLLISION`): two entries with the same `root` whose paths are equal after ASCII
case folding (A–Z → a–z).

## Manifest schema v1 (both modes)
Required: `format` = "jeiobs-workspace"; `major` integer ≥ 1 (≠ 1 → readonly path, see below);
`minor` integer ≥ 0; `compat` object with arrays of strings `incompat`, `ro_compat`;
`created_utc` string; `creator` object; `active` object; `target` object with `os` string;
`app_keys` array; `entries` array; `dropped_ext` array. Optional: `label` string.
Local only, required: `workspace_id` string, `seq` integer ≥ 1, `parent` (null or 64 lowercase
hex), `secret_refs` array; forbidden: `origin`, `exported_utc`.
Export only, required: `origin` {`workspace_id`, `revision_id` (64 hex)}, `exported_utc`;
forbidden: `workspace_id`, `seq`, `parent`, `secret_refs`, and any entry with
`fidelity: "opaque-unreviewed"` (→ `E_SCHEMA fidelity`) or `redactions[].ref`/`.was`.
Entry: `id` string unique (dup → `E_SCHEMA id`), `status` "stored"|"omitted", `root` one of
config user scenes profiles plugin_manager, `path` string. Stored: `role` string, `size` integer
≥ 0, `hash` 64 lowercase hex, `fidelity` exact|redacted|opaque-unreviewed. Omitted: `reason`
string, `size` integer ≥ 0, and no `hash` (present → `E_SCHEMA hash`). `bak`, if present, names
another entry's id; local `redactions[].ref` names a `secret_refs[].id`; `secret_refs[].entry`
names an entry id (dangling → `E_SCHEMA <field>`). Wrong type or missing required field →
`E_SCHEMA <field name>`.
Limits (section 12): file > 4 MiB → `E_LIMIT manifest` (checked before parsing); > 10,000
entries → `E_LIMIT entries`; any string > 64 KiB → `E_LIMIT string`; > 250,000 JSON values →
`E_LIMIT values`; nesting depth > 64 → `E_LIMIT depth` (the top-level object is depth 1);
stored `size` > 64 MiB → `E_LIMIT blob`; sum of stored sizes > 256 MiB → `E_LIMIT revision`.
Parser (during the parse, not after): duplicate key at any level → `E_DUPKEY`; invalid UTF-8 →
`E_UTF8`; `\u0000` or a raw NUL → `E_NUL`; a number that overflows double (`1e400`) →
`E_NONFINITE`; UTF-8 BOM → `E_BOM`; other syntax error → `E_JSON`.
Compatibility: no features are known in v1. Any `incompat` entry → `E_INCOMPAT <first>`. Any
`ro_compat` entry → flag `ro_compat`. Unknown `major`: if the rest validates under v1 rules →
flag `readonly`; otherwise the v1 error. Unknown fields anywhere are ignored by `manifest`.
