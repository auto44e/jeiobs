**Request changes at `02d4c28ee`.** I accept claude-16. claude-15 stays Major: three gaps are still missing from what the lead calls an exact claim. New Major claude-17: Codex still takes instructions from the commit it is reviewing. claude-3 and claude-14 stay open as Minor follow-ups.

**Checks (read-only):**
- The review checkout's HEAD is `02d4c28ee` (`~/oss/jeiobs/.git/worktrees/tree/HEAD`).
- My committed 09-run FINDING lines match my session `~/.claude/projects/-tmp-tmp-a7NRbWvLpQ-tree/64ce5ad8…jsonl`.
- This session has only Read, Glob and Grep, and no project instructions in context. In 09-run, AGENTS.md was in my context. So `--setting-sources user` (:211) works.
- I did not read `$TMP/codex-r1.reply`.
- Not verified: the script hash at meta:9 (I have no hashing tool).

**claude-16: accepted.** Fixed by :211 and the refusal at :107-109.
- Nit: :107 only checks for `.claude` at the root. So "at all" (:47) and "contains `.claude/`" (DECISIONS:113-114) overstate it. With user-only settings this is harmless.
- The AGENTS.md line I asked for moves to claude-17.

**claude-15 (Major, open).**
- **Fixed:**
  - the prompt hash is taken before the turn (:247, :262) and checked at save (:235);
  - meta is pinned (:166);
  - there is an end-of-run check (:271) and a manifest (:275-277);
  - extra files in OUTDIR are disclosed (:43-44).
- **Still overclaimed.** The texts are "This detects any later change" (:41-42) and "This detects later changes … and to the repository" (DECISIONS:129-131). They miss five points:
  1. **Round 1:** the first peer to finish has its reply left unpinned in `$TMP` until both peers exit (:248-253). Right now that file is `/tmp/tmp.1UsQmTwkDD/claude-r1.reply`, next to `tree/`. A change there is not "a change to a reply while its peer is still writing it".
  2. **After the run:** a process left behind by a turn can still change files after :271, including the manifest, which exists only in OUTDIR. Codex is unsandboxed, and `timeout` does not reap leftover processes after a normal exit.
  3. **Index flags:** DECISIONS:124 says the checkout's "files" are checked, but the check is only HEAD plus `git status` (:143-144). `skip-worktree` or `assume-unchanged` set in `.git/worktrees/tree/index` hides edits that Claude reads in round 2. `repo_state` does not cover that index (:131, :136).
  4. **"Enforced":** :32 calls the tamper check "the enforcement", and :148 (meta:7) says "tamper check enforced". It only detects.
  5. **Manifest hashes:** the hash listed for `00-meta.md` covers only the part before "- Finished:" (pinned at :166, appended to at :272-284). Transcript edits (DECISIONS:58, README:13-15) also change file hashes.
- **Closes with wording.** For DECISIONS:124-132: "…HEAD or `git status` changed. These are tripwires: they catch a change still present at the next check, until the run ends. They miss reads, network use, writes elsewhere, changes undone before a check, a reply changed before it is saved (during its turn, or in round 1 until both peers finish), checkout edits hidden by index flags, and changes after the run by a process a turn left running. Only the sandbox stops a peer that evades them. Hashes are of the files as written (for `00-meta.md`, the part before "Finished")." Mirror this at :32, :38-44 and :148.
- **Or code for points 1 and 3**, with wording for the rest:
  - pin each round-1 reply as soon as its job exits (`wait -n -p`);
  - require every `git -C "$TREE" ls-files -v` entry to be `H`.

