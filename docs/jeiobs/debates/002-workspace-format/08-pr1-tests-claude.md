Committed as `778797801` on the detached HEAD: 208 files, all under `shared/jeiobs-workspace/tests/cases/`. That is `run-cases.cmake`, `cases.txt` with 328 cases, `fixtures/`, `expected/`, and a `.gitattributes` set to `* -text`. The attribute matters because the root file says `* text=auto`, and line-ending conversion on a Windows checkout would change the hashed fixtures.

**Testing.** I could not run the cases against the real `jw-check`, because it isn't built here. I ran them with CMake 3.28.3 on Linux against a throwaway Python fake (in /tmp, not committed) that implements my reading of the contract: 328/328 pass. Runs with broken fakes fail correctly: wrong text or exit code, a JSON difference reported by member path (an integer printed as `48211.0` is caught), extra output after the JSON, a missing binary, and a bad or duplicate line in `cases.txt`. A fault audit confirmed every error fixture has exactly one fault and every OK fixture has none. Windows and macOS were not run; I only simulated the Windows skip of `@posix` cases.

## Case list (full list in `cases.txt`)
| Group | n | Covers |
|---|---|---|
| usage | 9 | exit 2 for no args, unknown command, missing operand or option value, unknown option |
| hash | 9 | `""` and `abc` known answers, 128/129 bytes, 1 MiB + 1, every byte value (with Ctrl-Z, to catch text-mode reads), the spec example file, missing file, directory → `E_IO` |
| path | 80 | 20 valid (incl. `OK unchecked`), every rule with variants, 6 POSIX-only `not-utf8`, the three length limits incl. a 256-byte multibyte component |
| valid manifests | 14 | spec §4 local and export examples, base manifests, `parent: null`, no label, no entries, omitted directory, non-ASCII path → `unchecked-paths`, local/export mode swapped, `E_IO` |
| parser | 29 | duplicate keys at 4 levels, 7 invalid UTF-8 kinds (incl. in a key), `\u0000` in value and key, raw NUL inside a string and after `}`, ±1e400, BOM, 7 syntax errors |
| limits | 26 | both sides of every boundary: manifest 4 MiB, entries 10,000, string 64 KiB (and a key), values, depth 64/65 (+ 100,000), blob, revision, path, components, component |
| path rules in manifests | 16 | 12 rules through entry paths, ASCII-case and identical collisions, other-root and non-ASCII-case non-collisions |
| schema (local) | 47 | each required field missing, wrong types and values, local with `origin` / `exported_utc` |
| schema (entries/refs) | 29 | per-entry fields, duplicate id, omitted entry with hash, dangling `bak`, `bak` naming itself, dangling `ref` and `entry` |
| schema (export) | 11 | opaque-unreviewed, `ref`, `was`, forbidden local fields, missing or bad `origin` |
| compat | 10 | `incompat` (first listed), `ro_compat`, future major valid/invalid, flag order, future minor, unknown fields everywhere |
| revision | 14 | OK (incl. seq 9999999999), `ro_compat` and error passthrough, 7 `E_NAME` shapes, `E_REVID`, `E_SEQ`, `E_IO` |
| child | 13 | top-level kept, draft wins, entry kept, dropped on changed hash or now-omitted, removed entry not listed, nested fields not copied, all rules together, 5 errors |
| export | 21 | spec example, base projection, `E_UNDECIDED` (first / second), bad decisions, `parse:"invalid"`, unknown fields listed, projections and child output re-validated, refusals |

## How I read the ambiguous points
1. **"nothing on stdout" for `E_UNDECIDED`:** I read it as "no partial JSON", with the ERR line on stdout. Export error cases (prefix `?`) also accept empty stdout with the ERR line on stderr.
2. **`E_SCHEMA` field names:** bare names, as in the contract's examples (`id`, `hash`, `fidelity`). For nested fields outside entries I also accept the dotted form: `target.os`, `compat.incompat`, `origin.revision_id`, `redactions.ref`, `secret_refs.entry`.
3. **Codes listed without a detail** (`E_IO`, `E_JSON`, `E_DUPKEY`, `E_UTF8`, `E_NUL`, `E_NONFINITE`, `E_BOM`, `E_COLLISION`, `E_MAJOR`, `E_NAME`, `E_REVID`, `E_SEQ`): asserted with no detail token.
4. **Overlapping path rules:** `.` and `..` also end in a dot, and `/a` and `""` also contain an empty component. I asserted the rule the contract names (`dot`, `dotdot`, `absolute`, `empty`). `...` → `trailing-dot-space`; `a/` → `dot`.
5. **Limit edge cases:**
   - Object keys count as strings for `E_LIMIT string`.
   - The values counting method isn't specified, so I test 249,000 → OK and 250,001 → error, which hold however values are counted.
   - Depth fixtures use empty containers, so it doesn't matter whether scalars carry a depth.
   - Omitted sizes count toward neither `blob` nor `revision` (1 TiB omitted → OK).
6. **`not-utf8`** is unreachable through manifests (the parser reports `E_UTF8` first), so it is tested only through `path` on POSIX. `\ud800` is accepted as `E_JSON` or `E_UTF8`, `NaN` as `E_JSON` or `E_NONFINITE`, and a top-level array as `E_JSON` or `E_SCHEMA`.
7. **Raw NUL after `}` → `E_NUL`:** nlohmann's lexer treats NUL as end of input, so a straightforward implementation would accept this file. It is the most likely real failure.
8. **Decisions:**
   - A decision for an unknown id → `E_SCHEMA decisions`.
   - Malformed decisions (include without fidelity, fidelity `opaque-unreviewed`, unknown verb) → any `E_SCHEMA`.
   - "first" for `E_INCOMPAT` and `E_UNDECIDED` means array / entry order.
9. **child:**
   - Matching is by root+path+hash, not id: draft ids deliberately differ from the parent's.
   - A parent entry with no same root+path draft entry → not listed.
   - Nested unknown fields → not copied and not listed.
   - The draft's `parent` is left untouched.
   - New `dropped_ext` items go after existing ones, in an order that is the same by draft or by parent.
10. **export:** the omit form keeps role/profile/collection and drops bak/load_rule/kind. Each output has at most one new `dropped_ext` item, the input's `dropped_ext` is always empty, and `target.audio` is always present.
11. **`bak` naming its own entry → `E_SCHEMA bak`** ("another entry's id").

## Not covered
- Fault precedence (excluded by the contract) and the Unicode collision key (deferred by owner decision 3).
- An exact 250,000-value boundary, and raw vs decoded string length for escaped strings.
- Nested unknown fields in export output; an input with existing `dropped_ext`; an omit decision for a `parse:"invalid"` entry; both entries omitted in child; a draft with `ro_compat` or a future major; `E_IO` for child/export inputs; `--now` validation; extra arguments.
- **Windows:**
  - 6 `@posix` cases are skipped, because invalid UTF-8 can't be passed in argv there.
  - 8 non-ASCII `path` cases (café, the superscripts, `${C255U}`/`${C256U}`) will fail unless `jw-check` reads UTF-8 arguments, through `wmain` or a UTF-8 code-page manifest. The same rules are also covered through manifests, so those failures would point at argument handling.
  - The generated files contain no LF and are size-checked after writing, because `file(WRITE)` may convert newlines on Windows.
- **Possible spec gap (not tested):** I believe Windows also treats names like `CON .txt` as the device, because it trims trailing spaces before comparing. The contract's "part before the first dot equals" rule would accept them. Worth checking against Windows behaviour.

On a fresh build directory the runner writes about 14 MB into `WORK_DIR` and never deletes anything.
