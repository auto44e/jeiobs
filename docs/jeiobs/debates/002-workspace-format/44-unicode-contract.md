# Contract: Unicode collision key (blind case writing)

You write black-box test cases for `jw-check unicode` and for manifest collision handling. You have NOT
seen the implementation and must not read `shared/jeiobs-workspace/src/`. Derive every expected value from
this contract and the Unicode standard (use python3 `unicodedata` to compute expected NFD and casefold
values; Python's data is Unicode 15.0, so use only characters assigned and stable since Unicode 6 or so).

## Spec (WORKSPACE_FORMAT.md section 12)
No two paths in one manifest (same `root`) may be equal under Unicode canonical caseless matching,
`key(p) = NFD(toCasefold(NFD(p)))` (Unicode D145). toCasefold is full case folding (CaseFolding.txt
statuses C and F; no Turkic T mappings, so U+0130 folds to `i` + U+0307 and U+0049 folds to `i`).
NFD is canonical decomposition with canonical ordering (Hangul syllables decompose algorithmically). No
compatibility (NFKD) mapping: U+FB01 (fi ligature) is unchanged by NFD, and it has a full case folding
`fi` (CaseFolding F) but U+2126 OHM SIGN has canonical singleton decomposition U+03A9. Unassigned code
points and everything not mentioned pass through unchanged.

## CLI
`jw-check unicode nfd TEXT`, `jw-check unicode fold TEXT`, `jw-check unicode key TEXT`
- TEXT is one argument; `%XX` is the byte 0xXX (so UTF-8 bytes are written `%C3%A9`; `%20` is a space).
  `""` is an empty argument.
- Success: stdout `OK <hex>` where hex is the lowercase hex of the result's UTF-8 bytes with no
  separators (`OK ` followed by nothing visible for an empty result is written `OK ` with a trailing space,
  so avoid empty-result cases). Exit 0.
- TEXT that is not strict UTF-8 (overlong, surrogate, above U+10FFFF, truncated, stray continuation):
  stdout `ERR E_PATH not-utf8`, exit 1. `nfd`, `fold` and `key` all do this. Such cases need the name suffix
  `@posix` (the Windows argv cannot carry invalid UTF-8).
- `fold` does NOT normalize: `fold` of U+00C9 is U+00E9. `nfd` does NOT fold case.

`jw-check path P` still prints `OK` for every valid path, including non-ASCII ones (no `unchecked`).
`jw-check manifest FILE` prints `ERR E_COLLISION` for two entries with the same `root` whose paths have
equal keys, and `OK` when roots differ or keys differ. Paths in fixtures must otherwise be valid.

## Case file format
Read `shared/jeiobs-workspace/tests/cases/cases.txt` header (lines 1 to 60) for the line format
(`NAME | EXIT | EXPECTED | ARGS`; fields separated by " | "; no semicolons, square brackets or
backslashes anywhere; `%XX` escapes). Existing manifest collision fixtures:
`tests/cases/fixtures/manifest/manifest-collision-ascii-case.json` and
`manifest-collision-non-ascii-case.json` (copy their structure; the entries need valid ids, size, hash).

## Deliverables (write only under the scratch directory)
- `cases.txt`: the new case lines, ready to append (names prefixed `unicode-` or `ucollision-`).
- `fixtures/manifest/*.json`: any new manifest fixtures, named as referenced by the cases
  (`manifest fixtures/manifest/NAME.json`).
- `notes.md`: for each case, one line of justification and how the expected value was computed.

Cover at least: ASCII upper/lower; Latin-1 precomposed vs combining (`e` U+0301 vs U+00E9) equal keys;
Latin accent case pairs (U+00C9 vs U+00E9); sharp s U+00DF vs `ss` vs U+1E9E (equal keys); final sigma U+03C2
vs U+03C3 vs U+03A3 (equal keys by C mapping); U+0130 (dotted capital I) key vs `i` + U+0307 equal, and
unequal to `i`; Kelvin sign U+212A vs `k`; Angstrom U+212B vs U+00C5 vs `A` U+030A; ohm U+2126 vs U+03C9;
Hangul syllable U+AC01 vs jamo U+1100 U+1161 U+11A8; canonical ordering (`a` U+0323 U+0301 equal to
`a` U+0301 U+0323, but `a` U+0300 U+0301 differs from `a` U+0301 U+0300 because both have class 230);
U+FB01 key is `fi`; compatibility characters are NOT unified (U+FF21 fullwidth A keeps its identity apart
from `a`, and U+00B2 superscript two differs from `2`); non-BMP (U+1D15E musical symbol half note
decomposes: use unicodedata.normalize('NFD')); Cherokee, Greek extended (U+1F88) full-fold F mapping;
Georgian U+10D0 vs U+1C90; invalid UTF-8 cases for each of nfd/fold/key (overlong `%C0%80`, `%C1%BF`,
`%E0%80%80`, surrogate `%ED%A0%80`, above-max `%F4%90%80%80`, truncated `%E2%82`, stray `%80`); same path
in different roots not colliding; a manifest where the collision is two non-ASCII paths that differ by
normalization form only; a manifest with a case-only non-ASCII pair in the same component of a deeper
path; and the three-way collision. Do not invent behaviour the contract does not state; if a case would
rely on an unstated reading, leave it out and say so in notes.md.
