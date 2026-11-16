# Peer debate — 2026-11-16T04:53:11Z
Topic: Design D6 (remember failed VST3 bundles) per the brief. Round 1: propose/critique. Round 2: respond to the other's points and converge on one concrete design; list any point you still disagree on.
Rounds: 2   Roles: claude=implementor codex=adversarial-reviewer
Context file: /tmp/jeiobs-d6/brief.md


[R1 CLAUDE] **Position:** the strawman's shape is right. I'd change six things: (a) put the whole feature in the probe hook and leave `VST3Scanner.{h,cpp}` alone; (b) a timeout needs two strikes, and the second probe runs at 2× the timeout; (c) never remember probe setup failures; (d) write the file on every change, not at scan end; (e) make the fingerprint stricter; (f) add a UI hint, because D6 makes failures stick.

**1. Storage.** Use a separate `<cachedir>/vst3skip.json`.
- A per-entry flag inside `plugins` breaks upstream: its `vst3JSONLoad` lists every entry that has a path.
- An extra top-level key gets dropped silently when upstream rewrites the file, and it puts the skip data in the same corruption domain as the list.

```json
{"version":1,"bundles":[{"path":"…/hang.vst3","fingerprint":"v1-5a0c2f9e1b7d3a44",
 "kind":"timeout","strikes":2,"reason":"probe timed out","last_failed":1763251200}]}
```
If the version is unknown or an entry is malformed, ignore it. Worst case is today's behavior: one extra probe. Drop entries whose path no longer exists when writing.

**2. Fingerprint.** FNV-1a-64 over:
- `obs_get_version_string()`, so no remembered failure survives an OBS update;
- `fs::canonical(path)`, which catches installs that swap a symlink;
- for each file, sorted by relative name: the name, `file_size`, the `last_write_time` ticks, and on POSIX also `st_ino` and `st_ctime`.

Use only POSIX names. Avoid `st_mtim` vs `st_mtimespec`, so macOS needs no `#ifdef`.

Which files: on Windows, the DLL that is probed (the hook path is always a file there). On Linux/macOS, regular files at depth ≤ 2 under `Contents/`, excluding `Resources/`, capped at 64 after sorting.

The bias is deliberate. A false miss costs one probe; a false match hides a plugin. ctime and inode catch two cases a user expects to fix the plugin:
- a reinstall of the same bytes;
- `xattr -d com.apple.quarantine`.

The fingerprint can't see dependencies outside the bundle (licence daemon, VC++ runtime, Wine prefix). Expiry and Rescan cover those.

**3. Policy.** `probeVst3Module` gains a timeout parameter and an outcome out-parameter. The hook decides.

| outcome | remember | skip while |
|---|---|---|
| `probe failed`, `no result`, `invalid`/`nonce`/`no class list`/`empty id` | first time | fingerprint matches, < 30 d |
| `timed out`, `timed out (killed)` | strike 1, then 2 | strikes = 2, matches, < 7 d |
| coordinator-side rejects (466–541), new probe exit 87 | never | — |

- **Crash vs. load failure:** these can't be told apart. On a signal death, POSIX `os_process_pipe_destroy` returns waitpid's PID, so both are one kind.
- **Strike 1 is not skipped.** The next probe uses min(2×base, 120 s). Another timeout makes it strike 2; a success removes the entry and the bundle is cached for good. This answers D5: a valid plugin that needs 31–60 s gets listed on launch 2 instead of sticking. The 2× applies only to that confirmation probe; Rescan and expiry retries use the base timeout.
- **Clock:** if the clock went back more than a day, the entry is due for a retry.
- **Exit 87:** `runProbe`'s failures before loading (lines 404–420) and the result-save failure (445, e.g. disk full) currently exit 1, the same code as "plugin won't load". A failed Windows `OpenProcess` in the watchdog would then mark *every* code-loading plugin as broken.

