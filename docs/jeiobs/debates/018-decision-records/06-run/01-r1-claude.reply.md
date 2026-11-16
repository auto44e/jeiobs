**Request changes at `374f842c7`.** Claude's confinement and the SHA pinning check out. Codex is still not confined: on this host its sandbox fails and its commands are auto-approved to run outside it. Round 1 is also not blind for whoever speaks second. My other findings are accepted or withdrawn.

**Verified this turn**
- **My tools.** I have only Read, Glob and Grep, and no MCP. I could not write, run git or post anything.
- **Pinning.** `~/oss/jeiobs/.git/worktrees/tree/HEAD` is `374f842c7a22…`. Its reflog has one entry, `0000…→374f842c7`, created at 05:24:06Z, which matches `06-run/00-meta.md:3`.
- **Ledger and transcripts.** The ledger rows match my FINDING lines (`findings.md:12-35` against `05-final/03-r2-claude.reply.md:44-67`). Both committed 05-final replies match the raw files in `/tmp/jeiobs-pr18/05-final`.
- **CI fix.** `jeiobs-checks.yaml:30-35` is sound: under `bash -e`, the `[[ ]] &&` line does not abort the step.
- **Records.** History note :39-44, README :4-6 and HANDOFF :106-107 are present.

**N2 (Major, open): Codex runs outside its sandbox**
Evidence is the 05-final Codex main thread, `~/.codex/sessions/2026/11/16/rollout-2026-11-16T05-19-21-01a1e2cb-acc3-….jsonl`:
- **Effective settings (:6).** `"approval_policy":"on-request"`, `"approvals_reviewer":"auto_review"` and `"sandbox_policy":{"type":"read-only"}`. The auto reviewer comes from `~/.codex/config.toml:5`.
- **The sandbox fails here.**
  - The first sandboxed command (:16) printed `bwrap: loopback: Failed RTM_NEWADDR: Operation not permitted` and exited 1.
  - Later commands asked for `require_escalated` (23 lines). A child thread (`…ace3…`, "You are judging one planned coding-agent action") answered `"outcome":"allow"` each time, and the commands exited 0.
  - So they ran unsandboxed, with the full environment (`config.toml:26-27`, `inherit = "all"`). The only boundary was an AI judge.
- **MCP tools were offered.** :3 lists `mcp__arcc_mcp`, `mcp__builder_mcp` and `mcp__pippin_mcp`.
- **The new script doesn't address this.** `review.sh:114-115` changes only the MCP setting. Nothing forces `approval_policy=never` or a working sandbox, so this run's Codex turn will behave the same way.
- **The texts claim otherwise.** DECISIONS:107-108, `review.sh:23-24` and the fixed meta text (`:66`, `06-run/00-meta.md:7`) describe a confinement that does not exist on this host.
- **`-c 'mcp_servers={}'` is unproven.** If `-c` values are merged into the user config table by table, an empty table removes nothing. This run's Codex rollout ("Deferred tool namespaces") will show which.
- **Fix.**
  - Get a sandbox that works on this host, or run Codex in a container with the tree mounted read-only.
  - Force `approval_policy=never`.
  - After each turn, check the rollout and write FAILED if it shows any `require_escalated` call, `mcp__` namespace or sandbox error.
  - Record the observed policy in `00-meta.md` instead of a fixed string.
  - Until then, check this run's Codex rollout for writes.

**claude-3 (Major, open): round 1 is not blind for the second speaker**
- `review.sh:104` tells every round-1 peer to "check it against the raw replies in $OUT" (`06-run/01-r1-claude.prompt.md:32`).
- Turns run one after the other (`:128-129`), so `01-r1-claude.reply.md` is complete in $OUT before Codex starts. Codex can read `/tmp` (and runs unsandboxed, see N2). When Claude goes second, `--add-dir "$OUT"` (`:111`) gives it the same access.
- In round 1 the pointer has no use: the ledger's sources are the committed 05-final files.
- **Fix.** Drop the $OUT pointer and `--add-dir` in round 1. Stage round-1 replies in a separate `mktemp -d` and move them into $OUT only after both turns finish.

