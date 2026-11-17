# Worker brief (Codex, fixes round 2): S1-PR2

Work ONLY in your current directory (JeiOBS worktree, detached HEAD, the PR #28 branch plus the first
fix round and the r3 cases). Read `/tmp/jeiobs-s2/contract-r3.md` (including section "r3.1") and run
`ctest --test-dir /tmp/jeiobs-s2/build-r3 -V -R jw-cases 2>&1 | grep ' FAIL '` (rebuild first with
`cmake --build /tmp/jeiobs-s2/build-r3`) to see the 14 failing cases. Fix `shared/jeiobs-workspace/src/`
(IniScan, Redactor, Manifest) so they pass. Do NOT edit `tests/cases/`. For INI questions use
`/tmp/gt/gt <file>` (libobs's own parser) as the oracle: where an expected report contradicts what
libobs reports, do not change the case; list it in your final reply. Failing groups:
(a) partb-export-audio-unknown-member: export `target.audio` errors are `E_SCHEMA target.audio`;
(b) redact-url-hash-before-first-question-mark (a leak: `?` after `#`);
(c) the redact-basic-r3-* and redact-user-r3-* cases (IniScan must follow libobs sections:
`[Twitch` and `[Twitch]]` are section Twitch; fallback 1b conditions; a leak where `[General] Token`
survives after a `[Twitch] ;comment` header);
(d) redact-collection-versioned-id (count 6 vs 7; empty `versioned_id` falls back to `id`).
Then run the whole ctest, and once under `-fsanitize=address,undefined`. Format with
`/tmp/cf-venv/bin/clang-format`. Commit on the detached HEAD (no branch, no push):
`git -c user.name=auto44e -c user.email=auto44e@gmail.com commit -m "shared: libobs-exact INI sections, URL query after the first ?, fallback triggers, versioned_id"`.
Final reply: what changed, test results, any case you believe contradicts libobs.
