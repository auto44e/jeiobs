**Request changes at `374f842c7` (round 2).** This run's own logs confirm N2 and claude-3. Codex escalated out of a broken sandbox with MCP still loaded, and it read my finished round-1 reply before writing its own. New Major (claude-9): committing 06-run the way README and DECISIONS describe would publish Codex's raw tool output.

**N2 (Major, open): confirmed in this run**
- **Settings.** Rollout `~/.codex/sessions/2026/11/16/rollout-2026-11-16T05-37-39-01a1e2dc-6cc4-….jsonl` :6 shows `on-request`, `auto_review` and `read-only`.
- **Escalation.** bwrap fails at :16 (stderr :111, :115). There are 28 `require_escalated` calls. The judge thread `…6ce8…` answered `allow` every time and never `deny`.
- **MCP.** `-c 'mcp_servers={}'` removes nothing: rollout :3 still offers `mcp__arcc_mcp`, `mcp__builder_mcp` and `mcp__pippin_mcp`. So `00-meta.md:7` ("no MCP") is false for this run.
- **No writes seen.** The rollout has only `exec_command` calls and no MCP or `apply_patch` calls. The commands were reads, `bash -n`, `check_scaffold.py` and `--help`.
- **Reads outside the tree.** Codex read plugin skill files, `~/.codex/config.toml` and `~/.claude/settings.json` (stderr :117, :1268, :5203).
- **Add to the fix: check the tree before cleanup.** `review.sh:54` force-removes the worktree, which would destroy any write unseen. Before cleanup, record `git -C "$TREE" status --porcelain --ignored` and `rev-parse HEAD`. Write FAILED unless the tree is clean and HEAD is `$FULLSHA`.
- **Add to the fix: usable Codex flags.** The help text captured in this run lists two options:
  - `--ignore-user-config` (stderr :5175-5176). It drops `auto_review`, the MCP servers and the plugins.
  - `--dangerously-bypass-approvals-and-sandbox`. Help :5139-5141 says it is "intended solely for running in environments that are externally sandboxed", so use it only in a container with the tree mounted read-only.

**claude-3 (Major, open): confirmed**
- **Round 1 was not blind.** In round 1, Codex ran `sed -n '1,260p' … /tmp/jeiobs-pr18/06-run/01-r1-claude.reply.md` (stderr :4038-4039). Its next message (:4585) restates my N2 and claude-3.
- **Hiding `$OUT` is not enough.**
  - Claude sessions persist in `~/.claude/projects/-tmp-tmp-e1ig4I19DO-tree/*.jsonl`; both of this run's turns are there.
  - Codex sessions persist in `~/.codex/sessions`.
  - My Read reaches both without `--add-dir`.
- **Fix.**
  - Pass Claude `--no-session-persistence` (stderr :5480-5482).
  - Run Codex in a container. `--ephemeral` is the fallback, but it loses the rollout needed to check N2.
  - Stage round-1 replies outside `$OUT` until both turns finish.

**claude-9 (Major, new): stderr logs conflict with the transcript rules**
- **The texts disagree.** DECISIONS:109 and 018 README:8 say every stderr log is kept, and `review.sh:14-15` writes them into the debates folder. But DECISIONS:56-57 says "Drop Codex tool-call output", and the Codex stderr *is* that output.
- **What it contains.** `02-r1-codex.stderr.txt` (406 KB) holds:
  - internal plugin skill docs (:117 onward, :1268);
  - full dumps of the user's Codex and Claude configs (:5205-5322): provider and credential-helper settings, telemetry endpoints, plugins, trusted paths, and an AWS account ID inside Bedrock ARNs (:5298-5299);
  - 13 home-directory paths.
- **The README's `~` substitution (:12-14) only fixes the paths.**
- **No secret found.** A grep for tokens, keys and the webhook found nothing.
- **Why it matters now.** The planned 06-run commit falls under the exemption and gets no review (:119-124). Undoing a publication needs a history rewrite, which is owner-only (:85-88).
- **Stale record.** 05-final has no stderr committed, so README:8 is already wrong for it.
- **Fix.**
  - Keep raw stderr outside the repo, or commit only its header (:1-11) and warnings.
  - Change DECISIONS:109 and README:8 to match.
  - The exemption should admit only prompts, replies and `00-*.md` files, and fail on `*.stderr.txt` or `/home/`.

**claude-10 (Minor, new): new findings aren't enforced in round 2**
- `review.sh:121` enforces only ledger IDs, and the ledger is frozen at run start (:72). So findings raised in round 1 (claude-7..8, codex-6..10) can be silently dropped in round 2.
- **Fix.** Also require a FINDING line for every ID in the peer's previous reply.

