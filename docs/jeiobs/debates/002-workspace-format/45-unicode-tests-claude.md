# Unicode PR: blind case notes (Claude)

The Claude worker's notes for the 251 case lines it wrote from `44-unicode-contract.md` without reading the library. Verbatim except trailing whitespace.

# Notes: blind cases for `jw-check unicode` and Unicode path collisions

Written from `/tmp/jeiobs-uni/contract.md`, WORKSPACE_FORMAT.md (sections 4, 12 and owner decision 3)
and the Unicode standard.
Nothing under `shared/jeiobs-workspace/src` or `include` (nor the unit or conformance tests) was read.
Also read, for format and fixture validity only: the `tests/cases/cases.txt` header and its path and
manifest sections, the fixtures `manifest-collision-ascii-case`, `manifest-collision-non-ascii-case`,
`manifest-collision-identical`, `manifest-no-collision-other-root`, `manifest-non-ascii-path-escaped`,
`allowlist-omitted-entries-not-bound` and `count-names`, and `_jw_decode`/`_jw_exec` in `run-cases.cmake`.

Deliverables: `cases.txt` (251 cases: 44 nfd, 40 fold, 80 key, 41 not-utf8, 18 path,
28 manifest), `fixtures/manifest/ucollision-*.json` (28 files, one per manifest case).
Tests run: only the generator's own checks below. The cases have NOT been run against `jw-check`.

## How the expected values were computed

- Primary source, as the contract asks: Python 3.12.3 `unicodedata` (Unicode 15.0.0).
  `nfd` is `normalize('NFD', t)`, `fold` is `t.casefold()` (CPython applies CaseFolding C and F, never T),
  and `key` is `normalize('NFD', normalize('NFD', t).casefold())`. An OK line carries
  `result.encode('utf-8').hex()`.
- Cross-check 1: my own implementation over the system UCD 15.1.0 files. It decomposes recursively from
  UnicodeData field 5 (mappings without a `<tag>`), computes Hangul per chapter 3.12, sorts each run of
  non-starters stably by ccc, and folds with CaseFolding C and F.
- Cross-check 2: the same code over the pinned UCD 18.0.0 files (UnicodeData, CaseFolding and DerivedAge
  from unicode.org/Public/18.0.0/ucd/).
- All three agree on every expected value. Over every code point assigned in 15.0, they also give
  identical NFD, fold, key and ccc, so the Python values are the 18.0.0 values.
- not-utf8 cases: Python's strict decoder rejects each byte string, and every OK input decodes.
- Fixtures: `manifest-collision-non-ascii-case.json` with its e6 and e7 replaced by the listed omitted
  entries (ids from e6, `size` 1, `reason` "unknown"). All other bytes are unchanged. The generator groups
  every entry, base e1-e5 included, by (root, key) under all three computations. A case expects
  `ERR E_COLLISION` exactly when some group holds two or more entries, and `OK` otherwise.
- Self-check: the generator reads `cases.txt` back, decodes `%XX` as `_jw_decode` does, and recomputes
  every expectation from the decoded bytes. It also checks that the file has no `;`, square brackets or
  backslash, that names are unique and prefixed, and that exactly the not-utf8 names end in `@posix`.

## Characters newer than Unicode 6.0, or unassigned

- U+AB70 and U+13F8 (Cherokee small letters, 8.0) and U+1C90 (Mtavruli, 11.0) are the characters the
  contract asks for. Their CaseFolding entries are the same in 15.0, 15.1 and 18.0.0.
- U+07FF (the largest 2-byte sequence) and U+1F600 have no decomposition and no folding in any version.
- U+ABFF, U+D7A4, U+D7FF and U+40000 are unassigned in 18.0.0 (DerivedAge), so they pass through under
  the contract's rule. A later Unicode version could assign them, but moving the pinned version is a
  reviewed change.

## Readings taken (the contract states them; listed so a reviewer can check)

- U+FFFF and U+10FFFF are noncharacters but well-formed UTF-8, and the contract's invalid list
  (overlong, surrogate, above U+10FFFF, truncated, stray continuation) does not name them. Expected: OK.
- U+0080 (a C1 control) is valid UTF-8 with no mapping. Expected: OK. Section 12's control rule names only
  U+0000-U+001F and U+007F.
- No TEXT value contains a path-rule character (`\ : < > " | ? *`, C0 controls, DEL), a device name or a
  trailing dot or space. No case therefore depends on whether `unicode` also applies path rules.
- Two entries of one root collide whatever their status. The existing manifest-collision-ascii-case
  already pairs a stored entry with an omitted one, and `ucollision-stored-vs-omitted-long-s` relies on this.
- The added omitted entries use only roots config, scenes, profiles and plugin_manager. Each of these
  already holds an omitted entry in a passing fixture (allowlist-omitted-entries-not-bound, count-names,
  manifest-non-ascii-path).
- `ucollision-nfc-vs-nfd-escaped` assumes the key is taken over the decoded JSON string. BMP `\u` escapes
  are accepted already (manifest-non-ascii-path-escaped).

## Left out (the contract does not state the behaviour)

- Usage errors of `jw-check unicode`: no operation, an unknown operation, a missing TEXT or extra
  arguments. The contract states neither exit code nor output.
- Empty TEXT or an empty result, which would print `OK ` with a trailing space (the contract says to avoid
  this). NUL cannot be passed at all: argv cannot carry it and `_jw_decode` rejects %00.
- Prefix collisions between a file and a directory, such as `Foo` and `foo/x.json`. The contract compares
  the keys of whole paths only.
- Two colliding stored entries with non-ASCII paths. These would need allowlist-valid stored entries
  with profile or collection ids, which the contract does not describe. Omitted-omitted and
  omitted-stored pairs are covered.
- Collisions in `plan` (planner warnings) under the Unicode key: outside this contract.
- Non-BMP JSON escapes (surrogate pairs) and invalid UTF-8 inside a manifest: these are parser
  behaviour. Non-BMP fixture paths are written as raw UTF-8.
- Characters first assigned in Unicode 15.1 to 18.0: Python 15.0 cannot compute them.
- A case that needs the outer NFD of the key. None exists in 18.0.0. An exhaustive check found that no
  character folds to a string containing a non-starter, and that no character NFD leaves unchanged folds
  to a string NFD would change. The only non-starter with a folding is U+0345 (ccc 240, folding to
  U+03B9 with ccc 0). The inner NFD already places it last in its run, because 240 is the highest class
  and U+0345 the only character with it. The inner NFD itself is observable and is covered:
  `unicode-key-greek-ypogegrammeni-before-psili`, `unicode-key-greek-alpha-ypogegrammeni-then-psili`
  and `ucollision-greek-inner-nfd`.
- Which pair a collision of three or more paths reports: the contract's line is exactly `ERR E_COLLISION`.

## Platform note

Only the not-utf8 cases carry `@posix`. The contract says Windows argv cannot carry invalid UTF-8, and
valid TEXT runs everywhere. No existing non-`@posix` case in cases.txt passes 4-byte UTF-8 or U+FFFF
through argv. On Windows, a failure in a non-BMP or noncharacter case (for example
`unicode-key-max-code-point`, `unicode-nfd-musical-half-note` or `unicode-path-ok-non-bmp`) may therefore
come from argv conversion rather than from the key.

## Cases

One line per case: the justification, then the derivation. In backticks, printable ASCII stands for
itself and `<U+XXXX>` for one code point. NFD means full canonical decomposition, Hangul by arithmetic,
then a stable sort of each run of non-starters by ccc. CaseFolding statuses (C, F) are cited from 18.0.0.
The hex after `=` is the UTF-8 of the result, the value on the OK line. Each manifest case lists the
omitted entries added to the base `manifest-collision-non-ascii-case.json` (e1-e5 kept, e6/e7 replaced)
with their keys.


### unicode nfd

