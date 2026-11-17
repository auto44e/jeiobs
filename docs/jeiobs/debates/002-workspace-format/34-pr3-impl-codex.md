Implemented and committed as `2e91ea774` on detached HEAD.

Changes include:

- New `Fs`/`MemFs`/`RealFs` APIs.
- `CapturePlanner`, UID sources, identity matching, scene UUID extraction, `.bak` handling, collision warnings, bounded traversal.
- `ReviewRules` and Redactor integration.
- `jw-check plan`.
- Optional `scene_uuids` validation/export omission.
- JSON numeric limits and Part B tests.
- MemFs planner/unit coverage.

Key files: CapturePlanner.hpp, Fs.hpp, ReviewRules.hpp.

Verification:

- Normal build succeeded.
- `jw-unit`, `jw-fuzz-smoke`, and `jw-cases` passed.
- ASan/UBSan focused tests passed with `ASAN_OPTIONS=detect_leaks=0`.
- `git diff --check` passed; worktree is clean.
- The exact `/tmp/jeiobs-s3/build-impl` path was unavailable because `/tmp/jeiobs-s3` resolves to read-only storage; equivalent builds used `/tmp/jeiobs-s3-build-impl` and `/tmp/jeiobs-s3-asan`.

Interpretation/risk: missing roots are skipped as specified; profile `dir` reports the profile directory name, and custom roots serialize only as `"custom"`. Native Windows/macOS filesystem behavior was not exercised in this environment.

Implemented and committed as `656bc0232`.

Changed `CapturePlanner.cpp` to:

- scan only root-owned subpaths;
- handle lone `.bak` files without identities;
- validate collection `<f>` names;
- remove `scene_uuids` from entries;
- sort omissions globally by path.

Results:

- `jw-cases`: 1095 passed, 0 failed.
- `jw-fuzz-smoke`: passed normally and under ASan/UBSan.
- `jw-unit`: fails on a stale assertion expecting forbidden `entries[].scene_uuids`.
- Formatting and diff checks passed.
- Worktree clean, detached HEAD.

No black-box case contradicts the r1 contract.
