# Worker brief (Codex, fixes round 2): JeiOBS K1-PR1

Work ONLY in your current directory (JeiOBS worktree, detached HEAD = PR #33 head). Read
`/tmp/jeiobs-k1pr1/contract.md` section "r2" and the review replies in
`~/.local/share/jeiobs-work/reviews/pr33/01-run/`. Implement r2 items 1-8 in `shared/jeiobs-control/src/` (and
include/), with unit tests. Do NOT edit `tests/cases/` (another worker updates them; some existing cases change:
item 4 makes `url` denied). Build and run `cmake -S shared/jeiobs-control -B /tmp/jeiobs-k1pr1/build-r2 -G Ninja &&
cmake --build /tmp/jeiobs-k1pr1/build-r2 && ctest --test-dir /tmp/jeiobs-k1pr1/build-r2`, and once under
`-fsanitize=address,undefined` (`ASAN_OPTIONS=detect_leaks=0`). Keep MSVC in mind (no native 128-bit integers:
the library must already have a portable 128-bit type; keep it). Format with `/tmp/cf-venv/bin/clang-format`.
Commit on the detached HEAD (no branch, no push): `git -c user.name=auto44e -c user.email=auto44e@gmail.com commit -m "shared: jeiobs-control review fixes (clamping, one strict parser, hotkeys privileged, source URL deny, linear parsing, injective keys)"`.
Final reply: each item with file:line, results, ambiguities.
