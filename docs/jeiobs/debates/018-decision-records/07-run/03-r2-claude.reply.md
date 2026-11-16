**Request changes at `d9f946609`.** I've narrowed N2: it stays Major only because the gate text, the script header and the meta string still describe confinement that doesn't exist. Rewording them closes it. I no longer ask for the owner's written acceptance, and I've split the hardening work out as claude-13 (Minor).

**What I read outside the tree this round:**
- Codex's round-1 reply and `00-meta.md` in `/tmp/jeiobs-pr18/07-run`.
- Key names and three values from the review `config.toml`, plus file listings of `skills/.system` and `~/.toolbox/tools/codex`.
- My worktree's HEAD file, which is `d9f946609`.
- From Codex's main rollout `…5993….jsonl`, only the approval and sandbox keys and the first 140 characters of each `cmd`.

I did not read Codex's reasoning or the judge thread.

**N2 (Major, open, narrowed)**
- **Evidence still holds:**
  - Rollout :6-7 shows `approval_policy` `on-request`, `approvals_reviewer` `auto_review` and `sandbox_policy` `read-only`. `require_escalated` appears 34 times.
  - `config.toml:7` sets `auto_review`, `:8-9` sets `[shell_environment_policy] inherit = "all"`, and `:34-35` sets `[otel] log_user_prompt = true`.
  - Five `skills/.system/*/SKILL.md` files exist.
  - `review.sh:58` picks `<ver>/bin/codex`, the launcher (`00-meta.md:7` reports 1.0.7804.0), not the engine `<ver>/codex` (0.169.0).
- **New fact:** every escalated command in Codex's round 1 only read files: `sed -n`, `nl`, `git show/diff/status/rev-parse`, `find`, `rg`, `bash -n`, `env | grep`. There were 0 `mcp__` calls, so nothing was damaged.
  - That is why I drop the owner-acceptance condition: DECISIONS:111-114 already defines the no-sandbox path, and :194 already makes the AppArmor change an owner action.
- **Still false, and it would be merged into the gate:**
  - DECISIONS:108-111 says "real binary" and "only model and provider settings (no MCP servers, plugins or skills)".
  - :112-114 overstates the tamper check's coverage (see claude-12).
  - :188-192 says "Until 2026-11-16", but the problem still applies to runs 05-07.
  - The same claims are in `review.sh:23-30` and in the hard-coded string at `review.sh:110` that every `00-meta.md` copies.
- **To close it,** make those lines state:
  - Codex runs the launcher binary.
  - The review home has no MCP servers but does have system skills, auto_review approvals, the full environment and otel prompt logging.
  - On this host every Codex command runs unsandboxed after automatic approval.
  - What the tamper check actually covers.

**Codex's findings (severity is Codex's call):**
- **N2:** agree.
- **codex-11:** half right.
  - Porcelain v2 lines carry `hH hI` (`review.sh:91`), so a staged change to a tracked path does change the hash.
  - The real index gap is `skip-worktree`/`assume-unchanged`: they hide edits from both `status` and `git diff`.
  - The untracked/ignored-contents gap is real; it is the same as claude-12.
  - I'd rate it Minor if each run records that Codex's escalated commands only read.
- **codex-12:** real.
  - Nothing kills the sibling peer (`:189-192`), and `:80` removes `$TREE` while it may still run.
  - A late write by that sibling becomes the next run's `BASE_STATE` (`:98`).
  - I'd rate it Minor, because it only happens on the failure path.
- **codex-6, codex-8 and N6, which Codex accepted:** I still see open issues.
  - claude-3: replies sit in `$TMP`, the parent of the tree.
  - claude-7: `:135` pins `<reviewed SHA>`, so the merge is refused after a transcript commit. That fails safe.
  - claude-11: a failed turn loses its prompt and reply.

**For this run:** Codex speaks last (`review.sh:55`; `00-meta.md:5`), and my round-2 reply sits unhashed in `$OUT` (`:199`) until `:210` reads it. Before the transcript commit, the lead should:
- compare my committed round-2 reply with my saved session in `~/.claude/projects/-tmp-tmp-3q6e04KyKC-tree/`;
- check that Codex's round-2 escalations only read files;
- record both results.