- `unicode-nfd-ascii-unchanged`: nfd does not fold case and ASCII has no decomposition. `ABCabc`: NFD `ABCabc` = 414243616263.
- `unicode-nfd-e-acute-precomposed`: Latin-1 precomposed e-acute decomposes to e + U+0301. `<U+00E9>`: NFD `e<U+0301>` = 65cc81.
- `unicode-nfd-e-acute-decomposed`: already decomposed text is unchanged (NFD never composes). `e<U+0301>`: NFD `e<U+0301>` = 65cc81.
- `unicode-nfd-capital-e-acute-keeps-case`: capital E-acute decomposes and stays capital: nfd does not fold. `<U+00C9>`: NFD `E<U+0301>` = 45cc81.
- `unicode-nfd-sharp-s-unchanged`: sharp s has no canonical decomposition. `<U+00DF>`: NFD `<U+00DF>` = c39f.
- `unicode-nfd-capital-sharp-s-unchanged`: capital sharp s has no canonical decomposition. `<U+1E9E>`: NFD `<U+1E9E>` = e1ba9e.
- `unicode-nfd-dotted-capital-i`: U+0130 decomposes canonically to I + U+0307. `<U+0130>`: NFD `I<U+0307>` = 49cc87.
- `unicode-nfd-kelvin-singleton`: Kelvin sign has a canonical singleton decomposition to K. `<U+212A>`: NFD `K` = 4b.
- `unicode-nfd-angstrom-full-decomposition`: Angstrom sign -> U+00C5 -> A + U+030A (decomposition applied recursively). `<U+212B>`: NFD `A<U+030A>` = 41cc8a.
- `unicode-nfd-ohm-singleton`: ohm sign has a canonical singleton decomposition to U+03A9. `<U+2126>`: NFD `<U+03A9>` = cea9.
- `unicode-nfd-fi-ligature-unchanged`: U+FB01 has only a compatibility decomposition, which NFD does not apply. `<U+FB01>`: NFD `<U+FB01>` = efac81.
- `unicode-nfd-fullwidth-a-unchanged`: fullwidth A has only a compatibility decomposition. `<U+FF21>`: NFD `<U+FF21>` = efbca1.
- `unicode-nfd-superscript-two-unchanged`: superscript two has only a compatibility decomposition. `<U+00B2>`: NFD `<U+00B2>` = c2b2.
- `unicode-nfd-hangul-lvt`: Hangul LVT syllable U+AC01 decomposes algorithmically to L V T jamo. `<U+AC01>`: NFD `<U+1100><U+1161><U+11A8>` = e18480e185a1e186a8.
- `unicode-nfd-hangul-first-syllable`: first syllable U+AC00 (LV, TIndex 0) gives L V only. `<U+AC00>`: NFD `<U+1100><U+1161>` = e18480e185a1.
- `unicode-nfd-hangul-lv-second-vowel`: LV syllable U+AC1C (SIndex 28) gives L + second vowel, no T. `<U+AC1C>`: NFD `<U+1100><U+1162>` = e18480e185a2.
- `unicode-nfd-hangul-last-syllable`: last syllable U+D7A3 (SIndex 11171) gives U+1112 U+1175 U+11C2. `<U+D7A3>`: NFD `<U+1112><U+1175><U+11C2>` = e18492e185b5e18782.
- `unicode-nfd-hangul-after-last-syllable`: U+D7A4, one past the syllable range and unassigned in 18.0.0, passes through. `<U+D7A4>`: NFD `<U+D7A4>` = ed9ea4.
- `unicode-nfd-hangul-before-first-syllable`: U+ABFF, one before the syllable range and unassigned in 18.0.0, passes through. `<U+ABFF>`: NFD `<U+ABFF>` = eaafbf.
- `unicode-nfd-hangul-jamo-not-composed`: conjoining jamo stay as they are (NFD does not compose). `<U+1100><U+1161><U+11A8>`: NFD `<U+1100><U+1161><U+11A8>` = e18480e185a1e186a8.
- `unicode-nfd-order-dot-below-before-acute`: canonical ordering moves U+0323 (ccc 220) before U+0301 (ccc 230). `a<U+0301><U+0323>`: NFD `a<U+0323><U+0301>` = 61cca3cc81.
- `unicode-nfd-order-already-ordered`: an ordered sequence is unchanged. `a<U+0323><U+0301>`: NFD `a<U+0323><U+0301>` = 61cca3cc81.
- `unicode-nfd-order-same-class-grave-acute`: U+0300 and U+0301 both have ccc 230: their order is kept. `a<U+0300><U+0301>`: NFD `a<U+0300><U+0301>` = 61cc80cc81.
- `unicode-nfd-order-same-class-acute-grave`: U+0301 and U+0300 both have ccc 230: their order is kept. `a<U+0301><U+0300>`: NFD `a<U+0301><U+0300>` = 61cc81cc80.
- `unicode-nfd-order-three-classes-reversed`: classes 230 220 202 are fully sorted to 202 220 230. `a<U+0301><U+0323><U+0328>`: NFD `a<U+0328><U+0323><U+0301>` = 61cca8cca3cc81.
- `unicode-nfd-order-stable-equal-classes`: classes 230 220 230 sort to 220 230 230, the two 230s in input order. `a<U+0301><U+0323><U+0300>`: NFD `a<U+0323><U+0301><U+0300>` = 61cca3cc81cc80.
- `unicode-nfd-order-after-precomposed`: U+00E1 decomposes, then U+0323 moves before the acute. `<U+00E1><U+0323>`: NFD `a<U+0323><U+0301>` = 61cca3cc81.
- `unicode-nfd-order-d-dot-above-dot-below`: U+1E0B + U+0323 gives d U+0323 U+0307 (UAX 15 example). `<U+1E0B><U+0323>`: NFD `d<U+0323><U+0307>` = 64cca3cc87.
- `unicode-nfd-order-marks-without-base`: reordering also applies to marks at the start of the text. `<U+0301><U+0323>`: NFD `<U+0323><U+0301>` = cca3cc81.
- `unicode-nfd-order-not-across-starter`: marks are never moved across a starter. `a<U+0301>b<U+0323>`: NFD `a<U+0301>b<U+0323>` = 61cc8162cca3.
- `unicode-nfd-order-non-bmp-mark`: U+1D165 (ccc 216, non-BMP) moves before U+0301 (ccc 230). `a<U+0301><U+1D165>`: NFD `a<U+1D165><U+0301>` = 61f09d85a5cc81.
- `unicode-nfd-recursive-u-diaeresis-macron`: U+01D5 -> U+00DC U+0304 -> U U+0308 U+0304. `<U+01D5>`: NFD `U<U+0308><U+0304>` = 55cc88cc84.
- `unicode-nfd-recursive-iota-dialytika-tonos`: U+0390 -> U+03CA U+0301 -> U+03B9 U+0308 U+0301. `<U+0390>`: NFD `<U+03B9><U+0308><U+0301>` = ceb9cc88cc81.
- `unicode-nfd-musical-half-note`: non-BMP U+1D15E decomposes to U+1D157 U+1D165. `<U+1D15E>`: NFD `<U+1D157><U+1D165>` = f09d8597f09d85a5.
- `unicode-nfd-musical-eighth-note-recursive`: U+1D160 -> U+1D15F U+1D16E -> U+1D158 U+1D165 U+1D16E. `<U+1D160>`: NFD `<U+1D158><U+1D165><U+1D16E>` = f09d8598f09d85a5f09d85ae.
- `unicode-nfd-cjk-compat-ideograph`: CJK compatibility ideograph U+F900 has a canonical singleton to U+8C48. `<U+F900>`: NFD `<U+8C48>` = e8b188.
- `unicode-nfd-cjk-compat-ideograph-supplement`: non-BMP U+2F800 has a canonical singleton to U+4E3D. `<U+2F800>`: NFD `<U+4E3D>` = e4b8bd.
- `unicode-nfd-greek-1f88`: U+1F88 decomposes to U+0391 U+0313 U+0345; nfd keeps the capital and the ypogegrammeni. `<U+1F88>`: NFD `<U+0391><U+0313><U+0345>` = ce91cc93cd85.
- `unicode-nfd-greek-prosgegrammeni-singleton`: U+1FBE has a canonical singleton to U+03B9. `<U+1FBE>`: NFD `<U+03B9>` = ceb9.
- `unicode-nfd-long-s-dot-above`: U+1E9B decomposes to U+017F U+0307; nfd does not fold the long s. `<U+1E9B>`: NFD `<U+017F><U+0307>` = c5bfcc87.
- `unicode-nfd-pass-through-cjk-emoji`: characters without decomposition, BMP and non-BMP, pass through. `<U+4E2D><U+1F600>`: NFD `<U+4E2D><U+1F600>` = e4b8adf09f9880.
- `unicode-nfd-pass-through-unassigned-plane-4`: unassigned U+40000 passes through unchanged. `<U+40000>`: NFD `<U+40000>` = f1808080.
- `unicode-nfd-pass-through-private-use`: private-use U+E000 passes through unchanged. `<U+E000>`: NFD `<U+E000>` = ee8080.
- `unicode-nfd-max-code-point`: U+10FFFF (F4 8F BF BF) is the last valid scalar value and passes through. `<U+10FFFF>`: NFD `<U+10FFFF>` = f48fbfbf.

### unicode fold

