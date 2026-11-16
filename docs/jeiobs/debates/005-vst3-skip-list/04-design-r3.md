# Peer debate — 2026-11-16T05:17:03Z
Topic: Settle the two open D6 disputes using the lead's compromise.
Rounds: 2   Roles: claude=implementor codex=adversarial-reviewer
Context file: /tmp/jeiobs-d6/r3-context.md


[R1 CLAUDE] **Point 1: agree, with three amendments. Point 2: agree on a lock held for the whole scan, but I reject the "wait 5 s, then scan read-only" fallback because the code makes it lose the scan result. Not CONVERGED yet.**

## 1. No time-based expiry: agree
- **Why I drop the 30 days.** The expiry was there for causes the fingerprint can't see (licence, runtimes, Gatekeeper). The `VST3_SKIPPED n` warning next to Rescan covers them: a stale skip shows up exactly where the user looks for the plugin. A 30-day expiry instead brings back a crash report every month that nobody asked for.
  - Condition: that warning now carries weight and can no longer be the first thing cut.
- **The version string is effectively a build id.** `OBS_VERSION` comes from `git describe --always --tags --dirty=-modified` (versionconfig.cmake:11); today it is `33.0.1-49-gac008df4c`, so it changes on every commit.
  - Tarball builds fall back to a fixed `0.0.1` (bootstrap.cmake:52), so they never retry on their own; Rescan only. Acceptable.
- **Amendment A: store the version as its own `host` field, not inside the fingerprint hash.**
  - Host changed, binary fingerprint unchanged: probe once. If the same failure repeats, keep `failures`.
  - Binary changed: reset `failures`.
  - Why: if the version is hashed in, a timeout entry drops back to strike 1, so a hanging plugin costs two ~37 s probes after every update instead of one. A separate field also lets the log say why the plugin was retried.
- **Amendment B (known limitation, accept it):** two jeiobs builds used in turn on one config dir re-probe every remembered failure on each switch. Not worth keeping a list of versions per entry.
- **Amendment C: pruning.**
  - New field `missing_since`, evaluated once at load, under the lock.
  - An `fs::exists` error counts as present.
  - Drop the entry when it is older than 30 days; write the file only if something changed.

## 2. Lock: agree to hold it across the scan; reject the read-only fallback
- **Why I concede the lock.** Without expiry, Codex's case of a stale failure written back after another scanner's success sticks until Rescan, and re-read plus merge can't prevent it.
- **Why read-only fails.** The host gets the plugin list only by reading the file: after `VST3_SCAN_DONE` it calls `vst3JSONLoad()` (obs-vst3.cpp:328).
  - `scanVst3List` returns `vst3CacheSave()` (vst3-scanner.cpp:757), and `main` exits 1 without printing `VST3_SCAN_DONE` when the save fails (843–846).
  - So scanner B probes the same unknown bundles as A (the duplicate crashes and hangs D6 exists to remove) and then throws its result away.
  - Its host shows A's last file, or on a first run `Failed to load VST3 list` and an empty list.
- **Replacement: block instead of falling back.**
  - Try a non-blocking lock first. If it is busy, log `Waiting for another VST3 scan` once and block (`flock(LOCK_EX)` with an EINTR retry; `LockFileEx` without `FAIL_IMMEDIATELY`).
  - B then scans against A's fresh cache and skip list: cache hits, no duplicate probes, a correct list.
  - The host can still cancel: `cancelVst3Scan` kills the scanner even while it waits (obs-vst3.cpp:410), so OBS exit is not delayed.
  - Each instance already runs one scanner at a time (Rescan cancels, joins and reaps first), so blocking can't deadlock an instance against itself.
  - Windows releases a dead process's locks "depending on available system resources", which is another reason not to cap the wait at 5 s.
- **Lock error, as opposed to busy** (ENOLCK, EOPNOTSUPP, open fails):
  - Warn and run this scan without D6: don't read or write the skip list.
  - Still write `vst3list.json` as today. Making that write depend on the lock would break the existing #16 cache on filesystems without locking.
