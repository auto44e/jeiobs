# D6 — round 4: lead's resolution of the remaining disputes (accept or reject each, with reasons)

Read /tmp/jeiobs-d6/design-debate.md and /tmp/jeiobs-d6/design-debate-r3.md first.

R1. Outcome taxonomy: explicit status markers, not exit codes. Before loading plugin code the probe
    writes `<result>.status` = `setup`; the watchdog overwrites it with `timeout` before its hard
    exit; a load failure writes `load-failed`; a successful result file supersedes all. The
    coordinator classifies: result+nonce valid -> success; status `timeout` -> timeout; coordinator
    deadline kill -> timeout; status `load-failed`, or `setup` with abnormal exit (crash after the
    marker, i.e. inside plugin code) -> remembered failure; scanner-side failures before the
    `setup` marker or result-save failure (child writes `save-failed`) -> no entry change. Exit
    codes are diagnostics only. (Status writes: small file, write+rename, no fsync.)
R2. Cancel vs timeout: the coordinator kills a probe ONLY from its deadline timer; host cancel kills
    the coordinator itself (os_process_pipe_kill on the coordinator), which then writes nothing. So
    "coordinator kill" is always a deadline timeout. Verified: obs-vst3.cpp cancelVst3Scan kills
    vst3ScanPipe (the coordinator); the probe watchdog exits on parent death.
R3. Lock contention: block (no read-only fallback, no 5 s cap). Verified fact: the host reads the list
    only from vst3list.json after VST3_SCAN_DONE (obs-vst3.cpp vst3JSONLoad), and the scanner exits 1
    without VST3_SCAN_DONE if the save fails, so a read-only scan loses its result. Host cancel kills
    a blocked scanner. flock/LockFileEx retried on EINTR. Lock unavailable (ENOLCK etc.): D6 off for
    this scan, vst3list.json written via tmp-<pid>. Lock fd O_CLOEXEC / non-inheritable.
R4. Unresolved binary (SDK resolution fails, no code loads): not remembered; logged once per scan;
    tested.
R5. Valid-then-fails (a cached non-discardable bundle replaced in place by a bad binary): a #16 cache
    behavior (path-keyed, no change detection), out of scope for D6, filed as a follow-up with a test
    plan; D6 tests use Claude's scoped variant.
R6. Version: separate `obs_version` field (both agreed); a version change triggers one retry; the
    retry is consumed only by an attributable probe outcome.
Reply per item ACCEPT/REJECT with reasons, any remaining blocker, and CONVERGED if none.
