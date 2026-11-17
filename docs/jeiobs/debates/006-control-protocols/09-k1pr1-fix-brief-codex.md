# Worker brief (Codex, fixes): JeiOBS K1-PR1

Work ONLY in your current directory (JeiOBS worktree, detached HEAD: your K1-PR1 commit plus the
black-box cases). Read `/tmp/jeiobs-k1pr1/contract.md` section "r1" (it wins). Rebuild and run:
`cmake -S shared/jeiobs-control -B /tmp/jeiobs-k1pr1/build-int -G Ninja && cmake --build /tmp/jeiobs-k1pr1/build-int &&
ctest --test-dir /tmp/jeiobs-k1pr1/build-int -V -R jc-cases 2>&1 | grep ' FAIL '` (23 failures). Fix
`shared/jeiobs-control/src/` (and include/) so every case passes; do NOT edit `tests/cases/`. Run jc-unit and once
under `-fsanitize=address,undefined` (`ASAN_OPTIONS=detect_leaks=0`); format with `/tmp/cf-venv/bin/clang-format`.
Commit on the detached HEAD (no branch, no push):
`git -c user.name=auto44e -c user.email=auto44e@gmail.com commit -m "shared: jeiobs-control follows contract r1 (required fields, address length, escaped keys, strict events, exact doubles, nested limits)"`.
Final reply: what changed, results, any case you believe contradicts the contract.
