**Request changes at `ce0e3c26b`.** claude-18 and claude-19 are fixed. claude-3, -14, -15 and -17 still hold at this head (run 11 did not count). New Major, claude-20: the review CODEX_HOME is called "dedicated", but implementation workers use it too. A Codex S1-PR1 worker ran in it ten minutes before this run, and the runner checks only config.toml's section headers, once. New Minor, claude-21: the records still describe this work as PR #18.

**Checks (read-only, some outside the tree):**
- `.git/worktrees/tree/HEAD` is `ce0e3c26b`, with one reflog entry.
- This run's Codex logs (exec `…08-15-17-…c0aa…`, guardian `…c0c5…`):
  - no match for "instructions for" followed by `$TREE`;
  - one skill root, `CODEX_HOME/skills/.system` (c0aa:3).
- My committed 11-run reply matches the `/tmp` copy (38 FINDING/VERDICT lines each).
- The memory dir is empty (k8).
- I did not read Codex's round-1 reply.

**Accepted:**
- **claude-18:** :175-182 collect the hits in a variable, fail on grep status 2 (:178), and use neither `!` nor `grep -q`. Two or more matching logs now fail the run.
- **claude-19:** :135-137 refuse `.agents` and `.codex` at any depth, and DECISIONS:124-126 match. The premise at :39-40 is disputed under claude-20.
- **Re-checked:**
  - claude-3: :293-314;
  - claude-14: :34-36, :196; DECISIONS:121-123;
  - claude-15: :41-64, :126, :174, :302-310, :349-350;
  - claude-17: :255 and this run's logs.
- **codex-18 (Codex decides):** :266-270 look right. Under pipefail the last-line pipeline can only fail closed.

**claude-20 (new, Major): the review CODEX_HOME is not dedicated.**
- **Evidence:** `~/.config/jeiobs-agent/codex-review/sessions/2026/11/16/rollout-2026-11-16T08-05-59-01a1e364-3cda-….jsonl` shows:
  - cwd `~/oss/jeiobs-wt/s1-impl` (:1);
  - `"sandbox_policy":{"type":"workspace-write"…` (:6);
  - the prompt "# Worker brief (Codex, implementation): JeiOBS S1-PR1 workspace library" (:8);
  - a guardian_review thread, `…3cf0…`.
- **Why it matters:** the sandbox does not start on this host (DECISIONS:224-228). So that worker can write the home that will review its own PR.
- **Claims it breaks:** "dedicated CODEX_HOME" (:29; DECISIONS:116) and "which the lead controls" (:39-40).
- **What the runner does not check:** :99-103 look only at section headers and `inherit`, once, before round 1. Nothing checks:
  - top-level keys the engine reads (`developer_instructions` and `model_instructions_file` are strings in the engine);
  - the credential `command` it executes (config.toml:15, :22);
  - `skills/` (four bundled skills are listed to every turn, c0aa:3);
  - a global `AGENTS.md` ("Failed to read global AGENTS.md instructions" is an engine string).

  00-meta hashes the script but records nothing about this config.
- **Also wrong:** the launcher wrote the `[otel]` block (config.toml:28-33), not the engine. `credential-source-since|builder-mcp-setup-complete` match 3× in `…/0.169.0.960/bin/codex` and 0× in the engine. So "(the engine itself adds … telemetry sections)" at :30-31 and DECISIONS:117 is wrong.
- **Claude side (not yet used):** this session's auto-memory dir is `~/.claude/projects/-home-ANT-AMAZON-COM-lubuel-oss-jeiobs/memory/`.
  - It is keyed to the main repository, while this session's log sits under `-tmp-tmp-…-tree`.
  - So every Claude session in any worktree of the repo shares it, and its MEMORY.md would be loaded into review turns.
  - It is empty today, so k8 holds, but DECISIONS:33-34 depends on that.
