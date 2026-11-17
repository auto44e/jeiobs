# Worker brief (Claude, black-box cases): JeiOBS S1-PR3

Your current directory is a JeiOBS worktree (detached HEAD, main). You own
`shared/jeiobs-workspace/tests/cases/` (see `docs/jeiobs/debates/002-workspace-format/30-pr2-cases3-claude.md`
for how you built it). Another worker implements the library in parallel: do NOT read
`shared/jeiobs-workspace/src/` or `include/`. Work from `docs/jeiobs/WORKSPACE_FORMAT.md` and
`/tmp/jeiobs-s3/contract.md` (wins for PR3).
Extend `run-cases.cmake`/`cases.txt` (keep every existing case passing) with a case kind for `jw-check
plan`: first BUILD a synthetic config tree under `${WORK}` with cmake (`file(WRITE ...)`,
`file(MAKE_DIRECTORY ...)`; symlinks via `file(CREATE_LINK ... SYMBOLIC)` guarded by a `@posix` tag as
you did for invalid UTF-8 args; non-UTF-8 and reserved names on POSIX only, tagged `@posix`; very many
files generated), then run the command and compare stdout JSON to an expected file by JSON equality
(with `--uid-seed` for deterministic UIDs).
Cases (both sides of every rule): a full realistic tree (user.ini, plugin_manager/modules.json,
two collections with `.bak`, two profiles with basic.ini/service.json/.bak/encoders, obs-websocket
config.json and persistent_data.json, obs-browser dir, vst3list.json, logs, crashes, unknown files);
every omitted reason; the 100-file collapse (99, 100, 101 files; size and count); symlinks to a file, a
dir, and a dangling one (never followed, `symlink` entries); `.bak` without main; unrepresentable
names (one entry per folder with count, no names); case collision warning and `collision:true`
(POSIX); custom roots via `--roots`; a missing root; identity matching against a `--parent` manifest
(same path; renamed collection with the same scene-UUID set; a tie; a parent claimed twice;
empty scene set; profile rename gets a new UID; collection unparseable warning); `scene_uuids`
validator cases (valid, unsorted, duplicate, non-string, too long, on a non-collection entry, in export
mode dropped by the projection); ReviewRules via `jw-check redact collection` counts with only
scene/group vs other types; Part B number cases for `redact` (5,000-digit number, 7-digit exponent,
`1.5e300`, underflow). Use generated large trees sparingly (cmake speed).
Keep it portable (cmake -P only, Windows/macOS/Linux). Allowed commands: Read/Write/Edit/Glob/Grep,
`cmake`, `python3` (authoring only), `git status/diff/add/-c … commit`. Commit on the detached HEAD
(no branch, no push): `git -c user.name=auto44e -c user.email=auto44e@gmail.com commit -m "shared: black-box cases for the capture planner and identity (S1-PR3)"`.
Final reply: case list, ambiguities and how you read them, what you could not cover.
