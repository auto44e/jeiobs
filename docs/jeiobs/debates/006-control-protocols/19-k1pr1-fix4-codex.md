Implemented and committed as `7320d3aa6`.

Changes:
- Sign-extended `Int` raw storage with round-trip unit coverage.
- Removed unused `jc-check` variable.
- Replaced quadratic emit-cycle adjacency storage with lazy bounded traversal.
- No `tests/cases` changes.

Results:

- Normal build/ctest: 2/2 passed.
- ASan/UBSan ctest: 2/2 passed.
- clang-format and whitespace checks passed.
- Worktree clean, detached HEAD.
