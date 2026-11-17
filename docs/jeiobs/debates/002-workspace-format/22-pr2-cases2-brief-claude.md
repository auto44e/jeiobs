# Worker brief (Claude, black-box cases): S1-PR2 review run 01

Your current directory is a JeiOBS worktree (detached HEAD = the PR #28 head). You own
`shared/jeiobs-workspace/tests/cases/`. Another worker changes the library in parallel: do NOT read
`shared/jeiobs-workspace/src/` or `include/`. Work from `/tmp/jeiobs-s2/contract-r3.md` (addendum r3;
wins over `docs/jeiobs/debates/002-workspace-format/14-pr2-contract.md`) and the review replies it
cites in `/tmp/jeiobs-s2/review/01-run/` (repros there).
Update the cases: (a) existing cases that encode old rules (855-857 and any that keep `input`
credentials; 428/665/666 target errors: expect only `E_SCHEMA target.audio`; the ffmpeg-options case
keeping `#PLAINvalue`; the BOM-in-JSON-document cases; the app.modules object-only cases;
collection count for non-string registry values: expect +1) now expect the r3 behaviour;
(b) add cases for every r3 item: the five INI inputs of item 1 (each with a marker secret that must
NOT survive), 1b fallback (CR-only file, malformed header: Token/RefreshToken redacted in any
section), input always redacted (is_local_file 0, null, "false", 1, "true", true, false, absent),
app.modules array and object, overlapping URL ranges (`https://h/?token=https://u:p@x/`), versioned_id,
BOM in documents and BOM in manifests still rejected, SRT `#!::` streamid/passphrase, passphrase
containing `#`, ffmpeg_options with quotes/spaces (`http_proxy='http://u:pa ss@proxy.example:3128'`),
INI key/section count limits (generated, > 1,000,000 keys; > 100,000 sections), and performance
sanity: big generated inputs (a 4 KiB key with 10^5 array items; 10^4 sources with URLs; a 1 MiB
section header with 10^5 keys) each must finish (the runner already has timeouts: set one per case
if not, 60 s) — they must not time out.
Keep it portable (cmake -P only) and the marker-secret scan for every stored OUTFILE. Allowed
commands: Read/Write/Edit/Glob/Grep, `cmake`, `python3` (authoring only), `git status/diff/add/-c … commit`.
Commit on the detached HEAD (no branch, no push): `git -c user.name=auto44e -c user.email=auto44e@gmail.com commit -m "shared: cases for S1-PR2 review fixes (contract r3)"`.
Final reply: what changed, new cases, ambiguities and how you read them, what you could not cover.
