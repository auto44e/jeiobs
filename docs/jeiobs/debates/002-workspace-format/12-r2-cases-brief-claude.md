# Worker brief (Claude, black-box cases): S1-PR1 review run 01

Your current directory is a JeiOBS worktree (detached HEAD at 13cf59408). You wrote
`shared/jeiobs-workspace/tests/cases/` (see `docs/jeiobs/debates/002-workspace-format/08-pr1-tests-claude.md`).
Another worker is changing the library in parallel; do NOT read `shared/jeiobs-workspace/src/`
or `include/`. Read `docs/jeiobs/debates/002-workspace-format/09-pr1-contract-r2.md` (addendum
r2, wins over the original contract) and the review replies in `review-01/` of that folder.

Update ONLY `shared/jeiobs-workspace/tests/cases/`:
- make every existing fixture conform to r2 (stored entries must use an allowed (root, role,
  path) shape; opaque-unreviewed entries need `unreviewed`; omitted entries carry no fidelity,
  etc.) while keeping what each case was meant to test, and update expected outputs that r2
  changes (export objects rebuilt, derived fidelity, dropped_ext contents, ERR-only stdout);
- add cases for each r2 item, both sides of each rule: every allowlist row OK; wrong role, wrong
  path, path for another root, `<f>` with `/`, `machine-id`, `workspaces/x`; opaque without
  `unreviewed`, bad `unreviewed`, `unreviewed` on exact, `parse` misuse, omitted with each
  forbidden field; `--now` good and bad (bad month, 24:00:00, missing Z, offset form); decisions
  file faults (dup key, BOM, 1e400, > 4 MiB generated, directory → E_IO, unknown id, bad verb,
  fidelity mismatch); export rebuild (unknown in creator/active/compat/app_keys/target listed as
  dotted names; existing dropped_ext kept first); `--export` with target.machine_id and with a
  restore:"never" app key; `CON .txt`, `nul  .json`, `aux .`, and `CON ` still
  trailing-dot-space; revision over 4 MiB (generated) → E_LIMIT manifest, revision of a directory
  → E_IO; child whose carried fields push a limit → ERR, no JSON; every ERR case asserts stdout
  is exactly the ERR line (tighten the old `?` export cases to that).
- Keep it portable (Windows/macOS/Linux) and cmake -P only. Validate with a throwaway fake if
  useful (not committed).
Allowed commands: Read/Write/Edit/Glob/Grep, `cmake`, `b2sum`, `python3` (authoring only),
`git status/diff/add/-c … commit`. Commit on the detached HEAD (no branch, no push):
`git -c user.name=auto44e -c user.email=auto44e@gmail.com commit -m "shared: cases for S1-PR1 review fixes (contract r2)"`.
Final reply: what changed, the new case list, ambiguities and how you read them.
