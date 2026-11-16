# Worker brief (Claude, tests): task 005 (D6) review gaps

Your current directory is a git worktree of JeiOBS (detached HEAD at 6fb0cd502). You wrote
`tools/jeiobs/vst3-check/skip-list.sh` and `skiplist.py` earlier (see
`docs/jeiobs/debates/005-vst3-skip-list/10-tests-claude.md`). Another worker is changing the
implementation in parallel; do NOT read `plugins/obs-vst3/`. Work from the spec
(`docs/jeiobs/tasks/005-vst3-skip-list.md`) plus these lead decisions:
- a valid result with the matching nonce removes the entry regardless of markers;
- a coordinator deadline kill after `setup` is a timeout strike; a wait error is not;
- probe exit code 90 (`setup`, no result, could not write the `save-failed` marker) leaves the
  entry unchanged; exit 91 (in-probe watchdog could not write `timeout`) is a timeout strike;
- entries with non-integer numeric fields are ignored; a non-integer `version` ignores the file;
- on Linux the binary is `Contents/<uname -m>-linux/<stem>.so`.

Add checks (review finding claude-4 and the decisions above), each one failing for the right
reason against the current implementation where it differs:
1. load-failed with a stable fingerprint (a bundle whose `.so` loads but has no
   `GetPluginFactory`, so `Module::create` fails) → remembered failure, skipped next scan;
2. setup + clean exit without a result (the `exit0` fixture) → remembered failure entry;
3. frozen probe killed by the coordinator after setup → `kind: timeout`, failures 1;
4. a fixture that exits 90 after setup → no entry; one that exits 91 → timeout strike;
5. a hand-written `vst3skip.json` entry with `size` as a double or string → ignored (bundle
   probed again); `version: 1.5` → whole file ignored;
6. update the task doc's test list (`docs/jeiobs/tasks/005-vst3-skip-list.md`, "Tests" and
   "Not done") to match.
Use existing fixture-building patterns in `tools/jeiobs/vst3-check/`. Never use the real HOME.
Run `tools/jeiobs/vst3-check/run.sh <build> ~/oss/jeiobs-deps` with the build at
`~/oss/jeiobs-wt/d6-impl/_build` only to see which new checks fail before the fixes (the other
worker may be rebuilding it; if a run is inconsistent, rerun). Commit on the detached HEAD
(no branches, no push): `git -c user.name=auto44e -c user.email=auto44e@gmail.com commit -m "tools: D6 checks for load-failed, exit0, deadline kill, downgrade exit codes, malformed entries"`.
Final reply: the new checks, their results against the build you ran, and doubts.
