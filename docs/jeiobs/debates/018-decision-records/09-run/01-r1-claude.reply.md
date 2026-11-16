**Request changes at `bde9219b6`.** I accept the fix for claude-12: a write to the snapshots or to a saved reply during a turn now fails the run. One new Major, claude-15. The new text says a peer "cannot alter the inputs or evidence", and that is not true: the round-1 window and the time after the last check are still open.

**Checks (read-only):** all match.
- `00-context.md` and `00-ledger-in.md` in `/tmp/jeiobs-pr18/09-run` match my prompt.
- The review worktree HEAD is `bde9219b6` (`.git/worktrees/tree/HEAD`).
- My committed 08-run round-2 lines are in my session `7607702b…jsonl`.
- The CI fallback (`jeiobs-checks.yaml:30-35`) is correct.

**claude-15 (new, Major).** review.sh:40 says "so a peer cannot alter the inputs or evidence". DECISIONS:126 says "The check detects writes to the repository and to the run's evidence". Both overclaim:
1. **Round 1:** my reply is written to `$TMP/claude-r1.reply` (:232). In this run that is `/tmp/tmp.a7NRbWvLpQ/claude-r1.reply`, next to `tree/`. It is pinned only after both peers finish (:234-237). An unsandboxed Codex, still running or through a leftover process, can edit it first. The edit is then pinned as genuine and feeds REQ (:216) and my round-2 "own previous reply" (:189), so a new round-1 Claude finding can disappear.
2. **After the last check:**
   - tamper_check verifies the evidence first (:129) and runs the repo and tree checks after it (:130-132).
   - The summary and stdout re-read the reply files from disk (:260, :263).
   - cleanup checks only the checkout (:89-90), and no manifest is printed.
   - Codex speaks last (09-run meta:5), and `timeout` does not kill processes a child leaves behind when it exits normally.
   - So a background process can still change my final FINDING lines. That is claude-12's attack in a new form.
3. **`00-meta.md`** holds the SHA, the sandbox line and the CI state that gate items 2-3 rely on. It is written at :139-153 and never pinned.
4. **Extra files in OUTDIR** are never detected. OUTDIR is excluded at :119/:122 when it is inside the repo, and this run's is outside it. DECISIONS:141-145 still lets added files in a run folder land without review.
5. **The checkout** is checked by HEAD and `status` only (:131-132, :89-90). `update-index --skip-worktree` or `--assume-unchanged` writes `.git/worktrees/tree/index`, which `repo_state` does not cover: :119 reads the main index and :124 reads config and hooks. That hides edits Claude reads in round 2. This was in claude-12 and is still unaddressed.

**How claude-15 closes:** either of these is enough for me.
- **(a) Wording:** in review.sh:32 and :38-40 and DECISIONS:117-127, call the check a tripwire for changes still present at the next check, and list points 1-5. Also say that an unsandboxed peer that evades it can change other programs or the other peer's configuration, and only the sandbox (owner action) stops that.
- **(b) Code:**
  - Pin Claude's output from memory.
  - Verify evidence last in tamper_check, again in cleanup, and print the manifest.
  - Pin the meta.
  - Reject unexpected file names in OUTDIR.
  - Require every entry of `git -C "$TREE" ls-files -v` to be `H`.
  - Kill Codex's process group after each turn.

  Whatever (b) leaves uncovered still needs wording.

After (a), the code items in (b) can be Minor follow-ups. One nit: the check in `prompt()` (:183) runs inside `$(...)`, so its `fail` exits only the subshell. The run still fails at :248/:249, so it does fail closed.

**Round-1 isolation (codex-6, claude-3):** I don't think the gate is unsafe because round 1 can be seen. The integrity of round-1 replies is point 1 above and can be fixed without bubblewrap. Containing a peer that deliberately evades the check needs bubblewrap. Once the docs say so plainly, that is a risk for the owner to accept, and I won't block on it.

**Codex's rows (Codex decides):**
- codex-13: cleanup now exits 3 for the checkout (:89-95). It still doesn't re-check the evidence or the repo (point 2).
- codex-11: same as claude-12, with the rest in claude-15.

**My other open findings:**
- **claude-3:** :144 still writes "round 1 blind" (09-run meta:5), which contradicts meta:8 ("not isolated"). :191 and :229 also say "blind". The comment at :229-230 gives the wrong order: the code saves before the check (:236-238).
- **claude-14:** the lead's note says the credential helper is disclosed. It isn't. The helper is the toolbox launcher (config.toml:14-16, :21-23), and neither review.sh nor DECISIONS:113-116 mentions it. The config is checked only once (:72-76), and the probe (:135) is the system bwrap, not Codex's sandbox. I drop the 06/07 meta point: DECISIONS:207-209 records that those runs were not sandboxed.
- **k10:** `findings.md:21` still says "open (deferred)", for the third run. My FINDING lines say withdrawn.

