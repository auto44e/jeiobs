# Findings ledger — PR #18

Kept by the lead under DECISIONS gate item 4. Severity and status are each
finder's latest, taken from its own FINDING lines in the last review run (the
"source" column points at them). Findings first raised in rounds 1–2 are quoted in
`01`, `02` and
`04`; the final run is in `05-final`. "open → fixed"
means the lead changed something and the finder has not yet accepted it.

ID | severity | status | finder | source | lead's note
--- | --- | --- | --- | --- | ---
k1 | Major | withdrawn | claude | 05-final/03-r2-claude.reply.md (FINDING k1) | —
k2 | Major | withdrawn | claude | 05-final/03-r2-claude.reply.md (FINDING k2) | —
k3 | Major | withdrawn | claude | 05-final/03-r2-claude.reply.md (FINDING k3) | —
k4 | Minor | accepted-fix | claude | 05-final/03-r2-claude.reply.md (FINDING k4) | —
k5 | Minor | accepted-fix | claude | 05-final/03-r2-claude.reply.md (FINDING k5) | —
k6 | Minor | accepted-fix | claude | 05-final/03-r2-claude.reply.md (FINDING k6) | —
k7 | Minor | withdrawn | claude | 05-final/03-r2-claude.reply.md (FINDING k7) | —
k8 | Minor | withdrawn | claude | 05-final/03-r2-claude.reply.md (FINDING k8) | —
k9 | Minor | accepted-fix | claude | 05-final/03-r2-claude.reply.md (FINDING k9) | —
k10 | Minor | open → fixed (awaiting finder) | claude | 05-final/03-r2-claude.reply.md (FINDING k10) | deferred: dependency change (owner-only class); tracked in HANDOFF next steps
M1 | Major | accepted-fix | claude | 05-final/03-r2-claude.reply.md (FINDING M1) | —
M2 | Major | accepted-fix | claude | 05-final/03-r2-claude.reply.md (FINDING M2) | ledger rebuilt: finder severities, pointers to the finder's FINDING line, one scale; DECISIONS item 4
M3 | Major | accepted-fix | claude | 05-final/03-r2-claude.reply.md (FINDING M3) | —
N1 | Major | accepted-fix | claude | 05-final/03-r2-claude.reply.md (FINDING N1) | —
N2 | Major | open → fixed (awaiting finder) | claude | 05-final/03-r2-claude.reply.md (FINDING N2) | review.sh: Claude --tools Read,Glob,Grep + --strict-mcp-config; Codex -c mcp_servers={}; consults vs gate reviews defined in item 3
N3 | Major | accepted-fix | claude | 05-final/03-r2-claude.reply.md (FINDING N3) | —
N4 | Major | accepted-fix | claude | 05-final/03-r2-claude.reply.md (FINDING N4) | —
N5 | Minor | accepted-fix | claude | 05-final/03-r2-claude.reply.md (FINDING N5) | —
claude-1 | Major | open → fixed (awaiting finder) | claude | 05-final/03-r2-claude.reply.md (FINDING claude-1) | review.sh checks out the reviewed SHA in a detached worktree and runs both peers there
claude-2 | Major | open → fixed (awaiting finder) | claude | 05-final/03-r2-claude.reply.md (FINDING claude-2) | DECISIONS item 3: a transcript-only commit under debates/<pr>/ needs no re-review (git diff --name-only check); PR body refreshed
claude-3 | Major | open → fixed (awaiting finder) | claude | 05-final/03-r2-claude.reply.md (FINDING claude-3) | review.sh: round 1 blind; -f picks the first cross-talk speaker (the non-author lineage)
claude-4 | Minor | open → fixed (awaiting finder) | claude | 05-final/03-r2-claude.reply.md (FINDING claude-4) | review.sh keeps per-turn stderr; meta records CLI versions, models, full SHA
claude-5 | Minor | open → fixed (awaiting finder) | claude | 05-final/03-r2-claude.reply.md (FINDING claude-5) | CI failure runs 43805282657 and 43805282753 recorded in the history note; README discloses that round 2 used the helper (no per-turn prompts)
claude-6 | Minor | open → fixed (awaiting finder) | claude | 05-final/03-r2-claude.reply.md (FINDING claude-6) | DECISIONS item 4: inconclusive = two consecutive review runs with the finding still open and no further change
c1 | Blocker | withdrawn | codex | 05-final/04-r2-codex.reply.md (FINDING c1) | —
c2 | Major | withdrawn | codex | 05-final/04-r2-codex.reply.md (FINDING c2) | —
c3 | Major | withdrawn | codex | 05-final/04-r2-codex.reply.md (FINDING c3) | —
c4 | Major | withdrawn | codex | 05-final/04-r2-codex.reply.md (FINDING c4) | —
c5 | Minor | accepted-fix | codex | 05-final/04-r2-codex.reply.md (FINDING c5) | —
c6 | Major | withdrawn | codex | 05-final/04-r2-codex.reply.md (FINDING c6) | —
c7 | Minor | accepted-fix | codex | 05-final/04-r2-codex.reply.md (FINDING c7) | —
c8 | Minor | withdrawn | codex | 05-final/04-r2-codex.reply.md (FINDING c8) | —
M1 | Major | accepted-fix | codex | 05-final/04-r2-codex.reply.md (FINDING M1) | —
M2 | Major | open → fixed (awaiting finder) | codex | 05-final/04-r2-codex.reply.md (FINDING M2) | ledger rebuilt: finder severities, pointers to the finder's FINDING line, one scale; DECISIONS item 4
M3 | Major | accepted-fix | codex | 05-final/04-r2-codex.reply.md (FINDING M3) | —
N1 | Major | accepted-fix | codex | 05-final/04-r2-codex.reply.md (FINDING N1) | —
N2 | Major | open → fixed (awaiting finder) | codex | 05-final/04-r2-codex.reply.md (FINDING N2) | review.sh: Claude --tools Read,Glob,Grep + --strict-mcp-config; Codex -c mcp_servers={}; consults vs gate reviews defined in item 3
N3 | Major | accepted-fix | codex | 05-final/04-r2-codex.reply.md (FINDING N3) | —
N4 | Major | accepted-fix | codex | 05-final/04-r2-codex.reply.md (FINDING N4) | —
N5 | Minor | accepted-fix | codex | 05-final/04-r2-codex.reply.md (FINDING N5) | —
N6 | Major | accepted-fix | codex | 05-final/04-r2-codex.reply.md (FINDING N6) | —
N7 | Major | accepted-fix | codex | 05-final/04-r2-codex.reply.md (FINDING N7) | —
codex-1 | Major | open → fixed (awaiting finder) | codex | 05-final/04-r2-codex.reply.md (FINDING codex-1) | as claude-1
codex-2 | Major | open → fixed (awaiting finder) | codex | 05-final/04-r2-codex.reply.md (FINDING codex-2) | as claude-2; 05-final committed
codex-3 | Major | open → fixed (awaiting finder) | codex | 05-final/04-r2-codex.reply.md (FINDING codex-3) | as claude-3
codex-4 | Minor | open → fixed (awaiting finder) | codex | 05-final/04-r2-codex.reply.md (FINDING codex-4) | as claude-5 (README)
codex-5 | Major | open → fixed (awaiting finder) | codex | 05-final/04-r2-codex.reply.md (FINDING codex-5) | review.sh fails unless a FINDING line exists for every ledger row the peer owns
CI | Major | fixed | owner | owner instruction (DECISIONS.md, verbatim) | quick checks failed after the force push (runs 43805282657, 43805282753, "bad object"); workflow falls back to HEAD
