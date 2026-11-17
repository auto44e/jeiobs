# Worker brief (Codex, implementation): JeiOBS S1-PR3

Work ONLY in your current directory (JeiOBS worktree, detached HEAD, main). Read
`docs/jeiobs/WORKSPACE_FORMAT.md` (§6-9, §12, §15) and `/tmp/jeiobs-s3/contract.md` (the exact PR3
contract; it lists earlier contracts that still hold). The library is `shared/jeiobs-workspace/`.
Implement everything in the contract there: `Fs`/`MemFs`/`RealFs`, `CapturePlanner`, `UidSource`,
identity matching, the optional `scene_uuids` validator rule and export drop, `ReviewRules`, the
`jw-check plan` command, and Part B items 1-2. Another worker writes black-box cases under
`shared/jeiobs-workspace/tests/cases/` in parallel: do NOT edit that directory. Add your own unit
tests (`tests/jw_unit.cpp` or new files registered in CMake) with `MemFs`. No new dependency; portable
C++17 (GCC/Clang `-Wall -Wextra -Werror`, MSVC `/W4 /WX`; Apple libc++ lacks some of libstdc++'s
conveniences and has different container performance: no `unordered_multimap` for large groups, no
floating-point `from_chars` assumption); never throw out of the CLI; bounded memory and time (a
directory with 10^6 files must plan in a few seconds and bounded memory: stream the listing, count,
collapse); explicit stack, no deep recursion; do not follow symlinks. Format with
`/tmp/cf-venv/bin/clang-format`; do not touch files outside `shared/jeiobs-workspace/`.
Verify: `cmake -S shared/jeiobs-workspace -B /tmp/jeiobs-s3/build-impl -G Ninja && cmake --build
/tmp/jeiobs-s3/build-impl && ctest --test-dir /tmp/jeiobs-s3/build-impl --output-on-failure -R "jw-unit|jw-fuzz"`
(and `jw-cases` for the existing cases) and once under `-fsanitize=address,undefined`
(`ASAN_OPTIONS=detect_leaks=0`).
Commit on the detached HEAD (no branch, no push): `git -c user.name=auto44e -c user.email=auto44e@gmail.com commit -m "shared: CapturePlanner, identity and review rules (S1-PR3)"`.
Final reply: files and public API, what you verified, ambiguities and how you read them, risks.
