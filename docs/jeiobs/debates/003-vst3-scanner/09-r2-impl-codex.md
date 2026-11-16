# Round 2 implementation worker — Codex (summary)

_Raw peer correspondence, task 003 (2026-11-15/16). Codex tool-call output trimmed; prompt and final answer kept verbatim. AI-assisted, not human review._

## Prompt

````text
# Worker brief, round 2: JeiOBS task 003 — VST3 scanner robustness (IMPLEMENTATION)

Same rules as before: work ONLY in the worktree
`~/oss/jeiobs-wt/scanner-impl` (branch
`wip/vst3-scanner-impl`), build with `cmake --build _build -j10` (warnings are
errors; do not reconfigure), no network, no push, do NOT edit
`tools/jeiobs/vst3-check/*` or `docs/`. Commit with
`git -c user.name=auto44e -c user.email=auto44e@gmail.com commit`.

The branch already contains the round-1 implementation (commit 1e0de02dd, by you in
an earlier session; you do not remember it, so read it: `git show 1e0de02dd` and
`git show 0e63cf295` (host now re-logs `Skipped` lines)), plus the test suite
(76d6528db). The full suite passes 15/15 on Linux:
`tools/jeiobs/vst3-check/run.sh $PWD/_build ~/oss/jeiobs-deps`
(takes ~45 s; it builds throwaway fixtures under /tmp and never uses the real HOME).
Run it at the end; it must stay 15/15. Read `plugins/obs-vst3/{obs-vst3.cpp,
VST3Scanner.h,VST3Scanner.cpp,vst3-scan/vst3-scanner.cpp}` and `libobs/util/pipe*.{h,c}`.

An adversarial review (Claude, read-only, by reading code and the pinned SDK)
found the issues below. I (the lead) have decided what to fix. Fix exactly these;
keep the diff focused; keep Windows/macOS compiling by careful #ifdefs (you cannot
compile them here: re-read every #ifdef branch you touch).

## Must fix
1. **Coordinator-side probe timeout (Windows blocker, also defence in depth).** In
   `probeVst3Module`, run a timer thread (condition variable + `wait_for`) that calls
   `os_process_pipe_kill(pipe)` when the probe outlives `timeout + 2000 ms`, and is
   cancelled/joined after the pipe is drained. Distinguish "killed by coordinator"
   in the skip reason ("probe timed out (killed)"). Make sure kill vs destroy cannot
   race (destroy only after the timer thread is joined).
2. **Watchdog start handshake.** In `--probe`, the main thread must not load the
   plugin until the watchdog thread has actually started running (it signals via a
   promise/condition variable/atomic+wait). Reason: on Windows a new thread needs the
   loader lock for DLL_THREAD_ATTACH, so a plugin hanging in DllMain could otherwise
   stop the watchdog from ever running.
3. **Windows handle inheritance.** In coordinator mode (normal scanner start), at
   startup clear `HANDLE_FLAG_INHERIT` on its own STD_INPUT/OUTPUT/ERROR handles, so
   probes (and helpers plugins start) never inherit OBS's pipes. Check in
   `libobs/util/pipe-windows.c` that the probe still receives its own fresh pipe
   handles (they are passed explicitly in STARTUPINFO and created inheritable there).
4. **Parent-death on Windows, and pass the parent PID explicitly everywhere.** Add the
   coordinator's PID as an extra `--probe` argument. POSIX: the watchdog exits when
   `getppid()` differs from that PID (this also closes the race where the coordinator
   died before the child read getppid). Windows: `OpenProcess(SYNCHRONIZE, FALSE, pid)`
   once and in each watchdog tick `WaitForSingleObject(h, 0) == WAIT_OBJECT_0` →
   hard-exit; if OpenProcess fails, exit immediately (parent already gone).
5. **Architecture filter must not drop single-file plugins.** In `getVST3Paths`
   (Windows), apply the architecture-folder check ONLY when
   `path.parent_path().parent_path().filename() == "Contents"` (bundle layout); a
   `.vst3` file anywhere else (single-file DLL, e.g. `VST3\Foo.vst3`,
   `VST3\Vendor\Foo.vst3`) is accepted as before. Accepted arch folders:
   x64 → `x86_64-win`; native ARM64 (`_M_ARM64` without `_M_ARM64EC`) → `arm64-win`
   only; ARM64EC (`_M_ARM64EC`) → `arm64ec-win`, `arm64x-win`, `x86_64-win`.
   Note MSVC defines `_M_ARM64` is NOT defined under ARM64EC but `_M_X64` IS — check
   `_M_ARM64EC` first.
6. **Distinctive watchdog exit code.** Use a code unlikely to collide with abort()/
   plugin exit(3), e.g. 86, as a named constant; map only that to "probe timed out".
7. **Do not report success if the cache could not be saved.** Print `VST3_SCAN_DONE`
   only if `vst3CacheSave()` succeeded (and the scan did not throw); otherwise keep
   the upstream behaviour ("Scan failed", exit failure).
8. **Cancellation re-check.** In host `getScanResult`, re-check the cancelled flag
   right before `obs_source_update_properties`.
9. **`hasVST3()` early exit.** Restore upstream's cheap behaviour: stop at the first
   matching bundle (same filtering rules as `getVST3Paths`), instead of walking and
   sorting the whole tree during `obs_module_load`.
10. **Stale probe result files.** At coordinator start, remove `probe-*.json` and
    `probe-*.json.tmp` files in the cache directory whose mtime is older than 1 hour
    (never delete fresh ones: two OBS instances may scan concurrently). Use
    std::filesystem with error_code overloads.
11. **Default probe timeout 30000 ms** (slow but valid plugins on a cold start, e.g.
    antivirus or licence checks); env override unchanged (clamp 500..120000).
12. Remove the unused `hasClasses` field (or make it meaningful); log a warning when
    directory iteration stops early on an error other than permission denied.

## Explicitly NOT in this round (recorded as follow-ups)
- Remembering failed bundles between launches (skip list keyed by path/mtime/size).
- Pre-existing upstream races: `vst3Update` reading `list` while a rescan deletes
  it; the rescan holding a strong source reference.

## Done
Build passes, suite 15/15, then commit ("obs-vst3: harden scanner probes for
Windows and review findings" or similar) and print: what changed per item (1-12),
anything you could not do and why, and remaining Windows/macOS risks.
````

## Answer

Implemented and committed as `be2bdff9e`.

1. Added coordinator probe timeout/kill at `timeout + 2000 ms`, with safe timer joining and `(killed)` reason.
2. Added watchdog startup handshake before plugin loading.
3. Cleared Windows standard-handle inheritance; preserved explicit fresh pipe handles.
4. Added parent PID argument and POSIX/Windows parent-death detection.
5. Fixed Windows architecture filtering, including single-file plugins and ARM64EC.
6. Added named watchdog exit code `86`; only it maps to probe timeout.
7. `VST3_SCAN_DONE` now requires successful cache save.
8. Added final cancellation re-check before property updates.
9. Restored early-exit `hasVST3()` discovery.
10. Added safe cleanup of probe results older than one hour.
11. Increased default timeout to 30 seconds; retained 500–120000 ms clamping.
12. Removed unused `hasClasses`; added non-permission directory-iteration warnings.

Verification:

- `cmake --build _build -j10` passed.
- Scanner suite passed 15/15.
- Worktree is clean.
- No docs or protected test-tool files changed.

Windows/macOS compilation and runtime validation remain untested in this Linux environment; platform-specific handle, loader-lock, and ARM64/ARM64EC behavior still need native validation.
