Implemented and committed as `8b1547dbb`.

1. Identity two-pass matching — `CapturePlanner.cpp:1079-1203`. Path matches claim first; unique scene-set matching considers only unmatched collections.

2. Theme handling — `CapturePlanner.cpp:601-646`. Only direct config-root `themes` is skipped; nested themes are processed.

3. Symlinked roots — `Fs.cpp:63-76`, `CapturePlanner.cpp:334-352`. Symlink/reparse roots produce omitted entries and `root-symlink` warnings.

4. Listing errors — `Fs.cpp:250-299`, `CapturePlanner.cpp:504-568`. Partial/unreadable listings roll back and emit `unreadable` plus `listing-error`.

5. `count` field — `Manifest.cpp:853-856`, `1126-1133`, `1255-1262`. Validated, preserved, and recognized by carry-over/export.

6. Bounded scans/complexity — `CapturePlanner.cpp:719-784`, `215-228`, `1018-1037`; `ReviewRules.cpp:39-53`. UUIDs cap at 100,000; metadata and entry limits are bounded; indexed lookups replace quadratic scans.

7. Collapse accounting — `CapturePlanner.cpp:431-495`, `294-304`. Recursive regular-file size/count accounting avoids symlink traversal and truncation.

8. Profiles require `basic.ini` — `CapturePlanner.cpp:913-967`. Folders without it become one unknown directory omission with no profile UID.

9. Too-large handling — `CapturePlanner.cpp:370-381`, `649-670`. Files over 64 MiB or exceeding the 256 MiB stored total become `too-large`.

Results:

- Normal build/unit/fuzz: passed.
- Normal `jw-cases`: 1,094/1,095 passed.
- ASan/UBSan (`ASAN_OPTIONS=detect_leaks=0`): same result, no sanitizer findings.
- Remaining failure: unchanged `plan-symlinks@posix` expects zero warnings, conflicting with r2’s required `root-symlink` warning. `tests/cases/` was intentionally not modified.
- Worktree is clean.

Implemented and committed `6da909af6`.

Changed `CapturePlanner.cpp` to:

- Process roots in plan order for correct 256 MiB accounting.
- Sort collection files by name before applying size limits.

Results:

- Normal CTest: 3/3 passed; 1134 cases passed.
- ASan/UBSan CTest: 3/3 passed.
- clang-format and diff checks passed.
- Worktree clean, detached HEAD.
