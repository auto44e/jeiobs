# S1-PR2 contract: bounded scanners and the redactor (shared by both workers)

Spec: `docs/jeiobs/WORKSPACE_FORMAT.md` §6 (fidelity), §7 (allowlist), §8 (secrets), §12 (limits and
parser rules), §13 (PR2 row). Earlier contracts: `docs/jeiobs/debates/002-workspace-format/04-pr1-contract.md`
and `09-pr1-contract-r2.md` still hold. Where this file differs, it wins for PR2.

## Library (shared/jeiobs-workspace, C++17, no Qt, no libobs)
- `JsonScan`: a bounded JSON scanner over bytes (§12 limits: depth 64, string 16 MiB, 1,000,000
  values, document size 64 MiB; rejects invalid UTF-8, NUL, non-finite numbers, BOM). It does NOT
  build a DOM: it reports every string value and every object member with byte offsets
  `[start,end)` (for strings: the bytes BETWEEN the quotes), the RFC 6901 pointer, and for object
  members the key. It reports ALL occurrences of duplicate keys and flags `duplicate_key=true`
  (the redactor then treats the file as invalid, see below). Must not recurse beyond depth 64 (no
  stack overflow on deep input).
- `IniScan`: INI reader keeping the exact bytes: optional UTF-8 BOM, CRLF or LF per line, duplicate
  sections and keys, comments (`;` and `#` lines), `key=value` with the value from after the FIRST
  `=` to the end of the line (excluding the line terminator). Reports sections, keys, value byte
  ranges, pointers `/<section>/<key>` (RFC 6901 escaping `~0`,`~1`). Line limit 1 MiB (§12).
- `Redactor`: `Redact(role, bytes, options) -> {outcome, outbytes, report}` (see CLI for the
  report shape). Edits only the byte ranges it reports; every other byte is unchanged; the same
  ranges are replaced in every duplicate occurrence.
- `jw-fuzz-smoke` ctest: deterministic mutation fuzzing (fixed seeds, bounded iterations, wall-clock
  under 60 s on a CI runner) over all fixture files of the redactor cases: for each mutated input
  the redactor must not crash/hang/throw, and if it returns `stored` then (a) the report offsets
  are inside the input, ordered and non-overlapping, (b) the output contains none of the marker
  secrets planted in the seed fixtures that the mutation left intact in a registry position... keep
  it simple: assert (a) plus "output length = input length - sum(range lengths) + sum(placeholder
  lengths)". Plus an optional libFuzzer target `jw-fuzz` built only when
  `-DJW_ENABLE_LIBFUZZER=ON` (Clang).

## CLI (existing `jw-check`)
`jw-check redact ROLE INFILE OUTFILE REPORT [--ws-placeholder STR]` → stdout `OK` (exit 0) or one
`ERR <CODE> [detail]` line (exit 1), usage errors exit 2. Writes OUTFILE (bytes) only when the
outcome is `stored`. REPORT is a JSON file:
```
{"outcome":"stored"|"omitted",
 "reason":"unparseable",                       // only when omitted
 "fidelity":"exact"|"redacted"|"opaque-unreviewed",  // stored only
 "parse":"invalid",                            // stored only, only when the file does not parse
 "redactions":[{"pointer":"/settings/key","kind":"stream_key","start":12,"end":20}, ...],
 "unreviewed":{"rules":"u1","count":N}}        // only when fidelity is opaque-unreviewed
```
`start`/`end` are byte offsets in INFILE of the replaced range. `redactions` is sorted by `start`.
`kind` ∈ stream_key, bearer_token, rtmp_auth, oauth_token, ws_password, url_credential, forced.
A `forced` redaction also has `"was":false`. Fidelity: opaque-unreviewed if the role's unreviewed
rule applies, else redacted if `redactions` is non-empty, else exact. Registry secrets are
redacted in opaque-unreviewed files too. `--ws-placeholder` (default: 32 random `[A-Za-z0-9]`
characters from the OS CSPRNG) is the websocket password replacement; if given it must be exactly
32 chars of `[A-Za-z0-9]` else `ERR E_SCHEMA ws-placeholder`.
Roles: profile.service, profile.service.bak, profile.basic, module.obs-websocket, collection,
collection.bak, profile.encoder.stream, profile.encoder.record, and their `.bak`, app.modules,
app.user. Any other role → `ERR E_SCHEMA role`. Input over 64 MiB → `ERR E_LIMIT blob`;
unreadable or not a regular file → `ERR E_IO`.

## Validity ("parse")
- JSON roles: the document must be one JSON value (object) parsed by the scanner rules in §12.
  Duplicate keys at any level, invalid UTF-8, NUL, 1e400, BOM, syntax errors, depth/size limits
  make it INVALID. (OBS itself rejects duplicate keys, `JSON_REJECT_DUPLICATES`.)
