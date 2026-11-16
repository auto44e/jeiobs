**Request changes at `02d4c28ee` (round 2).** The commit is unchanged since round 1, so my dispositions stand. Both new Codex findings hold: codex-15 is my claude-15 point 2 and codex-16 is point 5. This run's own Codex logs confirm claude-17.

**Checks (read-only).** Outside the tree I read the 10-run OUTDIR, my round-1 session, the review `config.toml`, strings in the engine binary, and matched tokens from this run's Codex rollouts.
- My saved `/tmp/jeiobs-pr18/10-run/01-r1-claude.reply.md` matches my session (`…/-tmp-tmp-1UsQmTwkDD-tree/2c484437….jsonl:291`: first line and FINDING lines). So my unpinned round-1 reply (point 1) was not changed this time.
- Codex's saved `02-r1-codex.reply.md` matches its rollout (`…c947…:169`).

**codex-15 / codex-16 (Codex decides): I agree with both.**
- **codex-15:** a re-check in `cleanup` (:95-103) narrows the gap but cannot close it, because a process left running by a turn can outlive the runner. Killing each turn's process group narrows it further, but an unsandboxed process can call `setsid` and escape. So the claim has to be bounded in words: the tripwires stop when the run ends. My round-1 wording for DECISIONS:124-132 says this.
- **codex-16:** two ways to fix it:
  - write the manifest as a separate last file and hash the finished `00-meta.md`;
  - or label the `00-meta.md` hash "header, before Finished".

  The transcript edits listed in README:13-15 also change the hashes.

**claude-17 (Major, open), confirmed in run 10:**
- Line 4 of both of this run's Codex rollouts says "AGENTS.md instructions for /tmp/tmp.1UsQmTwkDD/tree".
  - `rollout-2026-11-16T07-20-43-01a1e33a-c947-…jsonl` is the exec thread.
  - `…c967-…jsonl` has `"thread_source":"guardian_review"`. This is the auto-reviewer (`approvals_reviewer = "auto_review"`, config.toml:7) that approves Codex's `require_escalated` commands.
  - So the reviewed commit also gives instructions to the approver of Codex's unsandboxed commands.
- The engine's `core/src/agents_md.rs` names `AGENTS.override.md`. The trust gate only covers "project configuration and MCP servers".
- **Closes with either:**
  - refuse a root `AGENTS.override.md` in the check at :107, and make it owner-only next to `AGENTS.md` (DECISIONS:73);
  - or run Codex with `-c project_doc_max_bytes=0`. The engine has this key (default 32768). It closes only if the rollout then has no "AGENTS.md instructions" item. This would match Claude's `--setting-sources user`.

  Either way, also fix :47-48 and DECISIONS:114-115 so they say what Codex reads.

**claude-15 (Major, open):** unchanged. It closes with my round-1 wording (DECISIONS:124-132, mirrored at review.sh:32, :38-44, :148) or with code for points 1 and 3.

**claude-3 and claude-14 (Minor, open):** unchanged.

