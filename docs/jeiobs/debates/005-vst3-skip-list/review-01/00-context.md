# PR #20 review run 01 — task 005 (D6: remember failed VST3 bundles), head 6fb0cd502

Your working directory is a checkout of this commit. The change is `git diff 270f9d6c9 HEAD`
(main is 270f9d6c9). Spec and record: docs/jeiobs/tasks/005-vst3-skip-list.md; design debate and
worker transcripts: docs/jeiobs/debates/005-vst3-skip-list/. Inherited background:
docs/jeiobs/tasks/003-vst3-scanner-robustness.md (the probe/coordinator design this extends).

Authorship: Codex worker wrote the implementation (plugins/obs-vst3/); Claude worker wrote the
checks (tools/jeiobs/vst3-check/skip-list.sh, skiplist.py) from the spec without reading the
implementation; the lead integrated (temp-file fix e6c164fe7, clang-format ea1dbda5f, docs).

Lead's evidence (Linux, Ubuntu 24.04, ENABLE_VST3=ON): tools/jeiobs/vst3-check/run.sh at this
head: 32 PASS, 0 FAIL (16 task-003 checks + 16 skip-list checks). Before the lead's fix: 31 PASS,
#8 stale temp files FAIL. main with the new checks: the 16 new checks FAIL. Windows and macOS are
not run; a 3-OS full build is in progress on this SHA.

Review for correctness against the spec, crash/hang/lock/race behaviour, persistence-file safety
(vst3skip.json is new user state: atomic writes, torn files, unknown versions), platform code
(Windows LockFileEx/CreateFileW, macOS CFBundle), the host side (Rescan flag, warning), test
validity (do the checks prove what they claim), and doc accuracy.

Severity: Blocker = must not merge; Major = must be fixed before a self-merge; Minor = follow-up.
Other agent work runs on this host in separate worktrees; it does not touch this checkout.