**claude-13 (Minor, new): confinement hardening follow-up.**
- Run Codex with `approval_policy="never"` and the diff inlined into its prompt, or get bubblewrap working.
- Give it a minimal environment instead of `config.toml:9` `inherit = "all"`.
- Run the engine binary instead of the launcher picked at `review.sh:58`.
- Add the AppArmor owner action to HANDOFF: a case-insensitive grep for apparmor/bubblewrap/bwrap/sandbox finds 0 matches.
- Until then, record in each run whether Codex's escalated commands only read.

**Unchanged from round 1:** claude-3, -7, -9, -11 and -12 stay Minor and open. I add the `skip-worktree` gap to claude-12, and to claude-7 the point that `:128-129` lets an unreviewed commit edit earlier transcripts. The ledger row for k10 (`findings.md:21`) should read withdrawn.

FINDING k1 | Major | withdrawn | finder-only closure holds (DECISIONS:141-145)
FINDING k2 | Major | withdrawn | tests pass or owner accepts (:95-99)
FINDING k3 | Major | withdrawn | owner-only classes and owner FLEX (:70-89, :204-206)
FINDING k4 | Minor | accepted-fix | merge and review pinned; :131 vs :135 tracked in claude-7
FINDING k5 | Minor | accepted-fix | verbatim owner quotes, terms, merge marker (:8-41, :153-155)
FINDING k6 | Minor | accepted-fix | CONTRIBUTING:14-16; lead's own PR defined (:40-41)
FINDING k7 | Minor | withdrawn | withdrawn in r2 (04-r2-crosstalk.md:118)
FINDING k8 | Minor | withdrawn | memory dir still empty; no carry-over
FINDING k9 | Minor | accepted-fix | 018 README:15-16 points to the SHA table
FINDING k10 | Minor | withdrawn | deferred to an owner-merged dependency PR; findings.md:21 should read withdrawn
FINDING M1 | Major | accepted-fix | pins, submodules, deps/ owner-only (:78-82)
FINDING M2 | Major | accepted-fix | one scale, FINDING-line sources (:136-145)
FINDING M3 | Major | accepted-fix | 01-06 committed and linked
FINDING N1 | Major | accepted-fix | owner instruction and rewrite recorded (:18-28)
FINDING N2 | Major | open | narrowed to false records: DECISIONS:108-114,:188-192 and review.sh:23-30,:110 claim real binary and settings-only home; actual launcher (00-meta:7), auto_review, inherit=all, skills; owner acceptance no longer required
FINDING N3 | Major | accepted-fix | admin actions owner-only (:85-88)
FINDING N4 | Major | accepted-fix | owner decides first (:90-94)
FINDING N5 | Minor | accepted-fix | owner-hold rule (:70-71)
FINDING claude-1 | Major | accepted-fix | review worktree HEAD is d9f946609 (.git/worktrees/tree/HEAD)
FINDING claude-2 | Major | accepted-fix | transcript-only exemption (:127-132); wording in claude-7
FINDING claude-3 | Minor | open | replies sit in $TMP, the tree's parent, while the other works (review.sh:73-74,:189-190); sessions persist (:159-160); DECISIONS:105-106 false
FINDING claude-4 | Minor | accepted-fix | stderr local, CLI versions, full SHA (review.sh:70-71,:107-110)
FINDING claude-5 | Minor | accepted-fix | CI runs recorded; README :4-6
FINDING claude-6 | Minor | accepted-fix | inconclusive defined (:146-149)
FINDING claude-7 | Minor | open | :131 pins new head vs :135 --match-head-commit reviewed SHA; :34-36 stale; :128-129 lets unreviewed commits edit earlier transcripts
FINDING claude-8 | Minor | accepted-fix | CI state in 00-meta:13-19; nit: -C optional (review.sh:52)
FINDING claude-9 | Minor | open | 018 README:8 still says every stderr log is kept
FINDING claude-10 | Minor | accepted-fix | raised IDs become required (review.sh:177-178)
FINDING claude-11 | Minor | open | failed turn loses prompt and reply: save after tamper/validate (:193-195,:199,:202), rm -rf $TMP (:80)
FINDING claude-12 | Minor | open | tamper check (:89-96) misses .git/config, hooks, review HEAD, untracked/ignored contents, ignored dirs, skip-worktree flags, $OUT
FINDING claude-13 | Minor | open | hardening follow-up: approval_policy=never or bwrap, minimal env (config.toml:9), engine binary (review.sh:58), owner action in HANDOFF (0 mentions)
VERDICT: request changes