- **The handle must not be inheritable; this is required, not tidiness.**
  - POSIX spawns probes with `posix_spawn` (pipe-posix.c:99), and libobs marks only its own pipes CLOEXEC (73–76). So open the lock with `O_CLOEXEC`.
  - Windows calls `CreateProcessW(…, bInheritHandles=true, …)` (pipe-windows.c:67). So use `CreateFileW` with `lpSecurityAttributes=NULL`.
  - Take the lock in coordinator mode only, after `os_mkdirs`. Never delete the lock file.
- **Code the lock lets us remove from my round-2 design:**
  - Load `vst3skip.json` once and write it on every change (still needed in case the scan is killed). No re-read before each write.
  - Use the default `tmp`/`bak` names; drop `tmp-<pid>` and its cleanup.
  - `vst3list.json` writes are serialized too. Today two scanners share `vst3list.json.tmp`.

## Tests
- Drop the 30-day expiry test.
- Edit `host` in the file:
  - a crash entry is probed once, then skipped on the third scan;
  - a timeout entry with `failures=2` stays backed off after one re-probe.
- `missing_since` at −31 days is pruned; at −1 day it is kept; a path that reappears clears it.
- Hold the lock with `flock "$cache/vst3scan.lock" sleep 3 &` (`/usr/bin/flock` is present): the scanner logs `Waiting`, takes at least 3 s, and both files are valid. Killing it while it waits leaves both files unchanged.
- Hang fixture: SIGKILL the scanner mid-probe and start another at once. There must be no `Waiting` line, which proves the orphaned probe doesn't hold the lock.

**Still open:** the read-only fallback (to be replaced by the blocking wait) and Amendment A (version as a separate field).
[R1 CODEX] Point 1: mostly agree.

I agree that the OBS version should be a separate `obs_version` field, not part of the binary fingerprint. A version change should force exactly one retry while preserving the existing failure/backoff count; hashing it would reset timeout strikes and make a hung bundle cost two 30-second probes after every update.

I also agree with 30-day pruning for genuinely absent paths, provided:

- `missing_since` is persisted atomically and only evaluated under the lock.
- “Not found” is distinguished from filesystem errors and temporary unmounted/inaccessible volumes.
- Pruning is based on `fs::exists(path)`, not merely absence from the current enumeration.
- A version mismatch is consumed only after an attributable probe attempt; setup failure, cancellation, or lock failure must not consume the retry.

Use `obs_version`, not ambiguous `host`. The fixed `0.0.1` tarball limitation is acceptable because `--retry-failed` remains available.

The stable binary fingerprint requirement still stands: stat → digest → stat, and do not record a `size:-1` fingerprint. An unresolved binary must not match every future unresolved bundle.

Point 2: agree with rejecting the read-only fallback.

A scanner that probes and then cannot save its result is functionally broken: it duplicates crashes/hangs and can fail entirely on a first scan with no list. Blocking until the lock is available is the correct behavior. The lock should cover skip/list load, probing, pruning, and all writes; it must be non-inheritable and never deleted. Cancellation while waiting must terminate the scanner promptly.

I dispute Claude’s lock-error fallback combined with dropping PID-unique temporary files. If lock acquisition fails and the scanner still writes `vst3list.json` “as today,” shared `vst3list.json.tmp` remains unsafe. Atomic rename prevents partial files, not temp-file collisions or stale last-writer-wins behavior. Either retain unique temp names for the lockless fallback or make the fallback read-only/fail the list write. This is a concrete inconsistency in the proposed design.

Also, `flock`’s EINTR retry must honor scanner cancellation; blindly retrying forever can defeat a signal-based cancel path.

Missed or underweighted issues:

1. Crash versus watchdog timeout is policy-critical. Claude calls PID-namespace misclassification “harmless,” but classifying a crash as a timeout introduces automatic seven-day retry, contradicting “no expiry for crash/load/invalid.” The watchdog must explicitly record that it initiated the kill. Ambiguous abnormal termination should conservatively be treated as non-timeout, with Rescan as the retry path.

2. Add a regression test for a previously valid bundle that later fails. The failed bundle must be removed from the newly written `vst3list.json`; otherwise the skip list and successful cache can disagree.

3. The orphan-probe lock test must start the second scanner while the orphaned child is still alive and assert acquisition latency, not merely absence of a `Waiting` log.

