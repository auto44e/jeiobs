# Peer debate — 2026-11-16T05:37:10Z
Topic: Accept or reject the lead's D6 resolutions R1-R6.
Rounds: 1   Roles: claude=checker codex=adversarial-reviewer
Context file: /tmp/jeiobs-d6/r4-context.md


[R1 CLAUDE] **R2–R6: ACCEPT. R1: ACCEPT only with amendment (a). As worded, R1 can turn a false timeout into a permanent skip, which breaks D5. Not CONVERGED until (a) is adopted; (b) and (c) also need to be in the spec before anyone implements it.**

**R1: conditional ACCEPT.** Status markers fix the real ambiguity. Today exit 1 means both "scanner setup failed" and "the plugin called exit(1)". On Windows a deadline kill is exit 1 as well (`TerminateProcess(pp->process, 1)`, pipe-windows.c:204).

The flaw: "exit codes are diagnostics only" combined with "`setup` + abnormal exit → remembered" means that when a marker is missing, the coordinator picks the stickiest outcome:
- **Timeout write fails or the watchdog wedges.** The `timeout` overwrite can fail (ENOSPC, ACL; on Windows it goes through `ReplaceFileW`, platform-windows.c:767, which fails if another process holds the target open). The coordinator then sees `setup` + exit 86, and a slow but valid plugin is skipped permanently.
- **Save fails, then `save-failed` fails too.** Both writes go to the same directory and fail for the same reasons, so this is the typical save-failure path. The coordinator sees `setup` + exit 1, and a valid plugin is hidden.
- **Write-order race.** If setup takes ≥ T, the `setup` rename can land after `timeout`. If the two threads share one `.status.tmp`, the file can be garbled.

Required changes:
- **(a) Missing information never escalates.** Markers stay primary. Fallbacks may only move toward a less sticky outcome (no change < timeout < remembered).
  - No valid result, `setup` present, and any of: `timeout` marker, deadline kill, or elapsed ≥ T → timeout.
    - Measure elapsed from just before `os_process_pipe_create2` (:490). That point always comes before the child's watchdog start. :496 does not if the coordinator is descheduled after the spawn.
    - A crash inside that gap becomes a 7-day timeout, which is the safe direction.
  - Save failure writes `save-failed` and also exits with a reserved code, 88. Either one → no change. A crashed probe whose waitpid PID happens to be 88 (pipe-posix.c:218-222) is then just probed again next launch.
  - An unknown or garbled status → no change. The watchdog and the main thread use different tmp names.
- **(b) Timeout requires `setup`.** R1 says "deadline kill → timeout" unconditionally, which contradicts its own rule that failures before the marker change nothing.
  - The watchdog writes `timeout` only after the main thread has set an atomic flag following its `setup` write.
  - A kill with no status file → no change.
  - The frozen check (run.sh:313) stops the probe only after hang.so is mapped, so it is unaffected.
- **(c) Close two gaps:**
  - `setup` + exit 0 + no result → remembered. This is the tested `exit0` fixture (`_exit(0)` in a constructor), and "abnormal exit" doesn't cover it.
  - A result that exists but is unparsable, has a nonce mismatch, has no class list or has an empty id → no change. Our serializer writes through tmp+rename, and the id comes from `ID().toString()` (VST3Scanner.cpp:417), which plugin data can't make empty. None of these is caused by the bundle.

Implementation must-dos (not disputes):
- Put the nonce in the status file.
- Unlink `.status` and its tmp before spawning; a failed unlink is a scanner-side reject (:464-467).
- Extend `isProbeResultFileName` (:140-152 matches only `.json` and `.json.tmp`). Otherwise every cancel during a probe leaves a status file that is never removed.
- The watchdog writes `timeout` only on the deadline branch (:356), never on parent death (:361).
- `load-failed` means `Module::create` failed (:403); zero audio classes is a success.

**R2: ACCEPT.** Verified: cancel SIGKILLs the coordinator (obs-vst3.cpp:404-417), and the watchdog exits when its parent dies. Two guards for the implementation:
- The kill after a thread-start failure (:523-527) stays a scanner-side reject.
- The post-drain kill (:561) counts as a deadline kill only if now ≥ deadline, because `os_process_pipe_wait` also returns false on a waitid error.

**R3: ACCEPT.** Verified: the host loads only after `VST3_SCAN_DONE` (obs-vst3.cpp:326-331), and a failed save means exit 1 with no DONE (:757, :844-848).
- The noisy fixture's grandchild (forked in a constructor, holds inherited fds for 30 s) is exactly why O_CLOEXEC is required; keep the `/proc` fd assertion.
- Non-blocking: `os_process_pipe_destroy` reaps with no timeout (pipe-posix.c:218, pipe-windows.c:222). An unkillable (D-state) probe therefore wedges the coordinator while it holds the lock. That is not a D6 regression, since today such a plugin wedges every scan. But persisting the deadline-kill timeout *before* the final reap makes that case converge in two launches instead of never.

