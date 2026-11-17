# Worker brief (Claude, black-box cases): JeiOBS S1-PR2

Your current directory is a JeiOBS worktree (detached HEAD, main). You own
`shared/jeiobs-workspace/tests/cases/` (see `docs/jeiobs/debates/002-workspace-format/13-r2-cases-claude.md`
for how you built it before). Another worker implements the library in parallel: do NOT read
`shared/jeiobs-workspace/src/` or `include/`. Work from `docs/jeiobs/WORKSPACE_FORMAT.md` and
`/tmp/jeiobs-s2/contract.md` (wins over the spec for PR2).
Extend `run-cases.cmake`/`cases.txt` (keep every existing case passing) with a case kind for
`jw-check redact`: run the command, compare stdout/exit, compare REPORT JSON by equality against an
expected file, and compare OUTFILE BYTES against an expected file (or against rules you can state in
cmake: e.g. "equals INFILE with these byte ranges removed/replaced"). Use `--ws-placeholder` for a
deterministic websocket value; for the default random placeholder assert the length 32 and
charset and that two runs differ.
Fixtures (synthetic, one marker secret each, such as `SECRETvalue-<role>-<n>` so a "no secret
byte remains" scan is a simple string search over OUTFILE; do not use real tokens): rtmp_common
service.json with key; rtmp_custom with server userinfo+query token+username/password; whip_custom with
bearer_token; basic.ini (LF and CRLF, with and without UTF-8 BOM, duplicate sections and duplicate
keys, comments) with Twitch/YouTube/Restream Token and RefreshToken and FFURL with credentials;
obs-websocket config.json with password and `"auth_required": false`, true, absent, and with extra
unknown keys; collections with browser_source url, ffmpeg_source (local and remote), vlc_source
playlist, mixed with scenes/groups; encoder JSON and modules.json; user.ini with and without
ExtraBrowserDocks.
Cases (both sides of every rule): every registry row; non-string values (number/null/object) at
registry positions; duplicate registry keys in JSON (→ omitted unparseable) and in INI (all
occurrences redacted); escaped quotes and `\uXXXX` inside values (range covers raw bytes between
the quotes); URL rule: userinfo only, query only, both, multiple params, mixed case names
(`Token=`, `PASSWORD=`), fragments, no scheme, `@` in the path, percent-encoded, an
`ffmpeg_options` string with several tokens; invalid files (UTF-8, NUL, BOM in JSON, depth 65,
string over limit generated, 64 MiB+1 generated → E_LIMIT blob, truncated JSON) for roles with and
without secrets; every unreviewed rule and its count (service type, FFURL non-empty, unknown INI
sections incl. case differences, websocket unknown keys, collection count); fidelity precedence;
roles list (unknown role → E_SCHEMA role); `--ws-placeholder` bad values; offsets: for every
redaction in a report assert 0 <= start <= end <= input size and sorted. For every `stored` OUTFILE
assert no marker secret bytes remain (string(FIND)). Also add the Part B validator cases
(app_keys restore, dropped_ext shape, target.audio shapes, creator/active value types, export reads
once is not testable: skip).
Keep it portable (cmake -P only, Windows/macOS/Linux; generated big inputs in WORK_DIR; avoid text
newline conversion; keep `.gitattributes` `* -text` in tests/cases).
Allowed commands: Read/Write/Edit/Glob/Grep, `cmake`, `b2sum`, `python3` (authoring only),
`git status/diff/add/-c … commit`. Validate with a throwaway fake in /tmp if useful (not committed).
Commit on the detached HEAD (no branch, no push): `git -c user.name=auto44e -c user.email=auto44e@gmail.com commit -m "shared: black-box cases for the redactor and scanners (S1-PR2)"`.
Final reply: case list, ambiguities and how you read them, what you could not cover.
