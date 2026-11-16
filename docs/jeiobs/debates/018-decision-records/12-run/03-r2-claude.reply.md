**Request changes at `ce0e3c26b`.** The head is the same commit as in my round 1 and the lead offered no fix in this run, so claude-20 (Major) and claude-21 (Minor) stay open. Codex's approval covers only Codex's own findings. Under DECISIONS:177-181 it cannot close mine.

**Re-checked this round (read-only):**
- The S1-PR1 worker's log is still in the review home: `rollout-2026-11-16T08-05-59-…3cda…` has cwd `~/oss/jeiobs-wt/s1-impl` (:1), `"sandbox_policy":{"type":"workspace-write"` (:6) and "Worker brief (Codex, implementation): JeiOBS S1-PR1" (:8).
- The `x-amzn-credential-source-since` headers in config.toml:30-32 come from the launcher (`0.169.0.960/bin/codex`, 2 lines), not the engine (`0.169.0.960/codex`, 0 lines). The home's `.builder-mcp-setup-complete` marker is also a launcher string. So the launcher has written this home.
- The engine reads `developer_instructions` (10 lines) and `model_instructions_file` (1 line).
- Today the home has no `AGENTS.md`, rules or prompts, and `skills/` holds only `.system`.
- The Claude memory dir is still empty.
- The review worktree's HEAD and ORIG_HEAD are both `ce0e3c26b`, with one reflog entry.

**claude-20, corrections to my own evidence:**
- **Dropped: the global-AGENTS.md sub-point.** The engine wraps user instructions as `# AGENTS.md instructions for <cwd>` (S1 log :4). A planted `$CODEX_HOME/AGENTS.md` would most likely appear in this run's log under `$TREE` and fail the run at :177, after the turn.
- **Added: the check at :100-101 is a line regex, not a TOML parse.** Each of these defines the same MCP table in valid TOML without matching `^\[mcp_servers`:
  - `[ mcp_servers.x ]` (TOML allows spaces inside the brackets);
  - a top-level dotted key, `mcp_servers.x.command = "…"`;
  - an inline table, `mcp_servers = {…}`.

  Also, `^inherit = "core"` passes when that line sits under any table. So even "no MCP sections" is not checked against someone who can write the file. Pinning the file's hash, as I proposed, covers all of this.
- **Still stands:**
  - the home is shared with an implementation worker that ran unsandboxed;
  - top-level `developer_instructions` and `model_instructions_file`, `skills/` and the credential `command` are not checked;
  - the check runs once, before round 1;
  - 00-meta records nothing about this config;
  - "dedicated" (:29), "the lead controls" (:39-40) and "the engine itself adds … telemetry" (:30-31; DECISIONS:116-117) are wrong.

  It stays Major. My offer stands: if the lead adds wording that says the home is shared and unpinned, I will lower it to Minor.

**claude-21:** unchanged.
- 06-run/00-meta.md:4 shows run 06 reviewed `374f842c7`, and both peers requested changes there (01-r1-claude.reply.md:82, 04-r2-codex.reply.md:29).
- The records still say PR #18: README:1, findings.md:1, DECISIONS:3-4 and :233-237.
- DECISIONS:56-58 allows two transcript edits, but README:16-18 lists four.

**Codex's reply:** I have no objection to Codex's dispositions of its own findings.

