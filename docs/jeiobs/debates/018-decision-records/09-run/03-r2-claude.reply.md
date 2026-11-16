**Request changes at `bde9219b6`.** This is the same commit as round 1, so my round-1 findings all stand. I'm adding staged prompts to claude-15 and raising one new Major, claude-16: the reviewed commit's own Claude project settings get loaded into the Claude reviewer. I confirm Codex's codex-14.

**Checks (read-only):**
- My saved `01-r1-claude.reply.md` and the "own previous reply" in my prompt match my round-1 session: every FINDING line is the same as `~/.claude/projects/-tmp-tmp-a7NRbWvLpQ-tree/abcaad8c…jsonl:242`. So nothing edited my reply in this run's round-1 window. That doesn't close the window.
- `02-r1-codex.reply.md` matches the Codex reply in my prompt.
- The tree HEAD is still `bde9219b6`.

**claude-15 (Major, open): one addition, prompts.**
- `prompt()` writes `$TMP/<peer>-rN.prompt` (:184-192). The file is copied after the turn (:220) and pinned only then (:226).
- So the committed prompt can differ from what the peer actually read. `claude-r1.prompt`, `codex-r1.prompt` and `claude-r2.prompt` are sitting in `/tmp/tmp.a7NRbWvLpQ/` now.
- Pinning inside `prompt()` would not work, because it runs in `$(...)` (:231, :245) and the pin would be lost with the subshell. Pin in the parent shell.
- Round-1 closure options (a) wording and (b) code are unchanged. Each now needs to cover this point too.

**claude-16 (new, Major).** :197-198 runs `claude -p` in the reviewed checkout without `--setting-sources`.
- The installed CLI's `-p` help (2.1.325.1734) says: "The workspace trust dialog is skipped when Claude is run in non-interactive mode … Only use this in directories you trust."
- So if a reviewed commit tracks `.claude/settings.json`, its hooks run in the reviewer's session. They can run shell commands. `SessionStart` and `UserPromptSubmit` hooks can also add context that never appears in the committed prompt.
- A hook could also rewrite Codex's config before Codex's turn. That config is only checked at start (:72-76; see claude-14).
- `.claude/` and `CLAUDE.md` are not owner-only (DECISIONS:72-88). A self-mergeable PR can therefore steer its own review.
- review.sh:25 and DECISIONS:111-112 ("exactly as enforced") don't mention this.
- The reviewed commit's `AGENTS.md` is also loaded as overriding project instructions: it is in this session's context.
- `bde9219b6` tracks no `.claude/`, `CLAUDE.md`, `.mcp.json` or `.codex/` (checked with Glob), so this run is unaffected.
- **Closes with** a line in DECISIONS:111-116 saying the reviewed commit's `AGENTS.md`/`CLAUDE.md` reach the peers as instructions, plus either:
  - `--setting-sources user` on :197-198 (the flag exists: "Comma-separated list of setting sources to load (user, project, local)"), or
  - failing the run when `git -C "$TREE" ls-files -- .claude CLAUDE.md` is non-empty.

**Codex's findings (Codex decides):**
- **codex-14: confirmed.**
  - `pin` and `verify_evidence` both reduce a missing file to an empty string (:104, :108), so a missing file counts as a match.
  - A prompt deleted from `$TMP` before `save()` (:220) is just absent from OUTDIR, and the run still passes.
  - Replies are covered by `touch` plus the `-s` check (:235, :247, :206).
  - The `cp` at :101 is unchecked too.
  - Root fix: make `pin` and `verify_evidence` fail on a missing file, and use `cp … || fail`.
- **codex-6:**
  - The wording half is the same as claude-3 and can be fixed now (:144, :191, :229).
  - Enforced independence needs an OS-level boundary, because the same user can read both lineages' session stores. I just read my own. That half is the owner's call.

**Unchanged:** claude-3, claude-14 and k10 stand as in round 1. findings.md:21 still lists k10 as "open (deferred)". My 08-run line says withdrawn (08-run/03-r2-claude.reply.md:48).

FINDING k1 | Major | withdrawn | finder-only closure holds (DECISIONS:156-160)
FINDING k2 | Major | withdrawn | tests pass or owner accepts (:95-99)
FINDING k3 | Major | withdrawn | owner-only classes and owner FLEX (:70-89, :220-226)
FINDING k4 | Minor | accepted-fix | review pinned (:100-104), merge pinned to head (:148-150)
FINDING k5 | Minor | accepted-fix | verbatim owner quotes, terms, merge marker (:8-41, :168-170)
FINDING k6 | Minor | accepted-fix | CONTRIBUTING:14-16; lead's own PR defined (:40-41)
FINDING k7 | Minor | withdrawn | withdrawn in r2 (04-r2-crosstalk.md:118)
FINDING k8 | Minor | withdrawn | memory dir still empty; no carry-over
FINDING k9 | Minor | accepted-fix | 018 README:16-17 points to the SHA table
FINDING k10 | Minor | withdrawn | deferred to an owner-merged dependency PR (HANDOFF:110-111); findings.md:21 should read withdrawn (08-run/03-r2-claude.reply.md:48)
FINDING M1 | Major | accepted-fix | pins, submodules, deps/ owner-only (:78-82)
FINDING M2 | Major | accepted-fix | one scale, FINDING-line sources (:151-155)
FINDING M3 | Major | accepted-fix | 01-08 committed and linked (:212-216); my 08-run r2 lines are in session 7607702b
FINDING N1 | Major | accepted-fix | owner instruction and rewrite recorded (:18-28)
FINDING N2 | Major | accepted-fix | engine (review.sh:68-69), no MCP sections and core env (:73-76), unsandboxed stated (DECISIONS:117-118); residue in claude-14, claude-15, claude-16
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
FINDING claude-15 | Major | open | "so a peer cannot alter the inputs or evidence" (review.sh:40; DECISIONS:126) is false: round-1 reply pinned only after both finish (:232-237), staged prompts pinned only after the turn (:184-192, :220, :226), verify-first and re-read after last check (:129, :260, :263; cleanup :89-90), meta unpinned (:139-153), OUTDIR extras unchecked (:119/:122; DECISIONS:141-145), checkout index flags (:131-132); closes with exact wording or the code fixes
FINDING claude-16 | Major | open | review.sh:197-198 runs claude -p in the reviewed checkout without --setting-sources and -p skips workspace trust (CLI help), so a tracked .claude/settings.json (not owner-only, DECISIONS:72-88) runs hooks and injects context absent from the committed prompt; undisclosed (review.sh:25; DECISIONS:111-112); closes with --setting-sources user or a fail on tracked .claude/CLAUDE.md, plus a line that AGENTS.md reaches the peers
VERDICT: request changes
