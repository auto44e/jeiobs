# Worker brief (Codex, implementation): JeiOBS K1-PR2 (transform execution)

Work ONLY in your current directory (JeiOBS worktree, detached HEAD, main with K1-PR1 merged). Read
`docs/jeiobs/tasks/006-control-protocols.md` (K1 design, A10, A11) and `/tmp/jeiobs-k1pr2/contract.md` (the exact
PR2 contract). Extend `shared/jeiobs-control/`: add `include/jeiobs/control/Transform.hpp` and
`src/Transform.cpp` (add to CMakeLists), the pipeline position rules in `ValidateMapping`, and the
`jc-check transform` command, exactly as the contract says; reuse `Value` (`AsDouble`, `Make*`) and the
existing JSON value helpers in `src/jc-check.cpp` (read how `jc-check value` parses and prints value JSON).
Do NOT edit `shared/jeiobs-control/tests/cases/` (another worker is writing cases from the contract; the
existing cases must still pass). Add unit and property tests to `tests/jc_unit.cpp` (or a new
`tests/jc_transform_unit.cpp` registered in CMake) as the contract's Tests section lists. No new third-party
dependency; portable C++17 (GCC/Clang `-Wall -Wextra -Werror`, MSVC `/W4 /WX`); never throw out of the CLI or
the public API; no recursion; format with `/tmp/cf-venv/bin/clang-format` (version 22).
Verify: `cmake -S shared/jeiobs-control -B /tmp/jeiobs-k1pr2/build-impl -G Ninja && cmake --build
/tmp/jeiobs-k1pr2/build-impl && ctest --test-dir /tmp/jeiobs-k1pr2/build-impl --output-on-failure` (all of
jc-unit and jc-cases) and once under `-fsanitize=address,undefined` (`ASAN_OPTIONS=detect_leaks=0`, a separate
build dir). Commit on the detached HEAD (no branch, no push): `git -c user.name=auto44e -c user.email=auto44e@gmail.com
commit -m "shared: jeiobs-control transform execution (K1-PR2)"`.
Final reply: files and public API, what you verified, ambiguities and how you read them, risks.
