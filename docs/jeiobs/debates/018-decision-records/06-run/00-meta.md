# Review run

- Started: 2026-11-16T05:24:06Z
- Reviewed commit: `374f842c7a2289eb6127fd5b0ce2da8025228c80` (peers ran in a detached worktree at this commit)
- Rounds: 2 (round 1 blind), first cross-talk speaker: claude, timeout per turn: 1500s
- Claude: claude 2.1.325.1734 (ASBX Claude Code, channel stable), model opus, effort max; tools Read,Glob,Grep; no MCP
- Codex: 0.169.0.959 (stable), effort xhigh; sandbox read-only; no MCP
- Script: `tools/jeiobs/peer-review/review.sh`, sha256 `7a79c102c947e0416acf78e8a357cd16c0e115d867b95dd37343d135af072547`
- Context: `00-context.md`, sha256 `8962a7ec5a3c84fe49232e3d5f7991378c710011bbdf3a50fc8033e55131cbf3`
- Topic: Review run 06 of PR #18 at 374f842c7: dispose of your findings; review the new review.sh and gate text.
- Finished: 2026-11-16T05:49:32Z

## Final dispositions

```text
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
FINDING c1 | Blocker | withdrawn | Missing records block self-merge (DECISIONS.md:63-64).
FINDING c2 | Major | withdrawn | Two review rounds are required (DECISIONS.md:99-106).
FINDING c3 | Major | withdrawn | Finder-only closure is explicit (DECISIONS.md:133-137).
FINDING c4 | Major | withdrawn | Owner-only classes are explicit (DECISIONS.md:70-88).
FINDING c5 | Minor | accepted-fix | Owner authority wording is corrected (CONTRIBUTING.md:14-16).
FINDING c6 | Major | withdrawn | Transcript path rules are explicit (DECISIONS.md:56-60).
FINDING c7 | Minor | accepted-fix | Public fork identity exception is recorded (DECISIONS.md:58-60).
FINDING c8 | Minor | withdrawn | D2 wording is corrected (tasks/003-vst3-scanner-robustness.md:128-136).
FINDING M1 | Major | accepted-fix | Third-party changes are owner-only (DECISIONS.md:78-83).
FINDING M2 | Major | accepted-fix | Ledger scale and finder-line sourcing are corrected (DECISIONS.md:128-137).
FINDING M3 | Major | accepted-fix | Prior correspondence is committed and linked (DECISIONS.md:53-61,178-182).
FINDING N1 | Major | accepted-fix | Rewrite authorization and SHA mapping are recorded (DECISIONS.md:23-28).
FINDING N2 | Major | open | Codex sandbox failure escalates instead of failing closed (review.sh:114-115; 06-run/02-r1-codex.stderr.txt:6,111-117).
FINDING N3 | Major | accepted-fix | Repository administration is owner-only (DECISIONS.md:85-88).
FINDING N4 | Major | accepted-fix | Persistence and ABI breaks require owner decisions (DECISIONS.md:90-94).
FINDING N5 | Minor | accepted-fix | Owner holds are defined (DECISIONS.md:70-71).
FINDING N6 | Major | accepted-fix | Prompts are saved before peer execution (review.sh:100-108).
FINDING N7 | Major | accepted-fix | Peer failures and missing outputs fail closed (review.sh:88,118-123).
FINDING codex-1 | Major | accepted-fix | Peers now use a detached reviewed-SHA worktree (review.sh:47-56).
FINDING codex-2 | Major | accepted-fix | Transcript-only head changes are permitted (DECISIONS.md:119-124).
FINDING codex-3 | Major | open | `-f` does not verify the selected speaker is the non-author lineage (review.sh:31-45; DECISIONS.md:104-106).
FINDING codex-4 | Minor | accepted-fix | README discloses helper prompt limitations (debates/018-decision-records/README.md:3-8).
FINDING codex-5 | Major | accepted-fix | Ledger-owned IDs are checked (review.sh:90-123).
FINDING codex-6 | Major | open | Round 1 exposes `$OUT` and the first reply to the second peer (review.sh:103-107,111,127-129).
FINDING codex-7 | Major | open | `git diff --name-only` cannot prove add-only transcript changes (DECISIONS.md:119-124).
FINDING codex-8 | Minor | open | Merge pins the old reviewed SHA despite the transcript exemption (DECISIONS.md:123-127).
FINDING codex-9 | Major | open | Prompts reread live context and ledger files instead of snapshots (review.sh:71-72,103-104).
FINDING codex-10 | Major | open | Validation checks only finding-ID prefixes, not disposition syntax/status (review.sh:118-123; DECISIONS.md:136-137).
VERDICT: request changes
```