Conclusion: I agree with Point 1’s no-expiry policy, separate version field, and path pruning, subject to the state-machine constraints above. I agree with Point 2’s full-scan blocking lock and rejection of read-only fallback, but disagree with the proposed lock-error/temp-file handling. Not CONVERGED.
[R2 CLAUDE] **Point 1: converged except one sub-point. Point 2: converged. I accept Codex's temp-file fix, and its EINTR concern doesn't apply. Codex is right that crash vs. timeout classification is now a policy question, but its fix breaks D5, so I propose another one.**

## Point 1
- **Agree:**
  - Name the field `obs_version`.
  - Fingerprint with stat → crc → stat.
  - Write `missing_since` atomically and evaluate it under the lock.
  - Prune on `fs::exists`; an exists error counts as present.
- **Agree, and extend:** an outcome the bundle didn't cause never creates, changes or deletes an entry. That covers a scanner-side exit, a coordinator reject, `could not start`, a cancel and a lock error. So a pending `obs_version` retry survives all of them.
- **Agree on no `size:-1`, but not for Codex's reason.** Entries are keyed by bundle path, so an unresolved fingerprint can only match its own path. The real reason: if the SDK's own resolution fails (`getSOPath`, `CFBundleCopyExecutableURL`), no plugin code loads. The failure is cheap and leaves no crash report, so there is nothing worth remembering. The resolver must mirror the SDK exactly.
- **Dispute "tell unmounted volumes apart":** a path on an unmounted volume returns ENOENT, the same as a deleted one, so this can't be done without guessing. The 30-day threshold is the mitigation, and a wrong prune costs one probe.

## Point 2
- **Conceded:** when the lock can't be taken, D6 is off and `vst3CacheSave` writes through `tmp-<pid>`.
  - `removeStaleProbeResults` already walks the cache directory with a 1 h cutoff (vst3-scanner.cpp:154-185). It also deletes `vst3list.json.tmp-*`.
  - The locked path keeps plain `tmp`.
  - The shared `.tmp` is how #16 works today; this fixes it in the only path that stays unlocked.
- **EINTR doesn't apply.**
  - The scanner installs no signal handlers.
  - Cancel is `kill(pid, SIGKILL)` (obs-vst3.cpp:410 → pipe-posix.c:147), and TerminateProcess on Windows. A process blocked in `flock` just dies; EINTR only follows a caught signal.
  - The host has no deadline for the scanner, only `VST3_SCAN_DONE` or cancel, so a blocked scanner can't trip a host timeout.
  - Add a code comment: any signal handler added later must call `_exit`.

## Missed #1: crash vs. timeout
I agree it matters now. I reject "treat ambiguous exits as non-timeout":
- **Exit 86 is the normal hang path.** The child's watchdog exits 86 at T (vst3-scanner.cpp:329, 355-357). The coordinator only kills at T + max(5 s, T/4) (497-498), which in practice means frozen probes. If 86 counted as a crash, every ordinary hang would become a permanent skip, which breaks D5.
- **Fix (2 lines):** 86 counts as a timeout only if `now − spawnTime ≥ T` (spawnTime is set at :496).
  - The rule is exact, because the child's deadline starts after the spawn.
  - An 86 that arrives earlier is a signal death whose waitpid PID happened to be 86 (pipe-posix.c:217-222), so it's a crash.
- **Close the other collisions too (this changes my R2 exit codes):**
  - Keep exit 1 for every scanner-side failure: setup (404-420) and save (445), both unchanged.
  - Change only the load failure (425) to 87, and drop 85 and 88.
  - A probe child is never PID 1, so a crash can't masquerade as exit 1.
  - A crash whose PID is 87 lands on the same "remember" policy.
  - A plugin that calls `exit(1)` isn't remembered: one cheap probe per launch, no crash report.
- **Final mapping:**

| Outcome | Result |
|---|---|
| killed by the coordinator, or 86 with elapsed ≥ T | timeout |
| exit 1 | entry unchanged |
| exit 0 without a valid result, 87, any other status | remembered failure |

- The libobs bug that returns the PID as the status is a follow-up, not D6.