**4. Rescan and UI.**
- Scanner command: `obs-vst3-scanner <list> [--retry-failed]`.
- Host: `start_pipe(path, retryFailed)`; `vst3_rescan_callback` calls `startVst3Scan(src, true)`. `retrieveVst3List(src)` keeps its signature (plugin-main.cpp has an extern for it). The host never deletes the file, so it doesn't need to know the format.
- The skip list applies even when there is no `vst3list.json`. Otherwise a scan that is always killed before it saves never converges.
- Log, WARNING, once per scan: `Skipped <path>: remembered (probe timed out on 2026-11-16); Rescan VST3 list to retry`.
- UI: the host counts the `Skipped ` lines it forwards. If there are any, the properties show an info line next to Rescan: `VST3.Skipped="Some VST3 plugins are not listed because they crashed, hung or failed to load (see the log). Rescan VST3 list tries them again."` This is about 15 lines and the first thing to cut if needed.

**5. Concurrency and durability.**
- Every change re-reads the file, merges that one path and writes, immediately. `cancelVst3Scan` killing the scanner mid-scan is the normal case, and writing at scan end would lose exactly the hang strikes.
- Save with `obs_data_save_json_safe(…, "tmp-<pid>", "bak")` so two instances never share a temp file. Load with `_safe(…, "bak")`, which covers the gap between the two renames in `os_safe_replace`.
- `removeStaleProbeResults` also removes `vst3skip.json.tmp-*` files older than 1 h.
- A lost update between two instances costs one extra probe. A failed write is a warning; it never fails the scan.

**6. Changes.**
- `vst3-scanner.cpp`: the skip list (load/save/update), `bundleFingerprint`, the probe outcome, `scanVst3List(bool)`, argv parsing, exit 87.
- `obs-vst3.cpp` and `en-US.ini`: host argument, counter, info line.
- `VST3Scanner`: no change. `addModuleClasses` → `moduleProbe_` is already the only path that loads code, including the cached discardable modules (vst3-scanner.cpp:711).

**7. Tests.** Each fixture appends its name to `$HOME/loads.log` (built with `-DFIXTURE`), so "not loaded" is asserted, not inferred.
1. crash, scan ×2: loaded once; the second line says `remembered`.
2. hang, scan ×3: about 2 s, 4 s, then < 1 s; loaded twice.
3. New `slow` shim (sleeps 3 s, then forwards `ModuleEntry`/`GetPluginFactory` to adelay.so): timeout, then listed with the entry removed, then loaded from cache.
4. `touch` crash.so → probed again. Then copy adelay.so over it → listed, entry removed.
5. `--retry-failed` → a remembered crash is probed again.
6. Garbage `vst3skip.json` → exit 0, crash probed, file rewritten valid.
7. `last_failed` set back 31 d (failed) or 8 d (timeout) → probed again.
8. `vst3list.json` keeps exactly upstream's shape after tests 1–4.
9. Frozen probe → a timeout strike. hhang → skip file absent or valid.
10. Host: quit-probe clicks Rescan through `obs_property_button_clicked` → the second scan logs a fresh crash line.

**Open for debate:** 2× confirmation vs. plain two strikes at the base timeout; whether the UI hint is in scope; exit 87.
[R1 CODEX] Claude’s storage direction is right, but several implementation details are unsafe.

I agree with:

- A separate `vst3skip.json`; modifying `vst3list.json` risks upstream rewrite/drop behavior.
- Keeping skip decisions in `vst3-scanner.cpp`, where enumeration and `probeVst3Module` already meet. `VST3Scanner` need not change if `addModuleClasses` remains the only code-loading path.
- `--retry-failed`, applying skips even when no list exists, malformed-file fallback, and atomic writes.
- Persisting timeout strike 1 before the scanner can be killed. Writing only at scan end loses the exact failure D6 is meant to remember.

I dispute these points:

1. Fingerprint is still too weak and too broad.

FNV over metadata does not solve the explicitly identified “same size/mtime” case. `st_ino` and `st_ctime` are platform-dependent metadata, and ctime can change for irrelevant operations such as xattr updates. The bounded walk also creates false matches: excluding `Resources/` or capping at 64 files can omit a load-critical change.

