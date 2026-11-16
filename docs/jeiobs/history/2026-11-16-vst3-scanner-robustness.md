# VST3 scanner robustness — 2026-11-16

Branch: `wip/vst3-scanner-impl`

Implemented the task 003 scanner coordinator design:

- Added `os_process_pipe_kill` for POSIX and Windows.
- Moved scanner code-loading behind sequential self-probes.
- Added probe watchdogs, POSIX parent-death detection, output hygiene,
  nonce-checked atomic result files and bounded child-pipe draining.
- Preserved usable classes when another bundle crashes or times out.
- Made module-info parsing distinguish “parsed” from “has audio classes” and
  made valid empty caches usable.
- Made discovery permission-tolerant, architecture-filtered on Windows and
  deterministic.
- Added host pipe locking and unload cancellation before joining the scan
  thread.

Evidence:

- `cmake --build _build -j10`: passed.
- Isolated Linux scanner with `adelay`, `again-sample-accurate`, crash and
  hang bundles, `OBS_VST3_PROBE_TIMEOUT_MS=2000`: exit 0 in about 2 seconds,
  three classes in the cache, one crash skip and one timeout skip.
- Disposable OBS quit probe with the same bad bundles and a 10000 ms probe
  timeout: exit 0 in about 5 seconds, zero reported memory leaks.

Windows/macOS compilation and runtime behavior were not tested. The Windows
handle-inheritance and architecture-folder branches, macOS process spawning,
and platform-specific VST3 loader behavior remain release risks.
