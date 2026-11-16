**Request changes at `ac008df4c`.** Most of my earlier findings are now fixed. Four Majors are still open: N2 is not fixed, and I found three new problems in `review.sh` and the gate.

**Checked and correct**
- Local HEAD, `origin` and the PR #18 head are all `ac008df4c`, and the PR checks pass at that SHA.
- All 37 old→new pairs in `history/2026-11-16-history-rewrite.md` have identical trees (`old^{tree}` = `new^{tree}`). That includes `55dc7db76`→`f4e2e6d35`.
- The `review.sh` sha256 recorded in `00-meta.md` matches the committed file.
- `03` and `04` match their `/tmp` sources, apart from the allowed `~` substitution.
- CI fix: run 43805282753 on main `b76d7cdb1` failed with `fatal: bad object 0171f16ef…`, and the guard at `jeiobs-checks.yaml:30-31` handles that case. A note, not a finding: the fallback at `:35` checks only the tip commit, and on a merge commit it checks almost nothing. A `git merge-base origin/main HEAD` base would be better.
- `check_scaffold.py` passes. The fence stripping hides links only inside the two verbatim transcripts (three relative links in `03` that cannot resolve from that folder).

**N2: still Major**
- **The Claude peer is not read-only.**
  - `ps` shows this turn was started as `claude -p --model opus --effort max --allowedTools Read,Glob,Grep --add-dir /tmp/jeiobs-pr18/05-final`.
  - `--allowedTools` only pre-approves tools. The flag that limits the available set is `--tools` (`claude --help`: "Specify the list of available tools").
  - `~/.claude/settings.json` sets `"defaultMode": "auto"`. So this session has Bash, Edit, Write, Agent and MCP tools that can write, and my Bash calls (`git`, `ps`, `gh`, `python3 -I`) ran without a prompt. I wrote nothing.
  - So `review.sh:16` and `DECISIONS.md:102-103` ("runs both peers read-only") are false. A peer could edit the reviewed tree or the files in OUT, including the other peer's reply.
  - Codex: `--sandbox read-only` covers its shell, not the MCP servers in `~/.codex/config.toml`. I did not check whether `codex exec` would call them.
  - Fix:
    - Run Claude with `--tools Read,Glob,Grep --strict-mcp-config` and a permission mode other than auto (e.g. `dontAsk`).
    - Write `git diff` and `git log` output into OUT. `00-context.md:3` tells peers to "run git commands", which a Read/Glob/Grep peer cannot do.
    - After each turn, fail closed if `git status` or any earlier OUT file changed.
- **The out-of-repo script still runs decision debates.**
  - A run is live now: `~/.kiro/bin/peer-debate.sh -r 2 -f /tmp/jeiobs-d6/r3-context.md … -R claude=implementor,codex=adversarial-reviewer "Settle the two open D6 disputes using the lead's compromise."` (sha256 `b555a064…`).
  - That D6 debate is about the scanner cache's persistent format.
  - The script's lines 99-100 still say "The orchestrator makes the final call". It keeps no prompts (N6) and fails open (N7).
  - DECISIONS names `review.sh` only for self-merge reviews (`:100-101`). It sets no tool rule for the debates behind decision records (`:53-61`).
  - Fix: require the in-repo tool for those debates too.

**claude-1 (Major, new): `review.sh` does not pin what the peers read.**
- `-s SHA` is only printed (`:13`, `:79`). Nothing checks that HEAD equals SHA or that the tree is clean, and the peers read the lead's live working tree.
- Yet `DECISIONS.md:34-36` says "the reviewed commit is therefore pinned by SHA".
- N1 already showed the head moving during a review. During this run the tree has an untracked `docs/jeiobs/s1-persistence-audit.md`, and an unrestricted Claude is running at the same time.
- A Claude peer limited to Read/Glob/Grep could not notice a mismatch.
- Fix: run both peers in `git worktree add --detach "$TMP/wt" "$SHA"`, keep OUT outside it, and record the full SHA.
  - A fresh path also gives Claude its own auto-memory directory. Without it, `:34`'s "keep no memory between calls" is not true for Claude Code.

**claude-2 (Major, new): records written after the final round cannot satisfy the gate.**
- The rules collide:
  - `:53-55` and `:63-64` require the review transcript to be committed, and a missing part blocks a self-merge.
  - `:109` says any head change needs both lineages to review again.
  - `:112` says to merge the reviewed SHA.
- Committing `05-final` and the updated ledger (planned in `README.md:7-8`, already claimed at `DECISIONS:163-165`) creates a head nobody reviewed. Leaving them out is a missing part. Every self-merge hits this, starting with #18.
- The PR body has the same problem: it is a decision-record location (`:45`) that the SHA review does not cover, and it is out of date.
  - It says "Flex: review rounds per tier, and transcript trimming, can change at a planning pause", which contradicts the owner-only FLEX at `:169-171`.
  - It says "findings fixed or rejected with reasons", which contradicts `:117-121`.
  - It paraphrases the directive instead of quoting it, and it lacks "who wrote what / who reviewed it" (`:126`).
