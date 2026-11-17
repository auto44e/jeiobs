# Worker brief (Codex, implementation): JeiOBS K1-PR1

Work ONLY in your current directory (JeiOBS worktree, detached HEAD, main). Read
`docs/jeiobs/tasks/006-control-protocols.md` ("K1 design", A1-A18) and `/tmp/jeiobs-k1pr1/contract.md`
(the exact PR1 contract). Create `shared/jeiobs-control/` as a new standalone CMake project modelled
on `shared/jeiobs-workspace/` (read its CMakeLists.txt, src layout, `Utf8Path.hpp`, the bounded JSON
parsing in `JsonScan`/`Manifest`, and its portability lessons in `docs/jeiobs/WORKSPACE_FORMAT.md`
section 15). Implement the C++ API, the schema parser/validator, the compile checks and the `jc-check`
CLI exactly as the contract says, with unit tests (`jc-unit`, an in-tree test header, no framework) and
a CTest registration `jc-cases` that runs `tests/cases/run-cases.cmake` if present (the other worker
writes `tests/cases/`; do NOT create or edit that directory). Do not copy code from jeiobs-workspace
into a shared dependency: duplicate what you need (this library must not depend on it); do not touch
files outside `shared/jeiobs-control/`. No new third-party dependency; portable C++17; never throw out
of the CLI or public API; bounded memory/time (10,000 routes with 64-wide fan-out must validate in
seconds); format with `/tmp/cf-venv/bin/clang-format`.
Verify: `cmake -S shared/jeiobs-control -B /tmp/jeiobs-k1pr1/build-impl -G Ninja && cmake --build
/tmp/jeiobs-k1pr1/build-impl && ctest --test-dir /tmp/jeiobs-k1pr1/build-impl --output-on-failure -R jc-unit`
and once under `-fsanitize=address,undefined` (`ASAN_OPTIONS=detect_leaks=0`).
Commit on the detached HEAD (no branch, no push): `git -c user.name=auto44e -c user.email=auto44e@gmail.com commit -m "shared: jeiobs-control types, mapping schema, validator and compiler (K1-PR1)"`.
Final reply: files and public API, what you verified, ambiguities and how you read them, risks.
