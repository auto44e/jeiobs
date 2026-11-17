**Verdict: request changes.** I re-checked all eight of my earlier findings and every citation still holds; the commit is unchanged. This round adds one Major (claude-9) and two Minors (claude-10, claude-11). As before, I only read code; I built and ran nothing.

## The other reviewer's findings
- **codex-1:** I agree. It is the same defect as my claude-5 (`https://h/?token=https://u:p@x/` gives ranges [17,31) and [25,28), which Redactor.cpp:601-602 rejects). I keep claude-5 at Minor because it fails closed, but I don't object to codex's Major; one fix closes both. The fuzz smoke can't catch this class: `CheckResult` passes every ERR (jw_fuzz_smoke.cpp:53).
- **codex-2:** The finding is technically correct, but I'd rate it Minor.
  - JsonScan.cpp:197 checks the raw bytes consumed before every append.
  - The decoded string never outgrows the raw bytes, because escapes only shrink.
  - So the string ends at most 4 bytes past the limit before :305 fires.
  - The allocation that actually breaks §12 is the per-value pointer copy in claude-3.
- **codex-3:** I agree. Manifest.cpp:485 and :491 return `target`. 14-pr2-contract.md:5 says the PR2 contract wins over r2 item 7 (09-pr1-contract-r2.md:51-52). Cases 428, 665 and 666 accept both codes and should expect only `target.audio`.

## Addition to claude-2
I re-verified `null`. obs-data.c:486-488 and :507-508 import it as an OBJECT item, so the default is skipped (:923-924) and `obs_data_get_bool` returns false (:1712). OBS therefore opens `input` for `null` too. Applying the URL rule to `input` every time also covers a credential that sits unused while `is_local_file` is true and becomes live once the user unticks the box.

## New findings
**claude-9 (Major): the URL rule ends the query at `#`, but OBS's SRT code does not.**
- Repro, a `service.json` with `rtmp_custom`: `"server":"srt://in.example:9000?streamid=#!::r=live/x,s=SECRET1&passphrase=SECRET2"`.
- What the redactor does:
  - `query_end` stops at the `#` (Redactor.cpp:273-274).
  - `streamid` gets an empty range (:284), then the loop breaks (:286-287).
  - OUTPUT keeps both secrets.
- How OBS reads it:
  - `server` (rtmp-custom.c:25) becomes `config->url` (obs-ffmpeg-mpegts.c:847) and is passed to `libsrt_open(uc, uc->url)` (:347).
  - That calls `strchr(uri,'?')` and then `av_find_info_tag` for `passphrase` and `streamid` (obs-ffmpeg-srt.h:676, :684-686, :769-771).
  - FFmpeg's `av_find_info_tag` ends a value only at `&` or NUL, so `#` is data. Its source is not in this tree.
  - `FFURL` and media `input` reach FFmpeg's libsrt, which reads its query the same way.
- `#!::` is the SRT access-control convention for streamid, so this is the common form, not an edge case. A passphrase that contains `#` also keeps everything after the `#`.
- Case `redact-collection-ffmpeg-options-tokens.out:20` currently locks the behaviour in (`?Token=#PLAINvalue-fo-6` is kept).
- Fix: let parameter values run to `&` or the end of the string. Over-redacting an http fragment is harmless. Update contract lines 97-100 and that case, and add SRT cases.

**claude-10 (Minor): `ffmpeg_options` is split on whitespace, but OBS parses it with `av_dict_parse_string(…, "=", " ", 0)` (media.c:680).** That parser keeps quoted spaces, backslash escapes and tabs inside a value.
- Repro: `http_proxy='http://u:pa ss@proxy.example:3128'`.
- The first token has no `@` in its authority, and the second has no `://` (Redactor.cpp:342-358), so nothing is removed.
- FFmpeg receives `http://u:pa ss@…` and uses it for proxy authentication.
- Fix: tokenize the way `av_get_token` does.

**claude-11 (Minor): `ParseNumber` uses `std::strtod`, which depends on the locale (JsonScan.cpp:402-407).**
- Under a decimal-comma `LC_NUMERIC`, `1.5` stops at the `.`, so `*parse_end != '\0'` and the scanner returns E_NONFINITE.
- Collections always contain fractional numbers, so every collection would be omitted as unparseable.
- This is latent today: OBS forces "C" only on Unix (OBSApp.cpp:948-953), and jw-check never calls `setlocale`. The library is meant to work in other hosts too.
- Fix: check finiteness in a locale-independent way.

FINDING claude-1 | Major | open | IniScan line/header rules differ from config-file.c; OBS-live [Twitch]/[YouTube]/[Restream] tokens stay in OUTFILE (5 repros)
FINDING claude-2 | Major | open | input redacted only for literal false; OBS opens input for any non-boolean is_local_file (0, null, "false"); cases 855-857 keep the credentials
FINDING claude-3 | Major | open | per-value pointer/section copies and repeated full scans: in-limit inputs need terabytes of memory or hours of CPU
FINDING claude-4 | Major | open | app.modules requires an object but OBS writes an array; every real modules.json becomes parse invalid / opaque-unreviewed
FINDING claude-5 | Minor | open | overlapping URL ranges make ApplyEdits fail, so a valid file returns ERR E_JSON (same defect as codex-1; one fix closes both)
FINDING claude-6 | Minor | open | collection rule ignores versioned_id, groups and the audio-device keys that OBS loads as sources
FINDING claude-7 | Minor | open | BOM rejected in JSON documents although OBS strips it; §12 rejects BOM only in manifests
FINDING claude-8 | Minor | open | doc/contract drift: §4:98 audio check, user.ini u1 rules, decklink/AJA roles, collection non-string count
FINDING claude-9 | Major | open | URL rule ends the query at '#' but OBS's SRT path (obs-ffmpeg-srt.h:676,684,769) reads values to '&'/end; "streamid=#!::…&passphrase=…" keeps both secrets
FINDING claude-10 | Minor | open | ffmpeg_options split on whitespace, but OBS uses av_dict_parse_string (media.c:680, quotes/escapes); "http_proxy='http://u:pa ss@…'" keeps the userinfo
FINDING claude-11 | Minor | open | ParseNumber uses locale-dependent strtod (JsonScan.cpp:405); under a decimal-comma LC_NUMERIC every fraction is E_NONFINITE and collections are omitted
VERDICT: request changes
