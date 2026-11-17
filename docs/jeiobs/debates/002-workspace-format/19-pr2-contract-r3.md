# S1-PR2 contract addendum r3 (after review run 01). Wins over `14-pr2-contract.md` where they differ.

Source: the findings of `/tmp/jeiobs-s2/review/01-run/` (read `01-r1-claude.reply.md`,
`02-r1-codex.reply.md`, `03-r2-claude.reply.md`: they contain repros and file:line evidence).

1. INI parsing follows libobs (claude-1, MAJOR: live tokens stayed in the output). Mirror
   `libobs/util/config-file.c` and `lexer.c/h`: CR, LF, CRLF and LFCR are line breaks; CR and LF are
   skipped before a key; a section header ends at the FIRST `]` or line break; a malformed header
   is ignored (no section change) and the REST OF THAT LINE is read as keys; `;`/`#` comments as
   OBS. Read the libobs source and reproduce its behaviour for these five inputs, each of which
   OBS reads as `[Twitch] Token=S`: `[Twitch] ;x\nToken=S\n`, `[Twitch\nToken=S\n`,
   `[Twitch]\rToken=S\n`, `[Twitch]\n\rToken=S\n`, `[Twitch]]\nToken=S\n` (the last gives section
   `Twitch]` in OBS: then the token is NOT in Twitch; but redact it anyway if the key is
   `Token`/`RefreshToken` in ANY section whose name begins with Twitch/Restream/YouTube — see 1b).
   1b. Fail closed: when the INI scanner cannot be sure (a CR not followed by LF, or a `[` line that
   is not exactly `[name]` plus trailing blanks), additionally redact every `Token` and
   `RefreshToken` key anywhere in the file regardless of section, and mark the file
   opaque-unreviewed (count +1). Offsets stay in file bytes.
2. Media source `input` (claude-2, MAJOR): ALWAYS apply the URL rule to `/settings/input` of an
   `ffmpeg_source`, whatever `is_local_file` is (a local path has no `scheme://`). Still
   opaque-unreviewed as before.
3. Bounded memory and time (claude-3, MAJOR): no per-value or per-key copies of pointer/section
   strings (store parent index or a view and build pointers on demand, only for reported
   redactions); no repeated whole-document scans per source (build one index: member lookup by
   (parent value, key) in O(1) or O(log n)); URL scanning must be linear in the string length (no
   scan to the string start/end per URL or parameter); boundary mappings (decoded offset to raw
   offset) computed once per string. Add INI limits: at most 1,000,000 keys and 100,000 sections
   (over → `ERR E_LIMIT values`). Add unit tests with generated worst cases that finish in under
   5 s and under 512 MiB: a 4 KiB key with 10^5 array items; one 1 MiB section header plus 10^5
   keys; 10^4 sources each with a long URL.
4. app.modules (claude-4, MAJOR): accept a JSON array OR object as the document (OBS writes an
   array); every other JSON role keeps requiring an object where the contract said so.
5. Overlapping edit ranges (codex-1, claude-5): merge overlapping or touching ranges into one (the
   first kind wins; a ws_password or forced edit never merges with others). `Redact` must NEVER
   return ERR for any input except `E_LIMIT blob`, `E_IO`, `E_SCHEMA role|ws-placeholder`; an
   internal failure to apply edits is a bug, not an ERR. Tighten `jw_fuzz_smoke.cpp` accordingly
   (`CheckResult` must not accept arbitrary ERR).
6. Collections (claude-6): a source's type is `versioned_id` if present and a string, else `id`;
   match browser_source/ffmpeg_source/vlc_source on that. A non-string registry value adds 1 to the
   unreviewed count (count = max(1, ...) formula otherwise as before).
7. JSON documents (claude-7): a UTF-8 BOM at the start is allowed and skipped (OBS strips it);
   offsets are in file bytes, including the BOM. Manifests still reject a BOM.
8. URL rule (claude-9, MAJOR; replaces the query paragraph of the contract): after the FIRST `?` in
   a URL, the query runs to the END of the string (a `#` does not end it); parameters are split on
   `&` only; for each parameter whose name (before the first `=`) matches the credential names
   (case-insensitive: key, token, access_token, auth, password, passphrase, secret, streamid) the
   value after `=` up to the next `&` or the end is removed. So `srt://h:9?streamid=#!::r=x,s=SECRET1&passphrase=SECRET2`
   loses both values. Userinfo rule unchanged (up to the LAST `@` in the authority; the `@` stays).
