**Request changes at `207875cad`.** Most of claude-20 is fixed, but four items I raised in run 12 are still open, so it stays Major. The claude-21 fix puts run 06 under the wrong PR, so it stays Minor. No new findings.

**Checks (read-only):**
- The worktree's HEAD and ORIG_HEAD are `207875cad`, with one reflog entry. The memory dir is empty.
- This run's home, `codex-review-b`, has only this run's two logs (`08-53-54 …1a36`, `…1a4b`).
  - Neither contains "instructions for /tmp/tmp.tzkufDOC7P/tree".
  - The only skills are in `skills/.system`.
  - `developer_instructions` is null (1a36:6).
- Workers have moved out. `codex-worker` rollout `08-49-29 …0e48:1` has cwd `~/oss/jeiobs-wt/d6-impl`. Both `codex-review` logs written after run 12 come from review checkouts.

**claude-20: fixed**
- :111-117 and :199 refuse to start, or fail the run, when another process has `CODEX_HOME=<home>`.
- config.toml is hashed at :110 and checked against that hash at :198.
- The wording is fixed at :29-34 and DECISIONS:120-126.

**claude-20: still open**
1. **The config check is still a line filter.** So "may only have … no MCP … keys" (:31-34; DECISIONS:123-126) overstates what is enforced:
   - `'mcp_servers'.x.command = "…"` (a TOML literal-string key) passes :106, and so does `developer_instructions = "…"`.
   - `inherit = "core"` placed inside any table or string passes :107.
   - :105 pipes into `grep -vq` under pipefail, which is the pattern :191 rules out. A bad header followed by more than 64 KiB of `[` lines passes on SIGPIPE. :114 has the same shape.
2. **`skills/` is not checked.** This was one of my run-12 closure conditions. :42-43 mentions only `.system`. A `SKILL.md` that an unsandboxed peer or worker drops into `CODEX_HOME/skills/` would be listed to later Codex turns, and nothing fails.
3. **00-meta does not record the config.** :206-220 name neither the home nor `CFG_HASH`. Runs now use separate copies (`-b` here, `-c` in the parallel run), so the record cannot show which config a run started with. Recording both would pin the config across runs, which also covers item 1.
4. **Wording I asked for in run 12 (r1 :48) is still missing.**
   - The refusal (:150-153; DECISIONS:133-135) covers only what the commit contains.
   - Processes on this account can still write the review home, `~/.claude` and the launcher.
   - From `~/.claude`, `rules/*.md` and `outputStyle` (settings.json:59) reach this Claude turn under `--setting-sources user`.
   - Every worktree shares the one memory dir.

**What closes claude-20:** items 2–4, plus item 1 either fixed (parse with `python3 -I` and tomllib, and allowlist the top-level keys) or described as a line filter. If you fix all four by wording only, I will lower it to Minor.

**claude-21:**
- **Fixed:** the titles and the owner merge (README:1-4, findings.md:1, DECISIONS:3-7 and :244-247), and the edit lists now match (DECISIONS:61-63; README:19-23).
- **Wrong:** run 06 reviewed #18's merged head, not #19.
  - 06-run/00-meta.md:4 shows `374f842c7`, and :10 reads "Review run 06 of PR #18".
  - Both peers requested changes there (:44, :73).
  - README:3-4, DECISIONS:5 and DECISIONS:246-247 still put run 06 under #19.
- **Missing:** findings.md has no line naming the merge.
- **Stale:** "06–12" goes out of date with this run, and the exempt transcript commit cannot edit DECISIONS or README (:170-175).
- **What closes it:** "run 06 reviewed #18's merged head (both peers requested changes); runs 07 onward review #19" in those places and in findings.md.

**codex-19 (Codex decides):** :121 refuses a non-empty OUT, and :362-364 check the append and its last line before pinning the file again. This looks right to me.

**Note:** the setsid stub keeps `CODEX_HOME` in its environment. Real Codex commands get the "core" environment (:107), which probably drops `CODEX_HOME`, so :199 likely would not see a daemon that a peer's command leaves behind. :61-62 still list setsid as a miss, so the text does not overclaim.

