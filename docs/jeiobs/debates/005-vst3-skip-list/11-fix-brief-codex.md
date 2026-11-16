# Worker brief (Codex, fixes): task 005 (D6) review run 01 findings

Work ONLY in your current directory (a git worktree of JeiOBS, detached HEAD at 6fb0cd502, with
a configured build in `_build`). Read `docs/jeiobs/tasks/005-vst3-skip-list.md` and the review
replies in `/tmp/jeiobs-d6/review/01-run/03-r2-claude.reply.md` and `04-r2-codex.reply.md`.
Fix these in `plugins/obs-vst3/vst3-scan/vst3-scanner.cpp` (and only where needed elsewhere in
`plugins/obs-vst3/`), using the lead's decisions:

1. codex-1: an `os_process_pipe_wait` (or other wait/poll) error is NOT a deadline kill. Record a
   timeout strike only when the coordinator's own deadline (monotonic clock) has actually passed
   and it killed the probe after `setup`. A wait error leaves the entry unchanged.
2. codex-2 + claude-2: exit codes stay diagnostics, but two of them may only DOWNGRADE:
   - the probe exits with code 90 when it could not save its result AND could not write the
     `save-failed` marker; `setup` + no result + exit 90 → entry unchanged;
   - the in-probe watchdog exits with code 91 after its deadline when it could not write the
     `timeout` marker; `setup` + no result + exit 91 → timeout strike (not a remembered failure).
   A plugin that happens to exit 90/91 is thereby treated less stickily, never more.
3. claude-1: a valid result with the matching nonce means "removed" (the bundle is fine)
   regardless of markers; check it before any marker logic.
4. codex-4: make "setup marker written" and its publication to the watchdog atomic with respect
   to the watchdog (e.g. a mutex held across the marker rename and the flag store, taken by the
   watchdog before it reads the flag).
5. codex-3: when loading `vst3skip.json`, ignore an entry whose numeric fields are not integers
   (check the item type/number type; do not accept doubles, strings or missing values silently
   coerced by `obs_data_get_int`); a non-integer `version` means the file is ignored as unknown.
6. claude-3: on Linux resolve the arch directory exactly as the VST3 SDK does
   (`<uname().machine>-linux`, see the SDK's `module_linux.cpp` under
   `~/oss/jeiobs-deps/vst3` or the vendored SDK used by the build), instead of a fixed mapping.

Constraints: keep the existing behaviour and checks passing; tabs and repo `.clang-format`
(clang-format 22: `/tmp/cf-venv/bin/clang-format`; format only lines you change, e.g. with
`PATH=/tmp/cf-venv/bin:$PATH git clang-format --binary /tmp/cf-venv/bin/clang-format HEAD`).
Do not touch `tools/`, `docs/`, `.github/`, `deps/`. Verify: `cmake --build _build -j8` and
`tools/jeiobs/vst3-check/run.sh $PWD/_build ~/oss/jeiobs-deps` (32 checks must still pass).
Commit on the detached HEAD (no branches, no push):
`git -c user.name=auto44e -c user.email=auto44e@gmail.com commit -m "obs-vst3: D6 review fixes (wait errors, downgrade-only exit codes, result first, setup race, integer fields, SDK arch dir)"`.
Final reply: each fix with file:line, what you verified, and anything you could not do.