- `unicode-fold-ascii`: ASCII capitals fold to small letters; space, digits and hyphen are unchanged. `Hello<U+0020>World-123`: toCasefold (ASCII capitals C) `hello<U+0020>world-123` = 68656c6c6f20776f726c642d313233.
- `unicode-fold-capital-e-acute-not-normalized`: contract example: fold of U+00C9 is U+00E9 (C), not decomposed. `<U+00C9>`: toCasefold (U+00C9 C -> `<U+00E9>`) `<U+00E9>` = c3a9.
- `unicode-fold-decomposed-stays-decomposed`: fold of E + U+0301 is e + U+0301: fold does not compose. `E<U+0301>`: toCasefold (ASCII capitals C) `e<U+0301>` = 65cc81.
- `unicode-fold-small-e-acute-unchanged`: U+00E9 has no folding and fold does not decompose. `<U+00E9>`: toCasefold (no C or F entry applies) `<U+00E9>` = c3a9.
- `unicode-fold-sharp-s-full`: full folding F maps U+00DF to ss. `<U+00DF>`: toCasefold (U+00DF F -> `ss`) `ss` = 7373.
- `unicode-fold-capital-sharp-s-full`: full folding F maps U+1E9E to ss (the S mapping to U+00DF is not used). `<U+1E9E>`: toCasefold (U+1E9E F -> `ss`) `ss` = 7373.
- `unicode-fold-final-sigma`: C maps final sigma U+03C2 to U+03C3. `<U+03C2>`: toCasefold (U+03C2 C -> `<U+03C3>`) `<U+03C3>` = cf83.
- `unicode-fold-capital-sigma`: C maps U+03A3 to U+03C3. `<U+03A3>`: toCasefold (U+03A3 C -> `<U+03C3>`) `<U+03C3>` = cf83.
- `unicode-fold-sigma-no-final-context`: folding has no final-sigma context rule: every capital sigma becomes U+03C3. `<U+03A3><U+0391><U+03A3>`: toCasefold (U+03A3 C -> `<U+03C3>`, U+0391 C -> `<U+03B1>`) `<U+03C3><U+03B1><U+03C3>` = cf83ceb1cf83.
- `unicode-fold-dotted-capital-i-full`: F maps U+0130 to i + U+0307 (the Turkic T mapping to i is not used). `<U+0130>`: toCasefold (U+0130 F -> `i<U+0307>`) `i<U+0307>` = 69cc87.
- `unicode-fold-capital-i-not-turkic`: U+0049 folds to i, not dotless i (no T mappings). `I`: toCasefold (ASCII capitals C) `i` = 69.
- `unicode-fold-dotless-i-unchanged`: U+0131 has no C or F entry and stays dotless. `<U+0131>`: toCasefold (no C or F entry applies) `<U+0131>` = c4b1.
- `unicode-fold-kelvin`: C maps the Kelvin sign to k. `<U+212A>`: toCasefold (U+212A C -> `k`) `k` = 6b.
- `unicode-fold-angstrom-not-normalized`: C maps the Angstrom sign to U+00E5, which fold leaves precomposed. `<U+212B>`: toCasefold (U+212B C -> `<U+00E5>`) `<U+00E5>` = c3a5.
- `unicode-fold-ohm`: C maps the ohm sign to U+03C9. `<U+2126>`: toCasefold (U+2126 C -> `<U+03C9>`) `<U+03C9>` = cf89.
- `unicode-fold-fi-ligature-full`: F maps U+FB01 to fi. `<U+FB01>`: toCasefold (U+FB01 F -> `fi`) `fi` = 6669.
- `unicode-fold-long-s-t-ligature-full`: F maps U+FB05 to st. `<U+FB05>`: toCasefold (U+FB05 F -> `st`) `st` = 7374.
- `unicode-fold-long-s`: C maps long s U+017F to s. `<U+017F>`: toCasefold (U+017F C -> `s`) `s` = 73.
- `unicode-fold-micro-sign`: C maps the micro sign to U+03BC. `<U+00B5>`: toCasefold (U+00B5 C -> `<U+03BC>`) `<U+03BC>` = cebc.
- `unicode-fold-ypogegrammeni`: C maps combining U+0345 to U+03B9. `<U+0345>`: toCasefold (U+0345 C -> `<U+03B9>`) `<U+03B9>` = ceb9.
- `unicode-fold-greek-1f88-full`: F maps U+1F88 to U+1F00 U+03B9 (not the simple S mapping U+1F80), left precomposed. `<U+1F88>`: toCasefold (U+1F88 F -> `<U+1F00><U+03B9>`) `<U+1F00><U+03B9>` = e1bc80ceb9.
- `unicode-fold-greek-1f80-full`: F maps small U+1F80 to U+1F00 U+03B9 as well. `<U+1F80>`: toCasefold (U+1F80 F -> `<U+1F00><U+03B9>`) `<U+1F00><U+03B9>` = e1bc80ceb9.
- `unicode-fold-greek-upsilon-dialytika-tonos`: F maps U+03B0 to U+03C5 U+0308 U+0301. `<U+03B0>`: toCasefold (U+03B0 F -> `<U+03C5><U+0308><U+0301>`) `<U+03C5><U+0308><U+0301>` = cf85cc88cc81.
- `unicode-fold-cherokee-small-to-capital`: Cherokee folds to the capital letters: C maps U+AB70 to U+13A0. `<U+AB70>`: toCasefold (U+AB70 C -> `<U+13A0>`) `<U+13A0>` = e18ea0.
- `unicode-fold-cherokee-capital-unchanged`: U+13A0 has no folding (it is the folded form), not lowercased to U+AB70. `<U+13A0>`: toCasefold (no C or F entry applies) `<U+13A0>` = e18ea0.
- `unicode-fold-cherokee-small-ye`: C maps U+13F8 to U+13F0. `<U+13F8>`: toCasefold (U+13F8 C -> `<U+13F0>`) `<U+13F0>` = e18fb0.
- `unicode-fold-georgian-mtavruli`: C maps Mtavruli U+1C90 to Mkhedruli U+10D0. `<U+1C90>`: toCasefold (U+1C90 C -> `<U+10D0>`) `<U+10D0>` = e18390.
- `unicode-fold-georgian-mkhedruli-unchanged`: U+10D0 has no folding. `<U+10D0>`: toCasefold (no C or F entry applies) `<U+10D0>` = e18390.
- `unicode-fold-georgian-asomtavruli`: C maps Asomtavruli U+10A0 to Nuskhuri U+2D00. `<U+10A0>`: toCasefold (U+10A0 C -> `<U+2D00>`) `<U+2D00>` = e2b480.
- `unicode-fold-fullwidth-capital-a`: C maps U+FF21 to fullwidth U+FF41, not to a. `<U+FF21>`: toCasefold (U+FF21 C -> `<U+FF41>`) `<U+FF41>` = efbd81.
- `unicode-fold-roman-numeral-one`: C maps U+2160 to U+2170, not to i. `<U+2160>`: toCasefold (U+2160 C -> `<U+2170>`) `<U+2170>` = e285b0.
- `unicode-fold-circled-capital-a`: C maps U+24B6 to U+24D0. `<U+24B6>`: toCasefold (U+24B6 C -> `<U+24D0>`) `<U+24D0>` = e29390.
- `unicode-fold-superscript-two-unchanged`: U+00B2 has no folding. `<U+00B2>`: toCasefold (no C or F entry applies) `<U+00B2>` = c2b2.
- `unicode-fold-math-bold-capital-a-unchanged`: U+1D400 has no folding (compatibility mappings are not applied). `<U+1D400>`: toCasefold (no C or F entry applies) `<U+1D400>` = f09d9080.
- `unicode-fold-deseret-non-bmp`: C maps non-BMP U+10400 to U+10428. `<U+10400>`: toCasefold (U+10400 C -> `<U+10428>`) `<U+10428>` = f09090a8.
- `unicode-fold-n-preceded-by-apostrophe`: F maps U+0149 to U+02BC n. `<U+0149>`: toCasefold (U+0149 F -> `<U+02BC>n`) `<U+02BC>n` = cabc6e.
- `unicode-fold-armenian-ech-yiwn`: F maps U+0587 to U+0565 U+0582. `<U+0587>`: toCasefold (U+0587 F -> `<U+0565><U+0582>`) `<U+0565><U+0582>` = d5a5d682.
- `unicode-fold-dz-titlecase`: C maps titlecase U+01C5 to U+01C6. `<U+01C5>`: toCasefold (U+01C5 C -> `<U+01C6>`) `<U+01C6>` = c786.
- `unicode-fold-hangul-not-decomposed`: U+AC01 has no folding and fold does not decompose. `<U+AC01>`: toCasefold (no C or F entry applies) `<U+AC01>` = eab081.
- `unicode-fold-cyrillic`: C maps U+0416 U+0401 to U+0436 U+0451. `<U+0416><U+0401>`: toCasefold (U+0416 C -> `<U+0436>`, U+0401 C -> `<U+0451>`) `<U+0436><U+0451>` = d0b6d191.

### unicode key