**codex-14 nit (Codex's finding):** if the prompt hash check at :235 fails, it fails inside `$(save …)` (:252, :264), which only ends the subshell. The run then fails at :116 with "cannot pin missing file", and that message overwrites FAILED (:94). So a tamper event gets recorded as a missing file. Adding `|| exit 3` after the `save` calls fixes it.

**k10:** findings.md:21 still says "open (deferred)".

FINDING k1 | Major | withdrawn | finder-only closure holds (DECISIONS:161-165)
FINDING k2 | Major | withdrawn | tests pass or owner accepts (:95-99)
FINDING k3 | Major | withdrawn | owner-only classes and owner FLEX (:70-89, :225-227)
FINDING k4 | Minor | accepted-fix | review pinned (:100-104), merge pinned to head (:153-155)
FINDING k5 | Minor | accepted-fix | verbatim owner quotes, terms, merge marker (:8-41, :173-175)
FINDING k6 | Minor | accepted-fix | CONTRIBUTING:14-16; lead's own PR defined (:40-41)
FINDING k7 | Minor | withdrawn | withdrawn in r2 (04-r2-crosstalk.md:118)
FINDING k8 | Minor | withdrawn | memory dir still empty; no carry-over
FINDING k9 | Minor | accepted-fix | 018 README:16-17 points to the SHA table
FINDING k10 | Minor | withdrawn | deferred to an owner-merged dependency PR (HANDOFF:110-111); findings.md:21 should read withdrawn
FINDING M1 | Major | accepted-fix | pins, submodules, deps/ owner-only (:78-82)
FINDING M2 | Major | accepted-fix | one scale, FINDING-line sources (:156-160)
FINDING M3 | Major | accepted-fix | 01-09 committed and linked (:217-221); my 10-run r1 reply matches session 2c484437 :291
FINDING N1 | Major | accepted-fix | owner instruction and rewrite recorded (:18-28)
FINDING N2 | Major | accepted-fix | engine (review.sh:76-77), config checks (:80-84), unsandboxed stated (DECISIONS:120-121); residue in claude-14, claude-15, claude-17
FINDING N3 | Major | accepted-fix | admin actions owner-only (:85-88)
FINDING N4 | Major | accepted-fix | owner decides first (:90-94)
FINDING N5 | Minor | accepted-fix | owner-hold rule (:70-71)
FINDING claude-1 | Major | accepted-fix | review worktree HEAD is 02d4c28ee (.git/worktrees/tree/HEAD)
FINDING claude-2 | Major | accepted-fix | transcript-only exemption (:145-150)
FINDING claude-3 | Minor | open | "blind" gone (meta:5), but the comment at review.sh:244-245 says tamper check before save; code saves and pins first (:252-254)
FINDING claude-4 | Minor | accepted-fix | stderr local, CLI versions, full SHA (review.sh:89, :155-158)
FINDING claude-5 | Minor | accepted-fix | CI runs recorded (history :39-44); README :4-6
FINDING claude-6 | Minor | accepted-fix | inconclusive defined (:166-169)
FINDING claude-7 | Minor | accepted-fix | merge pins head (:153-155); exemption (:145-150); peers wording (:32-36)
FINDING claude-8 | Minor | accepted-fix | CI state in 10-run 00-meta:14-20
FINDING claude-9 | Minor | accepted-fix | 018 README:7-10 says stderr stays local, run 06 trimmed
FINDING claude-10 | Minor | accepted-fix | raised IDs become required (review.sh:230)
FINDING claude-11 | Minor | accepted-fix | saved and pinned before tamper check and validation (review.sh:252-257, :264-267)
FINDING claude-12 | Major | accepted-fix | in-turn writes to snapshots and saved replies fail the run (:116-123, :141, :196, :241); cleanup exits 3 (:95-104); residue in claude-15
FINDING claude-13 | Minor | accepted-fix | engine binary, core env, HANDOFF:106-109
FINDING claude-14 | Minor | open | credential helper is the toolbox launcher (config.toml:14-16, :21-23) while meta says "engine, not the toolbox launcher" (:158); undisclosed (review.sh:26-27, DECISIONS:116-117)
FINDING claude-15 | Major | open | "detects any later change" (:41-42; DECISIONS:124-131) omits the round-1 unpinned reply (:248-253), post-run changes (as codex-15), checkout index flags (:143-144 vs :131/:136), "enforcement" (:32, :148), manifest caveats (as codex-16; :166/:272); closes with my r1 wording or code for points 1 and 3
FINDING claude-16 | Major | accepted-fix | --setting-sources user (:211) and .claude/CLAUDE.md refusal (:107-109); nit: root-only .claude check
FINDING claude-17 | Major | open | run 10 repeats it: exec and guardian_review rollouts (…c947…:4, …c967…:4) load "AGENTS.md instructions for /tmp/tmp.1UsQmTwkDD/tree"; engine agents_md.rs names AGENTS.override.md, neither owner-only (DECISIONS:73) nor refused (:107); closes with a refusal or project_doc_max_bytes=0, plus a DECISIONS line
VERDICT: request changes
