# Worker brief (Codex, fixes): S1-PR1 review run 01

Work ONLY in your current directory (JeiOBS worktree, detached HEAD at 13cf59408). Read
`docs/jeiobs/debates/002-workspace-format/09-pr1-contract-r2.md` (addendum r2: it wins over the
original contract `04-pr1-contract.md`) and the review replies in
`docs/jeiobs/debates/002-workspace-format/review-01/03-r2-claude.reply.md` and
`04-r2-codex.reply.md`. Implement every item of addendum r2 in `shared/jeiobs-workspace/`
(library and `jw-check`), plus: drop the unused `LoadedManifest` if nothing uses it (claude-6).
Do NOT edit `shared/jeiobs-workspace/tests/cases/` (another worker updates the black-box cases
in parallel; some existing cases will fail until both land, that is expected). Keep `jw-unit`
passing and add unit tests in `tests/jw_unit.cpp` for the new rules.
Constraints: no new dependency; portable C++17 (Windows MSVC /W4 /WX, GCC/Clang -Wall -Wextra
-Werror); repo `.clang-format` via `/tmp/cf-venv/bin/clang-format`; never throw out of the CLI.
Verify with `cmake -S shared/jeiobs-workspace -B /tmp/jeiobs-s1/pr1/build-fix -G Ninja && cmake
--build /tmp/jeiobs-s1/pr1/build-fix && ctest --test-dir /tmp/jeiobs-s1/pr1/build-fix -R jw-unit`.
Commit on the detached HEAD (no branch, no push):
`git -c user.name=auto44e -c user.email=auto44e@gmail.com commit -m "shared: S1-PR1 review fixes (allowlist-bound stored entries, consistent entry fields, bounded decisions and revision reads, CON .txt, derived fidelity, rebuilt export objects)"`.
Final reply: each addendum item with file:line, what you verified, ambiguities and how you read them.
