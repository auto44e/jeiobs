# Round 1 review of tests — Codex

_Raw peer correspondence, task 003 (2026-11-15/16). Codex tool-call output trimmed; prompt and final answer kept verbatim. AI-assisted, not human review._

## Prompt

````text
You are an ADVERSARIAL REVIEWER of a test script (different model lineage from the author). Repo worktree: ~/oss/jeiobs-wt/scanner-impl. The test changes are in /tmp/jeiobs-scan/tests.diff (tools/jeiobs/vst3-check/: run.sh, bad-noisy.c, bad-exit0.c, infoonly-moduleinfo.json, moduleinfo-check.cpp). Context: docs/jeiobs/tasks/003-vst3-scanner-robustness.md, and the implementation diff /tmp/jeiobs-scan/impl.diff (VST3 scanner probes each code-loading bundle in a child with a watchdog; skips bad ones; OBS kills the scan on unload). Against the fixed build the suite prints 15 PASS / 0 FAIL in 45 s. Review whether the checks are LOAD-BEARING and honest: could any check pass while the behaviour it names is broken (e.g. greps that match unrelated lines, counts that would also match on the old code, timeouts too generous to detect a hang, the moduleinfo-only fixture not actually being valid so it is skipped for the wrong reason, the noisy grandchild not really holding the pipe, process-leak checks that cannot see the process)? Could it be flaky on a slower machine or a container (timing margins, Xvfb/DISPLAY assumptions, PulseAudio)? Does it ever write outside /tmp and DEPS_DIR, touch the real HOME, or leave processes behind on failure paths? Report concrete findings with file:line and severity, and suggested fixes. Max 700 words. Do not edit files.
````

## Answer

The 15/0 result is real for the supplied environment, but several checks are not reliable gates.

- **High — system plugins are not isolated.** `run.sh:102` only populates `$HOME/.vst3`, while the scanner also searches `/usr/lib/vst3` and `/usr/local/lib/vst3` (`VST3Scanner.cpp:113`). `nplug()` and `listed()` only count entries (`run.sh:112`, `run.sh:136`). Three unrelated system classes can therefore mask missing/ignored fixtures, including `infoonly`; system plugin constructors may also perform writes outside the claimed sandbox. Isolate search roots or add a test search-path override, fail on `cp` errors, and assert exact expected IDs/names/paths.

- **High — the noisy fixture does not exercise the claimed pipe hazard.** `bad-noisy.c:22` writes after the probe redirects stdout/stderr to `/dev/null` (`vst3-scanner.cpp:220`); the scanner also drains and discards probe output (`vst3-scanner.cpp:294`). The grandchild consequently does not hold the protocol pipe. `fork()` failure is ignored, and `run.sh:157` unconditionally kills it without checking that it existed. Use an explicit marker/PID and assert both pipe closure and zero remaining processes before cleanup.

- **Medium — `infoonly` is not proven to have been discovered.** `run.sh:163` only requires three total classes and no skip line. A scanner that ignores that bundle passes. The parser helper does establish that the JSON is accepted by the SDK parser, but not that the bundle copy was used. Give it a unique audio class and require four exact classes, while retaining non-discardable metadata.

- **Medium — timeout checks are too permissive.** `run.sh:120` allows ten seconds despite configuring a two-second probe timeout, and does not require the reason `probe timed out`; a much slower or generic skip can pass. The host hang check (`run.sh:176`) has no maximum elapsed-time assertion. Require the timeout reason and a tighter measured bound; assert that scanning actually started.

- **Medium — process checks fail open in minimal containers.** `run.sh:41` treats `pgrep` errors or absence as zero processes because of `| wc -l`; `pkill` errors are ignored (`run.sh:42`). Preflight these tools and fail closed.

- **Medium — timing/audio environment can be flaky.** The noisy host test quits after the probe’s fixed two-second timer (`run.sh:183`); the supplied run already took about 3 seconds, leaving little margin. PulseAudio/PipeWire are pointed at invalid sockets, but ALSA/JACK or another desktop-audio source is not disabled (`run.sh:132`). `QT_QPA_PLATFORM=offscreen` does avoid a DISPLAY/Xvfb dependency.

The script’s direct outputs are under `/tmp` and `DEPS_DIR`, but unisolated system-plugin execution prevents a strong “nothing outside” guarantee.
