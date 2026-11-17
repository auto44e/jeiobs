Implemented and committed as `3df509972`.

Changed:

- Propagated partial listing errors with `unreadable` entries and warnings.
- Removed owned-root invalid-name leaks.
- Prevented collapse sampling from silently dropping empty/non-regular entries.
- Replaced omitted-entry scans with indexed merges.
- Applied the 256 MiB budget in sorted plan order.
- Added focused unit tests; `tests/cases` unchanged.

Results:

- Normal ctest: 3/3 passed.
- ASan/UBSan (`ASAN_OPTIONS=detect_leaks=0`): 3/3 passed.
- clang-format and diff checks passed.
- Detached HEAD is clean.
