Committed `24448bc2b` on the detached HEAD (no branch, no push). It touches 363 files, all under `shared/jeiobs-workspace/tests/cases/`. There are 373 new cases: 299 `redact` and 74 Part B, for 882 in total. All 882 pass against my fake. They have **not** been run against the real PR2 library, or on Windows or macOS.

## Testing
- **My fake** (a Python model of the contract in `/tmp/jwauthor3`, not committed): 882/882 pass. Each fake-derived report was also checked against bytes I wrote out by hand for each case. Takes about 51 s.
- **PR1 `jw-check` built from main, used as a black box:** all 509 old cases still pass with the new runner. All 356 new failures are cases PR1 can't pass, since it has no `redact` and no Part B.
- **Broken variants of the fake:** 39 of 41 defect modes are caught. The other two are readings I accept on purpose (items 3 and 8 below). Examples of caught modes: BOM not skipped, only the first duplicate key redacted, CR kept in the value, first `@` instead of the last, `ffmpeg_options` without token boundaries, an empty or repeated placeholder, OUTFILE written when omitted, unsorted or shifted offsets, raw instead of decoded names, limits off by one, no 64 MiB check, decklink accepted as a role, Part B switched off.
- **Runner bug found and fixed:** an unexpected `OK` on a case expecting an error crashed the runner. It no longer does. Seven malformed-output modes (crash, garbage REPORT, empty OUTFILE, huge, negative or string offsets, no `redactions`) now all show up as FAILs.
- The scratch directory grows to about 260 MB.

## Runner
- **Redact cases:** `redact ROLE INFILE ${OUT} ${REPORT} [--ws-placeholder P]`, with expectation `OK report=FILE[,FILE] [out=FILE]`. Several report files appear only where the contract allows more than one reading.
- **Checks on every redact case:**
  - OUTFILE exists only when the outcome is `stored`, and never after an ERR line.
  - Each redaction has the right shape, with 0 ≤ start ≤ end ≤ INFILE size, sorted and not overlapping.
  - OUTFILE equals INFILE with exactly those ranges replaced: `""`, `true` for forced, or the placeholder.
  - No `SECRETvalue` is left, and every `PLAINvalue` (values the rules must keep) survives.
  - Without `--ws-placeholder`, the placeholder is 32 characters of `[A-Za-z0-9]`, and a second run gives the same REPORT and another placeholder.
- **Large inputs are generated** in the scratch directory: depth 64/65/100000, 16 MiB ± 1 strings, 1,000,000 values, 1 MiB INI lines, 64 MiB and 64 MiB + 1, and UTF-8/NUL/BOM faults. Nothing generated contains a newline; INI inputs start from committed prefixes.

## Cases
| Group | n | Covers |
|---|---|---|
| service.json and `.bak` | 41 | every registry row on its own; rtmp_common, rtmp_custom, whip_custom; the type rule (absent, number, null, object, case, trailing space, escaped); non-string values (number, null, object, array, true; count 3 when they stack); escapes and non-ASCII in values; escaped member name; non-registry pointers kept; five duplicate-key shapes omitted |
| URL rule | 25 | userinfo only, user only, query only, both, all 8 names, any case, similar names kept, no `=`, value up to `&`, fragment ends the query or authority, no scheme, `@` in path or query, last `@`, percent-encoding, two URLs, uppercase, `+.-` scheme, JSON escapes, empty value or userinfo |
| basic.ini | 32 | LF, CRLF, BOM+LF, BOM+CRLF; all 6 token rows plus FFURL; duplicate sections and keys; comments and similar names; registry keys in other sections; leading whitespace; first `=`; empty token; no final newline; unknown sections (distinct, case); FFURL empty, without credentials, CRLF, duplicate, plus unknown sections; one BOM+CRLF file with everything |
| obs-websocket | 19 | auth false, true, absent, with given and random placeholder; key order; whitespace; unknown and nested keys; forced edit alone; exact; empty password still gets the placeholder; escapes; non-string password; duplicates |
| collections and `.bak` | 26 | a mixed file (scene, group, browser, remote and local ffmpeg, vlc, image, text); count rules; `is_local_file` true, absent, `"false"`, 0, null; `ffmpeg_options` tokens and token boundary; vlc playlist; other ids and fields; non-top-level sources; escaped id; duplicates |
| roles without secrets | 18 | 4 encoder roles, `app.modules`, `app.user` with and without ExtraBrowserDocks (empty, BOM+CRLF, duplicate, other section, name case) |
| invalid documents | 40 | committed syntax, NUL-escape, 1e400, raw control character, duplicate and top-level-type faults, for roles with and without secrets |
| byte faults | 32 | generated UTF-8, NUL and BOM faults |
| limits | 30 | see Runner |
| roles and CLI | 36 | 15 bad roles, 8 bad placeholders plus one valid, E_IO (missing file, directory, `/dev/null`, `/dev/zero`, REPORT or OUTFILE is a directory), 6 usage errors |
| Part B | 74 | creator and active members, `app_keys` items, `dropped_ext` items, `target.audio` shapes, in local and export mode and through `export`, `child` and `revision` |

