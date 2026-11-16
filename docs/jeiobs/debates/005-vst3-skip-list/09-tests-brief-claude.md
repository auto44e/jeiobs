# Worker brief: JeiOBS task 005 (D6) — remember failed VST3 bundles (TESTS)

Work ONLY in `~/oss/jeiobs-wt/d6-test` (detached HEAD). Edit only `tools/jeiobs/vst3-check/`. No network, no push. Commit with
`git -c user.name=auto44e -c user.email=auto44e@gmail.com commit`.

Read the specification `/tmp/jeiobs-d6/005-vst3-skip-list.md`, especially "Tests"
(items 1-11), and the existing suite `tools/jeiobs/vst3-check/run.sh` (16 checks; its
contract: throwaway HOMEs, offscreen Qt, unreachable audio server, nothing written
outside /tmp and DEPS_DIR, no leftover processes, PASS/FAIL lines with measured values,
exit 0 only if all pass, exit 2 = no verdict). Another worker implements the feature in
parallel (you will not see it). Implement checks for spec test items 1-11 as new
sections of run.sh (or a sibling script called from run.sh), with new fixtures as
needed (e.g. the ADelay-forwarding shim for item 5, a load marker written by fixtures
so "not loaded" is asserted, not inferred). Each new check must FAIL against the
current build `~/oss/jeiobs-build/vst3`? — no: that build predates task 003. Use the
build at `~/oss/jeiobs-wt/d6-impl/_build` (current code without D6; the other worker
will change it): every new check should FAIL there before the implementation lands,
and the existing 16 must still PASS. Report the exact output. Commit and print a
summary, the run output, and doubts (timing margins, root-only items like ulimit).

IMPORTANT git rules (another process checks this repository's refs and config):
commit only on the detached HEAD of your worktree; never create, delete or move
branches or tags; never run git fetch, gc, stash, submodule, config or worktree
commands. Print the final commit SHA.
