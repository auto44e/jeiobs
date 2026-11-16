**Approve at `75c0ea435`.** The run-07 fixes make the Codex confinement records match what actually runs, so I accept N2 and claude-13. To answer the lead directly: I do **not** hold that a self-merge is unsafe until bubblewrap works, so this does not need to go to the owner on my account. The records now say Codex runs unsandboxed. The tamper check covers writes to the repository. The AppArmor fix is listed as an owner action (`HANDOFF.md:106-109`). What remains is all Minor: claude-3 (one stale line), claude-12 (narrowed to `$OUT`) and a new claude-14.

**What I read outside the tree:**
- `/tmp/jeiobs-pr18/08-run/00-meta.md`.
- The review `config.toml` and file listings of the review home.
- From this run's Codex rollouts, only match counts and the matched tokens.
- String counts in the toolbox binaries.
- `/etc/os-release`, and `apparmor_restrict_unprivileged_userns`, which is 1.
- The lead repo's worktree HEAD, and my run-07 sessions.

I did not open Codex's reply or its reasoning.

**Fixes verified**
- **N2 and claude-13:**
  - The script picks the engine and checks it (`review.sh:64-65`); the meta reports "codex-cli 0.169.0".
  - The config has no MCP, plugin or project headers, and sets `inherit = "core"` (`config.toml:8-9`; `review.sh:69-72`).
  - DECISIONS:116-122 states the unsandboxed execution and what the tamper check covers.
  - Run 08's main rollout matches that text: `on-request`/`auto_review`/`read-only` at :6, 24 `require_escalated` so far, and 0 MCP tool calls.
  - The lead declined `approval_policy=never` for a valid reason.
- **claude-7:** DECISIONS:143-145, :135-140 and :33-36.
- **claude-9:** 018 README:7-10.
- **claude-11:** `review.sh:212-213` and :223.
- **claude-1:** the review worktree's HEAD is `75c0ea435`.
- **Transcript integrity:** my committed 07-run round-2 reply matches my saved session `566f1917….jsonl`. I checked the N2–N4 and claude-11–13 FINDING lines and the VERDICT line.

**claude-3 (Minor, open).** DECISIONS:105-109 and `review.sh:35-36` are now correct. But `review.sh:20-21` still says "neither reply exists while the other works". That is false: `$TMP/<peer>-r1.reply` (:209-210) sits in my tree's parent and fills in when that peer finishes. Delete the clause.

**claude-12 (Minor, open, narrowed to `$OUT`).**
- `$OUT` is either excluded from the check (`review.sh:102`, :105) or, as in this run, outside the repo. So nothing checks the saved replies, and their FINDING lines are what close findings. Codex speaks last (meta :5).
- DECISIONS:117-121 does not mention the exclusion.
- The review checkout is checked with `status` only (:114), and `skip-worktree` hides edits from `status`.
- `cleanup` (:84-86) writes FAILED but leaves the exit code at 0.
- **Fix:** hash each saved file in memory, then verify the hashes in every `tamper_check` and before the summary.
- **Until then:** before the transcript commit, compare my committed replies with `~/.claude/projects/-tmp-tmp-KFwmoNT4ch-tree/*.jsonl` and record the result.

**claude-14 (Minor, new): the Codex record still has errors.**
- **The launcher still runs, and it writes the config.**
  - The config makes the toolbox launcher the credential helper: `config.toml:15-16` and :22-23 run `~/.toolbox/bin/codex amzn credential-process`.
  - The `[otel]` headers (:30-32) carry `x-amzn-credential-source-since = "2026-11-16T06:24:44.936…Z"`. That string occurs 2× in the launcher `<ver>/bin/codex` and 0× in the engine `<ver>/codex`. Its value matches the start of the exec session `06-24-44-…8b14` in the review home.
  - So "the engine adds telemetry sections itself" is wrong in both DECISIONS:114 and `review.sh:27-28`.
  - The launcher also contains `builder-mcp-setup-complete`, and that marker file is in the review home. Yet the config is checked only once, before round 1 (`review.sh:69-72`).
  - **Fix:** say this in DECISIONS:114 and the script header, and re-run the config check before each Codex turn and at the end.
- **The sandbox probe doesn't test Codex's sandbox.**
  - The probe at `:117` runs the system `bwrap`, which fails with "setting up uid map".
  - Codex's own sandbox fails at a different step: rollout `…4473…`:17-18 shows "bwrap: … RTM_NEWADDR: Operation not permitted".
  - If the AppArmor fix covers only `/usr/bin/bwrap`, the meta could say "available" while Codex still escalates.
  - **Fix:** record the state each turn actually observed, such as bwrap errors in its stderr or the escalation count.