- `unicode-key-ascii-upper`: ASCII capitals: key of ABC equals key of abc. `ABC`: NFD `ABC`, toCasefold (ASCII capitals C) `abc`, NFD `abc` = 616263.
- `unicode-key-ascii-lower`: ASCII small letters are their own key. `abc`: NFD `abc`, toCasefold (no C or F entry applies) `abc`, NFD `abc` = 616263.
- `unicode-key-ascii-path`: an ASCII path folds component by component; slash and dot are unchanged. `Basic/Scenes/Main.JSON`: NFD `Basic/Scenes/Main.JSON`, toCasefold (ASCII capitals C) `basic/scenes/main.json`, NFD `basic/scenes/main.json` = 62617369632f7363656e65732f6d61696e2e6a736f6e.
- `unicode-key-e-acute-precomposed`: U+00E9 and e + U+0301 have equal keys. `<U+00E9>`: NFD `e<U+0301>`, toCasefold (no C or F entry applies) `e<U+0301>`, NFD `e<U+0301>` = 65cc81.
- `unicode-key-e-acute-decomposed`: e + U+0301 and U+00E9 have equal keys. `e<U+0301>`: NFD `e<U+0301>`, toCasefold (no C or F entry applies) `e<U+0301>`, NFD `e<U+0301>` = 65cc81.
- `unicode-key-capital-e-acute-precomposed`: U+00C9 and U+00E9 have equal keys. `<U+00C9>`: NFD `E<U+0301>`, toCasefold (ASCII capitals C) `e<U+0301>`, NFD `e<U+0301>` = 65cc81.
- `unicode-key-capital-e-acute-decomposed`: E + U+0301 has the same key as U+00E9. `E<U+0301>`: NFD `E<U+0301>`, toCasefold (ASCII capitals C) `e<U+0301>`, NFD `e<U+0301>` = 65cc81.
- `unicode-key-sharp-s`: U+00DF, ss and U+1E9E have equal keys. `<U+00DF>`: NFD `<U+00DF>`, toCasefold (U+00DF F -> `ss`) `ss`, NFD `ss` = 7373.
- `unicode-key-ss`: ss, U+00DF and U+1E9E have equal keys. `ss`: NFD `ss`, toCasefold (no C or F entry applies) `ss`, NFD `ss` = 7373.
- `unicode-key-capital-sharp-s`: U+1E9E, U+00DF and ss have equal keys. `<U+1E9E>`: NFD `<U+1E9E>`, toCasefold (U+1E9E F -> `ss`) `ss`, NFD `ss` = 7373.
- `unicode-key-capital-ss`: SS has the same key as U+00DF. `SS`: NFD `SS`, toCasefold (ASCII capitals C) `ss`, NFD `ss` = 7373.
- `unicode-key-final-sigma`: U+03C2, U+03C3 and U+03A3 have equal keys (C mappings). `<U+03C2>`: NFD `<U+03C2>`, toCasefold (U+03C2 C -> `<U+03C3>`) `<U+03C3>`, NFD `<U+03C3>` = cf83.
- `unicode-key-small-sigma`: U+03C3, U+03C2 and U+03A3 have equal keys. `<U+03C3>`: NFD `<U+03C3>`, toCasefold (no C or F entry applies) `<U+03C3>`, NFD `<U+03C3>` = cf83.
- `unicode-key-capital-sigma`: U+03A3, U+03C3 and U+03C2 have equal keys. `<U+03A3>`: NFD `<U+03A3>`, toCasefold (U+03A3 C -> `<U+03C3>`) `<U+03C3>`, NFD `<U+03C3>` = cf83.
- `unicode-key-dotted-capital-i`: U+0130 has the key i U+0307, equal to i + U+0307 and unequal to i. `<U+0130>`: NFD `I<U+0307>`, toCasefold (ASCII capitals C) `i<U+0307>`, NFD `i<U+0307>` = 69cc87.
- `unicode-key-i-dot-above`: i + U+0307 has the same key as U+0130. `i<U+0307>`: NFD `i<U+0307>`, toCasefold (no C or F entry applies) `i<U+0307>`, NFD `i<U+0307>` = 69cc87.
- `unicode-key-capital-i-dot-above`: I + U+0307 (the NFD of U+0130) has the same key as U+0130. `I<U+0307>`: NFD `I<U+0307>`, toCasefold (ASCII capitals C) `i<U+0307>`, NFD `i<U+0307>` = 69cc87.
- `unicode-key-i`: i is its own key, unequal to the key of U+0130. `i`: NFD `i`, toCasefold (no C or F entry applies) `i`, NFD `i` = 69.
- `unicode-key-capital-i`: U+0049 folds to i (no Turkic T mapping). `I`: NFD `I`, toCasefold (ASCII capitals C) `i`, NFD `i` = 69.
- `unicode-key-dotless-i`: U+0131 keeps its identity: key unequal to the keys of i and I. `<U+0131>`: NFD `<U+0131>`, toCasefold (no C or F entry applies) `<U+0131>`, NFD `<U+0131>` = c4b1.
- `unicode-key-kelvin`: Kelvin sign and k have equal keys. `<U+212A>`: NFD `K`, toCasefold (ASCII capitals C) `k`, NFD `k` = 6b.
- `unicode-key-small-k`: k and the Kelvin sign have equal keys. `k`: NFD `k`, toCasefold (no C or F entry applies) `k`, NFD `k` = 6b.
- `unicode-key-capital-k`: K and the Kelvin sign have equal keys. `K`: NFD `K`, toCasefold (ASCII capitals C) `k`, NFD `k` = 6b.
- `unicode-key-angstrom`: U+212B, U+00C5 and A + U+030A have equal keys. `<U+212B>`: NFD `A<U+030A>`, toCasefold (ASCII capitals C) `a<U+030A>`, NFD `a<U+030A>` = 61cc8a.
- `unicode-key-a-ring-capital-precomposed`: U+00C5, U+212B and A + U+030A have equal keys. `<U+00C5>`: NFD `A<U+030A>`, toCasefold (ASCII capitals C) `a<U+030A>`, NFD `a<U+030A>` = 61cc8a.
- `unicode-key-a-ring-capital-decomposed`: A + U+030A, U+00C5 and U+212B have equal keys. `A<U+030A>`: NFD `A<U+030A>`, toCasefold (ASCII capitals C) `a<U+030A>`, NFD `a<U+030A>` = 61cc8a.
- `unicode-key-a-ring-small-precomposed`: U+00E5 has the same key as U+212B. `<U+00E5>`: NFD `a<U+030A>`, toCasefold (no C or F entry applies) `a<U+030A>`, NFD `a<U+030A>` = 61cc8a.
- `unicode-key-ohm`: ohm sign and U+03C9 have equal keys. `<U+2126>`: NFD `<U+03A9>`, toCasefold (U+03A9 C -> `<U+03C9>`) `<U+03C9>`, NFD `<U+03C9>` = cf89.
- `unicode-key-omega-small`: U+03C9 and the ohm sign have equal keys. `<U+03C9>`: NFD `<U+03C9>`, toCasefold (no C or F entry applies) `<U+03C9>`, NFD `<U+03C9>` = cf89.
- `unicode-key-omega-capital`: U+03A9 has the same key as the ohm sign. `<U+03A9>`: NFD `<U+03A9>`, toCasefold (U+03A9 C -> `<U+03C9>`) `<U+03C9>`, NFD `<U+03C9>` = cf89.
- `unicode-key-hangul-syllable`: U+AC01 and the jamo U+1100 U+1161 U+11A8 have equal keys. `<U+AC01>`: NFD `<U+1100><U+1161><U+11A8>`, toCasefold (no C or F entry applies) `<U+1100><U+1161><U+11A8>`, NFD `<U+1100><U+1161><U+11A8>` = e18480e185a1e186a8.
- `unicode-key-hangul-jamo`: the jamo sequence and U+AC01 have equal keys. `<U+1100><U+1161><U+11A8>`: NFD `<U+1100><U+1161><U+11A8>`, toCasefold (no C or F entry applies) `<U+1100><U+1161><U+11A8>`, NFD `<U+1100><U+1161><U+11A8>` = e18480e185a1e186a8.
- `unicode-key-hangul-lv-plus-trailing-jamo`: LV syllable U+AC00 + T jamo U+11A8 has the same key as U+AC01. `<U+AC00><U+11A8>`: NFD `<U+1100><U+1161><U+11A8>`, toCasefold (no C or F entry applies) `<U+1100><U+1161><U+11A8>`, NFD `<U+1100><U+1161><U+11A8>` = e18480e185a1e186a8.
- `unicode-key-order-dot-below-acute`: a U+0323 U+0301 and a U+0301 U+0323 have equal keys. `a<U+0323><U+0301>`: NFD `a<U+0323><U+0301>`, toCasefold (no C or F entry applies) `a<U+0323><U+0301>`, NFD `a<U+0323><U+0301>` = 61cca3cc81.
- `unicode-key-order-acute-dot-below`: a U+0301 U+0323 and a U+0323 U+0301 have equal keys. `a<U+0301><U+0323>`: NFD `a<U+0323><U+0301>`, toCasefold (no C or F entry applies) `a<U+0323><U+0301>`, NFD `a<U+0323><U+0301>` = 61cca3cc81.
- `unicode-key-order-grave-acute`: a U+0300 U+0301 differs from a U+0301 U+0300 (both ccc 230). `a<U+0300><U+0301>`: NFD `a<U+0300><U+0301>`, toCasefold (no C or F entry applies) `a<U+0300><U+0301>`, NFD `a<U+0300><U+0301>` = 61cc80cc81.
- `unicode-key-order-acute-grave`: a U+0301 U+0300 differs from a U+0300 U+0301 (both ccc 230). `a<U+0301><U+0300>`: NFD `a<U+0301><U+0300>`, toCasefold (no C or F entry applies) `a<U+0301><U+0300>`, NFD `a<U+0301><U+0300>` = 61cc81cc80.
- `unicode-key-order-capital-precomposed-then-mark`: U+00C1 + U+0323: decomposition, folding and reordering together. `<U+00C1><U+0323>`: NFD `A<U+0323><U+0301>`, toCasefold (ASCII capitals C) `a<U+0323><U+0301>`, NFD `a<U+0323><U+0301>` = 61cca3cc81.
- `unicode-key-order-precomposed-dot-below-then-acute`: U+1EA1 + U+0301 has the same key as a U+0301 U+0323. `<U+1EA1><U+0301>`: NFD `a<U+0323><U+0301>`, toCasefold (no C or F entry applies) `a<U+0323><U+0301>`, NFD `a<U+0323><U+0301>` = 61cca3cc81.
- `unicode-key-fi-ligature`: key of U+FB01 is fi (full folding F). `<U+FB01>`: NFD `<U+FB01>`, toCasefold (U+FB01 F -> `fi`) `fi`, NFD `fi` = 6669.
- `unicode-key-fi`: fi has the same key as U+FB01. `fi`: NFD `fi`, toCasefold (no C or F entry applies) `fi`, NFD `fi` = 6669.
- `unicode-key-capital-fi`: FI has the same key as U+FB01. `FI`: NFD `FI`, toCasefold (ASCII capitals C) `fi`, NFD `fi` = 6669.
- `unicode-key-long-s-t-ligature`: U+FB05 and U+FB06 both have the key st. `<U+FB05>`: NFD `<U+FB05>`, toCasefold (U+FB05 F -> `st`) `st`, NFD `st` = 7374.
- `unicode-key-st-ligature`: U+FB06 and U+FB05 both have the key st. `<U+FB06>`: NFD `<U+FB06>`, toCasefold (U+FB06 F -> `st`) `st`, NFD `st` = 7374.
- `unicode-key-fullwidth-capital-a`: U+FF21 keeps its identity apart from a: key is U+FF41. `<U+FF21>`: NFD `<U+FF21>`, toCasefold (U+FF21 C -> `<U+FF41>`) `<U+FF41>`, NFD `<U+FF41>` = efbd81.
- `unicode-key-fullwidth-small-a`: U+FF41 is the key of U+FF21, unequal to a. `<U+FF41>`: NFD `<U+FF41>`, toCasefold (no C or F entry applies) `<U+FF41>`, NFD `<U+FF41>` = efbd81.
- `unicode-key-superscript-two`: U+00B2 keeps its identity: key unequal to the key of 2. `<U+00B2>`: NFD `<U+00B2>`, toCasefold (no C or F entry applies) `<U+00B2>`, NFD `<U+00B2>` = c2b2.
- `unicode-key-digit-two`: 2 is its own key, unequal to the key of U+00B2. `2`: NFD `2`, toCasefold (no C or F entry applies) `2`, NFD `2` = 32.
- `unicode-key-roman-numeral-one`: U+2160 folds to U+2170, not to i (no compatibility mapping). `<U+2160>`: NFD `<U+2160>`, toCasefold (U+2160 C -> `<U+2170>`) `<U+2170>`, NFD `<U+2170>` = e285b0.
- `unicode-key-musical-half-note`: non-BMP U+1D15E and U+1D157 U+1D165 have equal keys. `<U+1D15E>`: NFD `<U+1D157><U+1D165>`, toCasefold (no C or F entry applies) `<U+1D157><U+1D165>`, NFD `<U+1D157><U+1D165>` = f09d8597f09d85a5.
- `unicode-key-musical-half-note-decomposed`: U+1D157 U+1D165 and U+1D15E have equal keys. `<U+1D157><U+1D165>`: NFD `<U+1D157><U+1D165>`, toCasefold (no C or F entry applies) `<U+1D157><U+1D165>`, NFD `<U+1D157><U+1D165>` = f09d8597f09d85a5.
- `unicode-key-cherokee-capital`: U+13A0 and U+AB70 have equal keys, U+13A0 (the folded form). `<U+13A0>`: NFD `<U+13A0>`, toCasefold (no C or F entry applies) `<U+13A0>`, NFD `<U+13A0>` = e18ea0.
- `unicode-key-cherokee-small`: U+AB70 and U+13A0 have equal keys. `<U+AB70>`: NFD `<U+AB70>`, toCasefold (U+AB70 C -> `<U+13A0>`) `<U+13A0>`, NFD `<U+13A0>` = e18ea0.
- `unicode-key-greek-1f88`: U+1F88 (F mapping) has the key U+03B1 U+0313 U+03B9. `<U+1F88>`: NFD `<U+0391><U+0313><U+0345>`, toCasefold (U+0391 C -> `<U+03B1>`, U+0345 C -> `<U+03B9>`) `<U+03B1><U+0313><U+03B9>`, NFD `<U+03B1><U+0313><U+03B9>` = ceb1cc93ceb9.
- `unicode-key-greek-1f80`: U+1F80 has the same key as U+1F88. `<U+1F80>`: NFD `<U+03B1><U+0313><U+0345>`, toCasefold (U+0345 C -> `<U+03B9>`) `<U+03B1><U+0313><U+03B9>`, NFD `<U+03B1><U+0313><U+03B9>` = ceb1cc93ceb9.
- `unicode-key-greek-alpha-psili-iota`: U+1F00 U+03B9 has the same key as U+1F88. `<U+1F00><U+03B9>`: NFD `<U+03B1><U+0313><U+03B9>`, toCasefold (no C or F entry applies) `<U+03B1><U+0313><U+03B9>`, NFD `<U+03B1><U+0313><U+03B9>` = ceb1cc93ceb9.
- `unicode-key-greek-ypogegrammeni-before-psili`: inner NFD moves U+0345 (ccc 240) after U+0313 before folding; folding first would give U+03B1 U+03B9 U+0313. `<U+03B1><U+0345><U+0313>`: NFD `<U+03B1><U+0313><U+0345>`, toCasefold (U+0345 C -> `<U+03B9>`) `<U+03B1><U+0313><U+03B9>`, NFD `<U+03B1><U+0313><U+03B9>` = ceb1cc93ceb9.
- `unicode-key-greek-alpha-ypogegrammeni-then-psili`: U+1FB3 + U+0313 has the same key as U+1F80 only because of the inner NFD (D145). `<U+1FB3><U+0313>`: NFD `<U+03B1><U+0313><U+0345>`, toCasefold (U+0345 C -> `<U+03B9>`) `<U+03B1><U+0313><U+03B9>`, NFD `<U+03B1><U+0313><U+03B9>` = ceb1cc93ceb9.
- `unicode-key-greek-1fb4`: U+1FB4 -> U+03B1 U+0301 U+0345, U+0345 folds to U+03B9. `<U+1FB4>`: NFD `<U+03B1><U+0301><U+0345>`, toCasefold (U+0345 C -> `<U+03B9>`) `<U+03B1><U+0301><U+03B9>`, NFD `<U+03B1><U+0301><U+03B9>` = ceb1cc81ceb9.
- `unicode-key-greek-prosgegrammeni`: U+1FBE has the same key as U+03B9 and U+0345. `<U+1FBE>`: NFD `<U+03B9>`, toCasefold (no C or F entry applies) `<U+03B9>`, NFD `<U+03B9>` = ceb9.
- `unicode-key-georgian-mkhedruli`: U+10D0 and U+1C90 have equal keys. `<U+10D0>`: NFD `<U+10D0>`, toCasefold (no C or F entry applies) `<U+10D0>`, NFD `<U+10D0>` = e18390.
- `unicode-key-georgian-mtavruli`: U+1C90 and U+10D0 have equal keys. `<U+1C90>`: NFD `<U+1C90>`, toCasefold (U+1C90 C -> `<U+10D0>`) `<U+10D0>`, NFD `<U+10D0>` = e18390.
- `unicode-key-long-s-dot-above`: U+1E9B has the same key as U+1E60 and U+1E61 (s U+0307). `<U+1E9B>`: NFD `<U+017F><U+0307>`, toCasefold (U+017F C -> `s`) `s<U+0307>`, NFD `s<U+0307>` = 73cc87.
- `unicode-key-s-dot-above-capital`: U+1E60 has the same key as U+1E9B. `<U+1E60>`: NFD `S<U+0307>`, toCasefold (ASCII capitals C) `s<U+0307>`, NFD `s<U+0307>` = 73cc87.
- `unicode-key-micro-sign`: micro sign and U+03BC have equal keys (C mapping, not compatibility). `<U+00B5>`: NFD `<U+00B5>`, toCasefold (U+00B5 C -> `<U+03BC>`) `<U+03BC>`, NFD `<U+03BC>` = cebc.
- `unicode-key-greek-capital-mu`: U+039C has the same key as the micro sign. `<U+039C>`: NFD `<U+039C>`, toCasefold (U+039C C -> `<U+03BC>`) `<U+03BC>`, NFD `<U+03BC>` = cebc.
- `unicode-key-deseret-capital`: non-BMP U+10400 has the key U+10428. `<U+10400>`: NFD `<U+10400>`, toCasefold (U+10400 C -> `<U+10428>`) `<U+10428>`, NFD `<U+10428>` = f09090a8.
- `unicode-key-cjk-compat-ideograph`: U+F900 and U+8C48 have equal keys (canonical singleton). `<U+F900>`: NFD `<U+8C48>`, toCasefold (no C or F entry applies) `<U+8C48>`, NFD `<U+8C48>` = e8b188.
- `unicode-key-dz-digraph-capital`: U+01C4 folds to U+01C6, which keeps its identity apart from d z U+030C. `<U+01C4>`: NFD `<U+01C4>`, toCasefold (U+01C4 C -> `<U+01C6>`) `<U+01C6>`, NFD `<U+01C6>` = c786.
- `unicode-key-path-mixed`: a multi-component path with spaces and accented capitals. `Basic/SC<U+00C8>NES/My<U+0020>Caf<U+00C9>.json`: NFD `Basic/SCE<U+0300>NES/My<U+0020>CafE<U+0301>.json`, toCasefold (ASCII capitals C) `basic/sce<U+0300>nes/my<U+0020>cafe<U+0301>.json`, NFD `basic/sce<U+0300>nes/my<U+0020>cafe<U+0301>.json` = 62617369632f736365cc806e65732f6d792063616665cc812e6a736f6e.
- `unicode-key-greek-word`: U+039F U+0394 U+038C U+03A3: capitals with tonos fold and decompose. `<U+039F><U+0394><U+038C><U+03A3>`: NFD `<U+039F><U+0394><U+039F><U+0301><U+03A3>`, toCasefold (U+039F C -> `<U+03BF>`, U+0394 C -> `<U+03B4>`, U+03A3 C -> `<U+03C3>`) `<U+03BF><U+03B4><U+03BF><U+0301><U+03C3>`, NFD `<U+03BF><U+03B4><U+03BF><U+0301><U+03C3>` = cebfceb4cebfcc81cf83.
- `unicode-key-min-2-byte`: U+0080 (C2 80) is the smallest 2-byte sequence and passes through. `<U+0080>`: NFD `<U+0080>`, toCasefold (no C or F entry applies) `<U+0080>`, NFD `<U+0080>` = c280.
- `unicode-key-max-2-byte`: U+07FF (DF BF) is the largest 2-byte sequence and passes through. `<U+07FF>`: NFD `<U+07FF>`, toCasefold (no C or F entry applies) `<U+07FF>`, NFD `<U+07FF>` = dfbf.
- `unicode-key-min-3-byte`: U+0800 (E0 A0 80) is the smallest 3-byte sequence and passes through. `<U+0800>`: NFD `<U+0800>`, toCasefold (no C or F entry applies) `<U+0800>`, NFD `<U+0800>` = e0a080.
- `unicode-key-before-surrogates`: U+D7FF (ED 9F BF), unassigned, is just below the surrogates and passes through. `<U+D7FF>`: NFD `<U+D7FF>`, toCasefold (no C or F entry applies) `<U+D7FF>`, NFD `<U+D7FF>` = ed9fbf.
- `unicode-key-after-surrogates`: U+E000 (EE 80 80) is just above the surrogates and passes through. `<U+E000>`: NFD `<U+E000>`, toCasefold (no C or F entry applies) `<U+E000>`, NFD `<U+E000>` = ee8080.
- `unicode-key-max-3-byte`: U+FFFF (EF BF BF), a noncharacter but well-formed UTF-8, passes through. `<U+FFFF>`: NFD `<U+FFFF>`, toCasefold (no C or F entry applies) `<U+FFFF>`, NFD `<U+FFFF>` = efbfbf.
- `unicode-key-min-4-byte`: U+10000 (F0 90 80 80) is the smallest 4-byte sequence and passes through. `<U+10000>`: NFD `<U+10000>`, toCasefold (no C or F entry applies) `<U+10000>`, NFD `<U+10000>` = f0908080.
- `unicode-key-max-code-point`: U+10FFFF (F4 8F BF BF), the largest scalar value, passes through. `<U+10FFFF>`: NFD `<U+10FFFF>`, toCasefold (no C or F entry applies) `<U+10FFFF>`, NFD `<U+10FFFF>` = f48fbfbf.
- `unicode-key-unassigned-plane-4`: unassigned U+40000 passes through unchanged. `<U+40000>`: NFD `<U+40000>`, toCasefold (no C or F entry applies) `<U+40000>`, NFD `<U+40000>` = f1808080.

