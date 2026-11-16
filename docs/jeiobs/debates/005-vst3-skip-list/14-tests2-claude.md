Stability check still running: two baseline suites in parallel, to see whether #14's freeze timing holds under load. Nothing is committed yet. Once both runs finish I'll commit and send the final report.

I've committed 8 new checks (#12–#17) as `8302b440e939938ac7d61be216328215bc834f4b`. Against the pre-fix code (6fb0cd502), the 5 checks that target the decided changes fail for the expected reasons; against the other worker's in-progress build, all 40 checks pass.

**Which build:** `d6-impl/_build` had already been rebuilt with the other worker's fixes (08:53/08:54) before I ran anything. So the "before the fixes" results come from my own build of 6fb0cd502 at `/tmp/jeiobs-d6test-base/build`. That build reports `obs_version` 0.0.1.

| Check | 6fb0cd502 | d6-impl (scanner 08:54:06, sha256 `b1fae616…`, uncommitted) |
| --- | --- | --- |
| #12 load-failed with a fingerprint (`.so` without `GetPluginFactory`, and `invalid.so`) → remembered, skipped next scan | PASS | PASS |
| #13 setup, then `_exit(0)` → remembered failure | PASS | PASS |
| #14 probe frozen after setup, killed at the deadline → `kind` timeout, failures 1, then strike 2 with backoff | PASS | PASS |
| #15 exit 90 → no entry; an existing entry stays unchanged after a `--retry-failed` probe that exits 90 | **FAIL**: stored as a failure; a Rescan raised `failures` from 1 to 2 | PASS |
| #15 exit 91 → timeout strike | **FAIL**: stored as a failure and skipped on the next scan | PASS |
| #16 malformed entries (`size` as `N.0` or a string; `failures` as a string or missing) → ignored | **FAIL**: `size` `N.0` accepted | PASS |
| #16 `version` 1.5 or 2 → whole file ignored | **FAIL**: 1.5 accepted | PASS |
| #17 valid result without a status marker → ADelay listed, entry removed | **FAIL**: "probe produced no valid status" | PASS |

Totals: 6fb0cd502 gives 35 PASS, 5 FAIL; d6-impl gives 40/40. The d6-impl result is from an uncommitted, in-progress state, so it doesn't prove the final fix. The new fixture modes are in `marked.c`; the task doc's Tests and Not done sections are updated.

**Doubts:**
- **Checks that can't fail on this code:** #12–#14 already pass on 6fb0cd502, so they only guard against regressions. In #16, only `size` `N.0` and `version` 1.5 fail before the fix; the other variants already get rejected there.
- **Beyond the brief:** I added the `failures` cases and `version` 2 to #16, and #17 for the "valid result first" decision. #17 relies on the result path being one of the probe's arguments (the fixture deletes `<arg>.status`). The fixture logs what it deleted, so a different layout fails with a clear message. Only a missing marker is covered, not `save-failed` or an unknown one.
- **#13 fixture:** it uses a new `exit0` build of `marked.c` instead of `bad/exit0.vst3`, so loads are counted.
- **Exit 90/91 are simulated:** the fixtures exit straight from the plugin's load code. That tests how the scanner classifies the exit, not the probe's own exit-90/91 paths, which would need fault injection.
- **Readings I chose where the spec is silent:**
  - a remembered failure's `kind` can be any non-empty string except `timeout`;
  - after an ignored entry is probed again there is exactly one entry for that path;
  - the `.bak` is removed before each hand edit, so how an ignored file interacts with a valid `.bak` is not tested.
- **#14 timing:** the probe is frozen after its load is logged and before its own 2 s watchdog. It froze at 88–93 ms in all 4 runs, including 2 run at the same time.
- **Running two suites at once breaks the OBS checks:** two `run.sh` suites in parallel made the real-OBS checks hang at startup and get killed at the 60 s limit (6 and 4 host-check failures). The skip-list results were unaffected, and serial runs pass. I didn't touch that section; suites should be run one at a time.
- **Not covered:**
  - a wait error that isn't a deadline kill (codex-1);
  - the race between the `setup` marker and the watchdog (codex-4);
  - a deadline kill before `setup`;
  - the architecture directory on a non-x86_64 machine (claude-3).
- **Docs:** I added an Evidence row for 6fb0cd502. The "this branch | 32 PASS" row is now out of date (there are 40 checks) and needs updating once the fix lands. I didn't touch HANDOFF.
- **Left in /tmp:**
  - `/tmp/jeiobs-d6test-base` (source now at 6fb0cd502, plus its build);
  - `/tmp/d6t2-cc.ejmA` (run logs);
  - the work dirs `/tmp/jeiobs-vst3check.{eYPp,jdWS,uIku,OoRT}`.
