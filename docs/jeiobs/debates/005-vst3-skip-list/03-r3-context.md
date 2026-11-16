# D6 design — focused round on the two open disputes

The full two-round transcript is /tmp/jeiobs-d6/design-debate.md (read it). Everything else has converged.
Open disputes:
1. Automatic expiry for non-timeout entries (crash/load/invalid). Claude: 30 days. Codex: never; only fingerprint change or Rescan.
2. Concurrency: Codex requires an OS advisory lock held across the scan; Claude says optional (re-read + single-entry merge).

Lead's compromise proposal (critique it; accept, amend, or reject with reasons):
1. No time-based expiry for crash/load/invalid entries. Instead, include the jeiobs/OBS version string
   (obs_get_version_string) in the fingerprint, so every jeiobs update retries remembered failures once
   (a new host may fix a hosting bug); otherwise only fingerprint change or Rescan (--retry-failed).
   Prune entries whose bundle path has been absent for 30 days.
2. One scanner per cache directory at a time: the coordinator takes an exclusive, non-blocking
   advisory lock on `<cachedir>/vst3scan.lock` (POSIX flock, Windows LockFileEx), close-on-exec /
   non-inheritable so probe children never hold it. If the lock is busy, wait up to 5 s polling; if still
   busy, scan WITHOUT writing either vst3skip.json or vst3list.json (read-only scan, logged), so
   two instances can never interleave writes. Lock released at exit (kernel releases on crash/kill).
State for each point whether you agree, and list any remaining disagreement. If both agree, say CONVERGED.