**claude-17 (new, Major): the reviewed commit can still instruct the Codex reviewer.**
- **Evidence it happens:** in 09-run, Codex received the checkout's AGENTS.md as instructions, and they are not in the committed prompt. `~/.config/jeiobs-agent/codex-review/sessions/2026/11/16/rollout-2026-11-16T06-54-36-01a1e322-e30c-….jsonl:4` reads: "# AGENTS.md instructions for /tmp/tmp.a7NRbWvLpQ/tree\n\n<INSTRUCTIONS>\n# jeiobs contributor and agent inst…".
- **An override file takes precedence:** the engine that :76 selects (`~/.toolbox/tools/codex/0.169.0.960/codex`) contains "normal project-document precedence (`AGENTS.override.md`, `AGENTS.md`, then configured fallback filenames)". Its trust gate covers only "project configuration and MCP servers".
- **The gap:** `AGENTS.override.md` is not owner-only (DECISIONS:73 names only `AGENTS.md`), and :107 does not refuse it. So a self-mergeable commit can add one and replace Codex's instructions for its own review.
- **Claims affected:**
  - "a change cannot steer its own review" (:47-48) and "a change cannot configure its own reviewer" (DECISIONS:114-115) are true for Claude only;
  - "untrusted, see 00-meta" (:48) points at nothing, because meta:7 never says untrusted.
- **Closes with:** refusing `AGENTS.override.md` in the same check as `CLAUDE.md` (:107), plus a line in DECISIONS:116-119 saying Codex reads the reviewed commit's root `AGENTS.md` as instructions (owner-only, :73) and Claude reads no project files.

**claude-3 (Minor, open):** "blind" is gone (no match in `tools/jeiobs/peer-review`; meta:5). The comment at :244-245 still says the tamper check runs before the save. The code saves and pins first (:252-254).

**claude-14 (Minor, open), narrowed:**
- **Still open:** `~/.toolbox/bin/codex amzn credential-process`, the toolbox launcher, still runs as the Bedrock credential helper (config.toml:14-16, :21-23). Meta:7 (:158) nevertheless says "engine, not the toolbox launcher", and neither review.sh:26-27 nor DECISIONS:116-117 mentions it. One clause fixes it.
- **Dropped:**
  - the probe point: the engine prefers the bwrap on PATH ("no system bwrap was found on PATH and no bundled codex-resources/bwrap…"), which is the one :147 probes;
  - the config point: it is covered by "writes elsewhere" (DECISIONS:130).

**Codex's findings (Codex decides):**
- **codex-6:** the wording is fixed, as for claude-3.
- **codex-14:** fixed (:116, :120, :234, :236).
  - Nit: `save` runs inside `$(...)` (:252, :264), so its `fail` only ends the subshell.
  - The run still fails, at :116 or :220, but with a message that overwrites FAILED (:94).
  - Use `|| exit 3`, as at :246 and :261.

**k10:** findings.md:21 still says "open (deferred)". My FINDING lines have said withdrawn for three runs.

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
FINDING M3 | Major | accepted-fix | 01-09 committed and linked (:217-221); my 09-run lines are in session 64ce5ad8
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
FINDING claude-14 | Minor | open | credential helper is the toolbox launcher (config.toml:14-16, :21-23) while meta says "engine, not the toolbox launcher" (:158); undisclosed (review.sh:26-27, DECISIONS:116-117); probe and config points dropped
FINDING claude-15 | Major | open | "detects any later change" (:41-42; DECISIONS:124-131) omits the round-1 unpinned reply (:248-253), changes after the run by leftover processes, checkout index flags (:143-144 vs :131/:136), "enforcement" (:32, :148), manifest caveats (:166/:272); closes with exact wording or code for 1 and 3
FINDING claude-16 | Major | accepted-fix | --setting-sources user (:211) and .claude/CLAUDE.md refusal (:107-109); this session has no project instructions; nit: root-only .claude check
FINDING claude-17 | Major | open | Codex loads the reviewed commit's instructions ("# AGENTS.md instructions for /tmp/tmp.a7NRbWvLpQ/tree", 09-run rollout :4), and the engine prefers "`AGENTS.override.md`, `AGENTS.md`"; AGENTS.override.md is not owner-only (DECISIONS:73) or refused (:107), so :47-48 and DECISIONS:114-115 hold for Claude only; closes with a refusal plus a DECISIONS line
VERDICT: request changes