**R4: ACCEPT, stated as a general rule:** no stable fingerprint → not remembered. That covers an unresolved path, a missing or unreadable binary, and a stat→crc→stat mismatch.
- `getSOPath` never checks that `<stem>.so` exists (module_linux.cpp:129-153). An arch directory without the .so passes SDK resolution and then fails dlopen, so it must fall under the same rule.
- Still probe when our resolver fails; don't skip on its word.
- Test both shapes: no `x86_64-linux/` directory, and the directory without the `.so`.

**R5: ACCEPT.** This meets Codex's own condition ("or document it as a D6 non-goal"). Verified: `updateModulesList` probes only paths missing from the cache (VST3Scanner.cpp:278), and cached non-discardable classes are reused without loading (vst3-scanner.cpp:707-717). The follow-up should reuse the D6 fingerprint to invalidate the cache.

**R6: ACCEPT.** Three clarifications:
- A version mismatch overrides an active timeout backoff.
- `obs_version` is rewritten only after an outcome the bundle caused, as defined by amended R1.
- `failures` is kept when the fingerprint is unchanged.

**Blocker:** R1(a), the D5 issue. R1(b) and (c) are spec holes, one of them in an already-tested fixture. Once the lead adopts all three: CONVERGED from my side.
[R1 CODEX] R1 — **REJECT as written; ACCEPT the marker direction with amendments.**

Agree with Claude on:

- `setup`, `timeout`, `load-failed`, and `save-failed` markers.
- Nonce in the status file.
- Separate temp files for watchdog/main-thread writes.
- Parent death must not create a timeout marker.
- `setup` + clean exit with no result must be remembered.
- Zero audio classes is a successful result.

Disputes:

- **Reject `elapsed >= T ⇒ timeout`.** Measuring from before `os_process_pipe_create2` includes spawn/descheduling time and can classify a scanner/setup crash as a hang. Timeout needs a watchdog marker, or a deadline kill of a still-live, setup-armed probe. Otherwise no entry change.
- **Reject exit code 88 as an independent no-change signal.** Plugin code can call `_exit(88)`; exit codes are explicitly diagnostic only. `save-failed` must be authoritative. A missing/garbled marker must not be “recovered” from a colliding exit code.
- **Claude’s blanket “invalid result ⇒ no change” is too broad.** Nonce mismatch or unparsable output is protocol/scanner failure and should not mutate state. A nonce-valid but semantically invalid plugin result needs an explicit classification; silently treating every such result as scanner-side loses attributable failures.
- The spec must define the exact point at which `setup` is written. It must be after scanner-side setup and immediately before code loading; otherwise `setup + abnormal exit` can remember a scanner failure.

R2 — **ACCEPT with a qualification.**

The cancellation distinction is correct, and the watchdog must not write `timeout` on parent death. However, “coordinator kill always means timeout” is only valid for a deadline kill of a setup-armed probe. A deadline reached before `setup` is a scanner-side failure and must leave the entry unchanged. The post-drain kill also needs the stated deadline check.

R3 — **ACCEPT.**

Blocking is required because the host reads `vst3list.json` only after `VST3_SCAN_DONE`; a read-only fallback discards the scan result. Lock non-inheritance and PID-unique list temp files on lock failure are correct.

The test should also verify that cancellation while blocked leaves both JSON files and temp files unchanged. `/proc` proves POSIX inheritance only; Windows needs behavioral coverage.

R4 — **ACCEPT with one correction.**

Unresolved SDK binaries must not create or match a remembered entry; no `size:-1` sentinel. However, “still probe when our resolver fails” is wrong if resolution genuinely fails: no plugin code can load, so this is a scanner-side/non-attributable outcome. Test resolver parity instead, including an architecture directory with no binary.

R5 — **ACCEPT as an explicit D6 non-goal.**

Claude’s scoped remove/re-add test is useful, but it must not be presented as testing in-place replacement. The follow-up must explicitly invalidate the path-keyed successful cache using the D6 fingerprint; otherwise a bad replacement remains unprobed indefinitely.

R6 — **ACCEPT, but specify precedence.**

`obs_version` must be separate, and a version mismatch must override active timeout backoff exactly once. The retry is consumed only after an attributable outcome. Also define the simultaneous-change case: a binary fingerprint change resets failures; a version-only change preserves them. Cancellation, lock failure, setup failure, and unresolved fingerprint must not consume the version retry.

Remaining blockers:

1. R1’s timeout fallback must be removed or constrained to a proven deadline kill/watchdog marker.
2. Exit-code collisions cannot be used to infer `save-failed`.
3. Invalid-result classification needs an explicit state-machine rule.
4. R5’s cache-replacement limitation must be recorded as a tested non-goal.

**Not CONVERGED.**