## Missed #2: valid, then fails
I accept a test, limited to what can actually happen.
- **Invariant already holds:** classes enter `pluginList` only after a successful probe (VST3Scanner.cpp:385; cached discardable modules go through vst3-scanner.cpp:711).
- **Codex's scenario mostly can't arise.** The #16 cache is keyed by path and doesn't detect changes (VST3Scanner.cpp:278-298). If a valid non-discardable bundle is overwritten in place with a crashing binary, nothing re-probes it: not D6, not Rescan. That's existing behavior; file it as a follow-up.
- **Test with existing fixtures:**
  1. adelay is listed at path P.
  2. P is removed for one scan.
  3. crash.so is put at P.
  4. Expect: P is absent from `vst3list.json`, the failure is remembered, and loads.log shows exactly one load over the next two scans.
- A discardable fixture would need a hand-written factory, which isn't worth it.

## Missed #3: orphaned probe test
My test was weak, but Codex's version is racy as well:
- An orphaned `hang` probe exits within 150 ms, because its watchdog polls getppid (:361-370).
- A frozen orphan gets SIGHUP and SIGCONT as soon as `timeout` exits. `scan_start` runs the scanner under coreutils `timeout`, which takes its own process group, so the stopped probe ends up in an orphaned process group.
- **Deterministic replacement:** in the existing frozen scenario (run.sh:313), while the coordinator holds the lock:
  - no link under `/proc/$FROZE/fd/` resolves to `vst3scan.lock`, which directly proves `O_CLOEXEC`;
  - `flock -n` fails during the scan and succeeds after `scan_wait`.

## Remaining disagreements
- (a) No guessing about unmounted volumes.
- (b) No change for EINTR.
- (c) The elapsed-time rule plus exit-1 mapping, instead of "ambiguous ⇒ non-timeout".
- (d) The valid-then-fails test as scoped above, with the cache gap filed as a follow-up.
- (e) The `/proc` fd check instead of measuring orphan latency.

If Codex accepts (a)–(e): CONVERGED.
[R2 CODEX] Point 1: agree with the lead’s compromise, but not Claude’s claimed convergence.

Agree:

- No time expiry for crash/load/invalid entries.
- `obs_version` participates in the fingerprint.
- `stat → CRC → stat`; unstable fingerprints are not persisted.
- `missing_since` is updated and pruning is evaluated under the lock.
- Scanner-side failure, cancellation, coordinator rejection, and lock failure must not mutate skip entries.

Disputes:

1. Claude’s exit-1 mapping is internally inconsistent. Setup/save failures use exit 1, while “exit 1 means unchanged.” A plugin calling `exit(1)` is therefore never remembered, despite being a crash. The parent needs a machine-readable outcome or distinct scanner-side status; raw exit 1 cannot represent both cases (`vst3-scanner.cpp:404–420, 445`).

2. “Killed by coordinator ⇒ timeout” contradicts the earlier invariant that cancellation never changes entries. Only a deadline-triggered kill is a timeout; explicit cancel, shutdown, or external termination must be non-mutating.

3. `86 + elapsed >= T` is a fragile substitute for wait-status classification. It still collides with a probe that exits 86 and with the documented waitpid-PID bug (`pipe-posix.c:217–222`). At minimum use a monotonic deadline and an explicit watchdog marker; do not rely solely on elapsed time.

4. I accept rejecting `size:-1` only if unresolved SDK binary resolution is explicitly classified as coordinator/setup failure. Otherwise malformed bundles retry forever and are excluded from D6’s remembered-invalid policy. Add a test for this classification.

Claude’s “valid-then-fails” test is insufficient: removing the path before replacing it does not exercise a failed reprobe against an existing successful list. It also does not prove stale module data is removed. Add that regression or explicitly document it as a D6 non-goal.

Point 2: agree with the lead’s locking contract.

The coordinator must hold the lock across the complete scan; after a 5-second acquisition deadline, the scan must be read-only, with neither JSON file written. Claude’s lock test should additionally assert unchanged file contents/mtimes and absence of temp writes. `/proc` is useful on Linux but does not verify Windows handle inheritance.

I dispute “EINTR doesn’t apply.” The lock helper should retry `flock`/polling on `EINTR`; relying on today’s signal-handler inventory is brittle. The proposed “future handlers must call `_exit`” comment is not a sound synchronization contract.

Remaining disagreements:

- Exit-status/outcome taxonomy.
- Cancellation versus timeout classification.
- Adequacy of the valid-then-fails test.
- EINTR handling.
- Explicit treatment and testing of unresolved binary resolution.

Therefore: **not CONVERGED**.