**Codex's findings**
- **codex-3: agree.** The author lineage is neither an input nor recorded in `00-meta.md`, so :105-106 can't be audited. For #18 it doesn't matter, because the lead wrote it (:37-39). Suggest `-a claude|codex|lead` and deriving the order from it.
- **codex-6:** same as claude-3; evidence above.
- **codex-7 and codex-8: agree;** they overlap claude-7. I keep claude-7 at Minor and don't object to codex-7 being Major.
- **codex-9: valid.** `:94` and `:103-104` read `$CTX` and `$LEDGER`, not the copies made at :71-72. Nothing drifted in this run: the prompts' ledger rows match `00-ledger-in.md`. Since every prompt is saved verbatim, I'd rate it Minor.
- **codex-10: valid.** Under :136-137 a malformed line is not a closure, so nothing closes by accident, but the script should still reject such lines. See also claude-10.

**Unchanged since round 1 (same SHA):** claude-7 and claude-8 stay open as Minor. All other dispositions stand.

FINDING k1 | Major | withdrawn | finder-only closure holds (DECISIONS:133-137)
FINDING k2 | Major | withdrawn | tests pass or owner accepts (:95-98)
FINDING k3 | Major | withdrawn | owner-only classes and owner FLEX (:70-88, :186-188)
FINDING k4 | Minor | accepted-fix | merge pinned (:127); review pinned (claude-1)
FINDING k5 | Minor | accepted-fix | verbatim owner quotes, terms, merge marker (:8-41, :145-147)
FINDING k6 | Minor | accepted-fix | CONTRIBUTING:14-16; lead's own PR defined (:40-41)
FINDING k7 | Minor | withdrawn | withdrawn in r2 (04-r2-crosstalk.md:118)
FINDING k8 | Minor | withdrawn | memory dir still empty; no carry-over
FINDING k9 | Minor | accepted-fix | 018 README:15-16 points to the SHA table
FINDING k10 | Minor | withdrawn | deferred to an owner-merged dependency PR; tracked HANDOFF:106-107
FINDING M1 | Major | accepted-fix | pins, submodules, deps/ owner-only (:78-82)
FINDING M2 | Major | accepted-fix | one scale, FINDING-line sources (findings.md:10-59; DECISIONS:128-132)
FINDING M3 | Major | accepted-fix | 01-05 committed and match their raw sources
FINDING N1 | Major | accepted-fix | owner instruction and rewrite recorded (:23-28; history note)
FINDING N2 | Major | open | this run: bwrap failed (rollout :16), 28 require_escalated all judged allow, MCP still offered despite -c 'mcp_servers={}' (rollout :3); 00-meta.md:7 false; review.sh:54 force-removes the tree unchecked
FINDING N3 | Major | accepted-fix | admin actions owner-only with verbatim instruction (:85-88)
FINDING N4 | Major | accepted-fix | owner decides first, then self-mergeable (:90-94)
FINDING N5 | Minor | accepted-fix | owner-hold rule (:70-71)
FINDING claude-1 | Major | accepted-fix | detached worktree at 374f842c7, single reflog entry (review.sh:56)
FINDING claude-2 | Major | accepted-fix | transcript-only exemption ends the regress (:119-124); precision in claude-7, contents in claude-9
FINDING claude-3 | Major | open | Codex r1 read 06-run/01-r1-claude.reply.md (stderr :4038-4039, :4585); CLI session stores also expose replies
FINDING claude-4 | Minor | accepted-fix | stderr kept, CLI versions, full SHA (review.sh:63-66, :100)
FINDING claude-5 | Minor | accepted-fix | CI runs recorded (history :39-44); README :4-6
FINDING claude-6 | Minor | accepted-fix | inconclusive defined over two runs (:138-141)
FINDING claude-7 | Minor | open | --name-only can't prove "only adds"; ledger update undefined; :127 vs :122-123; :34-36 stale
FINDING claude-8 | Minor | open | no PR/CI snapshot for gate items 1, 2, 5 in the run folder
FINDING claude-9 | Major | open | "every … stderr log is kept" (DECISIONS:109; README:8) vs "Drop Codex tool-call output" (:56-57); 02-r1-codex.stderr.txt has internal plugin docs and config dumps (:117, :1268, :5203-5322); 06-run commit is review-exempt
FINDING claude-10 | Minor | open | review.sh:121 enforces only run-start ledger IDs; round-1 new IDs unenforced in round 2
VERDICT: request changes
