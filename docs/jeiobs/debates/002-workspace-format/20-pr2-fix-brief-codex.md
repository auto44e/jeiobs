# Worker brief (Codex, fixes): S1-PR2 review run 01

Work ONLY in your current directory (JeiOBS worktree, detached HEAD = the PR #28 head).
Read `/tmp/jeiobs-s2/contract-r3.md` (addendum r3: it wins over `docs/jeiobs/debates/002-workspace-format/14-pr2-contract.md`)
and the review replies it cites in `/tmp/jeiobs-s2/review/01-run/`. Implement items 1-12 in
`shared/jeiobs-workspace/` (library, `jw-check`, and your own unit tests/fuzz smoke). Do NOT edit
`shared/jeiobs-workspace/tests/cases/` (another worker updates the cases in parallel; some cases will
fail until both land, that is expected, and some old cases encode the old rules: cases 855-857
(input kept), cases 428/665/666 (E_SCHEMA target), the ffmpeg-options case that keeps `#PLAINvalue`).
For item 1, read libobs `libobs/util/config-file.c`, `lexer.c`, `lexer.h` in this tree and reproduce
their line, header and key rules exactly, then add the fail-closed fallback of item 1b.
Constraints: no new dependency; portable C++17 (GCC/Clang -Wall -Wextra -Werror, MSVC /W4 /WX); never
throw out of the CLI; bounded memory/time as item 3 says; format with `/tmp/cf-venv/bin/clang-format`.
Verify: `cmake -S shared/jeiobs-workspace -B /tmp/jeiobs-s2/build-r3 -G Ninja && cmake --build
/tmp/jeiobs-s2/build-r3 && ctest --test-dir /tmp/jeiobs-s2/build-r3 -R "jw-unit|jw-fuzz"` must pass;
also run once under `-fsanitize=address,undefined`.
Commit on the detached HEAD (no branch, no push): `git -c user.name=auto44e -c user.email=auto44e@gmail.com commit -m "shared: S1-PR2 review fixes (libobs INI rules, input always redacted, bounded scanners, modules array, merged ranges, SRT query, ffmpeg options, locale)"`.
Final reply: each item with file:line, what you verified, ambiguities and how you read them.
