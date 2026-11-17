Implemented and committed as `0a809589c`.

1. INI/libobs rules and fail-closed fallback: IniScan.cpp:104, Redactor.cpp:819. Verified all five CR/header inputs.

2. Always redact ffmpeg `input`: Redactor.cpp:699. Covered by unit test.

3. Bounded indexing, URL scans, pointer materialization, and INI limits: Redactor.cpp:127, IniScan.cpp:141. Worst-case tests complete under five seconds.

4. `app.modules` accepts object or array: Redactor.cpp:950.

5. Edit merging, special-range handling, no arbitrary redaction errors, and stricter fuzz checks: Redactor.cpp:851, jw_fuzz_smoke.cpp:51. CLI remains catch-all protected at `jw-check.cpp:403`.

6. `versioned_id` source selection and non-string registry accounting: Redactor.cpp:659.

7. JSON BOM accepted while manifest BOM remains rejected: JsonScan.cpp:640, Manifest.cpp:359.

8. SRT/query handling through end-of-string with `#` preserved as query data: Redactor.cpp:414.

9. Whole-string ffmpeg options and VLC playlist URL handling: Redactor.cpp:699.

10. Locale-independent number parsing via `from_chars`: JsonScan.cpp:423.

11. Pre-append string limits for JSON and manifests: JsonScan.cpp:178, Manifest.cpp:92.

12. Export `target.audio` errors consistently report `E_SCHEMA target.audio`: Manifest.cpp:545.

Verification passed:

- Exact configure/build command.
- `jw-unit` and `jw-fuzz`: passed.
- ASan/UBSan run: passed.
- Formatting and `git diff --check`: passed.
- No `tests/cases/` files changed.
- Detached worktree is clean.

The parallel case harness still reports expected pre-r3 fixture drift: 865 passed, 17 old cases failed for changed BOM, local-input, query, whole-string options, VLC, and related collection rules. Main ambiguity: `[Twitch]]` is treated as a malformed header with no section change, but fail-closed redaction still removes the token, following addendum 1b.
