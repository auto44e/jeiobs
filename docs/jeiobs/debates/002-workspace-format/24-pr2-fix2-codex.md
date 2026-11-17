Implemented and committed as `a2a90344a`.

- Libobs-compatible INI parsing/fallbacks and URL query handling.
- Corrected `versioned_id` fallback and `target.audio` errors.
- No test cases modified.

Tests passed:

- Normal ctest: 3/3, including 968 cases.
- ASan/UBSan ctest: 3/3 with `ASAN_OPTIONS=detect_leaks=0` (ptrace prevented LeakSanitizer otherwise).
- No cases contradicted libobs according to `/tmp/gt/gt`.
