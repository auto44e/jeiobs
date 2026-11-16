# Task 005: remember failed VST3 bundles (task 003 D6)

Status: **Done on Linux**. Follow-up to
[task 003](003-vst3-scanner-robustness.md#decisions) decision D6. Windows and
macOS are written for but **not compiled or run**.

Authorship: Codex (worker, implementation `obs-vst3: remember failed bundles`),
Claude (worker, `tools/jeiobs/vst3-check/skip-list.sh` and `skiplist.py`,
written from this spec without the implementation), lead (integration fix for
the lock-less temp file, clang-format on the changed lines, this record).
Review: see the PR's review run under `debates/005-vst3-skip-list/`.
Peer work is AI-assisted, not human review.

## Problem

Since #16 a bundle that crashes or hangs during scanning is skipped and logged,
but it is probed again on every launch: a hanging plugin costs up to ~37 s of
background scanning each time (30 s timeout plus kill margin), and a crashing one
produces an OS crash report each time.

## Design

Agreed in four design rounds between Claude and Codex
([debates/005-vst3-skip-list](../debates/005-vst3-skip-list/)). Where they differed,
the lead took the stricter rule of each; both accepted the principle that missing
information never escalates to a stickier outcome.

### Storage
- A separate `vst3skip.json` next to `vst3list.json` in the module config
  directory. `vst3list.json` keeps exactly its version-1 shape, so upstream
  obs-vst3 can still read it.
- `{"version":1,"bundles":[{path, binary, size, mtime_ticks, crc32, obs_version,
  kind, reason, failures, first_failed, last_failed, retry_after, missing_since}]}`.
  Unknown versions or malformed entries are ignored (the worst case is one extra
  probe).

### Fingerprint
- The binary the SDK would load: Windows, the enumerated `.vst3` file; Linux,
  `Contents/<arch>-linux/<stem>.so`; macOS, the bundle executable from
  `CFBundleCopyExecutableURL` (no code load).
- `size`, `mtime_ticks` (opaque) and CRC32 over the bytes (libobs `calc_crc32`,
  64 KiB chunks), captured stat → hash → stat. If the two stats differ, nothing is
  recorded (an installer may be writing).
- No stable fingerprint (resolution fails, file missing or unreadable, unstable
  stat) means the outcome is never remembered. The bundle is still probed.
- `obs_version` is a separate field, not part of the hash.

### Outcomes (explicit status markers, not exit codes)
The probe child writes `<result>.status` (with the nonce) through its own temp
file: `setup` immediately before `Module::create`; `load-failed` when
`Module::create` fails; `save-failed` when the result cannot be saved. The
watchdog writes `timeout` (separate temp file) only on its deadline branch, and
only after `setup` was written; never on parent death. Exit codes are
diagnostics only.

| Observation | Entry |
| --- | --- |
| valid result with matching nonce (zero audio classes included) | removed |
| `timeout` marker, or a deadline kill by the coordinator after `setup` | timeout strike |
| `load-failed`; or `setup` and the process died or exited without a result | remembered failure |
| `save-failed`, nonce mismatch, unparsable result, no marker, unknown marker, `setup` never written, scanner-side reject, cancel, lock failure | unchanged |

### Policy
- Remembered failures are skipped while the fingerprint matches. No time-based
  expiry. They are retried when the binary fingerprint changes, when
  `obs_version` changes (once; `failures` kept), or on Rescan
  (`--retry-failed`).
- Timeouts: the first strike is retried on the next launch; the second backs
  off for 7 days (`retry_after`). A version change overrides an active backoff
  once. Rescan retries.
- A binary change resets `failures`; a version-only change keeps it.
- Entries whose path has not existed for 30 days are pruned (`missing_since`,
  evaluated under the lock; an `exists` error counts as present).
- Skips are logged once per bundle per scan as
  `[VST3 Scanner] Skipped <path>: remembered <reason> (Rescan to retry)`. The
  scanner prints `VST3_SKIPPED <n>` before `VST3_SCAN_DONE`, where n counts every
  bundle skipped in that scan (fresh failures and remembered ones); the filter
  properties show a warning next to Rescan when n > 0.

### Concurrency
- One scanner per cache directory: an exclusive advisory lock on
  `vst3scan.lock` (POSIX `flock`, Windows `LockFileEx`), opened close-on-exec /
  non-inheritable, held from load through the last write, retried on EINTR.
  When busy, block (a read-only scan would lose its result: the host only reads
  `vst3list.json` after `VST3_SCAN_DONE`). Host cancel kills a blocked scanner.
- Lock unavailable (ENOLCK and similar): D6 is off for that scan and
  `vst3list.json` is written through a `vst3list.json.tmp-<pid>` temp file
  (stale ones, older than 1 h, are removed like stale probe files).
- `vst3skip.json` is written on every change (a scan can be killed), atomically.

### Host
- `start_pipe(path, retryFailed)`; Rescan passes `--retry-failed` as `argv[2]`.
- New strings: the skipped-plugins warning and the Rescan tooltip.

### Non-goals
- A cached, non-discardable bundle replaced in place by a bad binary is not
  re-probed: the #16 cache is keyed by path without change detection. Follow-up:
  reuse this fingerprint to invalidate cache entries.
- Crash vs. timeout classification depends on markers; a libobs bug returns the
  waitpid PID as the status for signal deaths (follow-up).

## Tests (`run.sh` section "skip list", 2 s probe timeout)
1. Crash: the second scan logs "remembered"; a marker file proves the fixture
   was not loaded again.
2. Hang: probed twice, skipped on the third scan; `retry_after` in the past →
   probed again; `obs_version` changed → probed once, `failures` kept.
3. `--retry-failed` probes a remembered crash again.
4. Fingerprint: flip one byte and `touch -r` so size and mtime match → probed
   again.
5. A shim fixture that forwards to ADelay and crashes when an env var is set:
   remembered, then listed after `--retry-failed` without the variable.
6. Scanner-side setup failure (`ulimit -u`, non-root): nothing recorded.
7. Unresolved binary (no arch directory; arch directory without the `.so`):
   probed, never remembered.
8. Corrupt `vst3skip.json` with and without `.bak`; stale temp files removed.
9. Lock: hold it with `flock`, the scanner waits and both files stay valid;
   cancel while blocked leaves both files and temp files unchanged; a killed
   scanner's orphaned probe does not hold the lock (`/proc` fd check).
10. `vst3list.json` still has only `version` and `plugins`; stdout has
    `VST3_SKIPPED <n>`.
11. Scoped valid-then-fails: ADelay at P, P removed for a scan, crash.so at P →
    P absent from `vst3list.json`, failure remembered, one load over two scans.

The checks were written from this spec by a different lineage than the
implementation. Where the spec was silent the test author fixed these readings,
and the implementation matches them: `vst3skip.json` and `vst3scan.lock` sit
beside the list given as `argv[1]`; `VST3_SKIPPED` is printed on every finished
scan, also when 0; a crash after Rescan raises `failures` to 2; a torn
`vst3skip.json` with a valid `.bak` uses the `.bak`; stale-file cleanup runs
under the lock, so a blocked scanner changes nothing.

## Evidence (Linux, Ubuntu 24.04, `ENABLE_VST3=ON`)

| Build | Result |
| --- | --- |
| this branch | **32 PASS, 0 FAIL** (`tools/jeiobs/vst3-check/run.sh`): the 16 task-003 checks and 16 skip-list checks. |
| implementation before the lead's fix | 31 PASS; #8 "stale temp files" FAILED: the lock-less fallback wrote `<dir>/tmp-<pid>` and never removed a stale one. Fixed: `<list>.tmp-<pid>`, removed after 1 h. |
| unfixed (main, with the new checks) | the 16 task-003 checks PASS, **all 16 skip-list checks FAIL** (no `vst3skip.json` is written). |

As root, #6 prints `SKIP` (the process limit does not apply to root) without
changing the exit status.

## Not done / follow-ups

- **Windows and macOS were not compiled or run.** The platform code uses
  `CreateFileW`/`LockFileEx` (Windows) and `CFBundleCopyExecutableURL` with
  CoreFoundation linkage (macOS).
- Not covered by a check: the 30-day `missing_since` pruning, the lock-less
  fallback (ENOLCK), unknown `vst3skip.json` versions and malformed entries, and
  the host side (Rescan passing `--retry-failed`, the warning text).
- The non-goals above.
