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

## Local Linux build (Ubuntu 24.04, verified 2026-11-15)

Observed on Ubuntu 24.04, GCC 13.3, CMake 3.28.3, Ninja 1.11.1, x86_64, at main
`ab6332dd5`. Result: `cmake --build` 624/624 steps, 0 errors (16 `-Wswitch` and
12 deprecation warnings, all inherited). Debug build, browser disabled.

1. System packages. Upstream's list is in `.github/scripts/utils.zsh/setup_ubuntu`;
   `dh-cmake` (packaging), `libatk*` (browser/CEF) and `libfdk-aac-dev` (non-free,
   optional) are not needed for this build.
2. Ubuntu 24.04 ships three dependencies too old for this base (FFmpeg 6.1,
   MbedTLS 2.28, Qt 6.4: they fail configure or compile). Build them into a private
   prefix, no sudo:

   ```sh
   tools/jeiobs/linux-deps.sh            # -> ../jeiobs-deps (checksum-verified)
   ```

   This builds FFmpeg 8.0.3 (LGPL; signature checked when first fetched) and
   MbedTLS 3.6.7 from source and installs Qt 6.11.1, the Qt that obs-deps
   `2026-10-23` (the pin in `CMakePresets.json`) is built from. Qt 6.8.3 was also
   tried and fails to compile `frontend/plugin-manager` (`QString::arg(std::string_view)`).
3. The browser plugin needs CEF and is not set up here. `obs-websocket` is a
   required submodule; initialise only what you need:

   ```sh
   git submodule update --init --depth 1 plugins/obs-browser plugins/obs-websocket
   ```

4. Configure and build out of tree:

   ```sh
   D=$PWD/../jeiobs-deps; FF=$D/ffmpeg-8.0.3; MB=$D/mbedtls-3.6.7; Q=$D/Qt/6.11.1/gcc_64
   PKG_CONFIG_PATH=$FF/lib/pkgconfig:$MB/lib/pkgconfig \
   cmake --preset ubuntu -B ../jeiobs-build/ubuntu \
     "-DCMAKE_PREFIX_PATH=$Q;$FF;$MB" -DQt6_DIR=$Q/lib/cmake/Qt6 \
     "-DCMAKE_BUILD_RPATH=$Q/lib;$FF/lib;$MB/lib" -DENABLE_BROWSER=OFF
   cmake --build ../jeiobs-build/ubuntu -j"$(nproc)"
   ```

5. Run only with an isolated home. The binary still uses OBS's config paths
   (`~/.config/obs-studio`) until M0 lands, so never point it at a real profile:

   ```sh
   S=$(mktemp -d); cd ../jeiobs-build/ubuntu/rundir/Debug/bin
   LD_LIBRARY_PATH=$FF/lib:$MB/lib:$Q/lib HOME=$S XDG_CONFIG_HOME=$S/.config \
     ./obs --portable --disable-updater
   ```

   `LD_LIBRARY_PATH` is needed because `libswresample` is not on the build RPATH.
   Add `QT_QPA_PLATFORM=offscreen` for a headless smoke test.

   **VST3 host** (task 001): `linux-deps.sh` also fetches the Steinberg VST SDK
   3.8.0 (MIT) at upstream's pinned commit. Add
   `-DSTEINBERG_VST3_PATH=$D/vst3` to the configure command; the directory must
   *contain* `vst3sdk/`. `-DENABLE_VST3=OFF` builds without it.

   Runtime check for an `ENABLE_VST3=ON` build:
   `tools/jeiobs/vst3-check/run.sh ~/oss/jeiobs-build/vst3 $D`. It builds two
   Steinberg sample effects and some broken test bundles, then runs the scanner
   and OBS with a throwaway HOME. It exits 0 only if every check passes. Results
   and known gaps are in [task 001](tasks/001-vst3.md).

**Smoke-test evidence (headless, isolated HOME):** `obs --version` reported
`OBS Studio - 33.0.1-6-gab6332dd5`. A 25 s offscreen launch loaded 33 modules,
ran the graphics thread at 33.3 ms median and shut down with 0 memory leaks;
`~/.config/obs-studio` was not created. Errors in that run came from the headless
environment, not the build: no VAAPI/NVENC hardware, no GL swapchain under the
offscreen platform, and a first-run scene-file rename in an empty profile.
Not tested: on-screen UI, real capture, encoding, streaming, recording, audio
devices. The version string still says "OBS Studio" (M0).
