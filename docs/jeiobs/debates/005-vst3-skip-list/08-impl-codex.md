Implemented and committed on detached HEAD.

Commit: `ed0fbe347a0d39961fe4a2bca766ec0a44649d98`

Changed:

- Skip-list storage, fingerprinting, pruning, retry policy:
  `vst3-scanner.cpp` — `loadSkipList`, `saveSkipList`, `getBinaryFingerprint`, `shouldSkipBundle`, `rememberProbeFailure`, `pruneMissingSkipEntries`.
- Probe status markers and outcome classification:
  `vst3-scanner.cpp` — `writeProbeStatus`, `startProbeWatchdog`, `runProbe`, `probeVst3Module`.
- Locking and atomic cache fallback:
  `vst3-scanner.cpp` — `ScanLock`, `vst3CacheSave`, `main`.
- `VST3_SKIPPED` output and `--retry-failed` host plumbing:
  `obs-vst3.cpp` — `start_pipe`, `getScanResult`, `retrieveVst3List`, `vst3_rescan_callback`, `vst3Properties`.
- UI strings/tooltips and macOS CoreFoundation linkage.

Verified:

- `cmake --build _build -j8` passed.
- `tools/jeiobs/vst3-check/run.sh`: 16/16 passed.
- Throwaway-home crash, hang/backoff, retry, fingerprint, and lock self-checks passed.
- Worktree clean; no `docs/` or `tools/` changes.

Risks: Windows/macOS were not natively built or run here. Platform code uses `CreateFileW`/`LockFileEx` on Windows and `CFBundleCopyExecutableURL` plus CoreFoundation linkage on macOS.
tokens used