### unicode nfd, fold and key: not strict UTF-8

- `unicode-nfd-not-utf8-overlong-c0@posix`: contract list, nfd: overlong 2-byte NUL: bytes C0 80, rejected by Python's strict UTF-8 decoder.
- `unicode-nfd-not-utf8-overlong-c1@posix`: contract list, nfd: overlong 2-byte U+007F: bytes C1 BF, rejected by Python's strict UTF-8 decoder.
- `unicode-nfd-not-utf8-overlong-e0@posix`: contract list, nfd: overlong 3-byte NUL: bytes E0 80 80, rejected by Python's strict UTF-8 decoder.
- `unicode-nfd-not-utf8-surrogate@posix`: contract list, nfd: UTF-8 encoded surrogate U+D800: bytes ED A0 80, rejected by Python's strict UTF-8 decoder.
- `unicode-nfd-not-utf8-above-max@posix`: contract list, nfd: U+110000, above U+10FFFF: bytes F4 90 80 80, rejected by Python's strict UTF-8 decoder.
- `unicode-nfd-not-utf8-truncated@posix`: contract list, nfd: 3-byte sequence missing its last byte: bytes E2 82, rejected by Python's strict UTF-8 decoder.
- `unicode-nfd-not-utf8-stray-continuation@posix`: contract list, nfd: continuation byte without a lead byte: bytes 80, rejected by Python's strict UTF-8 decoder.
- `unicode-fold-not-utf8-overlong-c0@posix`: contract list, fold: overlong 2-byte NUL: bytes C0 80, rejected by Python's strict UTF-8 decoder.
- `unicode-fold-not-utf8-overlong-c1@posix`: contract list, fold: overlong 2-byte U+007F: bytes C1 BF, rejected by Python's strict UTF-8 decoder.
- `unicode-fold-not-utf8-overlong-e0@posix`: contract list, fold: overlong 3-byte NUL: bytes E0 80 80, rejected by Python's strict UTF-8 decoder.
- `unicode-fold-not-utf8-surrogate@posix`: contract list, fold: UTF-8 encoded surrogate U+D800: bytes ED A0 80, rejected by Python's strict UTF-8 decoder.
- `unicode-fold-not-utf8-above-max@posix`: contract list, fold: U+110000, above U+10FFFF: bytes F4 90 80 80, rejected by Python's strict UTF-8 decoder.
- `unicode-fold-not-utf8-truncated@posix`: contract list, fold: 3-byte sequence missing its last byte: bytes E2 82, rejected by Python's strict UTF-8 decoder.
- `unicode-fold-not-utf8-stray-continuation@posix`: contract list, fold: continuation byte without a lead byte: bytes 80, rejected by Python's strict UTF-8 decoder.
- `unicode-key-not-utf8-overlong-c0@posix`: contract list, key: overlong 2-byte NUL: bytes C0 80, rejected by Python's strict UTF-8 decoder.
- `unicode-key-not-utf8-overlong-c1@posix`: contract list, key: overlong 2-byte U+007F: bytes C1 BF, rejected by Python's strict UTF-8 decoder.
- `unicode-key-not-utf8-overlong-e0@posix`: contract list, key: overlong 3-byte NUL: bytes E0 80 80, rejected by Python's strict UTF-8 decoder.
- `unicode-key-not-utf8-surrogate@posix`: contract list, key: UTF-8 encoded surrogate U+D800: bytes ED A0 80, rejected by Python's strict UTF-8 decoder.
- `unicode-key-not-utf8-above-max@posix`: contract list, key: U+110000, above U+10FFFF: bytes F4 90 80 80, rejected by Python's strict UTF-8 decoder.
- `unicode-key-not-utf8-truncated@posix`: contract list, key: 3-byte sequence missing its last byte: bytes E2 82, rejected by Python's strict UTF-8 decoder.
- `unicode-key-not-utf8-stray-continuation@posix`: contract list, key: continuation byte without a lead byte: bytes 80, rejected by Python's strict UTF-8 decoder.
- `unicode-nfd-not-utf8-after-valid-text@posix`: nfd: valid decomposable text followed by FF: bytes C3 A9 FF, rejected by Python's strict UTF-8 decoder.
- `unicode-fold-not-utf8-after-valid-text@posix`: fold: valid foldable text followed by a stray continuation: bytes 41 C3 89 80, rejected by Python's strict UTF-8 decoder.
- `unicode-key-not-utf8-overlong-slash@posix`: key: overlong 2-byte slash: bytes C0 AF, rejected by Python's strict UTF-8 decoder.
- `unicode-key-not-utf8-overlong-e0-9f@posix`: key: overlong 3-byte U+07FF: bytes E0 9F BF, rejected by Python's strict UTF-8 decoder.
- `unicode-key-not-utf8-overlong-f0-80@posix`: key: overlong 4-byte NUL: bytes F0 80 80 80, rejected by Python's strict UTF-8 decoder.
- `unicode-key-not-utf8-overlong-f0-8f@posix`: key: overlong 4-byte U+FFFF: bytes F0 8F BF BF, rejected by Python's strict UTF-8 decoder.
- `unicode-key-not-utf8-surrogate-last@posix`: key: UTF-8 encoded surrogate U+DFFF: bytes ED BF BF, rejected by Python's strict UTF-8 decoder.
- `unicode-key-not-utf8-surrogate-pair-cesu@posix`: key: CESU-8 surrogate pair for U+1F600: bytes ED A0 BD ED B8 80, rejected by Python's strict UTF-8 decoder.
- `unicode-key-not-utf8-lead-f5@posix`: key: lead byte F5 (would be above U+10FFFF): bytes F5 80 80 80, rejected by Python's strict UTF-8 decoder.
- `unicode-key-not-utf8-lead-f7-max@posix`: key: lead byte F7 (U+1FFFFF): bytes F7 BF BF BF, rejected by Python's strict UTF-8 decoder.
- `unicode-key-not-utf8-five-byte@posix`: key: obsolete 5-byte form: bytes F8 88 80 80 80, rejected by Python's strict UTF-8 decoder.
- `unicode-key-not-utf8-byte-fe@posix`: key: byte FE never occurs in UTF-8: bytes FE, rejected by Python's strict UTF-8 decoder.
- `unicode-key-not-utf8-byte-ff@posix`: key: byte FF never occurs in UTF-8: bytes FF, rejected by Python's strict UTF-8 decoder.
- `unicode-key-not-utf8-truncated-at-end@posix`: key: valid ASCII then a 2-byte lead at the end: bytes 61 C3, rejected by Python's strict UTF-8 decoder.
- `unicode-key-not-utf8-truncated-4-byte@posix`: key: 4-byte sequence missing its last byte: bytes F0 9F 98, rejected by Python's strict UTF-8 decoder.
- `unicode-key-not-utf8-truncated-before-ascii@posix`: key: 3-byte sequence cut short by an ASCII byte: bytes E2 82 61, rejected by Python's strict UTF-8 decoder.
- `unicode-key-not-utf8-lead-after-lead@posix`: key: lead byte followed by another lead byte: bytes C3 C3 A9, rejected by Python's strict UTF-8 decoder.
- `unicode-key-not-utf8-stray-after-valid@posix`: key: valid U+00E9 followed by a stray continuation: bytes C3 A9 80, rejected by Python's strict UTF-8 decoder.
- `unicode-key-not-utf8-continuation-run@posix`: key: three continuation bytes: bytes 80 80 80, rejected by Python's strict UTF-8 decoder.
- `unicode-key-not-utf8-after-valid-text@posix`: key: valid foldable text followed by FF: bytes 41 C3 89 FF, rejected by Python's strict UTF-8 decoder.

