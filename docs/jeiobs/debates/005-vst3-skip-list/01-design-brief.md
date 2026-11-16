# Design brief: JeiOBS task 003 D6 — remember failed VST3 bundles

Repo: `~/oss/jeiobs` (main). Read `docs/jeiobs/tasks/003-vst3-scanner-robustness.md`
(Design, Decisions D1-D6) and the code: `plugins/obs-vst3/vst3-scan/vst3-scanner.cpp`
(coordinator: `probeVst3Module` ~451, `vst3CacheSave` ~626, `vst3CacheLoad` ~652,
scan entry ~720-780), `plugins/obs-vst3/VST3Scanner.{h,cpp}` (`scanForVST3Plugins`,
`updateModulesList`, `tryReadModuleInfo`, probe hook), `plugins/obs-vst3/obs-vst3.cpp`
(host: `retrieveVst3List`, `getScanResult`, `vst3_rescan_callback` ~1323, which calls
`freeVst3List()` + `retrieveVst3List(source)`).

## Problem
Since #16 each code-loading bundle is probed in a child with a 30 s default
timeout. A bundle that crashes or hangs is skipped and logged, but it is probed again
on every OBS launch: a hanging plugin costs up to ~37 s of background scanning per
launch (timeout + kill margin), a crashing one an OS crash report per launch (macOS
in particular), forever. Owner-approved follow-up D6.

## Constraints already decided (do not reopen unless you find a real flaw)
- D5: keep the 30 s default; a remembered *false* timeout must not stick forever.
- Cache `vst3list.json` stays format version 1 and readable by upstream obs-vst3
  (upstream ignores unknown files; prefer a separate file).
- The coordinator never loads plugin code; the host never loads plugin code during
  scanning. Linux-tested; Windows/macOS must compile (careful #ifdefs).
- Small PR; tests extend `tools/jeiobs/vst3-check/run.sh` (16 checks today; it uses
  `OBS_VST3_PROBE_TIMEOUT_MS=2000`, throwaway HOMEs, fixtures crash/hang/noisy/exit0/
  infoonly/infoone/frozen).

## Strawman (critique it)
- New file `vst3skip.json` next to `vst3list.json`:
  `{ "version": 1, "skipped": [ { "path", "fingerprint": {"size", "mtime_ns"},
  "reason", "failures", "first_failed", "last_failed" } ] }`, written atomically
  (`obs_data_save_json_safe` tmp/bak).
- Fingerprint: Windows single-file DLL or bundle binary: size + mtime of the binary
  probed; Linux/macOS bundle directory: size + mtime of the binary inside
  `Contents/<arch>/…` (or the newest mtime over a bounded walk of the bundle).
- On scan: a bundle with a matching skip entry is not probed; it is logged once per
  scan as `[VST3 Scanner] Skipped <path>: remembered <reason> (Rescan to retry)`.
- Rescan button: host passes `--retry-failed` to the scanner, which probes every
  skipped bundle again (and rewrites the list).
- Timeouts: remember them too, but retry automatically after N launches or T days?
  Or only on Rescan? (D5 flex says timeouts must not stick.)
- A successful probe removes the entry; a fingerprint change removes it.

## Questions to settle
1. Separate file vs extra key in `vst3list.json` vs per-entry flag.
2. Fingerprint definition per platform; what exactly is hashed/stat'ed; cost; how
   to avoid false matches after a plugin update that keeps size/mtime (rare) and
   false misses (touch).
3. Policy per failure kind: crash, timeout (watchdog), killed, no result, invalid
   result, load failure (`probe failed` from a bundle that is just not a valid
   VST3, e.g. the `invalid` fixture): remember all? retry rules?
4. How Rescan reaches the scanner (argument? env? deleting the file from the host?)
   and the UI text so a user knows a plugin is being skipped and how to retry.
5. Concurrency: two OBS instances (portable + normal) sharing a config dir;
   a scanner killed mid-write.
6. Exact tests to add (fixture flows across two scans, fingerprint change, Rescan,
   corrupt skip file, upstream-compat of vst3list.json unchanged).

Answer concretely; propose the final design with file/function-level changes.
Max ~700 words. Do not edit files.
