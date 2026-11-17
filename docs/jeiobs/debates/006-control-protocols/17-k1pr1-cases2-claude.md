I committed the r2 cases as `6f878c703` on the detached HEAD (no branch, no push). The commit has 83 files, all under `tests/cases/`, and takes `cases.txt` from 916 to 1128 cases. Against the latest r2 build available (`/tmp/jeiobs-k1pr1/build-r2/jc-check`, built at 10:57, possibly not Codex's final), 1045 pass and 83 fail:
- **81 fail** because they need a `value fromdouble` command, which neither the contract nor the CLI has yet.
- **2 fail** because the code disagrees with the contract (ambiguities 2 and 3).

## What changed, by r2 item
1. **Values:** 82 `fromdouble` cases. Expected values come from an exact rounding model:
   - halves round to even;
   - results clamp to the width (uint7 127.5 → 127, int8 127.5 → 127, plus the top codes of uint14, uint32, uint52, int53, and the 64- and 128-bit edges);
   - -inf gives the minimum, +inf the maximum, NaN gives `E_VALUE`.

   Also added: out-of-range values (int8 300, int7 128, uint7 256) are refused by `limits`, `double` and `roundtrip`. An ieee32 raw value above 32 bits is refused, while `ffffffff` is accepted. Twelve `limits` cases now expect exactly `ERR E_LIMIT value`.
2. **One strict parser:** 22 `strict-*` cases check that `compile` and `match` reject what `validate` rejects. 12 new event cases cover bad UTF-8, NUL, non-finite numbers, a duplicate key inside a segment, depth 32/33/100000, and 4 MiB / 4 MiB + 1. The event duplicate-key and BOM cases now expect exactly `E_DUPKEY` and `E_BOM`.
3. **Hotkeys:** the 9 `wild-ok-*-hotkey-{none,scene,mixer}` cases are flipped to `E_WILDCARD_PRIVILEGED` (fixtures renamed). New: claude-3's `OBSBasic.StartStreaming` declared as `scene`, a concrete route and a wildcard route on the same hotkey, and a disabled wildcard route. Every hotkey's class is now `output-control`, including in both `key-hotkey` expected files.
4. **Hard deny:**
   - `url`, `input`, `ffmpeg_options` and `playlist` are now denied for source and filter properties, including case variants, a disabled route, document order and `match`. `allowed-field-url` became `denied-sp-url`.
   - These stay allowed: near names (`urls`, `input_format`, `ffmpeg-options`, fullwidth `ｕｒｌ`, …) and the same words used in other kinds or as ids.
5. **Wide object:** a generated object with 499,201 keys (3.99 MB) inside an unknown member. It must pass `validate`, `compile` and `match`, a repeat of its first key at the end must give `E_DUPKEY`, and a variant puts the 499,200 keys directly in the mapping object.
6. **`target_key` escaping:**
   - Two expected files now carry the escapes: `key-unicode-and-escapes` (`\\ \/`) and `key-mixer-mute` (`Mic\/Aux`).
   - New `key-escapes-every-kind` covers 10 kinds with `\ | = /` in values.
   - 8 pairs of different emit addresses must compile, including codex-3's and claude-6's examples and a raw `\` before `/`.
   - 3 cases with identical escaped targets must still give `E_AMBIGUOUS_MERGE`.
7. **`requires`:** `utf8-raw.json` now uses ASCII `requires`. 8 non-ASCII cases (raw, escaped, Kelvin sign, fullwidth, emoji, non-breaking space, second item, second slot) expect exactly `ERR E_SCHEMA requires`. I also added the r1.3 pattern and count limits, valid and invalid.
8. **Complexity:** 10,000 routes on one target with distinct priorities, and the same with one repeated priority. Also 5,000 emit routes into one address that 5,000 other routes match, and the same closed into a cycle.

## Black-box runs (I copied the binaries and ran them)
- **Pre-r2 build:** 54 non-fromdouble cases fail, all of them r2 cases.
  - The wide object did not finish validating in 900 s, which confirms claude-5.
  - The 10,000-route merge case takes 64 s, so it still passes under the default 120 s timeout. That case only catches the quadratic merge check under sanitizers or a lower `JC_CASE_TIMEOUT`.
- **r2 sanitizer build:** the slowest new case takes 9.6 s.
- **Runner:** the full run takes 47 s and the scratch directory needs about 42 MB.

## Ambiguities
1. **`value fromdouble` is my proposed form, not in the contract:** `jc-check value fromdouble VALUE_JSON DOUBLE` prints the result as compact value JSON with sorted keys. DOUBLE is a JSON number or `inf`, `-inf`, `nan`. Either add it to the contract and the CLI, or drop these cases.
2. **4 MiB + 1 event:** I expect `E_LIMIT`, because r1.6 lists the size limit among the shared rules. Both builds print `E_SCHEMA event`.
3. **uint `"-1"` inside a tuple:** I expect `E_LIMIT value`, because r1.8 says "wherever it is nested". Both builds print `E_VALUE` there, but `E_LIMIT value` at the top level (claude-7's point).
4. **Emit key format:** the contract doesn't define how an emit address is printed in the key. So injectivity is tested only through the merge check, and no pinned value contains `:`.
5. **Hotkey `effect` in `target_key`:** both expected files are still kept. r2.3 (effect is only a hint) and A15 (identity is registerer + name) argue for deleting `key-hotkey-with-effect.json`. That's your call.
6. **Unpinned:** path-like fields such as `playlist/0/value`, and `access_token` as a field name. Missing `enabled` still matches loosely (`enabled|routes?`); both builds print `E_SCHEMA routes`, against r1.3.

## Not covered (needs jc-unit)
- Unit and Relative metadata kept by `FromDoubleInCodeSpace`: the value JSON has no fields for them.
- `fromdouble` with float templates: the contract doesn't say whether they clamp to max or go to infinity.
- `MakeInt`/`MakeUInt` masking itself: the CLI range-checks the input first, so the new rejection cases pass on both builds.
- The byte-level `ParseMapping`/`CompileMapping(bytes)` API.
- r2.8's index pruning: only the case timeout guards it.

I didn't read `src/`, `include/` or the worker logs. The authoring scripts are in `/tmp/jc-cases-r2/author` and are not committed.

That notification is the wide-object timing I had already reported: the pre-r2 build was killed at the 900 s cap without finishing. Nothing else is pending, and commit `6f878c703` and the report stand as written.
