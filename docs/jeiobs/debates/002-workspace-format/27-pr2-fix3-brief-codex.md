# Worker brief (Codex, fixes round 3): S1-PR2

Work ONLY in your current directory (JeiOBS worktree, detached HEAD = the PR #28 head). Read
`/tmp/jeiobs-s2/contract-r3.md` section "r3.2" and the review replies in `/tmp/jeiobs-s2/review/02-run/`.
Implement r3.2 items 1-4 in `shared/jeiobs-workspace/src/` and your own unit tests/fuzz smoke. Do NOT
edit `tests/cases/` (another worker updates the cases). Use `/tmp/gt/gt` (libobs's INI parser) as the
oracle for INI questions. Build: `cmake -S shared/jeiobs-workspace -B /tmp/jeiobs-s2/build-r32 -G Ninja &&
cmake --build /tmp/jeiobs-s2/build-r32 && ctest --test-dir /tmp/jeiobs-s2/build-r32`; also under
`-fsanitize=address,undefined` (ASAN_OPTIONS=detect_leaks=0). Format with `/tmp/cf-venv/bin/clang-format`.
Commit on the detached HEAD (no branch, no push): `git -c user.name=auto44e -c user.email=auto44e@gmail.com commit -m "shared: bounded INI pointers, fail-closed apply, whole-file fallback, audio-device and group sources"`.
Final reply: each item with file:line, results, ambiguities.
