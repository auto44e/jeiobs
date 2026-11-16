**Still requesting changes at `207875cad` (round 2).** The commit is the same one I reviewed in round 1, and no fix came between rounds. So claude-20 stays open as Major and claude-21 stays open as Minor. All my other dispositions are unchanged.

**Round-1 evidence re-checked (read-only):**
- The review worktree's HEAD is `207875cad`, with one reflog entry (`.git/worktrees/tree/logs/HEAD:1`). The memory dir is empty.
- `codex-review-b` has only this run's two session logs: exec `…1a36` and guardian_review `…1a4b`.
  - Neither contains "instructions for /tmp/tmp.tzkufDOC7P/tree".
  - The only skill root is `skills/.system` (1a36:3).
  - `developer_instructions` is null (1a36:6).

**Codex's reply:**
- **codex-19:** Codex accepted it. That matches my round-1 reading of :121 and :362-364.
- **codex-20 (Codex decides):** I agree. It has the same cause as claude-20 item 1. These cases pass:
  - At the top level, `'mcp_servers'.x.command = "sh"` (literal-string key) and `"mcp\u005fservers".x.command = "sh"` (escaped key) both pass :106.
  - `shell_environment_policy.inherit = "all"` at the top level passes :107, as long as some other line reads `inherit = "core"` (in another table or inside a multi-line string).
  - One fix closes both findings: parse the file with `python3 -I` and `tomllib`, allowlist the top-level keys as well as the tables, and require `shell_environment_policy.inherit == "core"`.
- **More evidence for item 1 (not a new condition):** the top-level keys matter beyond MCP.
  - `approvals_reviewer = "auto_review"` (config.toml:7) passes the check, and it decides whether Codex commands leave the sandbox.
  - Rollout 1a36:6 shows `on-request` with `auto_review`.
  - This run made 20 `require_escalated` calls (1a36:22-149). Every review in the guardian thread `…1a4b` answered `allow`; none answered `deny`.

**What closes my findings (unchanged):**
- **claude-20** closes when items 2–4 are done and item 1 is either fixed (parse and allowlist) or described as a line filter.
  - Item 2: check `skills/`.
  - Item 3: record the home path and `CFG_HASH` in 00-meta.
  - Item 4: state that other processes on this account can write the review home, `~/.claude` and the launcher.
  - If all four are fixed by wording only, I will lower it to Minor.
- **claude-21** closes when README:3-4, DECISIONS:5 and DECISIONS:246-247 say "run 06 reviewed #18's merged head (both peers requested changes); runs 07 onward review #19", and findings.md gets a line naming the merge.

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
FINDING claude-1 | Major | accepted-fix | review worktree HEAD 207875cad, one reflog entry (.git/worktrees/tree/logs/HEAD:1)
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
FINDING claude-17 | Major | accepted-fix | project_doc_max_bytes=0 (:272); this run's Codex logs have no instructions item for the checkout
FINDING claude-18 | Major | accepted-fix | session-log hits in a variable, grep status 2 fails (:190-197); new grep -q pipes (:105, :114) tracked under claude-20
FINDING claude-19 | Minor | accepted-fix | .agents/.codex refused at any depth (:150-153; DECISIONS:133-135); this run's only skill root is skills/.system
FINDING claude-20 | Major | open | unchanged commit; left: line-filter config check (literal/escaped keys pass :106, inherit anywhere passes :107, approvals_reviewer unchecked, :105 grep -vq under pipefail), skills/ unchecked, 00-meta omits home and config hash, account-wide writers unstated
FINDING claude-21 | Minor | open | run 06 reviewed #18's merged head 374f842c7 (06-run/00-meta.md:4, :10) but README:3-4 and DECISIONS:5, :246-247 put it under #19; no merge line in findings.md
VERDICT: request changes