**claude-7 (Minor, new): wording of the exemption and the merge rule**
- **The check can't prove "only adds".** DECISIONS:120-122 says "only adds the review run's own files under the PR's `debates/<pr>/` folder (checked with `git diff --name-only …`)". `--name-only` doesn't distinguish adds from edits, and that folder scope also allows edits to earlier transcripts and `findings.md`.
- **The ledger can't be updated.** Read strictly, nothing may change `findings.md` after the last run. Its header (:3-5, "each finder's latest … last review run") would then be false at merge. This is the claude-2 regress again, for the ledger.
- **The merge rule contradicts the exemption.** :127 says `--match-head-commit <reviewed SHA>`, but :122-123 says the merge "pins that head".
- **Stale terms.** :34-36 still says peers "read the lead's working tree", contrary to :102-103.
- **Fix.**
  - `git diff --name-status` may show only `A` under `debates/<pr>/<run>/`, plus `M findings.md` with values copied from the run's FINDING lines.
  - Add `git merge-base --is-ancestor <reviewed> <head>`.
  - :127 should pin the head that is merged.

**claude-8 (Minor, new): confined peers can't see GitHub state**
- Gate items 1, 2 and 5 depend on GitHub state: the owner-hold label and owner comments (:70-71), "remote CI … at the exact head SHA" (:97-98), and the PR body (:143-148). The peers have no network, so they can't check any of it. The PR-body part of claude-2 is unverifiable in this run.
- **Fix.** Before the run, save `gh pr view <n> --json headRefOid,body,labels,comments,statusCheckRollup` into the run folder and name it in `00-meta.md`.

**Accepted or withdrawn**
- **claude-1:** pinning verified (above).
- **claude-2:** the exemption ends the regress. What remains is in claude-7 and claude-8.
- **claude-4:** stderr is kept (`:100`); CLI versions and the full SHA are recorded (`:63-66`). Nit: the Claude model is recorded only as the alias "opus", and the Codex model only in its stderr.
- **claude-5:** history note :39-44, README :4-6, ledger rows taken from the FINDING lines.
- **claude-6:** DECISIONS:138-141.
- **k10:** withdrawn from #18. It is deferred to an owner-merged dependency PR and tracked at HANDOFF:106-107.

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
FINDING M2 | Major | accepted-fix | ledger on one scale with FINDING-line sources (findings.md:10-59; DECISIONS:128-132)
FINDING M3 | Major | accepted-fix | 01-05 committed and match their raw sources
FINDING N1 | Major | accepted-fix | owner instruction and rewrite recorded (:23-28; history note)
FINDING N2 | Major | open | Claude fixed; Codex not confined: bwrap fails here ("Failed RTM_NEWADDR"), auto_review approves require_escalated so commands run unsandboxed (05-final rollout :6, :16), review.sh:114 doesn't force approval_policy=never, MCP removal unproven, meta :66 asserts it
FINDING N3 | Major | accepted-fix | admin actions owner-only with verbatim instruction (:85-88)
FINDING N4 | Major | accepted-fix | owner decides first, then self-mergeable (:90-94)
FINDING N5 | Minor | accepted-fix | owner-hold rule (:70-71)
FINDING claude-1 | Major | accepted-fix | detached worktree HEAD = 374f842c7, single reflog entry (review.sh:56,110,114)
FINDING claude-2 | Major | accepted-fix | transcript-only exemption ends the regress (:119-124); precision in claude-7, PR body in claude-8
FINDING claude-3 | Major | open | round-1 prompt points to $OUT holding the first speaker's finished reply (review.sh:104,:128-129; 01-r1-claude.prompt.md:32); --add-dir $OUT (:111)
FINDING claude-4 | Minor | accepted-fix | stderr kept, CLI versions, full SHA (review.sh:63-66,:100); model alias only a nit
FINDING claude-5 | Minor | accepted-fix | CI runs recorded (history :39-44); README :4-6; ledger rows verbatim
FINDING claude-6 | Minor | accepted-fix | inconclusive defined over two runs (:138-141)
FINDING claude-7 | Minor | open | "only adds … (checked with git diff --name-only)" can't prove adds and admits edits to earlier transcripts and ledger; ledger can't be updated; :127 "--match-head-commit <reviewed SHA>" conflicts with :122-123; :34-36 stale
FINDING claude-8 | Minor | open | gate items 1, 2, 5 rely on GitHub state (:70-71, :97-98, :143-148) that confined peers can't see; no PR/CI snapshot in the run folder
VERDICT: request changes