Fingerprint the exact executable passed to the probe, not an approximate bundle walk:

```text
normalized bundle path
relative executable path
file size
last_write_time ticks
streaming digest of executable bytes
```

A digest is only computed when comparing an existing skip entry, so healthy plugins pay no hashing cost. A touch causes a safe false miss; same-size/same-mtime replacement is detected. This is more portable than POSIX inode/ctime logic and naturally handles Windows single-file DLLs and Linux/macOS architecture binaries.

2. The 2× timeout conflicts with D5’s explicit 30-second default.

A second launch taking 60 seconds can exceed the existing worst-case budget and makes the test behavior depend on undocumented timeout escalation. Use the base timeout for every probe. On the first timeout, persist a strike but retry on the next launch; after the second, skip for seven days and still retry on expiry or Rescan. That satisfies “must not stick forever” without silently changing the watchdog contract. If D5 permits an exceptional 60-second confirmation probe, this needs to be stated explicitly.

3. “Exit 87” is not enough as an outcome design.

At `vst3-scanner.cpp` around the probe setup/result-save paths, parent setup failure and child/plugin failure must be separated. A missing result can mean crash, invalid plugin, or result-save failure. Define an explicit outcome taxonomy:

- child timeout/killed: remember;
- child signal/abnormal exit: remember as crash;
- child-reported load/invalid-result failure: remember;
- parent process/pipe/watchdog setup failure or result persistence failure: never remember.

An exit code can implement the child-reported cases, but it must not be inferred from “no result.” The existing probe protocol needs a machine-readable status before result serialization fails.

4. Concurrency requires locking, not merely unique temp names.

Unique `tmp-<pid>` files prevent collisions, but two OBS instances can still read the same old skip file and overwrite each other. That can resurrect a successful entry, causing indefinite stale skips until expiry. Use a short advisory lock around load/merge/save, with POSIX/Windows implementations behind `#ifdef`; retain atomic replacement and backup recovery.

5. The proposed UI accounting is unsupported.

`vst3_rescan_callback` calls `freeVst3List()` and `retrieveVst3List(source)`; nothing here establishes that scanner log lines are forwarded to the host. Do not count `"Skipped "` log lines. Either show static explanatory text beside Rescan, or have the host read `vst3skip.json` for a count. The static text is smaller and safer.

The final design should also retain `first_failed` (Claude omitted it), deduplicate skip logs per scan, preserve entries for temporarily inaccessible paths, and add tests for same-size/same-mtime replacement, concurrent writers, killed-mid-save recovery, and parent-side setup failure.
[R2 CLAUDE] I accept points 1–2 (digest; base timeout). On points 3–5 several of Codex's facts are wrong, and I dispute the lock. Codex also missed four things that change the design.

**Agreed:** the timeout is always the base 30 s, and I drop my 2× confirmation probe. Timeout strikes are consecutive: strike 1 retries on the next launch, strike 2 skips for 7 days. Keep `first_failed`, dedupe skip logs, keep entries for paths that are temporarily missing. I drop ino/ctime (Linux `st_ctim`, macOS `st_ctimespec` and NTFS ChangeTime would be three untested code paths). Codex's digest wins.

**Point 1, fingerprint (refined):**
- On Linux/macOS there is no "executable passed to the probe": the probe gets the bundle directory. The coordinator has to find the binary the same way the SDK does:
  - Windows: the enumerated path itself (`VST3Scanner` lists the DLL, the SDK calls `loadAsDll`).
  - Linux: `Contents/<uname -m>-linux/<stem>.so` (`module_linux.cpp getSOPath`).
  - macOS: `CFBundleCopyExecutableURL` (`CFBundleCreate` loads no code).