- **Old records stay uncorrected.** DECISIONS:202-204 corrects only the claims that runs were "sandboxed". Two other wrong records are not corrected there:
  - `06-run/00-meta.md:7` says "no MCP", but MCP tools were offered (`06-run/03-r2-claude.reply.md:67`).
  - `07-run/00-meta.md:7` says "without … skills", but five system skills were present.

**Codex's open rows (Codex decides):** codex-7, -8 and -12 look fixed to me. codex-6 points at the same stale line as claude-3. The remaining gap in codex-11 is `$OUT`.

**Ledger:** `findings.md:21` still shows k10 as "open (deferred)". My FINDING line has said withdrawn since run 07.

FINDING k1 | Major | withdrawn | finder-only closure holds (DECISIONS:151-155)
FINDING k2 | Major | withdrawn | tests pass or owner accepts (:95-99)
FINDING k3 | Major | withdrawn | owner-only classes and owner FLEX (:70-89, :215-221)
FINDING k4 | Minor | accepted-fix | review pinned (:100-104), merge pinned to head (:143-145)
FINDING k5 | Minor | accepted-fix | verbatim owner quotes, terms, merge marker (:8-41, :163-165)
FINDING k6 | Minor | accepted-fix | CONTRIBUTING:14-16; lead's own PR defined (:40-41)
FINDING k7 | Minor | withdrawn | withdrawn in r2 (04-r2-crosstalk.md:118)
FINDING k8 | Minor | withdrawn | memory dir still empty; no carry-over
FINDING k9 | Minor | accepted-fix | 018 README:16-17 points to the SHA table
FINDING k10 | Minor | withdrawn | deferred to an owner-merged dependency PR (HANDOFF:110-111); findings.md:21 should read withdrawn
FINDING M1 | Major | accepted-fix | pins, submodules, deps/ owner-only (:78-82)
FINDING M2 | Major | accepted-fix | one scale, FINDING-line sources (:146-150)
FINDING M3 | Major | accepted-fix | 01-07 committed and linked (:207-211); my 07-run r2 reply matches my session
FINDING N1 | Major | accepted-fix | owner instruction and rewrite recorded (:18-28)
FINDING N2 | Major | accepted-fix | claims now match what runs: engine (review.sh:64-65), no MCP headers and core env (:69-72), unsandboxed and tamper scope stated (DECISIONS:110-122); residue in claude-14
FINDING N3 | Major | accepted-fix | admin actions owner-only (:85-88)
FINDING N4 | Major | accepted-fix | owner decides first (:90-94)
FINDING N5 | Minor | accepted-fix | owner-hold rule (:70-71)
FINDING claude-1 | Major | accepted-fix | review worktree HEAD is 75c0ea435 (.git/worktrees/tree/HEAD)
FINDING claude-2 | Major | accepted-fix | transcript-only exemption (:135-140)
FINDING claude-3 | Minor | open | DECISIONS:105-109 fixed; review.sh:20-21 still says "neither reply exists while the other works"
FINDING claude-4 | Minor | accepted-fix | stderr local, CLI versions, full SHA (review.sh:77-78, :124-128)
FINDING claude-5 | Minor | accepted-fix | CI runs recorded (history :39-44); README :4-6
FINDING claude-6 | Minor | accepted-fix | inconclusive defined (:156-159)
FINDING claude-7 | Minor | accepted-fix | merge pins head (:143-145); exemption only adds a new run folder plus findings.md (:135-140); peers wording (:33-36)
FINDING claude-8 | Minor | accepted-fix | CI state in 08-run 00-meta :14-20; nit: -C optional (review.sh:58)
FINDING claude-9 | Minor | accepted-fix | 018 README:7-10 says stderr stays local, run 06 trimmed
FINDING claude-10 | Minor | accepted-fix | raised IDs become required (review.sh:196-197)
FINDING claude-11 | Minor | accepted-fix | prompt and reply saved before tamper check and validation (review.sh:212-213, :223)
FINDING claude-12 | Minor | open | narrowed: $OUT unchecked (review.sh:102, :105; outside the repo here) though its FINDING lines close findings; review checkout status-only (:114); DECISIONS:117-121 omits it
FINDING claude-13 | Minor | accepted-fix | engine binary, core env, HANDOFF:106-109; approval_policy=never declined for a valid reason
FINDING claude-14 | Minor | open | launcher still runs as credential helper and writes [otel] (config.toml:15-16, :30-32; string only in bin/codex) vs DECISIONS:114; config checked once (review.sh:69-72); probe is not Codex's bwrap (:117); 06/07 meta uncorrected
VERDICT: approve
