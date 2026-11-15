# Development and CI

Use main for jeiobs development; master retains the original upstream snapshot.
Baseline is recorded in upstream.json. Start with a disposable build environment:

```sh
git clone --recurse-submodules https://github.com/auto44e/jeiobs.git
cd jeiobs
git switch main
python3 tools/jeiobs/check_scaffold.py
cmake --list-presets
```

Read CMakePresets.json and .github/scripts for the pinned upstream dependencies
and platform setup. Follow upstream build instructions linked from README.rst.
Do not install an unisolated build on a machine with valuable OBS configuration.
This scaffold does not provide a finished jeiobs installer or runnable isolation.

`jeiobs-checks.yaml` runs cheap repository checks on PRs/pushes. It is not an OBS
compile or audio test. `jeiobs-build.yaml` is an opt-in workflow_dispatch wrapper
around the upstream platform matrix, with signing/notarization/packaging forced
off for the fork and no inherited secrets. The upstream build scripts may still
produce build artifacts; they are not separately branded distributions.

Original build/format/analyze definitions are retained. Upstream entry-point,
publishing and signing jobs are guarded to run only in obsproject/obs-studio.
CI on inherited master/other upstream branches remains original: do not dispatch
those workflows or push release tags there. Use main and feature branches.
Re-audit workflow guards and unsigned overrides on every upstream merge.

Full builds are intentionally not mandatory for docs changes. For code PRs, run
the affected platform locally or dispatch the full matrix; include exact results.
GitHub Actions may require the owner to enable workflows on this new fork.
No required checks, rulesets, signing secrets or release credentials are configured
by this scaffold. CI policies here are guidance, not claimed GitHub enforcement.
