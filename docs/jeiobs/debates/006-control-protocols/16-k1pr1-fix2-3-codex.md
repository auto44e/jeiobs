Implemented and committed as `ca377bf6b` on detached HEAD.

1. Value clamping/validation — `Value.cpp:286-303, 440-446, 565-623`; tests `jc_unit.cpp:55-95`.
2. Unified strict parsing — `Mapping.cpp:35, 89-202, 1469-1559, 1708`; tests `jc_unit.cpp:135-150, 198-202`.
3. Hotkeys always privileged — `Mapping.cpp:1337-1338`; test `jc_unit.cpp:192-196`.
4. Denied source fields — `Mapping.cpp:964-980`; tests `jc_unit.cpp:152-159`.
5. Linear JSON object handling — `Mapping.cpp:35, 89-202`.
6. Injective escaped keys — `Mapping.cpp:1033-1086, 1395-1462`; tests `jc_unit.cpp:204-216`.
7. ASCII requirements — `Mapping.cpp:334-346`; test `jc_unit.cpp:148-150`.
8. Indexed cycle checks and O(n log n) merge checks — `Mapping.cpp:1157-1216, 1608-1643`.

Results:

- Normal build succeeded.
- `jc-unit`: passed.
- ASan/UBSan build and unit tests: passed, no sanitizer diagnostics.
- Full `ctest`: 899/916 passed; 17 failures are stale `tests/cases/` expectations for these intentional r2 changes. No cases were edited.
- Formatting and `git diff --check`: passed.
- Worktree is clean.

Ambiguities: `playlist` was denied as the exact r2 field name; nested playlist values and `access_token` were not added. No MSVC build was available, but the existing portable 128-bit types remain unchanged.

Implemented and committed as `1b1a4c820`.

Changes:
- Added `value fromdouble` CLI support and sorted value JSON serialization.
- Enforced `E_LIMIT event` for oversized event files.
- Fixed nested integer limit errors to return `E_LIMIT value`.
- Added finite clamping for float conversions.

Results:

- Normal CTest: 2/2 passed.
- ASan/UBSan CTest: 2/2 passed.
- Worktree clean, detached HEAD, no push.