FINDING k1 | Major | withdrawn | finder-only closure holds (DECISIONS:186-190)
FINDING k2 | Major | withdrawn | tests pass or owner accepts (:100-104)
FINDING k3 | Major | withdrawn | owner-only classes and owner FLEX (:75-93, :251-253)
FINDING k4 | Minor | accepted-fix | review pinned (:105-107), merge pinned to head (:178-180)
FINDING k5 | Minor | accepted-fix | verbatim owner quotes, terms, merge marker (:10-43, :198-200)
FINDING k6 | Minor | accepted-fix | CONTRIBUTING:14-16; lead's own PR defined (:42-43)
FINDING k7 | Minor | withdrawn | withdrawn in r2 (04-r2-crosstalk.md:118)
FINDING k8 | Minor | withdrawn | memory dir still empty (re-checked); shared-dir wording in claude-20
FINDING k9 | Minor | accepted-fix | 018 README:24-25 points to the SHA table
FINDING k10 | Minor | withdrawn | deferred to an owner-merged dependency PR (HANDOFF:110-111); findings.md:25 reads withdrawn
FINDING M1 | Major | accepted-fix | pins, submodules, deps/ owner-only (:83-87)
FINDING M2 | Major | accepted-fix | one scale, FINDING-line sources (:181-185)
FINDING M3 | Major | accepted-fix | runs committed and linked (:242-247)
FINDING N1 | Major | accepted-fix | owner instruction and rewrite recorded (:20-30)
FINDING N2 | Major | accepted-fix | engine (review.sh:98-99), config checks (:102-110), unsandboxed stated (DECISIONS:136-138); residue in claude-20
FINDING N3 | Major | accepted-fix | admin actions owner-only (:90-93)
FINDING N4 | Major | accepted-fix | owner decides first (:95-99)
FINDING N5 | Minor | accepted-fix | owner-hold rule (:75-76)
FINDING claude-1 | Major | accepted-fix | review worktree HEAD and ORIG_HEAD 207875cad, one reflog entry (.git/worktrees/tree)
FINDING claude-2 | Major | accepted-fix | transcript-only exemption (:170-175)
FINDING claude-3 | Minor | accepted-fix | round-1 comment (review.sh:310-312) matches code (:313-331)
FINDING claude-4 | Minor | accepted-fix | stderr local, CLI versions, full SHA (review.sh:124-125, :210-213)
FINDING claude-5 | Minor | accepted-fix | CI runs recorded (history :41-44)
FINDING claude-6 | Minor | accepted-fix | inconclusive defined (:191-194)
FINDING claude-7 | Minor | accepted-fix | merge pins head (:178-180); exemption (:170-175); peers wording (:35-38)
FINDING claude-8 | Minor | accepted-fix | CI state passed via -C into 00-meta (review.sh:219)
FINDING claude-9 | Minor | accepted-fix | 018 README:11-12 says stderr stays local, run 06 trimmed
FINDING claude-10 | Minor | accepted-fix | raised IDs become required (review.sh:296)
FINDING claude-11 | Minor | accepted-fix | saved and pinned before tamper check and validation (review.sh:325, :328-331, :339-342)
FINDING claude-12 | Major | accepted-fix | in-turn writes to snapshots and saved replies fail the run (:160-166, :185, :301); cleanup exits 3 (:133-147)
FINDING claude-13 | Minor | accepted-fix | engine binary (:98-99), core env (:107), HANDOFF:106-109
FINDING claude-14 | Minor | accepted-fix | launcher as credential helper stated (review.sh:37-39, :213; DECISIONS:130-132) matching codex-review-b config.toml:15, :22
FINDING claude-15 | Major | accepted-fix | process groups (:263-278); tripwire wording (:44-67; DECISIONS:136-157); final hashes printed (:365-368)
FINDING claude-16 | Major | accepted-fix | --setting-sources user (:268); refusal at any depth (:150-153)
FINDING claude-17 | Major | accepted-fix | project_doc_max_bytes=0 (:272); this run's Codex logs have no instructions item for $TREE
FINDING claude-18 | Major | accepted-fix | session-log hits in a variable, grep status 2 fails (:190-197); new grep -q pipes (:105, :114) tracked under claude-20
FINDING claude-19 | Minor | accepted-fix | .agents/.codex refused at any depth (:150-153; DECISIONS:133-135); this run lists only skills/.system
FINDING claude-20 | Major | open | workers moved and per-run pin done; left: line-filter config check ('mcp_servers'.x and developer_instructions pass :106; :105 grep -vq under pipefail), skills/ unchecked, 00-meta omits home and config hash, account-wide writers unstated
FINDING claude-21 | Minor | open | run 06 reviewed #18's merged head 374f842c7 (06-run/00-meta.md:4, :10) but README:3-4 and DECISIONS:5, :246-247 put it under #19; no merge line in findings.md
VERDICT: request changes