- Fix:
  - Allow one commit after the review that touches only `docs/jeiobs/debates/<pr>/`, checked with `git diff --name-only <reviewed> <new>`, then merge with `--match-head-commit <new>`.
  - Draft the final PR body before the last round and give it to the peers.

**claude-3 (Major, new): `review.sh` breaks `DECISIONS:105`.**
- `review.sh:97-100` always runs Claude first and feeds Claude's round-1 reply into Codex's round-1 prompt.
- For code Claude wrote (e.g. claude=implementor above), the author's lineage reviews first and the other lineage starts from its view. That contradicts `:105` and the "independent reviewers" preamble (`review.sh:58`).
- Fix: add `-a <author>` so the non-author goes first, with no relay in round 1.

**claude-4 (Minor, new): smaller `review.sh` problems.**
- The trap at `:70` deletes `$TMP/err` and `$TMP/log`, so a failed run keeps only the exit code.
- `00-meta.md` does not record CLI versions, models or the full SHA.
- `:81` says "raw replies in $OUT", but in a new run the earlier rounds' replies are not there.

**claude-5 (Minor, new): record accuracy.**
- `DECISIONS:23-28` does not say which "error in one of the commits" met the owner's condition. That identification is the lead's reading, backed by run 43805282753. The ledger's "CI | Error | owner" row uses a severity outside the one scale and credits the owner with a finding the lead made.
- 018 `README.md:3` says the prompts are verbatim, but round 2's per-turn prompts (script preamble plus relayed reply) were not kept. Disclose this as `:185-186` does for two other prompts.
- The ledger says "verbatim or close quote" where `:114-115` requires verbatim.
- The ledger marks k6 withdrawn, but my round 2 kept two Minor leftovers open (`04-r2-crosstalk.md:117`).

**My ledger items, on the one scale**
- k4: `--match-head-commit` at `:112` and identical trees close the merge side. The review side is now claude-1.
- k8: the lead's lineage is fixed, but "no memory" and "pinned by SHA" (`:34-36`) are still untrue.
- k10: this predates the PR and is tracked only in this ledger. Record it in HANDOFF or an issue.
- N5: I suggest the lead marks its own GitHub comments and never removes `owner-hold`, since both act under the same account (`:172-173`).

FINDING k1 | Major | withdrawn | finder-only closure holds (DECISIONS:117-124)
FINDING k2 | Major | withdrawn | tests must pass or the owner accepts (:95-98)
FINDING k3 | Major | withdrawn | owner-only classes and owner-only FLEX (:70-88, :169-171)
FINDING k4 | Minor | accepted-fix | merge pinned by --match-head-commit (:112); 37/37 rewrite trees identical; review side moved to claude-1
FINDING k5 | Minor | accepted-fix | verbatim quotes, terms and merge marker in DECISIONS (:10-41, :128-130); PR body covered by claude-2
FINDING k6 | Minor | accepted-fix | CONTRIBUTING:29 now says owner; the lead's own PR is defined (:40-41)
FINDING k7 | Minor | withdrawn | withdrawn in r2 (04-r2-crosstalk.md:118)
FINDING k8 | Minor | open | :34-36 "no memory" and "pinned by SHA" still untrue for Claude Code peers; see claude-1
FINDING k9 | Minor | accepted-fix | READMEs in the 003, 004 and 018 folders point to the SHA table
FINDING k10 | Minor | open | predates this PR, non-blocking; record it in HANDOFF or an issue so it outlives this ledger
FINDING M1 | Major | accepted-fix | third-party pins, submodules and deps/ are owner-only (:78-82)
FINDING M2 | Major | accepted-fix | FINDING lines, finder-only closure, own-reply relay; leftovers in claude-4/claude-5
FINDING M3 | Major | accepted-fix | 01-04 committed and match their sources; the final round is claude-2
FINDING N1 | Major | accepted-fix | owner instruction recorded (:23-24); this run reviews ac008df4c; trees identical
FINDING N2 | Major | open | Claude peer not read-only (--allowedTools ≠ --tools, auto mode); the out-of-repo script with "orchestrator makes the final call" still runs decision debates
FINDING N3 | Major | accepted-fix | admin actions owner-only (:85-88); quotes in DECISIONS and the history note; AGENTS:10-11
FINDING N4 | Major | accepted-fix | owner decides first, then the PR can be self-merged (:90-94)
FINDING N5 | Minor | accepted-fix | owner-hold rule (:70-71); suggest marking lead comments on the shared account
FINDING claude-1 | Major | open | review.sh never checks that the peers' tree is SHA ("-s" is only printed, :13/:79); run them in a detached worktree
FINDING claude-2 | Major | open | committing the final transcript or ledger changes the head (:109/:112), but omitting them blocks (:63-64); the PR body is out of date and not reviewed
FINDING claude-3 | Major | open | fixed Claude-first order and a round-1 relay (review.sh:97-100) break :105 for Claude-written code
FINDING claude-4 | Minor | open | failure logs deleted (:70), no CLI versions/models/full SHA in meta, wrong pointer to raw replies (:81)
FINDING claude-5 | Minor | open | which "error" met the owner's condition is not recorded; round-2 prompts not disclosed; ledger not verbatim; k6 status misstated
VERDICT: request changes
