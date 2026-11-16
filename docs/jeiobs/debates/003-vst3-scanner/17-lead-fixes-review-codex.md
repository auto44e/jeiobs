# Review of lead fixes — Codex

_Raw peer correspondence, task 003 (2026-11-15/16). Codex tool-call output trimmed; prompt and final answer kept verbatim. AI-assisted, not human review._

## Prompt

````text
Review commit HEAD (lead review fixes); prompt as sent is in the PR #16 history.
````

## Answer

1. **Windows redirect — no finding** (`vst3-scanner.cpp:232,261-270`). `_fileno(stdout/stderr)` is correct for MSVC. Since `_dup2` closes the prior target descriptor, comparing `originalHandle` with the pre-dup `_get_osfhandle()` value correctly prevents double-close; differing handles are still closed. ([learn.microsoft.com](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/dup-dup2?view=msvc-170), [learn.microsoft.com](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fileno?view=msvc-170))

2. **`os_process_pipe_wait` — Minor finding** (`pipe-posix.c:192-193`). The backoff is `1, 2, 4, 8, 16 ms`, not the stated `1, 2, 4, 8, 10 ms`; `8 < 10` doubles to 16. Clamp with `min(poll_ns * 2, 10000000L)`. This modestly increases timeout/detection overshoot.

   **`si_code` — no finding.** `CLD_EXITED`, `CLD_KILLED`, and `CLD_DUMPED` are the POSIX termination codes exposed through `<signal.h>` (included here), and the filter does not miss normal exits, signal kills, or core-dumping kills. ([pubs.opengroup.org](https://pubs.opengroup.org/onlinepubs/7908799/xsh/signal.h.html), [pubs.opengroup.org](https://pubs.opengroup.org/onlinepubs/9799919799/basedefs/sys_wait.h.html), [man7.org](https://man7.org/linux/man-pages/man2/wait.2.html))

3. **`run.sh` freeze and quit probe — no finding** (`run.sh:184-190`, `quit-probe.cpp:47`). The freeze helper now rejects already-stopped processes and failed `SIGSTOP`, avoiding false positives. Parenting `QTimer` to `qApp` gives it correct Qt lifetime ownership; the existing `deleteLater()` remains valid.