- INI roles: always valid except invalid UTF-8, NUL, a line over 1 MiB, or > 64 MiB.
- INVALID file: if the role has registry secrets (every role except profile.encoder.*, app.modules,
  app.user) → `{"outcome":"omitted","reason":"unparseable"}` with no OUTFILE. Otherwise
  `{"outcome":"stored","parse":"invalid","fidelity":"opaque-unreviewed","unreviewed":{"rules":"u1","count":1},"redactions":[]}`
  and OUTFILE = INFILE.

## Registry (what is redacted, per role; kind; replacement)
Replacement for every kind except ws_password is the empty string (the range [start,end) is
removed). A "string value" edit covers the bytes between the quotes of a JSON string; if the value
is not a string (number, object, null) it is NOT edited and the entry is treated as opaque
(unreviewed count +1). An INI edit covers from after the first `=` to the end of the line.
- profile.service / .bak (JSON): `/settings/key` stream_key; `/settings/bearer_token` bearer_token;
  `/settings/username` and `/settings/password` rtmp_auth; `/settings/server` url_credential.
  Unreviewed: `type` is not `"rtmp_common"` (count 1; also when `type` is absent or not a string).
- profile.basic (INI): in sections `[Twitch]`, `[Restream]`, `[YouTube]` the keys `Token` and
  `RefreshToken` oauth_token (key names case-sensitive, as OBS); `[AdvOut] FFURL` url_credential.
  Unreviewed: `[AdvOut] FFURL` has a non-empty value (count 1) and each distinct section name not in
  {General, Video, Audio, Output, SimpleOutput, AdvOut, Stream1, Panels, Hotkeys, Auth, Twitch,
  YouTube, Restream, BasicWindow} (count 1 each; compare case-sensitively).
- module.obs-websocket (JSON): `/server_password` ws_password, replaced by the placeholder (NOT
  empty), and `/auth_required`: if it is the literal `false` its bytes are replaced by `true`
  (kind forced, was false); if absent or already true, no edit. Unreviewed: each distinct top-level
  key not in {first_load, server_enabled, server_port, alerts_enabled, debug_enabled,
  auth_required, server_password} (count 1 each).
  If `server_password` is absent no ws_password edit is made. Missing key adds nothing.
- collection / .bak (JSON): for each item of the top-level `sources` array that is an object:
  - id `browser_source`: `/sources/<i>/settings/url` url_credential;
  - id `ffmpeg_source`: `settings/input` url_credential when `settings/is_local_file` is the literal
    `false`; `settings/ffmpeg_options` url_credential (URL rule on each whitespace-delimited token
    that contains `://`);
  - id `vlc_source`: each `settings/playlist/<j>/value` string, URL rule on the whole string.
  Always opaque-unreviewed (PR3 reviews the first-party list): count = max(1, number of `sources`
  items whose id is not `scene` and not `group`).
- profile.encoder.*, app.modules: no secrets; valid JSON → exact.
- app.user (INI): no secrets; exact, except non-empty `[BasicWindow] ExtraBrowserDocks` →
  opaque-unreviewed count 1.
URL rule (applied to the string content, one redaction per edited range, kind url_credential):
for each `scheme://` (scheme = letters/digits/+.-), remove the userinfo (the bytes between `://`
and the LAST `@` before the next `/`, `?`, `#` or end, only if that `@` exists); and in the query
string (after `?`, up to `#` or end) remove the value of every parameter whose name, compared
ASCII case-insensitively, is one of key, token, access_token, auth, password, passphrase, secret,
streamid (the value is the bytes after `=` up to `&` or end). Percent-encoding is not decoded.
Ranges that are not valid UTF-8 boundaries cannot occur (all delimiters are ASCII).

## Part B: carried review-02 Minors (library + validator, implement too)
1. claude-4: in `ValidateDocument` (both modes) type-check known values: `creator.*` and
   `active.*` values are strings; `app_keys` items are objects whose `root`, `file`, `section`,
   `key`, `value` are strings and `restore` is `"same-os"` or `"never"` (missing or other →
   `E_SCHEMA app_keys`); each `dropped_ext` item is an object `{"entry": string|null, "fields":
   [strings]}` (else `E_SCHEMA dropped_ext`). Export mode also: `app_keys` items with
   `restore:"never"` already rejected; the export projection copies only validated values.
2. codex-6: `target.audio`, when present, must be an object with `sample_rate` (integer > 0) and
   `channels` (non-empty string); `explicit` (bool) and `src` (object with string `entry`) are
   optional in local mode and forbidden in export mode → else `E_SCHEMA target.audio`.
3. claude-6: `jw-check export` reads the manifest ONCE and hashes and parses the same bytes.
