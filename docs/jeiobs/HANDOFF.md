# Current planning update — 2026-11-16

Implemented task 003 on branch `wip/vst3-scanner-impl`: VST3 discovery now
probes code-loading bundles in short-lived child processes with per-bundle
timeouts, crash/hang skipping, nonce-checked atomic results and deterministic
ordering. The host can kill the coordinator before joining during unload.
Linux verification passed the configured `cmake --build _build -j10`, a
throwaway crash/hang scan (three good classes retained, two skips, about two
seconds with a 2000 ms timeout), and a disposable OBS quit probe (exit 0,
zero leaks while a 10000 ms probe was active). Windows and macOS builds and
runtime checks remain untested.

---

Earlier planning update — 2026-11-15

Inspected main at `5e17d4f99b2c15756b2ff5bbeb2e4454b80feb8a`. Added the
[workspace saving/offline proposal](WORKSPACE_SAVING.md) and
[agent task 002](tasks/002-workspace-saving.md). Status: documented/planned only.
No runtime changes or tests. Begin with persistence inventory and synthetic fixtures;
VST3 task 001 retains first priority. This update does not refresh other feature status.
Earlier setup observations are retained below as historical evidence, not current-head tests.

---

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
- Fork notices (2026-11-15, branch jeiobs/fork-notices): [NOTICE.md](NOTICE.md),
  a JeiOBS-specific [SECURITY.md](../../SECURITY.md) using GitHub private
  vulnerability reporting (enabled on the repo), and "independent fork, not the
  original" banners on the inherited README.rst, CONTRIBUTING.md, COC.rst and
  docs/sphinx. COPYING, AUTHORS, COMMITMENT and source headers are untouched.
  Product name convention: "JeiOBS (based on OBS Studio)"; identifier `jeiobs`.
  The quick check now also guards these notices.
- Local Linux baseline build (2026-11-15, Ubuntu 24.04, main `ab6332dd5`):
  configures and builds 624/624, 0 errors, and passes a headless smoke run with an
  isolated HOME. Needs FFmpeg 8.0.3, MbedTLS 3.6.7 and Qt 6.11.1 from a private
  prefix (`tools/jeiobs/linux-deps.sh`); see [BUILDING.md](BUILDING.md). Browser/CEF
  is disabled. This unblocks the task 001 build/test evidence on Linux.
- Task 001 VST3 backport (2026-11-15, merged as PR #8 with a merge commit): five
  upstream commits cherry-picked with `-x`; builds ON (672/672) and OFF (624/624);
  `obs-vst3.so` loads; SDK 3.8.0 is MIT and fetched, not vendored. Runtime plugin
  tests are still open; see [task 001](tasks/001-vst3.md#progress-2026-11-15-branch-jeiobsvst3-upstream).
- Task 001 runtime check (2026-11-15, `jeiobs/vst3-runtime-check`):
  `tools/jeiobs/vst3-check/run.sh` passes 7/7 with two SDK sample effects. ADelay
  delays real audio by 1.078 s (expected 1.000). Inherited gaps: one crashing
  plugin hides all plugins, a hanging one has no timeout and blocks OBS exit, and
  AGain outputs silence. See [task 001](tasks/001-vst3.md#runtime-evidence-2026-11-15-branch-jeiobsvst3-runtime-check)
  and the proposed [task 003](tasks/003-vst3-scanner-robustness.md).

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
