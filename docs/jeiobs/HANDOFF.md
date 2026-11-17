# Current roadmap direction — 2026-11-15

Inspected main `dd2078b1ec8a0776b3148f661c94509da25df941`. Owner expanded implementation scope:
full S1-S6 saving/offline delivery on Windows/macOS/Linux, OBS-compatible native saves,
graceful audio recovery, PDC preserving A/V sync, sidechains, optional listen bus,
full adopted plugin host-spec coverage, mixed-media timing and optional recording
timecode. See [ROADMAP](ROADMAP.md) and [task 002](tasks/002-workspace-saving.md).
These changes document authorized work; they do not implement runtime features.
Current handoff below records VST3/Linux progress and native-platform gaps; those
results were not rerun here. Next: progress S1 into S2-S6 and shared audio/timing
contracts independently while completing VST3/native validation. Preserve small PRs.

## Historical handoff (retained verbatim)

# Current planning update — 2026-11-15

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
- Identity UI (2026-11-15, `jeiobs/identity-ui`): window title, `--version`,
  tray text and English dialog text say JeiOBS; the startup log adds a fork
  notice after the unchanged `OBS <version>` line; dead Patreon code removed.
  Log upload to obsproject.com and other upstream links await owner decisions in
  [task 004](tasks/004-upstream-services.md).
- No uploads (2026-11-15, `jeiobs/local-logs`): log and crash-report upload to
  obsproject.com removed; Help > Log Files saves a local copy to
  `Documents/JeiOBS/logs`. See [task 004](tasks/004-upstream-services.md).
- Help links (2026-11-15, `jeiobs/help-links`): repository and release notes
  point at the fork, Discord removed, help portal labelled upstream, What's New
  off by default.
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
  `tools/jeiobs/vst3-check/run.sh` passes 6/6 with two SDK sample effects. ADelay
  delays real audio by 1.078 s (expected 1.000). Inherited gaps: one crashing
  plugin hides all plugins, a hanging one has no timeout and blocks OBS exit, and
  AGain outputs silence. See [task 001](tasks/001-vst3.md#runtime-evidence-2026-11-15-branch-jeiobsvst3-runtime-check)
  and the proposed [task 003](tasks/003-vst3-scanner-robustness.md).
- Task 003 scanner robustness (2026-11-16, `jeiobs/vst3-scanner-robustness`):
  the scanner probes each code-loading bundle in its own child process with a
  watchdog, so a crashing or hanging plugin is skipped (and logged) instead of
  hiding all plugins; OBS kills a running scan on exit. New libobs exports
  `os_process_pipe_kill` and `os_process_pipe_wait`. `run.sh` grows to 16
  checks: 16/16 on the fix, and the new ones fail on the old code. Built in
  worker mode (Codex implementation, Claude tests, each cross-reviewed three
  times by the other lineage). Linux only;
  Windows/macOS not compiled. See [task 003](tasks/003-vst3-scanner-robustness.md).

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

Open small items (2026-11-16):
- Codex's shell sandbox works on the maintainer's host since 2026-11-16 (AppArmor
  profile for bubblewrap, see DECISIONS "Confinement history"). Earlier runs were
  unsandboxed. Read each run's `00-meta.md` for the state at that run.
- `aqtinstall` is pinned to 3.3.0 in `tools/jeiobs/linux-deps.sh`; its transitive dependencies are not
  hash-pinned (a full `--require-hashes` requirements file is a possible follow-up).
- Task 003 follow-up D6 is [task 005](tasks/005-vst3-skip-list.md) (remember
  failed VST3 bundles): done on Linux; Windows and macOS not compiled.
- Task 002 S1: audit written, format design in debate.

Execute [001-vst3](tasks/001-vst3.md) / issue #1. It may precede M0 completion,
but use disposable profiles/VMs. Before creating a feature PR, review dependencies
and SDK licenses, run builds and focused host tests, and record unresolved gaps.
In parallel planning, use [COEXISTENCE.md](COEXISTENCE.md) to split M0 into platform
identity tasks; do not claim installation safety from the renamed repository.

Rollback setup: close the PR before merge or revert its commit after merge.
Retain upstream history and COPYING. No user configuration migration was performed.
