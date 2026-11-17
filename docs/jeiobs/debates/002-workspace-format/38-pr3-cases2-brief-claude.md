# Worker brief (Claude, cases round 2): JeiOBS S1-PR3

Your current directory is a JeiOBS worktree (detached HEAD = PR #32 head). You own
`shared/jeiobs-workspace/tests/cases/`. Do NOT read `shared/jeiobs-workspace/src/` or `include/`. Read
`/tmp/jeiobs-s3/contract.md` section "r2" and the review replies in
`~/.local/share/jeiobs-work/reviews/pr32/01-run/`. Add cases for each r2 item (1-10), both sides, and update
expectations it changes: identity order independence (a renamed collection sorting before one that keeps its
path, both against the same parent), `themes` folders inside profiles/scenes vs config `themes/`, a symlinked
app dir and a symlinked `basic/scenes` (POSIX, `@posix`), an unreadable folder (POSIX: create it, then make it
unreadable with `file(CHMOD ...)` if your CMake has it, else skip and say so; skip when running as root), the
`count` field in manifests (valid on dir/names omitted entries, invalid elsewhere, kept by export), 100,001
scene UUIDs (generated), profiles without basic.ini, a >64 MiB collection (generated sparse if possible, else
skip and say so), remove the lone-`.bak` alternatives. Keep it portable (cmake -P only). Commit on the detached
HEAD (no branch, no push): `git -c user.name=auto44e -c user.email=auto44e@gmail.com commit -m "shared: cases for the planner review fixes (contract r2)"`.
Final reply: what changed, ambiguities, what you could not cover.
