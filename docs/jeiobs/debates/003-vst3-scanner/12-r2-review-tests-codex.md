# Round 2 review of tests — Codex

_Raw peer correspondence, task 003 (2026-11-15/16). Codex tool-call output trimmed; prompt and final answer kept verbatim. AI-assisted, not human review._

## Prompt

````text
You are the ADVERSARIAL REVIEWER (Codex lineage) of round 2 of a test change written by another model (Claude). Repo worktree: ~/oss/jeiobs-wt/scanner-impl. Round-2 test diff: /tmp/jeiobs-scan/tests-r2.diff (tools/jeiobs/vst3-check/). It responds to your own earlier findings (pin exact classes; system plugin folders; prove moduleinfo path; tighter timeout checks with reasons; noisy grandchild must be proven alive; fail closed on tooling). Verify each was addressed correctly and look for remaining vacuous passes, flakiness (timing on slow machines/containers), unsafe cleanup (pkill patterns that could kill unrelated processes on the machine — check every pkill/kill pattern carefully), writes outside /tmp and DEPS_DIR, or the quit-probe change counting the wrong processes. Current result: 15 PASS on the fixed build, original 6 PASS + 9 FAIL on the unfixed build. Report findings with file:line, severity, fix; 'no finding' per item if none. Max 600 words. Do not edit files.
````

## Answer

Round-2 review findings:

- `tools/jeiobs/vst3-check/run.sh:238-239` — **P2**: The standalone invalid-bundle check does not require an invalid-bundle skip line or `VST3_SCAN_DONE`; it only checks exit status and exact good IDs. A scanner silently ignoring the invalid bundle could pass this labeled check. Fix with `kept 10000 $GOOD_IDS; want skips scan.out invalid 1`.

- `tools/jeiobs/vst3-check/run.sh:243-248,277-279`, `quit-probe.cpp:40-45` — **P1**: Timing remains fragile. The quit probe closes OBS after a fixed 2 seconds, so slow startup can occur before the scanner exists, causing `scanning` to fail despite correct cancellation. The `<6000 ms` bound also leaves little margin over the 2-second probe timeout plus the scanner’s 2-second watchdog grace. Fix by polling for the exact scanner child before requesting quit, with a bounded fallback, and use generous/config-derived timing limits.

- `tools/jeiobs/vst3-check/run.sh:49-58,61`, `run.sh:228-229` — **P2**: Cleanup uses unescaped `pgrep/pkill -f` command-line regexes and raw PID kills. The private work-directory token makes accidental matches unlikely, but the self-test proves matching—not exclusivity—and any unrelated process whose command line contains the token can be killed. Fix by recording child PIDs/process groups, validating `/proc/<pid>/exe` and arguments, then killing exact owned PIDs.

No finding:

- Exact class/name assertions: `results.py:184-205`.
- System plugin-folder preflight: `run.sh:27-31`.
- Module-info path and parser validation: `run.sh:110-119`; scanner path `VST3Scanner.cpp:322`.
- Timeout reason matching: `results.py:208-216`.
- Noisy-grandchild liveness and identity checks: `run.sh:216-231`; `bad-noisy.c:40-61`.
- Quit-probe scanner identity/counting: `quit-probe.cpp:16-34`.
- Writes remain confined to `/tmp` work data and `DEPS_DIR`.

`bash -n` passed; malformed result input failed closed. No files were edited.