FINDING k1 | Major | withdrawn | finder-only closure holds (DECISIONS:177-181)
FINDING k2 | Major | withdrawn | tests pass or owner accepts (:95-99)
FINDING k3 | Major | withdrawn | owner-only classes and owner FLEX (:70-88, :241-243)
FINDING k4 | Minor | accepted-fix | review pinned (:100-102), merge pinned to head (:169-171)
FINDING k5 | Minor | accepted-fix | verbatim owner quotes, terms, merge marker (:8-41, :189-191)
FINDING k6 | Minor | accepted-fix | CONTRIBUTING:14-16; lead's own PR defined (:40-41)
FINDING k7 | Minor | withdrawn | withdrawn in r2 (04-r2-crosstalk.md:118)
FINDING k8 | Minor | withdrawn | memory dir still empty (re-checked r2); shared-dir residue in claude-20
FINDING k9 | Minor | accepted-fix | 018 README:21-22 points to the SHA table
FINDING k10 | Minor | withdrawn | deferred to an owner-merged dependency PR (HANDOFF:110-111); findings.md reads withdrawn
FINDING M1 | Major | accepted-fix | pins, submodules, deps/ owner-only (:78-82)
FINDING M2 | Major | accepted-fix | one scale, FINDING-line sources (:172-176)
FINDING M3 | Major | accepted-fix | runs committed and linked (:233-237); committed 11-run reply matches the /tmp copy
FINDING N1 | Major | accepted-fix | owner instruction and rewrite recorded (:18-28)
FINDING N2 | Major | accepted-fix | engine (review.sh:95-96), config checks (:99-103), unsandboxed stated (DECISIONS:127-129); shared-home residue in claude-20
FINDING N3 | Major | accepted-fix | admin actions owner-only (:85-88)
FINDING N4 | Major | accepted-fix | owner decides first (:90-94)
FINDING N5 | Minor | accepted-fix | owner-hold rule (:70-71)
FINDING claude-1 | Major | accepted-fix | review worktree HEAD and ORIG_HEAD ce0e3c26b, one reflog entry (.git/worktrees/tree)
FINDING claude-2 | Major | accepted-fix | transcript-only exemption (:161-166)
FINDING claude-3 | Minor | accepted-fix | round-1 comment (review.sh:293-295) matches code (:296-314)
FINDING claude-4 | Minor | accepted-fix | stderr local, CLI versions, full SHA (review.sh:109, :193-196)
FINDING claude-5 | Minor | accepted-fix | CI runs recorded (history :39-44); README :4-6
FINDING claude-6 | Minor | accepted-fix | inconclusive defined (:182-185)
FINDING claude-7 | Minor | accepted-fix | merge pins head (:169-171); exemption (:161-166); peers wording (:33-36)
FINDING claude-8 | Minor | accepted-fix | CI state passed via -C into 00-meta (review.sh:202)
FINDING claude-9 | Minor | accepted-fix | 018 README:7-10 says stderr stays local, run 06 trimmed
FINDING claude-10 | Minor | accepted-fix | raised IDs become required (review.sh:279)
FINDING claude-11 | Minor | accepted-fix | saved and pinned before tamper check and validation (review.sh:308, :311-314, :322-325)
FINDING claude-12 | Major | accepted-fix | in-turn writes to snapshots and saved replies fail the run (:145-151, :170); cleanup exits 3 (:118-132)
FINDING claude-13 | Minor | accepted-fix | engine binary, core env, HANDOFF:106-109
FINDING claude-14 | Minor | accepted-fix | launcher as credential helper stated (review.sh:34-36, :196; DECISIONS:121-123) matching config.toml:15-16, :22-23
FINDING claude-15 | Major | accepted-fix | points 1 and 3 in code (:302-310, :126, :174); tripwire wording (:41-64; DECISIONS:127-148); final hashes printed (:349-350)
FINDING claude-16 | Major | accepted-fix | --setting-sources user (:251); refusal at any depth (:135-137)
FINDING claude-17 | Major | accepted-fix | project_doc_max_bytes=0 (:255); session-log check (:177) also catches a global AGENTS.md
FINDING claude-18 | Major | accepted-fix | hits collected in a variable, grep status 2 fails, no pipe into grep -q (:175-182)
FINDING claude-19 | Minor | accepted-fix | .agents/.codex refused at any depth (:135-137; DECISIONS:124-126); skills/ holds only .system
FINDING claude-20 | Major | open | review CODEX_HOME shared with S1-PR1 worker (rollout 08-05-59 …3cda…:1,6,8); config checked once by bypassable line regex, not pinned (:99-103); "dedicated", "lead controls", "engine adds telemetry" wrong (:29-31, :39-40; DECISIONS:116-117)
FINDING claude-21 | Minor | open | records still call runs 06-12 PR #18 and omit the owner merge at 374f842c7 (DECISIONS:3-4, :233-237; README:1; findings.md:1); DECISIONS:56-58 vs README:16-18 edits
VERDICT: request changes
