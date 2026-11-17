# Worker brief (Codex, fixes): JeiOBS S1-PR3

Work ONLY in your current directory (JeiOBS worktree, detached HEAD: your PR3 commit plus the black-box
cases). Read `/tmp/jeiobs-s3/contract.md` section "r1" (it wins). Rebuild and run:
`cmake -S shared/jeiobs-workspace -B /tmp/jeiobs-s3/build-int -G Ninja && cmake --build /tmp/jeiobs-s3/build-int &&
ctest --test-dir /tmp/jeiobs-s3/build-int -V -R jw-cases 2>&1 | grep ' FAIL '` (20 failures). Fix
`shared/jeiobs-workspace/src/` so every case passes; do NOT edit `tests/cases/`. Also run jw-unit,
jw-fuzz-smoke, and once under `-fsanitize=address,undefined` (`ASAN_OPTIONS=detect_leaks=0`); format with
`/tmp/cf-venv/bin/clang-format`. Commit on the detached HEAD (no branch, no push):
`git -c user.name=auto44e -c user.email=auto44e@gmail.com commit -m "shared: planner follows contract r1 (owned subpaths per root, lone .bak, <f> rules, no scene_uuids on entries)"`.
Final reply: what changed, results, any case you believe contradicts the contract.
