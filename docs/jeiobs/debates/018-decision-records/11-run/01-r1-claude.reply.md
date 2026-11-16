**Request changes at `b98df7b23`.** I accept claude-3, claude-14, claude-15 and claude-17. This run's Codex logs show the AGENTS.md fix works. One new Major, claude-18: the new session-log tripwire fails open when two or more logs match, which is the normal case on this host. One new Minor, claude-19: checkout skill folders are not refused.

**What I checked (read-only, outside the tree):**
- this run's two Codex logs: the exec thread `…c3ef…` and the guardian_review thread `…c40c…`;
- strings in the engine binary, and `config.toml`;
- the 10-run `/tmp` copies; the committed versions differ only by stripped trailing whitespace;
- `.git/worktrees/tree/HEAD`, which is `b98df7b23`.

I did not read Codex's round-1 reply.

**Accepted:**
- **claude-17:**
  - :245 passes `project_doc_max_bytes=0`.
  - Neither of this run's logs has an instructions item for the checkout. Their only "AGENTS.md instructions" text (line 8) is the shared context quoting the lead's test.
  - :131 refuses `AGENTS.override.md` at any depth, and DECISIONS:112-126 says what each peer reads.
- **claude-15:**
  - Points 1 and 3 are now in code (:286-295; :169 and :122).
  - Points 2, 4 and 5 are now worded correctly (:54-62, :176, :320-321; DECISIONS:138-148), and the final hashes are printed (:334-335).
  - Note, not a finding: the lead tested the leftover-process kill with a stub only. All 24 Codex tool calls in `…c3ef…` are `exec_command`, and the engine sets up its own process groups ("failed to kill exec-server stdio process group", "failed to place bubblewrap child in its own process group"). So it is untested whether the kill at :250 reaches what a Codex command leaves running. If it does not, :52-57 and DECISIONS:137-142 should say so.
- **claude-3:** the comment at :278-280 now matches the code at :281-299.
- **claude-14:** :34-36, :186 and DECISIONS:121-123 match config.toml:15-16 and :22-23.

**claude-18 (new, Major): the session-log tripwire fails open.**
- The check is `! grep -rlF "instructions for $TREE" … | while …; do … echo "$f"; done | grep -q . || fail …` (:170-172), and the script runs under `set -uo pipefail` (:71).
- With two or more matching logs:
  1. `grep -q` exits after the first name;
  2. the loop's next `echo` is killed by SIGPIPE (status 141);
  3. pipefail makes the whole pipeline return 141;
  4. `!` turns that into success, so `fail` never runs.
- A grep read error (exit 2) together with a match has the same effect.
- Two or more matching logs is the normal case. On this host each Codex turn writes both an exec log and a guardian_review log. Run 10 had four matching logs (`…c947…`, `…c967…`, `…774e…`, `…775e…`). The lead's stub test used a single log, so it could not catch this.
- **Closes with:**
  - collect the hits in a variable, then `[ -z "$hits" ] || fail …`, with no `!` and no `grep -q`;
  - treat grep exit status 2 as a failure;
  - test with two matching logs.
- Side note, not needed to close: the check matches any text in the logs. A reply or command that quotes the header followed by the run's checkout path will fail the run and blame Codex. Run 10's round-2 logs carry my round-2 quote of that run's header, and `…c3ef…` carries the 10-run path 8 times, quoted from the committed transcripts. Peers should write `$TREE`, as I do here. Alternatively, the check can match only `"text":"# AGENTS.md instructions for`.

**claude-19 (new, Minor): checkout skill folders are not refused.**
- The engine looks for repo skills ("failed to stat repo skills root", ".agents/skills"). Its untrusted-project warning only covers "project configuration and MCP servers".
- :131 and DECISIONS:124-126 refuse only `.claude`, `CLAUDE.md` and `AGENTS.override.md`, and the tripwire only matches the AGENTS.md item.
- Not proven: the tree has no `.agents` or `.codex` folder, and `…c3ef…:3` lists a single skill root (`CODEX_HOME/skills/.system`).
- **Closes with either:**
  - adding `.agents` and `.codex` to :131 and to the matching sentences (nothing in the tree uses them today);
  - or a stub test showing that a checkout `.agents/skills/x/SKILL.md` does not appear in the skills table.