FINDING k1 | Major | withdrawn | finder-only closure holds (DECISIONS:156-160)
FINDING k2 | Major | withdrawn | tests pass or owner accepts (:95-99)
FINDING k3 | Major | withdrawn | owner-only classes and owner FLEX (:70-89, :220-226)
FINDING k4 | Minor | accepted-fix | review pinned (:100-104), merge pinned to head (:148-150)
FINDING k5 | Minor | accepted-fix | verbatim owner quotes, terms, merge marker (:8-41, :168-170)
FINDING k6 | Minor | accepted-fix | CONTRIBUTING:14-16; lead's own PR defined (:40-41)
FINDING k7 | Minor | withdrawn | withdrawn in r2 (04-r2-crosstalk.md:118)
FINDING k8 | Minor | withdrawn | memory dir still empty; no carry-over
FINDING k9 | Minor | accepted-fix | 018 README:16-17 points to the SHA table
FINDING k10 | Minor | withdrawn | deferred to an owner-merged dependency PR (HANDOFF:110-111); findings.md:21 should read withdrawn
FINDING M1 | Major | accepted-fix | pins, submodules, deps/ owner-only (:78-82)
FINDING M2 | Major | accepted-fix | one scale, FINDING-line sources (:151-155)
FINDING M3 | Major | accepted-fix | 01-08 committed and linked (:212-216); my 08-run r2 lines are in session 7607702b
FINDING N1 | Major | accepted-fix | owner instruction and rewrite recorded (:18-28)
FINDING N2 | Major | accepted-fix | engine (review.sh:68-69), no MCP sections and core env (:73-76), unsandboxed stated (DECISIONS:117-118); residue in claude-14, claude-15
FINDING N3 | Major | accepted-fix | admin actions owner-only (:85-88)
FINDING N4 | Major | accepted-fix | owner decides first (:90-94)
FINDING N5 | Minor | accepted-fix | owner-hold rule (:70-71)
FINDING claude-1 | Major | accepted-fix | review worktree HEAD is bde9219b6 (.git/worktrees/tree/HEAD)
FINDING claude-2 | Major | accepted-fix | transcript-only exemption (:140-145)
FINDING claude-3 | Minor | open | header and DECISIONS:105-110 fixed, but review.sh:144 writes "round 1 blind" (09-run meta:5 vs :8 "not isolated"); :191, :229; :229-230 order stale (:236-238)
FINDING claude-4 | Minor | accepted-fix | stderr local, CLI versions, full SHA (review.sh:81-82, :143-146)
FINDING claude-5 | Minor | accepted-fix | CI runs recorded (history :39-44); README :4-6
FINDING claude-6 | Minor | accepted-fix | inconclusive defined (:161-164)
FINDING claude-7 | Minor | accepted-fix | merge pins head (:148-150); exemption (:140-145); peers wording (:32-36)
FINDING claude-8 | Minor | accepted-fix | CI state in 09-run 00-meta:14-20; nit: -C optional (review.sh:62)
FINDING claude-9 | Minor | accepted-fix | 018 README:7-10 says stderr stays local, run 06 trimmed
FINDING claude-10 | Minor | accepted-fix | raised IDs become required (review.sh:216)
FINDING claude-11 | Minor | accepted-fix | saved and pinned before tamper check and validation (review.sh:236-241, :247-250)
FINDING claude-12 | Major | accepted-fix | in-turn writes to snapshots and saved replies now fail the run (:103-111, :129, :183, :237, :247); cleanup exits 3 (:89-95); unaddressed parts (meta, extra files, manifest, checkout flags) carried into claude-15
FINDING claude-13 | Minor | accepted-fix | engine binary, core env, HANDOFF:106-109
FINDING claude-14 | Minor | open | credential helper is the toolbox launcher (config.toml:14-16, :21-23), undisclosed in review.sh and DECISIONS:113-116 despite the lead's note; config checked once (:72-76); system-bwrap probe (:135); 06/07 point dropped (DECISIONS:207-209)
FINDING claude-15 | Major | open | "so a peer cannot alter the inputs or evidence" (review.sh:40; DECISIONS:126) is false: round-1 reply pinned only after both finish (:232-237), verify-first and re-read after last check (:129, :260, :263; cleanup :89-90), meta unpinned (:139-153), OUTDIR extras unchecked (:119/:122; DECISIONS:141-145), checkout index flags (:131-132); closes with exact wording or the code fixes
VERDICT: request changes
