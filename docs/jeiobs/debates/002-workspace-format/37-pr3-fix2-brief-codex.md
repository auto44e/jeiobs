# Worker brief (Codex, fixes round 2): JeiOBS S1-PR3

Work ONLY in your current directory (JeiOBS worktree, detached HEAD = PR #32 head). Read
`/tmp/jeiobs-s3/contract.md` section "r2" and the review replies in
`~/.local/share/jeiobs-work/reviews/pr32/01-run/`. Implement r2 items 1-9 in `shared/jeiobs-workspace/src/`
(and include/), with unit tests using `MemFs` (and a real temp dir for item 3/4 where needed, POSIX only).
Do NOT edit `tests/cases/` (another worker updates them in parallel; some cases change). Run jw-unit,
jw-fuzz-smoke and jw-cases (`cmake -S shared/jeiobs-workspace -B /tmp/jeiobs-s3/build-r2 -G Ninja && cmake --build
/tmp/jeiobs-s3/build-r2 && ctest --test-dir /tmp/jeiobs-s3/build-r2`), and once under `-fsanitize=address,undefined`
(`ASAN_OPTIONS=detect_leaks=0`). Format with `/tmp/cf-venv/bin/clang-format`. Commit on the detached HEAD (no branch,
no push): `git -c user.name=auto44e -c user.email=auto44e@gmail.com commit -m "shared: planner review fixes (two-pass identity, symlinked roots, listing errors, count field, bounded scans, profiles need basic.ini, too-large)"`.
Final reply: each item with file:line, results, ambiguities.
