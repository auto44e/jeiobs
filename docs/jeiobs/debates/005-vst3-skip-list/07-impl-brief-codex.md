# Worker brief: JeiOBS task 005 (D6) — remember failed VST3 bundles (IMPLEMENTATION)

Work ONLY in `~/oss/jeiobs-wt/d6-impl` (detached HEAD). A configured build exists at
`_build/` (warnings are errors): `cmake --build _build -j8`. No network, no push, do not
reconfigure, do not touch any other directory. Do NOT edit `tools/jeiobs/vst3-check/*`
or `docs/` (another worker owns tests; the lead owns docs). Commit with
`git -c user.name=auto44e -c user.email=auto44e@gmail.com commit`.
Your shell sandbox may not work on this host; behave as if read-only outside the
worktree anyway.

Read the specification first: `/tmp/jeiobs-d6/005-vst3-skip-list.md` (agreed by
Claude and Codex in four debate rounds; implement it exactly; if something in it is
impossible or wrong, stop and explain instead of improvising). Then read the code:
`plugins/obs-vst3/vst3-scan/vst3-scanner.cpp` (coordinator and probe: `runProbe`,
`startProbeWatchdog`, `probeVst3Module`, `vst3CacheSave/Load`, `removeStaleProbeResults`,
`main`), `plugins/obs-vst3/VST3Scanner.{h,cpp}`, `plugins/obs-vst3/obs-vst3.cpp`
(`start_pipe`, `retrieveVst3List`, `getScanResult`, `vst3_rescan_callback`,
properties), `plugins/obs-vst3/data/locale/en-US.ini`, `libobs/util/crc32.h`,
`libobs/util/pipe*.c`.

Implementation notes:
- Keep Windows and macOS compiling (#ifdef carefully; you cannot build them here).
  macOS binary resolution: `CFBundleCreate` + `CFBundleCopyExecutableURL` (link
  CoreFoundation only if not already linked; check the CMake files).
- Lock: POSIX `open(O_RDWR|O_CREAT|O_CLOEXEC)` + `flock(LOCK_EX)` with EINTR retry;
  Windows `CreateFileW` (lpSecurityAttributes = NULL, so not inheritable) +
  `LockFileEx` (blocking). Never delete the lock file.
- Status markers: write via a temp file + rename; the watchdog uses its own temp name;
  include the nonce; unlink stale `.status` and temps before spawning; extend
  `isProbeResultFileName` so stale status files are cleaned up.
- `--retry-failed` is argv[2] of the coordinator; host passes it from the Rescan path.
- Print `VST3_SKIPPED <n>` on stdout before `VST3_SCAN_DONE`; the host parses it and
  shows a warning text property when n > 0 (add strings to en-US.ini).
- Keep the existing 16-check suite green:
  `tools/jeiobs/vst3-check/run.sh $PWD/_build ~/oss/jeiobs-deps` (~50 s).
- Self-check the core flows with throwaway HOMEs (crash bundle twice: second scan
  logs "remembered"; hang twice then skipped; --retry-failed re-probes) using the
  fixtures in tools/jeiobs/vst3-check/ (build them as run.sh does). Never use the real HOME.

When done: commit on the detached HEAD, then print what you changed, how each spec row
maps to code (file:function), what you verified, and Windows/macOS risks.

IMPORTANT git rules (another process checks this repository's refs and config):
commit only on the detached HEAD of your worktree; never create, delete or move
branches or tags; never run git fetch, gc, stash, submodule, config or worktree
commands. Print the final commit SHA.
