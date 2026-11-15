# Current handoff

Observed: 2026-11-15T09:44:48-05:00. Repository: auto44e/jeiobs.
Setup branch: jeiobs/scaffold; base: OBS 33.0.1,
65c320b46e1af5c7a555161aea6182cdf7a8c524. The setup commit is this document's
containing commit (use `git log -1 -- docs/jeiobs/HANDOFF.md`).

## Implemented

- Public GitHub fork with upstream history/branches/tags retained.
- main created from stable 33.0.1 and selected as default; issues enabled.
- Scaffold: agent guide, agile contribution process, roadmap, templates, upstream
  pin, initial coexistence audit, quick-check script and unsigned manual build CI.
- Upstream publication/signing/entry-point jobs guarded in this branch.
- [Issue #1](https://github.com/auto44e/jeiobs/issues/1): first coding task, VST3.

## Evidence

Local Debian 13, Python 3.12.14, PyYAML 6.0.3; no CMake available.
Five VST3 cherry-picks with `-x` applied cleanly on the stable base in a separate
worktree; `git diff --check 33.0.1 HEAD` there passed. This did not change main.
The complete source commit sequence is in [task 001](tasks/001-vst3.md).
Final scaffold check and staged diff whitespace check passed. The first invocation
correctly reported the then-missing HANDOFF link. Do not report that first run as
passing. Workflow YAML parsed locally; remote Actions results are separate.

## Planned / unverified

Application identity isolation, build success, runtime/SDK compatibility and all
M0 coexistence tests remain unimplemented or unverified. This branch changes no
runtime C/C++ behavior. VST3 code is not included in the setup PR.
No rulesets, required checks, project board, signing or release infrastructure
have been configured. CI workflow definitions do not imply a successful run.
Existing upstream master/other branches retain upstream workflows; use main.

## Next bounded task

Execute [001-vst3](tasks/001-vst3.md) / issue #1. It may precede M0 completion,
but use disposable profiles/VMs. Before creating a feature PR, review dependencies
and SDK licenses, run builds and focused host tests, and record unresolved gaps.
In parallel planning, use [COEXISTENCE.md](COEXISTENCE.md) to split M0 into platform
identity tasks; do not claim installation safety from the renamed repository.

Rollback setup: close the PR before merge or revert its commit after merge.
Retain upstream history and COPYING. No user configuration migration was performed.