FINDING k1 | Major | withdrawn | finder-only closure holds (DECISIONS:177-181)
FINDING k2 | Major | withdrawn | tests pass or owner accepts (:95-99)
FINDING k3 | Major | withdrawn | owner-only classes and owner FLEX (:70-89, :241-243)
FINDING k4 | Minor | accepted-fix | review pinned (:100-102), merge pinned to head (:169-171)
FINDING k5 | Minor | accepted-fix | verbatim owner quotes, terms, merge marker (:8-41, :189-191)
FINDING k6 | Minor | accepted-fix | CONTRIBUTING:14-16; lead's own PR defined (:40-41)
FINDING k7 | Minor | withdrawn | withdrawn in r2 (04-r2-crosstalk.md:118)
FINDING k8 | Minor | withdrawn | no carry-over
FINDING k9 | Minor | accepted-fix | 018 README:18-19 points to the SHA table
FINDING k10 | Minor | withdrawn | deferred to an owner-merged dependency PR (HANDOFF:110-111); findings.md:21 now reads withdrawn
FINDING M1 | Major | accepted-fix | pins, submodules, deps/ owner-only (:78-82)
FINDING M2 | Major | accepted-fix | one scale, FINDING-line sources (:172-176)
FINDING M3 | Major | accepted-fix | runs committed and linked (:233-237); committed 10-run r2 reply matches the /tmp copy except stripped trailing whitespace
FINDING N1 | Major | accepted-fix | owner instruction and rewrite recorded (:18-28)
FINDING N2 | Major | accepted-fix | engine (review.sh:92-93), config checks (:96-100), unsandboxed stated (DECISIONS:127-129)
FINDING N3 | Major | accepted-fix | admin actions owner-only (:85-88)
FINDING N4 | Major | accepted-fix | owner decides first (:90-94)
FINDING N5 | Minor | accepted-fix | owner-hold rule (:70-71)
FINDING claude-1 | Major | accepted-fix | review worktree HEAD is b98df7b23 (.git/worktrees/tree/HEAD)
FINDING claude-2 | Major | accepted-fix | transcript-only exemption (:161-166)
FINDING claude-3 | Minor | accepted-fix | round-1 comment (review.sh:278-280) matches code (:281-299)
FINDING claude-4 | Minor | accepted-fix | stderr local, CLI versions, full SHA (review.sh:105, :183-186)
FINDING claude-5 | Minor | accepted-fix | CI runs recorded (history :39-44); README :4-6
FINDING claude-6 | Minor | accepted-fix | inconclusive defined (:182-185)
FINDING claude-7 | Minor | accepted-fix | merge pins head (:169-171); exemption (:161-166); peers wording (:33-36)
FINDING claude-8 | Minor | accepted-fix | CI state in 11-run 00-meta:14-20
FINDING claude-9 | Minor | accepted-fix | 018 README:7-10 says stderr stays local, run 06 trimmed
FINDING claude-10 | Minor | accepted-fix | raised IDs become required (review.sh:264)
FINDING claude-11 | Minor | accepted-fix | saved and pinned before tamper check and validation (review.sh:293, :296-299, :307-310)
FINDING claude-12 | Major | accepted-fix | in-turn writes to snapshots and saved replies fail the run (:140-146, :165, :224); cleanup exits 3 (:114-128)
FINDING claude-13 | Minor | accepted-fix | engine binary, core env, HANDOFF:106-109
FINDING claude-14 | Minor | accepted-fix | launcher as credential helper stated (review.sh:34-36, :186; DECISIONS:121-123) matching config.toml:15-16, :22-23
FINDING claude-15 | Major | accepted-fix | points 1 and 3 in code (:286-295, :169, :122); tripwire wording (:54-62; DECISIONS:138-148), meta "tripwires only" (:176), header-hash label (:320), final hashes printed (:334-335)
FINDING claude-16 | Major | accepted-fix | --setting-sources user (:241); refusal at any depth (:131)
FINDING claude-17 | Major | accepted-fix | project_doc_max_bytes=0 (:245); run-11 exec and guardian_review logs have no checkout instructions item; AGENTS.override.md refused (:131); DECISIONS:112-126
FINDING claude-18 | Major | open | session-log tripwire (:170-172) fails open with 2+ matching logs: under pipefail (:71) a SIGPIPE in the loop or grep exit 2 makes "! … | grep -q ." succeed; each Codex turn writes exec and guardian_review logs (run 10 had four)
FINDING claude-19 | Minor | open | engine discovers repo skills ("failed to stat repo skills root", ".agents/skills"); :131 and DECISIONS:124-126 do not refuse .agents/.codex; refuse them or show by stub they do not load
VERDICT: request changes
