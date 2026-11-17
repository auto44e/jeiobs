Committed `d9d50eff9` on the detached HEAD ("shared: cases for S1-PR2 review fixes (contract r3)"). It touches 127 files, all under `shared/jeiobs-workspace/tests/cases/`, and takes `cases.txt` from 882 to 968 cases. I have not run them against the other worker's r3 build, or on Windows or macOS.

## Testing
- **My r3 model of the contract** (a Python fake, not committed, in `/tmp/jwauthor4`): 968 of 968 pass. Every new expectation was also checked against bytes I listed by hand. The inputs `run-cases.cmake` generates were checked byte for byte against a Python copy (25 files, all identical).
- **PR #28 head binary** (`/tmp/jeiobs-s2/build-int`): 902 pass and 64 fail, and every failure is an r3 change. I skipped one case there: the 1 MiB header case would need about 100 GB on the old code.
- **Broken variants of the fake:** all 14 are caught (old INI line rules, no fallback, input only when false, `#` ends the query, per-token options, authority ends at a space, no merge, no merge of touching ranges, `versioned_id` ignored, non-strings not counted, modules object-only, BOM rejected, no INI limits, `E_SCHEMA target`).
- **Alternative readings:** all 5 that I accept pass.

## Existing cases changed
- **`input` credentials:** the 5 is_local_file cases no longer keep them. Their markers are now `SECRETvalue` and the `-kept-` names are gone. `mixed.json`'s local Clip input is now redacted too.
- **Export audio errors:** cases 428, 665 and 666 now expect only `ERR E_SCHEMA target.audio`.
- **ffmpeg_options and `#`:** the case that kept `#PLAINvalue` and the whitespace-boundary case are rewritten for the whole-string rule. `fragment-ends-query` becomes `hash-in-query-value`, and the VLC `#t=` value is now a secret.
- **BOM:** the five `redact-invalid-bom-*` cases are now valid documents, under `redact-bom-*`.
- **Strict now:** `app.modules` with a top-level array expects `exact` only, and `url-not-a-string` expects count 2 only. The old `report=*` case for a CR at end of file is now pinned.

## New cases
- **Item 1, INI:** the five inputs, each with a secret marker. Also a key on the header line, two headers on one line, a value or header ending at a CR, `#` after blanks, trailing blanks (no fallback), the 1b fallback (CR-only file, malformed header, CRLF), FFURL over SRT, and `app.user` docks.
- **Item 2:** `input` with is_local_file 1 and "true" (true, false, absent, 0, null and "false" are covered by existing cases), plus local paths that must be kept.
- **Item 3:** the INI limits at 1,000,000 and 1,000,001 keys and 100,000 and 100,001 sections. Performance cases, each with a 60 s timeout: the 4 KiB key over 10^5 values in 5 roles, the 1 MiB header over 10^5 keys, 10^4 sources with 1 KiB URLs (20,000 ranges), and URLs with 2^20 parameters or 2^20 `scheme://`.
- **Item 4:** `app.modules` arrays, top-level scalars, depth 64/65; object still required for collection, websocket and `.bak`.
- **Item 5:** the literal `https://h/?token=https://u:p@x/` (one range, [17,31) in the string), nested and inner-query overlaps, and touching ranges.
- **Item 6:** `versioned_id` and non-string values counted.
- **Item 7:** BOM in documents (offsets include it), two BOMs, a BOM after a space, a lone BOM. Manifests still give `E_BOM` through `revision`, `export` and `child`.
- **Item 8:** SRT `#!::` streamid plus passphrase, a passphrase containing `#`, a `?` after `#`, and `&` as the only separator.
- **Item 9:** the quoted proxy `http_proxy='http://u:pa ss@…'`, also with double quotes and a tab.
- **Item 12:** export audio with an extra member, `null`, or `explicit` → `target.audio`.

## Runner changes
- `EXIT` may list alternatives such as `0,1`. Whatever it allows, an OK line now needs exit 0 and an ERR line exit 1.
- A redact case can set `timeout=N`.
- `report=` and `out=` can name files the runner generates under `${WORK}/`.
- A REPORT with more than 256 ranges is checked by whole-report equality plus `out=`, because CMake's JSON calls reparse the whole report on every range.
- The marker scan now also covers files over 4 MiB, using `file(STRINGS)`.
- INI inputs with LF line ends are written with `file(CONFIGURE … NEWLINE_STYLE LF)`, and each file's size is checked.

## Ambiguities and how I read them
1. **`[Twitch]]`:** libobs ends the header at the first `]`, so the section is `Twitch`, not `Twitch]` as the r3 parenthetical says. I pinned libobs: `/Twitch/Token`, count 1.
2. **"Malformed header is ignored":** libobs keeps the name up to the line break, so `[Twitch` gives section `Twitch`. I pinned libobs, which matches r3's own "OBS reads each as `[Twitch] Token=S`".
3. **Comments:** `;` is not a comment in libobs, only `#` at the start of a key. The cases hold either way.
4. **LFCR:** its CR is not followed by LF, so the fallback applies (count 1).
5. **Either reading accepted:**
   - the fallback count for `app.user` (count 1 or 2);
   - the "name begins with Twitch/…" prefix rule (`[TwitchChat]` kept or redacted).
6. **INI limits:** item 3 says over the limit is `ERR E_LIMIT values`, but item 5 forbids that ERR. I accept either result. Distinct (section, key) pairs keep the count the same whichever way it is done.
7. **Empty `versioned_id` (possible leak):** r3 as written makes the type `""`, so the URL is kept. OBS falls back to `id` (`obs.c`: `if (!*v_id) v_id = id`) and opens the URL. I accept both; I recommend pinning the OBS reading.
8. **Smaller readings:**
   - the collection count is max(1, N) + 1 per non-string value;
   - a `?` after `#` still starts the query, as FFmpeg's `strchr` does;
   - only one BOM is skipped;
   - a top-level array counts as depth 1.
9. **Over-redaction pinned in `.out` files:** in ffmpeg_options the query runs to the end of the whole string, and the authority runs across spaces up to the next `/`, `?` or `#`.

## Not covered
- **Not observable from outside:**
  - item 10 (locale): `jw-check` does not set the process locale, so it needs a unit test or a test hook;
  - item 11 (check limits before allocating), and the 512 MiB memory bound.
  The cases bound only time.
- **Other worker's files:** the item 5 change to `jw_fuzz_smoke.cpp`.
- **Left out because the contract is unclear:** blanks before a header line, `[]` (libobs stops reading the file there), and a Token before the first section in a fallback file (no pointer form is given).
- **Not in r3:** sources under `groups` and the audio devices, and an escaped `\u0040` in URLs.
- **Not run:** the r3 implementation, Windows and macOS. That `file(CONFIGURE)` writes exact LF bytes on Windows is assumed; a wrong size stops the run.
- **Weak performance checks:** on the old binary the 10^4-source case took 36.6 s and the 2^20-parameter URL 22 s (machine loaded), both under the 60 s timeout. Only the 2^20-scheme URL timed out, so the timeouts alone would not have caught the old code's slowness.
