**Approve.** My two open findings are fixed: claude-13 (Major) and claude-16 (Minor). I'm raising two new Minors as follow-ups, and neither blocks a self-merge. I only read files and didn't build or run anything. I also couldn't check the CI claims, because no CI logs are in the tree.

**claude-13: accepted.**
- The fallback I asked for is in. JsonScan.cpp:111-131 uses `from_chars` only when `__cpp_lib_to_chars` is defined (and `JW_NO_FROM_CHARS` is not). Otherwise it reads the number with a stream set to the classic locale.
- An underflow such as `1e-400` is now accepted on both paths (:118-119, :127-128). That matches jansson and nlohmann, which the manifest side uses (Manifest.cpp:256-259, :349-353).
- Whether macos-15 passed at f7aa68ebe I take from your report.
- The gaps that remain are filed as claude-17 and claude-18.

**claude-16: accepted.**
- A section name over 1,024 bytes now sets `uncertain` (IniScan.cpp:231-234), so my run-03 repro now redacts `Token=B`.
- §4:86-89 documents `/~section-<n>`. "Section ordinal" is precise enough: neither libobs (config-file.c:229-230) nor the scanner (IniScan.cpp:236) creates a section for `[]`.
- §8:361-363 now states the header rule, replacing "a malformed header is ignored".
- §8:366-367 documents the report budget. It matches Redactor.cpp:46-53, with 64 bytes per item and no `sizeof` term.

**Other changes look correct.** The per-parent child vectors (Redactor.cpp:149-153, :203-212) are linear. The `.gitattributes` under tests/cases is fine.

**claude-17 (Minor, new): the exponent cap gives the wrong magnitude for very long numbers.**
- JsonScan.cpp:77-79 stops adding exponent digits once the exponent reaches 10^6. In a 64 MiB document, leading zeros in a fraction can move the first digit about 6.7×10^7 places, which is far past that cap.
- Repro: `{"bitrate":0.` followed by 999,999 zeros and `1e10000000}`. The number is 1,000,011 bytes long and its value is 10^8,999,999.
  - The code computes magnitude 1, takes the fast path (:106-110) and accepts the number.
  - §12:554 and jansson both reject it. The result is `exact` instead of stored-parse-invalid, and profile.service stores the file instead of omitting it.
- The same cap misclassifies the slow path (:118-119, :127-128).
  - Repro: `1` followed by 1,000,400 zeros and `e-10000000`. This underflows, and jansson accepts it.
  - The code computes magnitude 401 and reports `E_NONFINITE` with `from_chars` and with libc++. With the libstdc++ stream path (`-DJW_NO_FROM_CHARS`) it accepts the number.
- Fix: cap the exponent at 10^9 or more, which is above `kDocumentBytes` + 400.
- It is only Minor because the document structure is still parsed correctly, so no secret is exposed.

**claude-18 (Minor, new): tests that no longer test anything, and stale docs.**
- The locale test scans `1.5` (jw_unit.cpp:521). That number returns from the fast path before any conversion runs, so the test no longer covers r3 item 10. Use a number near the limits, such as `1.5e300`.
- No case reaches the underflow branch. I asked for `1e-400` in run 03. The only small number in the fixtures, `1.5e-300` in numbers.json, stays on the fast path.
- jw_unit.cpp:495-501 still passes if IniScan.cpp:231 is removed. The unknown section alone makes the file opaque, and `Twitch…` is an OAuth section anyway. My run-03 repro, with `[General]\nToken=B`, would actually test the new rule.
- WORKSPACE_FORMAT.md:682-685 still says "only on Linux" and "The macOS runner has not run yet".

FINDING claude-1 | Major | accepted-fix | IniScan follows config-file.c header/line/key rules; the r3 repros redact (jw_unit.cpp:483-493)
FINDING claude-2 | Major | accepted-fix | URL rule applied to ffmpeg_source input whatever is_local_file is (Redactor.cpp:736-737)
FINDING claude-3 | Major | accepted-fix | /~section-<n> for long names (Redactor.cpp:863-866); report capped, platform-independent (:38-55); escape scan limited to the value (:584-585); UrlRanges linear (:449-534)
FINDING claude-4 | Major | accepted-fix | app.modules accepts an array root (Redactor.cpp:1046-1052)
FINDING claude-5 | Minor | accepted-fix | overlapping and touching ranges merged (Redactor.cpp:975-980)
FINDING claude-6 | Minor | accepted-fix | groups items and six audio-device objects scanned (Redactor.cpp:798-812), as OBS loads them
FINDING claude-7 | Minor | accepted-fix | one leading BOM skipped in documents (JsonScan.cpp:748-751); manifests still reject it
FINDING claude-8 | Minor | accepted-fix | the four drift items fixed in run 02
FINDING claude-9 | Major | accepted-fix | query starts at the first '?' and splits on '&' only (Redactor.cpp:503-532)
FINDING claude-10 | Minor | accepted-fix | URL rule over the whole string; authority ends at / ? # (Redactor.cpp:465-501)
FINDING claude-11 | Minor | accepted-fix | number check is locale-free: manual fast path, from_chars or classic-locale stream (JsonScan.cpp:101-131)
FINDING claude-12 | Minor | accepted-fix | plan/apply failure gives omitted/unparseable for secret roles (Redactor.cpp:1070-1079,1096-1105); fuzz checks every range
FINDING claude-13 | Major | accepted-fix | fallback landed: from_chars only under __cpp_lib_to_chars, else classic-locale stream (JsonScan.cpp:111-131); underflow accepted on both paths; residual gaps filed as claude-17/18
FINDING claude-14 | Minor | accepted-fix | §8:348, §8:355-363 and §12:534 match r3; later drift tracked as claude-16
FINDING claude-15 | Minor | accepted-fix | pre-header keys recorded (IniScan.cpp:248-249) and redacted under the fallback (Redactor.cpp:916)
FINDING claude-16 | Minor | accepted-fix | long names set uncertain (IniScan.cpp:231-234); /~section-<n> in §4:86-89; header rule and 64-byte/4 MiB budget in §8:361-367 match Redactor.cpp:46-53
FINDING claude-17 | Minor | open | "if (exponent < 1000000)" (JsonScan.cpp:77) caps below possible fraction offsets: 0.<999,999 zeros>1e10000000 (=10^8,999,999) computes magnitude 1, fast path accepts it (:106-110) against §12:554; same cap misreads underflow as overflow on the slow path (:118-119,:127-128)
FINDING claude-18 | Minor | open | locale test scans "1.5" (jw_unit.cpp:521), which never reaches a conversion; no 1e-400 underflow case; jw_unit.cpp:495-501 passes without the IniScan.cpp:231 trigger; WORKSPACE_FORMAT.md:685 "The macOS runner has not run yet" stale
VERDICT: approve
