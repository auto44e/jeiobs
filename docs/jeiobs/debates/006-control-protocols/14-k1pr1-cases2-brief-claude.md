# Worker brief (Claude, cases round 2): JeiOBS K1-PR1

Your current directory is a JeiOBS worktree (detached HEAD = PR #33 head). You own
`shared/jeiobs-control/tests/cases/`. Do NOT read `shared/jeiobs-control/src/` or `include/`. Read
`/tmp/jeiobs-k1pr1/contract.md` section "r2" and the review replies in
`~/.local/share/jeiobs-work/reviews/pr33/01-run/`. Add cases for each r2 item (both sides) and update the
expectations r2 changes (item 4: `url`, `input`, `ffmpeg_options`, `playlist` are denied; item 3: hotkey class is
always output-control, so wildcards to hotkeys are refused and `compile` shows class output-control; item 6:
target_key escaping incl. `/`; item 7: non-ASCII requires). For item 1 use `jc-check value` (add a
`fromdouble` check if the contract's CLI cannot express it: say so instead of inventing a command). For item 5
generate a ~4 MiB object with ~500,000 keys inside a mapping (it must validate within the case timeout). Keep it
portable (cmake -P only). Commit on the detached HEAD (no branch, no push):
`git -c user.name=auto44e -c user.email=auto44e@gmail.com commit -m "shared: cases for the jeiobs-control review fixes (contract r2)"`.
Final reply: what changed, ambiguities, what you could not cover.
