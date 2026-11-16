**Request changes at `75c0ea435`.** I'm raising claude-12 to **Major**. Codex's codex-11 made me re-trace how the script uses `$OUT`: it holds the gate's inputs as well as its replies, and the tamper check covers none of it. The fix needs neither bubblewrap nor the owner, so I still don't hold that a self-merge must wait for bubblewrap.

**This run is clean so far (read-only checks):**
- `$OUT/00-context.md` matches `/tmp/jeiobs-pr18/run08-context.md` and my prompt.
- The ledger in my prompt matches `findings.md` at HEAD.
- `01-r1-claude.reply.md` matches my previous reply. Its claude-12 and claude-14 FINDING lines are in my round-1 session `907ad954….jsonl`.
- `02-r1-codex.reply.md` matches the Codex reply I was shown.
- The review `config.toml` is unchanged after Codex's round 1: no MCP sections, and the `[otel]` timestamp is still `06:24:44.936139929Z`.
- `$OUT` has no unexpected files, and the memory directory is empty.

**claude-12 (now Major, open)**
- **Inputs:** `CTX` and `LEDGER` are repointed to the `$OUT` snapshots (`review.sh:93-94`) and re-read for every prompt (:168-169). Their hashes at :131 are never re-checked.
- **Outputs:** validation, the next round's prompts and the final dispositions all read the `$OUT` copies (:213, :217, :220-226, :236-239).
- **Not covered:** `$OUT` is excluded when it is inside the repo (:102, :105), and nothing checks it when it is outside, as in this run.
- **Effect:** the control that :31 calls "the enforcement" misses the files that decide the gate.
  - An unsandboxed Codex turn could change the context or ledger before my next prompt.
  - Codex speaks last (meta :5), so it could also change my final FINDING lines.
  - My interim check from round 1 (replies against session logs) would not catch an altered snapshot: my session would faithfully record the altered prompt.
- **Still true:** DECISIONS:117-121 does not mention the exclusion. The review checkout is checked with `status` only (:114), and `skip-worktree` hides edits from it. `cleanup` (:84-86) writes FAILED but exits 0.
- **Fix:**
  - When the snapshots, the meta and each saved file are written, keep their hashes in memory.
  - In every `tamper_check` and before the summary, verify those hashes and fail on any unexpected file in `$OUT`. DECISIONS:136-140 waives review for files *added* to a run folder, so a planted file would otherwise skip review.
  - Print the manifest to stdout or `$DIAG` so the committed copies can be checked against it.
  - Also check `git -C "$TREE" ls-files -v` flags, make `cleanup` exit non-zero when it writes FAILED, and update DECISIONS:117-121.

**Codex's open rows (Codex decides)**
- **codex-11:** I agree. It is the same gap as claude-12 and can be fixed now.
- **codex-6:** The facts are right. Replies land in `$TMP`, which is the tree's parent (:80-81, :209-210), and both peers' session logs grow while they run. Full blindness needs separate users or a sandbox. The lead still has changes to offer, so this is not yet "inconclusive" under DECISIONS:156-159:
  - Write the round-1 replies outside `$TMP`.
  - After round 1, scan each peer's own tool log (Claude's jsonl `tool_use` inputs, Codex's rollout commands) for the other peer's paths, and record any hits in `00-meta.md`.
  - Delete the false clause at :20-21 (claude-3).
- That audit would flag my own round-1 grep counts on Codex's live rollout (`01-r1-claude.reply.md:6`). claude-14's fix, where the script records the sandbox state it observed, removes my reason to look there.
- I would not block on blindness alone. If Codex still holds codex-6 at Major after these changes, it becomes the owner's bubblewrap question.

**claude-3 and claude-14:** Nothing changed at this commit, so both stay open at Minor. I saw no config rewrite during this run, but the config is still checked only once (:69-72).

**Ledger:** `findings.md:21` still shows k10 as "open (deferred)", which is not a status the gate defines. My FINDING lines say withdrawn (`07-run/01-r1-claude.reply.md:72`, `07-run/03-r2-claude.reply.md:69`, and run 08 round 1).