9. `ffmpeg_options` and VLC playlist values (claude-10): apply the URL rule to the WHOLE string
   (every `scheme://` occurrence), not per whitespace token. The authority of a URL ends at the
   first of `/`, `?`, `#` or the end of the string (spaces and quotes do not end it), so
   `http_proxy='http://u:pa ss@proxy.example:3128'` loses `u:pa ss`.
10. Locale independence (claude-11): number parsing must not depend on `LC_NUMERIC` (use
   `std::from_chars` where available, else a classic-locale stream or a hand parser). Test under a
   decimal-comma locale if one is installed, else by calling a test hook.
11. String limits before allocation (codex-2): the 16 MiB string limit and the 64 KiB limit must
   be checked BEFORE appending to the decoded buffer (no append that can exceed the limit).
12. Export audio errors (codex-3): any `target.audio` field error in EXPORT mode is
   `E_SCHEMA target.audio` (not `target`).

## r3.1 (integration findings; both lineages ran, the cases are authoritative unless libobs says otherwise)
- Ground truth for INI: `/tmp/gt/gt FILE...` prints how libobs's own parser (config_open_string) reads a
  file: sections, and Token/RefreshToken/ExtraBrowserDocks/FFURL values. (Source: /tmp/gt/gt.c.) OBS
  reads `[Twitch` and `[Twitch]]` as section `Twitch`; the redactor's section/pointer for every key must be
  what libobs reports, and no secret that libobs reads may survive in OUTFILE.
- URL rule: the query is everything after the FIRST `?` in the string even when a `#` comes before it
  (`rtmp://h/live#frag?key=S&x=P` loses `S`).
- Fallback 1b triggers on any `[` line that is not exactly `[name]` plus trailing blanks, including
  `[Twitch] ;comment`, and on any CR not followed by LF (also CR right after LF). Then every
  `Token`/`RefreshToken` in ANY section is redacted (so `[General] Token=S` is redacted) and the file is
  opaque-unreviewed with count +1.
- `versioned_id` is used only when it is a non-empty string; otherwise `id` (as OBS does).

## r3.2 (review run 02 of PR #28; read `/tmp/jeiobs-s2/review/02-run/*.reply.md`)
1. (claude-3, MAJOR) INI pointers must not copy a huge section name per redaction: if a section name is
   longer than 1,024 bytes the pointer uses `/~section-<n>/<key>` where `<n>` is the 0-based section
   ordinal (this cannot collide with an RFC 6901-escaped name) and the file is opaque-unreviewed
   (count +1). Total report size must stay bounded. Also remove every per-URL scan to the document
   end (`find('\\')` and similar): all URL scanning is linear in the value length.
2. (claude-12) FAIL CLOSED: if applying the edits fails for any reason, the outcome is
   `omitted/unparseable` for roles with secrets (never `stored` with the unredacted input). Tighten
   `jw_fuzz_smoke.cpp`: a `stored` result must have report offsets valid and, when redactions exist,
   OUTFILE shorter than or different from INFILE exactly as the report says.
3. (claude-15, codex-4, MAJOR) The 1b fallback applies to the WHOLE file, including Token/RefreshToken
   lines BEFORE the first header: it triggers on any `[` line that is not exactly `[name]` plus trailing
   blanks, including an empty `[`, a lone `[`, `[]`, and `[ ]`. Then every `Token`/`RefreshToken` key at
   any position is redacted (pointer `/<section or empty>/<key>`).
4. (claude-6) Source rules also apply to: top-level objects named `DesktopAudioDevice1`,
   `DesktopAudioDevice2`, `AuxAudioDevice1`..`AuxAudioDevice4` that are objects with an `id` string,
   and to items of a top-level `groups` array that are objects with `id` (check
   `frontend/.../OBSBasic_SceneCollections.cpp` around lines 1259-1274 for what OBS loads; follow
   the code, and write what you found in your final reply).