- **Closes with code plus wording:**
  - Code, either:
    - a per-run CODEX_HOME under `$TMP`, seeded from a config whose sha256 goes in 00-meta (this also limits :177 to this run's logs); or
    - a review-only home that no worker uses. Pin config.toml's hash at start, and before each Codex turn re-check it and check that `skills/` holds only `.system`.
  - Wording:
    - fix :29-31, :39-40 and DECISIONS:116-117;
    - say the refusal covers only what the commit carries, and that workers on this account can still write `~/.claude` (that memory dir included) and the launcher.
  - If the lead only adds wording that says workers share the review home, I will lower this to Minor.

**claude-21 (new, Minor): the records still say PR #18.**
- Per the context, the owner merged #18 at 374f842c7, the commit run 06 reviewed. Both peers requested changes there (06-run/01-r1-claude.reply.md:82, 06-run/04-r2-codex.reply.md:29). Runs 07-12 review PR #19.
- The tree records neither fact:
  - DECISIONS:3-4 says "cleared this PR (#18) for a self-merge";
  - DECISIONS:233-237 is headed "Review record for this PR";
  - 018 README:1 and findings.md:1 say PR #18;
  - nothing in docs/jeiobs outside the 06/07-run transcripts mentions #19 or 374f842c7.
- Also, DECISIONS:56-58 allows two transcript edits, but README:16-18 lists four (it adds stripped trailing whitespace and flattened links).
- **Closes with:** a line in the README and findings.md naming the owner merge and the #19 runs, plus fixes to those DECISIONS lines (already part of this PR).

**Notes, not findings:**
- The kill at :259-261 reaches the CLI's own process group. Codex runs its commands in a separate exec-server group, which the engine kills itself ("failed to kill exec-server stdio process group"). :54-55 could say that for Codex this cleanup is done by the engine, not the runner.
- A D6 review (`.git/worktrees/tree1`, 6fb0cd50) started 20 s after this run, from the same repository and CODEX_HOME. Refs are shared across worktrees, so a fetch or branch commit by any agent fails both runs. That fails closed, as in run 11.

FINDING k1 | Major | withdrawn | finder-only closure holds (DECISIONS:177-181)
FINDING k2 | Major | withdrawn | tests pass or owner accepts (:95-99)
FINDING k3 | Major | withdrawn | owner-only classes and owner FLEX (:70-88, :241-243)
FINDING k4 | Minor | accepted-fix | review pinned (:100-102), merge pinned to head (:169-171)
FINDING k5 | Minor | accepted-fix | verbatim owner quotes, terms, merge marker (:8-41, :189-191)
FINDING k6 | Minor | accepted-fix | CONTRIBUTING:14-16; lead's own PR defined (:40-41)
FINDING k7 | Minor | withdrawn | withdrawn in r2 (04-r2-crosstalk.md:118)
FINDING k8 | Minor | withdrawn | memory dir still empty; no carry-over (shared-dir residue in claude-20)
FINDING k9 | Minor | accepted-fix | 018 README:21-22 points to the SHA table
FINDING k10 | Minor | withdrawn | deferred to an owner-merged dependency PR (HANDOFF:110-111); findings.md:25 reads withdrawn
FINDING M1 | Major | accepted-fix | pins, submodules, deps/ owner-only (:78-82)
FINDING M2 | Major | accepted-fix | one scale, FINDING-line sources (:172-176)
FINDING M3 | Major | accepted-fix | runs committed and linked (:233-237); committed 11-run reply matches the /tmp copy
FINDING N1 | Major | accepted-fix | owner instruction and rewrite recorded (:18-28)
FINDING N2 | Major | accepted-fix | engine (review.sh:95-96), config checks (:99-103), unsandboxed stated (DECISIONS:127-129); shared-home residue in claude-20
FINDING N3 | Major | accepted-fix | admin actions owner-only (:85-88)
FINDING N4 | Major | accepted-fix | owner decides first (:90-94)
FINDING N5 | Minor | accepted-fix | owner-hold rule (:70-71)
FINDING claude-1 | Major | accepted-fix | review worktree HEAD is ce0e3c26b, one reflog entry (.git/worktrees/tree)
FINDING claude-2 | Major | accepted-fix | transcript-only exemption (:161-166)
FINDING claude-3 | Minor | accepted-fix | round-1 comment (review.sh:293-295) matches code (:296-314)
FINDING claude-4 | Minor | accepted-fix | stderr local, CLI versions, full SHA (review.sh:109, :193-196)
FINDING claude-5 | Minor | accepted-fix | CI runs recorded (history :39-44); README :4-6
FINDING claude-6 | Minor | accepted-fix | inconclusive defined (:182-185)
FINDING claude-7 | Minor | accepted-fix | merge pins head (:169-171); exemption (:161-166); peers wording (:33-36)
FINDING claude-8 | Minor | accepted-fix | CI state in 12-run 00-meta:14-20
FINDING claude-9 | Minor | accepted-fix | 018 README:7-10 says stderr stays local, run 06 trimmed
FINDING claude-10 | Minor | accepted-fix | raised IDs become required (review.sh:279)
FINDING claude-11 | Minor | accepted-fix | saved and pinned before tamper check and validation (review.sh:308, :311-314, :322-325)
FINDING claude-12 | Major | accepted-fix | in-turn writes to snapshots and saved replies fail the run (:145-151, :170, :234); cleanup exits 3 (:118-132)
FINDING claude-13 | Minor | accepted-fix | engine binary, core env, HANDOFF:106-109
FINDING claude-14 | Minor | accepted-fix | launcher as credential helper stated (review.sh:34-36, :196; DECISIONS:121-123) matching config.toml:15, :22
FINDING claude-15 | Major | accepted-fix | points 1 and 3 in code (:302-310, :126, :174); tripwire wording (:41-64; DECISIONS:127-148); final hashes printed (:349-350)
FINDING claude-16 | Major | accepted-fix | --setting-sources user (:251); refusal at any depth (:135-137)
FINDING claude-17 | Major | accepted-fix | project_doc_max_bytes=0 (:255); this run's Codex logs have no instructions item for $TREE; DECISIONS:118-121
FINDING claude-18 | Major | accepted-fix | hits collected in a variable, grep status 2 fails, no pipe into grep -q (:175-182)
FINDING claude-19 | Minor | accepted-fix | .agents/.codex refused at any depth (:135-137; DECISIONS:124-126); this run lists only skills/.system
FINDING claude-20 | Major | open | review CODEX_HOME shared with an S1-PR1 implementation worker (rollout 08-05-59 …3cda…:1,6,8); runner checks only section headers once (:99-103); "dedicated", "lead controls", "engine adds telemetry" wrong (:29-31, :39-40; DECISIONS:116-117)
FINDING claude-21 | Minor | open | records still call runs 06-12 PR #18 and omit the owner merge at 374f842c7 (DECISIONS:3-4, :233-237; README:1; findings.md:1); DECISIONS:56-58 vs README:16-18 edits
VERDICT: request changes
