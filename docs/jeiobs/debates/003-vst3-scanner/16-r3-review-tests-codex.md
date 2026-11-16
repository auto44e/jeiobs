# Round 3 review of tests — Codex

_Raw peer correspondence, task 003 (2026-11-15/16). Codex tool-call output trimmed; prompt and final answer kept verbatim. AI-assisted, not human review._

## Prompt

````text
You are the ADVERSARIAL REVIEWER (Codex lineage) of round 3 of a test change (written by Claude, plus small lead edits), in worktree ~/oss/jeiobs-wt/scanner-impl. Diff since your last review: /tmp/jeiobs-scan/tests-r3.diff (tools/jeiobs/vst3-check/). It addresses your round-2 findings (invalid-bundle skip line required; hang bounds 8 s; quit-probe waits for a running scanner child when QUITPROBE_WAIT_SCANNER=1) and adds a check that SIGSTOPs the hang probe inside hang.so and expects the coordinator to kill it ('probe timed out (killed)', < 12 s). Result: 16/16 on the fixed build; on a pre-fix build only the new check fails. Look for: the frozen-probe check passing vacuously or racing (probe identification, stopping before vs after stdio redirect, state T check), stopped processes left behind on any path (including SIGINT/SIGTERM of the script), unsafe kill patterns, and the quit-probe polling (QTimer lifetime, leaks, running on the right thread). Report findings with file:line, severity, fix; 'no finding' per item if none. Max 400 words. Do not edit files.
````

## Answer

Findings:

- **P2 — `tools/jeiobs/vst3-check/run.sh:183-187`**: `freeze()` ignores `kill -STOP`’s result and treats any `T` state as proof that this check stopped the probe. A pre-stopped probe could satisfy `want froze` without a state transition. Fix: require a successful `kill`, verify the pre-state is not `T`, then require the transition to `T`.

- **P2 — `tools/jeiobs/vst3-check/quit-probe.cpp:47-61`**: `QTimer` is parentless and its `deleteLater()` is posted only after `mw->close()`. OBS queues application quit and drains deferred deletes during shutdown before that call, so the timer may remain allocated until process exit. Fix: parent the timer to `qApp` or perform cleanup before closing; avoid raw lifetime state where practical.

- Frozen-probe identification, stdio-redirection ordering, and process-tree targeting: **no finding**. Mapping `hang.so` occurs after probe stdio redirection, and argv plus `/proc/*/maps` checks are appropriately specific.

- Stopped-process cleanup, including SIGINT/SIGTERM paths: **no finding**. `SIGCONT` precedes `SIGKILL`, and the EXIT trap reaps the work directory.

- Kill-pattern safety: **no finding**. The per-run work directory and decoy self-test sufficiently constrain the `pkill -f` scope.

- Quit-probe thread affinity/polling behavior: **no finding**. The frontend loading callback and timer execute on OBS’s GUI thread, and the scanner wait has a bounded fallback.