- Digest: libobs `calc_crc32`, already exported, read in 64 KiB chunks. No new hash code. A missing binary is recorded as `size:-1`.
- Race with an installer (missed): stat before spawning the probe; after a failure, hash and stat again. If size or mtime moved, record nothing.
- The digest covers only the binary. Licence state, system libraries, Gatekeeper approval and `Frameworks/` change no fingerprint. So the fingerprint only triggers automatic retries; Rescan and expiry are what make it correct.
- Therefore I add a 30-day expiry for entries that are not timeouts. Cost: one crash report per broken plugin per month, instead of a false match that sticks forever.

**Point 3, outcome taxonomy:**
- "No result" cannot mean a crash or a save failure in this code. A crash gives a non-zero status, a save failure exits 1 (`vst3-scanner.cpp:445`), and exit 0 happens only after a successful save (448). So "no result" means the plugin called `exit(0)`: a plugin failure.
- The real problem is that every child exit returns 1: setup at 404/409/416/420, load at 425, save at 445. Split them:
  - **85** setup failure, **88** save failure: never remembered. Neither is any coordinator-side reject before spawn.
  - **87** load failure, plus exit 1, other non-zero, no result, invalid result, empty class ID: remembered as plugin failures.
  - **86** or killed: timeout.
- All pre-load exits are ours, so no extra status file is needed.
- Don't promise a "crash" label. POSIX `os_process_pipe_destroy` returns waitpid's PID for a child killed by a signal (`pipe-posix.c:217-222`). Inside a PID namespace (Flatpak, the cleanroom container), a crashed probe with PID 86 is already reported as "probe timed out". It is harmless under this policy; log a libobs fix as a follow-up. Store the existing reject string as `reason`.

**Point 4, lock (disputed):** an entry is resurrected only when a writer writes back state it didn't decide.
- Fix: after each probe outcome, re-read the file, set or erase only that one path, save with temp extension `tmp-<pid>` and `bak`.
- That narrows the race from the length of a scan to milliseconds. Losing it leaves one stale entry until expiry or Rescan.
- A lock adds Windows/macOS code nobody compiles, NFS/SMB locking semantics and a "lock busy" path, all for a few milliseconds of protection. If the lead wants one anyway: non-blocking `flock`/`LockFileEx`, at most 1 s, never held across a probe, and write without it if it fails.

**Point 5, UI:** the log lines are forwarded. Scanner `blog(LOG_WARNING)` goes to stdout as `warning: …` (`base.c:44-46`), the host forwards `[VST3 Scanner]` lines (`obs-vst3.cpp:279`), and `run.sh:324` checks for `Skipped` in obs.log. I agree not to parse log prose, though:
- The scanner prints `VST3_SKIPPED <n>` before `VST3_SCAN_DONE`.
- The properties show a warning only when n > 0.
- The Rescan button gets a static tooltip (`obs_property_set_long_description`).

**Missed by both:**
- Rescan today is incremental: it never touches `vst3list.json` (`obs-vst3.cpp:1334-1335`). Without the flag, a remembered bundle is never retried, so `--retry-failed` is required. It goes in `argv[2]`; `argv[1]` stays the list path (826).
- A probe that succeeds with zero audio classes is not cached, so it runs again on every launch. That's a follow-up, not D6.

**Final design**
- `vst3-scanner.cpp`:
  - `SkipEntry{path, binary, size, mtime_ticks, crc32, reason, kind, failures, first_failed, last_failed}`. `mtime_ticks` is an opaque value, because MSVC counts in 100 ns.
  - New functions `skipListLoad`, `skipListUpdate(path, optional<SkipEntry>)`, `resolveModuleBinary`, `fingerprintBinary`.
  - The skip check goes in the `setModuleProbe` lambda (733). Every code load passes through it, discardable reloads included.
  - `probeVst3Module` returns an outcome enum; `runProbe` uses exit codes 85/87/88.
  - `removeStaleProbeResults` also deletes `vst3skip.json.tmp-*` older than 1 h.
- `obs-vst3.cpp`: `start_pipe(path, retryFailed)` and `retrieveVst3List(src, retryFailed)`; Rescan passes true; parse the token; show the warning when the count is above 0.
- `en-US.ini`: two strings. `VST3Scanner.*`: unchanged.

