# Worker brief (Claude, cases round 3): S1-PR2

Your current directory is a JeiOBS worktree (detached HEAD = the PR #28 head). You own
`shared/jeiobs-workspace/tests/cases/`. Do NOT read `shared/jeiobs-workspace/src/` or `include/`.
Work from `/tmp/jeiobs-s2/contract-r3.md` (section "r3.2"; it also holds r3 and r3.1) and the
replies in `/tmp/jeiobs-s2/review/02-run/`. Add cases for each r3.2 item, each with a marker secret that must not survive:
(1) a 1 MiB `[...]` section header followed by 10^5 `Token=S` lines (generated; the report stays small
and uses `/~section-0/Token`; opaque count +1), a 64 MiB-class collection with many URL values must finish
(generated, bounded by the runner timeout); (2) fail-closed: not directly testable via CLI without a
fault, so skip and say so; (3) `Token=S\n[Twitch] ;x\nRefreshToken=T\n` (a token before the first
header), `[]`, `[`, `[ ]` headers followed by Token lines in later sections; (4) `DesktopAudioDevice1`,
`AuxAudioDevice1` top-level objects and `groups[]` items holding a `browser_source`-like object with
a credentialed URL. Update any existing expected reports changed by these rules.
Keep it portable (cmake -P only). Commit on the detached HEAD (no branch, no push):
`git -c user.name=auto44e -c user.email=auto44e@gmail.com commit -m "shared: cases for S1-PR2 review run 02 (contract r3.2)"`.
Final reply: new cases, ambiguities, what you could not cover.