FINDING k1 | Major | withdrawn | finder-only closure holds (DECISIONS:151-155)
FINDING k2 | Major | withdrawn | tests pass or owner accepts (:95-99)
FINDING k3 | Major | withdrawn | owner-only classes and owner FLEX (:70-89, :215-221)
FINDING k4 | Minor | accepted-fix | review pinned (:100-104), merge pinned to head (:143-145)
FINDING k5 | Minor | accepted-fix | verbatim owner quotes, terms, merge marker (:8-41, :163-165)
FINDING k6 | Minor | accepted-fix | CONTRIBUTING:14-16; lead's own PR defined (:40-41)
FINDING k7 | Minor | withdrawn | withdrawn in r2 (04-r2-crosstalk.md:118)
FINDING k8 | Minor | withdrawn | memory dir still empty; no carry-over
FINDING k9 | Minor | accepted-fix | 018 README:16-17 points to the SHA table
FINDING k10 | Minor | withdrawn | deferred to an owner-merged dependency PR (HANDOFF:110-111); findings.md:21 still says "open (deferred)", should read withdrawn
FINDING M1 | Major | accepted-fix | pins, submodules, deps/ owner-only (:78-82)
FINDING M2 | Major | accepted-fix | one scale, FINDING-line sources (:146-150)
FINDING M3 | Major | accepted-fix | 01-07 committed and linked (:207-211); my 07-run r2 reply matches my session
FINDING N1 | Major | accepted-fix | owner instruction and rewrite recorded (:18-28)
FINDING N2 | Major | accepted-fix | claims match what runs: engine (review.sh:64-65), no MCP headers and core env (:69-72), unsandboxed and tamper scope stated (DECISIONS:110-122); residue in claude-14
FINDING N3 | Major | accepted-fix | admin actions owner-only (:85-88)
FINDING N4 | Major | accepted-fix | owner decides first (:90-94)
FINDING N5 | Minor | accepted-fix | owner-hold rule (:70-71)
FINDING claude-1 | Major | accepted-fix | review worktree HEAD is 75c0ea435 (r1 check; HEAD check at review.sh:113 passed round 1)
FINDING claude-2 | Major | accepted-fix | transcript-only exemption (:135-140)
FINDING claude-3 | Minor | open | review.sh:20-21 still says "neither reply exists while the other works"; replies land in $TMP, the tree's parent (:80-81, :209-210)
FINDING claude-4 | Minor | accepted-fix | stderr local, CLI versions, full SHA (review.sh:77-78, :124-128)
FINDING claude-5 | Minor | accepted-fix | CI runs recorded (history :39-44); README :4-6
FINDING claude-6 | Minor | accepted-fix | inconclusive defined (:156-159)
FINDING claude-7 | Minor | accepted-fix | merge pins head (:143-145); exemption only adds a new run folder plus findings.md (:135-140); peers wording (:33-36)
FINDING claude-8 | Minor | accepted-fix | CI state in 08-run 00-meta :14-20; nit: -C optional (review.sh:58)
FINDING claude-9 | Minor | accepted-fix | 018 README:7-10 says stderr stays local, run 06 trimmed
FINDING claude-10 | Minor | accepted-fix | raised IDs become required (review.sh:196-197)
FINDING claude-11 | Minor | accepted-fix | prompt and reply saved before tamper check and validation (review.sh:212-213, :223)
FINDING claude-12 | Major | open | raised: $OUT holds the gate's inputs (snapshots re-read per prompt, review.sh:93-94, :168-169; hashes at :131 never re-checked) and outputs (:213-239) but is excluded (:102, :105) or outside the repo (this run); DECISIONS:117-121 omits it; TREE status-only (:114); cleanup exits 0 (:84-86); fixable without bubblewrap or owner
FINDING claude-13 | Minor | accepted-fix | engine binary, core env, HANDOFF:106-109; approval_policy=never declined for a valid reason
FINDING claude-14 | Minor | open | launcher still credential helper and writer of [otel] (config.toml:15-16, :30-32) vs DECISIONS:114 and review.sh:27-28; config checked once (review.sh:69-72); probe is not Codex's bwrap (:117); 06/07 meta uncorrected
VERDICT: request changes