**Tests** (2 s timeout)
1. Crash: scan 2 says "remembered" and a marker file shows the fixture never loaded.
2. Hang: probed twice, skipped on the third scan; set `last_failed` back 8 days and it is probed again.
3. `--retry-failed` probes the crash bundle again.
4. Flip one byte, then `touch -r` so size and mtime match: probed again.
5. Shim fixture (forwards to ADelay, crashes when an env var is set): remembered; `--retry-failed` without the env var lists it and drops the entry.
6. `ulimit -u` (needs non-root): "could not start probe", nothing recorded.
7. Corrupt skip file with and without `.bak`; stale tmp file removed.
8. Two scanners at once: valid file containing both entries.
9. `vst3list.json` still has only `version` and `plugins` at version 1; stdout has `VST3_SKIPPED 1`.

**Still disagree:** the lock (optional, not required) and the 30-day expiry for entries that are not timeouts.
[R2 CODEX] I agree with the separate `vst3skip.json`, binary resolution, reserved setup/load/save outcomes, `--retry-failed` in `argv[2]`, deduplicated logs, and the timeout policy of “retry once on the next launch, then back off for seven days.” Those fit D5 and preserve `vst3list.json` v1 compatibility.

I accept CRC32 plus size/mtime as a pragmatic cache fingerprint, but not as collision-proof. The implementation must capture a stable fingerprint: stat → hash → stat, requiring both stats and hash to remain consistent. Do that before spawning and after a failure; record only if the two fingerprints match. A `size:-1` fingerprint is unsafe: every bundle with an unresolved binary could match it. If the binary cannot be resolved or read, do not remember the failure.

I dispute three points.

1. **Thirty-day expiry for non-timeouts is wrong.** It recreates the original crash-report problem periodically and causes deterministic invalid bundles to be probed again without user intent. Crash/load/invalid-result entries should persist until fingerprint change or `--retry-failed`. Only timeout entries need automatic expiry. Prune entries for paths absent from the scan after a long retention period or cap the file size; retaining removed paths forever is also undesirable.

2. **The proposed lock avoidance is not sufficient.** Re-reading immediately before writing does not prevent lost updates:

   - Scanner A reads entry `x`.
   - Scanner B reads entry `x`.
   - A writes `x + a`.
   - B writes `x + b`, losing A.

   Worse, a slower failed probe can resurrect an entry after another scanner successfully removed it. Atomic rename prevents corruption, not stale-state races. Require an OS-level advisory lock. For this small PR, a single lock held from skip-state load through probing and final cache/skip writes is simplest; the second scanner waits, and the lock handle must be non-inheritable by the probe child. If lock acquisition fails, scan but do not overwrite skip state. A lock held only around read-modify-write still permits stale probe outcomes.

3. **“No result means plugin called `exit(0)`” is too broad.** It can also mean pipe EOF, scanner crash, IPC failure, or truncated output. Remember only a clean probe-process result that is attributable to the bundle: normal exit 0 with no valid result, load failure, malformed plugin result, or a signal from the code-loading child. Distinguish signal termination from watchdog termination; do not classify every abnormal status as timeout. Exit codes 85/87/88 are fine only for clean scanner exits, with the mapping documented and tested.

The final shape should be:

- Separate `vst3skip.json`; entries contain normalized bundle path, binary path, stable fingerprint, `kind`, `failures`, timestamps, and `retry_after`.
- Skip checks happen in the `setModuleProbe` path, with one log per bundle and a machine-readable `VST3_SKIPPED n` record.
- Timeout entries retry when `retry_after` expires; explicit rescan overrides it.
- Successful probes, including valid zero-class results, remove the entry.
- A skipped or newly failed bundle must not leave stale successful module data in `vst3list.json`; add a regression test for “previously valid, then fails.”
- Add concurrency tests that force stale writer ordering, not merely “two scanners finished with a valid file.”

I still disagree with Claude on the 30-day expiry and on treating locking as optional.
