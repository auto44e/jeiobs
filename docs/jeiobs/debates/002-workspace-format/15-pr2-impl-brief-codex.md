# Worker brief (Codex, implementation): JeiOBS S1-PR2

Work ONLY in your current directory (JeiOBS worktree, detached HEAD, main). Read
`docs/jeiobs/WORKSPACE_FORMAT.md` (esp. §6-8, §12, §13) and `/tmp/jeiobs-s2/contract.md` (the exact
PR2 contract; it also lists Part B, small validator fixes). The PR1 library is in
`shared/jeiobs-workspace/` (src/, include/, tests/). Implement everything in the contract in that
directory: `JsonScan`, `IniScan`, `Redactor`, the `jw-check redact` command, Part B, the
`jw-fuzz-smoke` ctest and the optional libFuzzer target.
Rules: another worker writes black-box cases under `shared/jeiobs-workspace/tests/cases/` in
parallel: do NOT edit that directory; the `jw-cases` registration already exists. Add your own unit
tests in `tests/jw_unit.cpp` (or new `tests/*.cpp` registered in CMake). No new dependency; portable
C++17 (GCC/Clang `-Wall -Wextra -Werror`, MSVC `/W4 /WX`); never throw out of the CLI; bounded
memory and no unbounded recursion; use the `OpenUtf8` helper for paths. Format with
`/tmp/cf-venv/bin/clang-format` (repo `.clang-format`). Do not touch files outside
`shared/jeiobs-workspace/`.
Verify: `cmake -S shared/jeiobs-workspace -B /tmp/jeiobs-s2/build-impl -G Ninja && cmake --build
/tmp/jeiobs-s2/build-impl && ctest --test-dir /tmp/jeiobs-s2/build-impl --output-on-failure`
(`jw-unit`, `jw-fuzz-smoke` and the existing `jw-cases` must pass; new redact cases arrive from the
other worker).
Commit on the detached HEAD (no branch, no push): `git -c user.name=auto44e -c user.email=auto44e@gmail.com commit -m "shared: JSON/INI scanners and the redactor (S1-PR2)"`.
Final reply: files and public API, what you verified, every ambiguity and how you read it, risks.