### path

- `unicode-path-ok-nfd-combining`: decomposed e + U+0301 in a file name: `basic/scenes/Cafe<U+0301>.json` breaks no section 12 path rule, so plain OK (no `unchecked`).
- `unicode-path-ok-sharp-s`: U+00DF in a file name: `basic/scenes/Stra<U+00DF>e.json` breaks no section 12 path rule, so plain OK (no `unchecked`).
- `unicode-path-ok-capital-sharp-s`: U+1E9E in a file name: `basic/scenes/STRA<U+1E9E>E.json` breaks no section 12 path rule, so plain OK (no `unchecked`).
- `unicode-path-ok-greek-final-sigma`: Greek with final sigma: `basic/scenes/<U+03BB><U+03CC><U+03B3><U+03BF><U+03C2>.json` breaks no section 12 path rule, so plain OK (no `unchecked`).
- `unicode-path-ok-dotted-capital-i`: U+0130 in a file name: `basic/scenes/<U+0130>stanbul.json` breaks no section 12 path rule, so plain OK (no `unchecked`).
- `unicode-path-ok-dotless-i`: U+0131 file name: `<U+0131>.txt` breaks no section 12 path rule, so plain OK (no `unchecked`).
- `unicode-path-ok-kelvin-angstrom-ohm`: the three canonical singletons as components: `<U+212A>/<U+212B>/<U+2126>.txt` breaks no section 12 path rule, so plain OK (no `unchecked`).
- `unicode-path-ok-hangul-syllables`: Hangul syllables: `<U+AC01>/<U+D7A3>.txt` breaks no section 12 path rule, so plain OK (no `unchecked`).
- `unicode-path-ok-hangul-jamo`: conjoining jamo: `<U+1100><U+1161><U+11A8>.txt` breaks no section 12 path rule, so plain OK (no `unchecked`).
- `unicode-path-ok-fi-ligature`: U+FB01 ligature: `<U+FB01>le.txt` breaks no section 12 path rule, so plain OK (no `unchecked`).
- `unicode-path-ok-fullwidth`: fullwidth A: `<U+FF21>.txt` breaks no section 12 path rule, so plain OK (no `unchecked`).
- `unicode-path-ok-superscript-two`: superscript two outside a device name: `x<U+00B2>.txt` breaks no section 12 path rule, so plain OK (no `unchecked`).
- `unicode-path-ok-non-bmp`: non-BMP components (4-byte UTF-8): `<U+1D15E>/<U+10400>.txt` breaks no section 12 path rule, so plain OK (no `unchecked`).
- `unicode-path-ok-greek-1f88`: U+1F88: `<U+1F88>.txt` breaks no section 12 path rule, so plain OK (no `unchecked`).
- `unicode-path-ok-cherokee`: Cherokee small and capital letters: `<U+AB70><U+13A0>.txt` breaks no section 12 path rule, so plain OK (no `unchecked`).
- `unicode-path-ok-georgian`: Georgian Mtavruli and Mkhedruli: `<U+1C90><U+10D0>.txt` breaks no section 12 path rule, so plain OK (no `unchecked`).
- `unicode-path-ok-mark-at-component-start`: a component starting with a combining mark (no rule forbids it): `a/<U+0301>.txt` breaks no section 12 path rule, so plain OK (no `unchecked`).
- `unicode-path-ok-deep-non-ascii`: non-ASCII directory in a 4-component path: `plugin_config/<U+00C4><U+00D6><U+00DC>/sub/state.json` breaks no section 12 path rule, so plain OK (no `unchecked`).