## Ambiguities and how I read them
1. **Possible contract defect: `modules.json`.** OBS writes it as a top-level **array** (`frontend/plugin-manager/PluginManager.cpp`), but the contract says JSON documents must be objects. Read literally, every real `modules.json` would be stored as parse:"invalid". The case accepts either `exact` or invalid.
2. **No `scheme://` means no URL**, so nothing is removed, query values included (cases `no-scheme`, `scheme-without-slashes`).
3. **Empty values:** an empty registry string (key, Token) is still a zero-length redaction, so the file is `redacted`. An empty websocket password still gets the placeholder. For an empty URL value or empty userinfo, either reading is accepted.
4. **Names and values are compared after JSON unescaping**, as OBS (jansson) reads them: `ke\u0079` is `/settings/key`, `rtmp\u005fcommon` is rtmp_common, `browser\u005fsource` is a browser source, and `bit\u0072ate` duplicates `bitrate`. The URL rule works on raw bytes.
5. **INI keys after leading whitespace are redacted.** OBS's parser skips that whitespace (`config-file.c`), so they are live tokens. Inside `[twitch]`, Token may be kept or redacted; `[twitch]` counts as unknown either way.
6. **A non-empty FFURL counts 1 per file**, not per occurrence.
7. **A collection source without `id` counts** toward the unreviewed count.
8. **A non-string value in a collection:** count `max(1,N)` or `max(1,N)+1`, either accepted.
9. **Possible contract/spec conflict: `app.user`.** Unknown sections keep it `exact`; the PR2 contract is narrower than §7.
10. **decklink and AJA roles are refused** with `E_SCHEMA role`, as the contract's role list says, although §7 allowlists them as `exact`.
11. **Part B:** only known `creator`/`active` members are type-checked. Existing cases need that, since non-string `active.x_active` is already accepted. The detail may be `creator`, `creator.app` or `app`. `explicit`/`src` in an export manifest's audio may give `target` (r2 item 7) or `target.audio` (Part B). I widened the old case `export-schema-target-audio-explicit` to accept both.
12. **Other readings:**
    - an omitted outcome prints `OK` (exit 0);
    - `redactions` is always present on stored reports;
    - exactly 64 MiB is read, tested with a file that is then omitted so no 64 MiB OUTFILE is written;
    - limits are inclusive;
    - INI line length is tested without a terminator;
    - raw control characters, leading zeros and non-object top levels are invalid.

## Not covered
- **Two live-secret gaps in the contract.** A Twitch token stays unredacted although OBS reads it as live in each of these:
  - an unterminated `[Twitch` header, which OBS still treats as section Twitch;
  - an escaped `@` (`\u0040`) in a URL, since the URL rule works on raw bytes.
- **INI edges:** trailing space in a key, keys before the first section, `[]` (OBS stops parsing there).
- **JSON and URL edges:** lone-surrogate escapes, `\t` as an `ffmpeg_options` separator, overlapping URL ranges.
- **Websocket and placeholder:** non-boolean `auth_required`; a bad placeholder with other roles.
- **Files:** symlinks, FIFOs, REPORT content after an ERR, the order of partial writes.
- Export reads once (skipped, as instructed). Windows and macOS runs.

The generator and fake are in `/tmp/jwauthor3` and are not committed.

That notice was an old wait timer running out; it changes nothing. The work is finished and committed as `24448bc2b`, and the summary above still stands.

That notice was another old wait timer running out, and nothing more needs doing. The cases are committed as `24448bc2b`, and the summary above still stands.

That was another old wait timer for the earlier test run, which I had already stopped and replaced. Nothing changed and there is nothing to do. The cases are committed as `24448bc2b`, as summarised above.

Another leftover wait timer from the first test run ran out; it changes nothing and needs no action. The cases are committed as `24448bc2b`, and the summary above still stands.

That was the last leftover wait timer from the first test run timing out. It changes nothing and needs no action; the cases are committed as `24448bc2b`, as summarised above.