### manifest

- `ucollision-nfc-vs-nfd`: two non-ASCII paths that differ by normalization form only (NFC U+00E9 vs NFD e U+0301). Added: e6 scenes `basic/scenes/Caf<U+00E9>.json`, key `basic/scenes/cafe<U+0301>.json`; e7 scenes `basic/scenes/Cafe<U+0301>.json`, key `basic/scenes/cafe<U+0301>.json`. Same root and equal key: e6 e7, so ERR E_COLLISION.
- `ucollision-nfc-vs-nfd-escaped`: normalization form only, the NFD path written with JSON \u escapes and the NFC path as raw UTF-8. Added: e6 scenes `basic/scenes/<U+00D1>and<U+00FA>.json`, key `basic/scenes/n<U+0303>andu<U+0301>.json`; e7 scenes `basic/scenes/N<U+0303>andu<U+0301>.json`, key `basic/scenes/n<U+0303>andu<U+0301>.json`. Same root and equal key: e6 e7, so ERR E_COLLISION.
- `ucollision-hangul-syllable-vs-jamo`: normalization form only: Hangul syllable U+AC01 vs its conjoining jamo. Added: e6 scenes `basic/scenes/<U+AC01>.json`, key `basic/scenes/<U+1100><U+1161><U+11A8>.json`; e7 scenes `basic/scenes/<U+1100><U+1161><U+11A8>.json`, key `basic/scenes/<U+1100><U+1161><U+11A8>.json`. Same root and equal key: e6 e7, so ERR E_COLLISION.
- `ucollision-angstrom-three-way`: three-way collision by normalization form only: U+212B, U+00C5, A U+030A. Added: e6 scenes `basic/scenes/<U+212B>.json`, key `basic/scenes/a<U+030A>.json`; e7 scenes `basic/scenes/<U+00C5>.json`, key `basic/scenes/a<U+030A>.json`; e8 scenes `basic/scenes/A<U+030A>.json`, key `basic/scenes/a<U+030A>.json`. Same root and equal key: e6 e7 e8, so ERR E_COLLISION.
- `ucollision-mark-order`: canonical ordering plus decomposition: a U+0301 U+0323 vs U+1EA1 U+0301. Added: e6 scenes `basic/scenes/a<U+0301><U+0323>.json`, key `basic/scenes/a<U+0323><U+0301>.json`; e7 scenes `basic/scenes/<U+1EA1><U+0301>.json`, key `basic/scenes/a<U+0323><U+0301>.json`. Same root and equal key: e6 e7, so ERR E_COLLISION.
- `ucollision-mark-order-same-class-ok`: U+0300 and U+0301 share ccc 230, so their two orders are different keys. Added: e6 scenes `basic/scenes/a<U+0300><U+0301>.json`, key `basic/scenes/a<U+0300><U+0301>.json`; e7 scenes `basic/scenes/a<U+0301><U+0300>.json`, key `basic/scenes/a<U+0301><U+0300>.json`. No two entries, base e1-e5 included, share root and key, so OK.
- `ucollision-deep-component-case`: case-only non-ASCII pair inside component 3 of a 5-component path (every ASCII byte identical). Added: e6 config `plugin_config/obs-websocket/<U+00C4><U+00D6><U+00DC>-<U+00C6><U+00D8><U+00C5>/sub/state.json`, key `plugin_config/obs-websocket/a<U+0308>o<U+0308>u<U+0308>-<U+00E6><U+00F8>a<U+030A>/sub/state.json`; e7 config `plugin_config/obs-websocket/<U+00E4><U+00F6><U+00FC>-<U+00E6><U+00F8><U+00E5>/sub/state.json`, key `plugin_config/obs-websocket/a<U+0308>o<U+0308>u<U+0308>-<U+00E6><U+00F8>a<U+030A>/sub/state.json`. Same root and equal key: e6 e7, so ERR E_COLLISION.
- `ucollision-deep-component-case-greek`: case-only Greek pair (with tonos) inside component 3 of a 4-component path. Added: e6 config `plugin_config/my-plugin/<U+0395><U+039B><U+039B><U+0386><U+0394><U+0391>/state.json`, key `plugin_config/my-plugin/<U+03B5><U+03BB><U+03BB><U+03B1><U+0301><U+03B4><U+03B1>/state.json`; e7 config `plugin_config/my-plugin/<U+0395><U+03BB><U+03BB><U+03AC><U+03B4><U+03B1>/state.json`, key `plugin_config/my-plugin/<U+03B5><U+03BB><U+03BB><U+03B1><U+0301><U+03B4><U+03B1>/state.json`. Same root and equal key: e6 e7, so ERR E_COLLISION.
- `ucollision-multi-component-case`: non-ASCII case differences in two components of the same path. Added: e6 config `plugin_config/<U+00D6>lfeld/<U+00DC>bersicht.json`, key `plugin_config/o<U+0308>lfeld/u<U+0308>bersicht.json`; e7 config `plugin_config/<U+00F6>lfeld/<U+00FC>bersicht.json`, key `plugin_config/o<U+0308>lfeld/u<U+0308>bersicht.json`. Same root and equal key: e6 e7, so ERR E_COLLISION.
- `ucollision-sharp-s-three-way`: three-way collision: U+00DF, ss and U+1E9E (full folding F). Added: e6 scenes `basic/scenes/Stra<U+00DF>e.json`, key `basic/scenes/strasse.json`; e7 scenes `basic/scenes/Strasse.json`, key `basic/scenes/strasse.json`; e8 scenes `basic/scenes/Stra<U+1E9E>e.json`, key `basic/scenes/strasse.json`. Same root and equal key: e6 e7 e8, so ERR E_COLLISION.
- `ucollision-sigma-three-way`: three-way collision: final sigma U+03C2, U+03C3 and U+03A3 (C mappings). Added: e6 scenes `basic/scenes/<U+03BB><U+03CC><U+03B3><U+03BF><U+03C2>.json`, key `basic/scenes/<U+03BB><U+03BF><U+0301><U+03B3><U+03BF><U+03C3>.json`; e7 scenes `basic/scenes/<U+03BB><U+03CC><U+03B3><U+03BF><U+03C3>.json`, key `basic/scenes/<U+03BB><U+03BF><U+0301><U+03B3><U+03BF><U+03C3>.json`; e8 scenes `basic/scenes/<U+03BB><U+03CC><U+03B3><U+03BF><U+03A3>.json`, key `basic/scenes/<U+03BB><U+03BF><U+0301><U+03B3><U+03BF><U+03C3>.json`. Same root and equal key: e6 e7 e8, so ERR E_COLLISION.
- `ucollision-dotted-capital-i`: U+0130 collides with i + U+0307. Added: e6 scenes `basic/scenes/<U+0130>stanbul.json`, key `basic/scenes/i<U+0307>stanbul.json`; e7 scenes `basic/scenes/i<U+0307>stanbul.json`, key `basic/scenes/i<U+0307>stanbul.json`. Same root and equal key: e6 e7, so ERR E_COLLISION.
- `ucollision-dotted-capital-i-ok`: no Turkic mappings: U+0130, I and U+0131 give three different keys (simple lowercasing or T mappings would collide). Added: e6 scenes `basic/scenes/<U+0130>stanbul.json`, key `basic/scenes/i<U+0307>stanbul.json`; e7 scenes `basic/scenes/Istanbul.json`, key `basic/scenes/istanbul.json`; e8 scenes `basic/scenes/<U+0131>stanbul.json`, key `basic/scenes/<U+0131>stanbul.json`. No two entries, base e1-e5 included, share root and key, so OK.
- `ucollision-kelvin`: Kelvin sign U+212A collides with k. Added: e6 scenes `basic/scenes/<U+212A>elvin.json`, key `basic/scenes/kelvin.json`; e7 scenes `basic/scenes/kelvin.json`, key `basic/scenes/kelvin.json`. Same root and equal key: e6 e7, so ERR E_COLLISION.
- `ucollision-ohm`: ohm sign U+2126 collides with U+03C9. Added: e6 scenes `basic/scenes/<U+2126>.json`, key `basic/scenes/<U+03C9>.json`; e7 scenes `basic/scenes/<U+03C9>.json`, key `basic/scenes/<U+03C9>.json`. Same root and equal key: e6 e7, so ERR E_COLLISION.
- `ucollision-fi-ligature`: U+FB01 folds to fi (F), so it collides with ASCII fi. Added: e6 scenes `basic/scenes/<U+FB01>le.json`, key `basic/scenes/file.json`; e7 scenes `basic/scenes/file.json`, key `basic/scenes/file.json`. Same root and equal key: e6 e7, so ERR E_COLLISION.
- `ucollision-micro-sign`: micro sign U+00B5 collides with U+03BC by its C folding. Added: e6 scenes `basic/scenes/<U+00B5>.json`, key `basic/scenes/<U+03BC>.json`; e7 scenes `basic/scenes/<U+03BC>.json`, key `basic/scenes/<U+03BC>.json`. Same root and equal key: e6 e7, so ERR E_COLLISION.
- `ucollision-fullwidth-case`: fullwidth U+FF21 and U+FF41 are a case pair (C mapping) and collide with each other. Added: e6 scenes `basic/scenes/<U+FF21>.json`, key `basic/scenes/<U+FF41>.json`; e7 scenes `basic/scenes/<U+FF41>.json`, key `basic/scenes/<U+FF41>.json`. Same root and equal key: e6 e7, so ERR E_COLLISION.
- `ucollision-compat-not-unified-ok`: compatibility characters are not unified: U+FF21 vs a, U+00B2 vs 2, U+2460 vs 1, U+2160 vs i, U+1D400 vs a, U+01C6 vs d z U+030C. Added: e6 scenes `basic/scenes/<U+FF21>.json`, key `basic/scenes/<U+FF41>.json`; e7 scenes `basic/scenes/a.json`, key `basic/scenes/a.json`; e8 scenes `basic/scenes/x<U+00B2>.json`, key `basic/scenes/x<U+00B2>.json`; e9 scenes `basic/scenes/x2.json`, key `basic/scenes/x2.json`; e10 scenes `basic/scenes/<U+2460>.json`, key `basic/scenes/<U+2460>.json`; e11 scenes `basic/scenes/1.json`, key `basic/scenes/1.json`; e12 scenes `basic/scenes/<U+2160>.json`, key `basic/scenes/<U+2170>.json`; e13 scenes `basic/scenes/i.json`, key `basic/scenes/i.json`; e14 scenes `basic/scenes/<U+1D400>b.json`, key `basic/scenes/<U+1D400>b.json`; e15 scenes `basic/scenes/ab.json`, key `basic/scenes/ab.json`; e16 scenes `basic/scenes/<U+01C6>.json`, key `basic/scenes/<U+01C6>.json`; e17 scenes `basic/scenes/dz<U+030C>.json`, key `basic/scenes/dz<U+030C>.json`. No two entries, base e1-e5 included, share root and key, so OK.
- `ucollision-stored-vs-omitted-long-s`: U+017F folds to s, so an omitted non-ASCII path collides with the stored ASCII entry e3 basic/scenes/Main.json. Added: e6 scenes `ba<U+017F>ic/<U+017F>cenes/Main.json`, key `basic/scenes/main.json`. Same root and equal key: e3 e6, so ERR E_COLLISION.
- `ucollision-deseret-case`: non-BMP case pair U+10400 / U+10428. Added: e6 scenes `basic/scenes/<U+10400>.json`, key `basic/scenes/<U+10428>.json`; e7 scenes `basic/scenes/<U+10428>.json`, key `basic/scenes/<U+10428>.json`. Same root and equal key: e6 e7, so ERR E_COLLISION.
- `ucollision-musical-symbol`: non-BMP canonical decomposition: U+1D15E vs U+1D157 U+1D165. Added: e6 scenes `basic/scenes/<U+1D15E>.json`, key `basic/scenes/<U+1D157><U+1D165>.json`; e7 scenes `basic/scenes/<U+1D157><U+1D165>.json`, key `basic/scenes/<U+1D157><U+1D165>.json`. Same root and equal key: e6 e7, so ERR E_COLLISION.
- `ucollision-greek-full-folding`: full folding F of U+1F88 is U+1F00 U+03B9, so it collides with that two-letter path. Added: e6 scenes `basic/scenes/<U+1F88>.json`, key `basic/scenes/<U+03B1><U+0313><U+03B9>.json`; e7 scenes `basic/scenes/<U+1F00><U+03B9>.json`, key `basic/scenes/<U+03B1><U+0313><U+03B9>.json`. Same root and equal key: e6 e7, so ERR E_COLLISION.
- `ucollision-greek-inner-nfd`: U+1FB3 U+0313 collides with U+1F80 only through the inner NFD of D145. Added: e6 scenes `basic/scenes/<U+1FB3><U+0313>.json`, key `basic/scenes/<U+03B1><U+0313><U+03B9>.json`; e7 scenes `basic/scenes/<U+1F80>.json`, key `basic/scenes/<U+03B1><U+0313><U+03B9>.json`. Same root and equal key: e6 e7, so ERR E_COLLISION.
- `ucollision-cherokee`: Cherokee U+13A0 and U+AB70 collide (small folds to capital). Added: e6 scenes `basic/scenes/<U+13A0>.json`, key `basic/scenes/<U+13A0>.json`; e7 scenes `basic/scenes/<U+AB70>.json`, key `basic/scenes/<U+13A0>.json`. Same root and equal key: e6 e7, so ERR E_COLLISION.
- `ucollision-georgian`: Georgian U+10D0 and Mtavruli U+1C90 collide. Added: e6 scenes `basic/scenes/<U+10D0>.json`, key `basic/scenes/<U+10D0>.json`; e7 scenes `basic/scenes/<U+1C90>.json`, key `basic/scenes/<U+10D0>.json`. Same root and equal key: e6 e7, so ERR E_COLLISION.
- `ucollision-other-root-same-path-ok`: the same non-ASCII path (identical bytes) in two different roots does not collide. Added: e6 scenes `basic/scenes/Caf<U+00E9>.json`, key `basic/scenes/cafe<U+0301>.json`; e7 config `basic/scenes/Caf<U+00E9>.json`, key `basic/scenes/cafe<U+0301>.json`. No two entries, base e1-e5 included, share root and key, so OK.
- `ucollision-other-root-equal-keys-ok`: four paths with equal keys (case and normalization variants), each in a different root. Added: e6 scenes `basic/scenes/Caf<U+00E9>.json`, key `basic/scenes/cafe<U+0301>.json`; e7 config `basic/scenes/CAF<U+00C9>.json`, key `basic/scenes/cafe<U+0301>.json`; e8 profiles `basic/scenes/Cafe<U+0301>.json`, key `basic/scenes/cafe<U+0301>.json`; e9 plugin_manager `basic/scenes/CAFE<U+0301>.json`, key `basic/scenes/cafe<U+0301>.json`. No two entries, base e1-e5 included, share root and key, so OK.
