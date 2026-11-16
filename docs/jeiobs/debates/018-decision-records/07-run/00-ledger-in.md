# Findings ledger — PR #18

Kept by the lead under DECISIONS gate item 4. Severity and status are each
finder's latest, taken from its own FINDING lines in the last review run (06-run) (the
"source" column points at them). Findings first raised in rounds 1–2 are quoted in
`01`, `02` and
`04`; later runs are in `05-final` and `06-run`. "open → fixed"
means the lead changed something and the finder has not yet accepted it.

ID | severity | status | finder | source | lead's note
--- | --- | --- | --- | --- | ---
k1 | Major | withdrawn | claude | 06-run/03-r2-claude.reply.md (FINDING k1) | —
k2 | Major | withdrawn | claude | 06-run/03-r2-claude.reply.md (FINDING k2) | —
k3 | Major | withdrawn | claude | 06-run/03-r2-claude.reply.md (FINDING k3) | —
k4 | Minor | accepted-fix | claude | 06-run/03-r2-claude.reply.md (FINDING k4) | —
k5 | Minor | accepted-fix | claude | 06-run/03-r2-claude.reply.md (FINDING k5) | —
k6 | Minor | accepted-fix | claude | 06-run/03-r2-claude.reply.md (FINDING k6) | —
k7 | Minor | withdrawn | claude | 06-run/03-r2-claude.reply.md (FINDING k7) | —
k8 | Minor | withdrawn | claude | 06-run/03-r2-claude.reply.md (FINDING k8) | —
k9 | Minor | accepted-fix | claude | 06-run/03-r2-claude.reply.md (FINDING k9) | —
k10 | Minor | open (deferred) | claude | 06-run/03-r2-claude.reply.md (FINDING k10) | deferred: dependency change (owner-only class); in HANDOFF
M1 | Major | accepted-fix | claude | 06-run/03-r2-claude.reply.md (FINDING M1) | —
M2 | Major | accepted-fix | claude | 06-run/03-r2-claude.reply.md (FINDING M2) | —
M3 | Major | accepted-fix | claude | 06-run/03-r2-claude.reply.md (FINDING M3) | —
N1 | Major | accepted-fix | claude | 06-run/03-r2-claude.reply.md (FINDING N1) | —
N2 | Major | open → fixed (awaiting finder) | claude | 06-run/03-r2-claude.reply.md (FINDING N2) | review.sh v3: real Codex binary + CODEX_HOME without MCP/plugins/skills; sandbox availability recorded; tamper check (lead repo incl. ignored files, review checkout) after every turn; owner asked to enable bwrap
N3 | Major | accepted-fix | claude | 06-run/03-r2-claude.reply.md (FINDING N3) | —
N4 | Major | accepted-fix | claude | 06-run/03-r2-claude.reply.md (FINDING N4) | —
N5 | Minor | accepted-fix | claude | 06-run/03-r2-claude.reply.md (FINDING N5) | —
claude-1 | Major | accepted-fix | claude | 06-run/03-r2-claude.reply.md (FINDING claude-1) | —
claude-2 | Major | accepted-fix | claude | 06-run/03-r2-claude.reply.md (FINDING claude-2) | —
claude-3 | Major | open → fixed (awaiting finder) | claude | 06-run/03-r2-claude.reply.md (FINDING claude-3) | round 1 runs both peers at the same time and copies replies afterwards; OUT path not given in round 1
claude-4 | Minor | accepted-fix | claude | 06-run/03-r2-claude.reply.md (FINDING claude-4) | —
claude-5 | Minor | accepted-fix | claude | 06-run/03-r2-claude.reply.md (FINDING claude-5) | —
claude-6 | Minor | accepted-fix | claude | 06-run/03-r2-claude.reply.md (FINDING claude-6) | —
claude-7 | Minor | open → fixed (awaiting finder) | claude | 06-run/03-r2-claude.reply.md (FINDING claude-7) | as codex-7; ledger updates included in the exemption
claude-8 | Minor | open → fixed (awaiting finder) | claude | 06-run/03-r2-claude.reply.md (FINDING claude-8) | -C CI summary copied into 00-meta (gate item 2)
claude-9 | Major | open → fixed (awaiting finder) | claude | 06-run/03-r2-claude.reply.md (FINDING claude-9) | stderr logs no longer committed; kept in ~/.cache/jeiobs-review; run 06 stderr removed from the folder
claude-10 | Minor | open → fixed (awaiting finder) | claude | 06-run/03-r2-claude.reply.md (FINDING claude-10) | IDs raised in a round must be disposed of in later rounds
c1 | Blocker | withdrawn | codex | 06-run/04-r2-codex.reply.md (FINDING c1) | —
c2 | Major | withdrawn | codex | 06-run/04-r2-codex.reply.md (FINDING c2) | —
c3 | Major | withdrawn | codex | 06-run/04-r2-codex.reply.md (FINDING c3) | —
c4 | Major | withdrawn | codex | 06-run/04-r2-codex.reply.md (FINDING c4) | —
c5 | Minor | accepted-fix | codex | 06-run/04-r2-codex.reply.md (FINDING c5) | —
c6 | Major | withdrawn | codex | 06-run/04-r2-codex.reply.md (FINDING c6) | —
c7 | Minor | accepted-fix | codex | 06-run/04-r2-codex.reply.md (FINDING c7) | —
c8 | Minor | withdrawn | codex | 06-run/04-r2-codex.reply.md (FINDING c8) | —
M1 | Major | accepted-fix | codex | 06-run/04-r2-codex.reply.md (FINDING M1) | —
M2 | Major | accepted-fix | codex | 06-run/04-r2-codex.reply.md (FINDING M2) | —
M3 | Major | accepted-fix | codex | 06-run/04-r2-codex.reply.md (FINDING M3) | —
N1 | Major | accepted-fix | codex | 06-run/04-r2-codex.reply.md (FINDING N1) | —
N2 | Major | open → fixed (awaiting finder) | codex | 06-run/04-r2-codex.reply.md (FINDING N2) | review.sh v3: real Codex binary + CODEX_HOME without MCP/plugins/skills; sandbox availability recorded; tamper check (lead repo incl. ignored files, review checkout) after every turn; owner asked to enable bwrap
N3 | Major | accepted-fix | codex | 06-run/04-r2-codex.reply.md (FINDING N3) | —
N4 | Major | accepted-fix | codex | 06-run/04-r2-codex.reply.md (FINDING N4) | —
N5 | Minor | accepted-fix | codex | 06-run/04-r2-codex.reply.md (FINDING N5) | —
N6 | Major | accepted-fix | codex | 06-run/04-r2-codex.reply.md (FINDING N6) | —
N7 | Major | accepted-fix | codex | 06-run/04-r2-codex.reply.md (FINDING N7) | —
codex-1 | Major | accepted-fix | codex | 06-run/04-r2-codex.reply.md (FINDING codex-1) | —
codex-2 | Major | accepted-fix | codex | 06-run/04-r2-codex.reply.md (FINDING codex-2) | —
codex-3 | Major | open → fixed (awaiting finder) | codex | 06-run/04-r2-codex.reply.md (FINDING codex-3) | -a author; the other lineage speaks first (lead -> claude)
codex-4 | Minor | accepted-fix | codex | 06-run/04-r2-codex.reply.md (FINDING codex-4) | —
codex-5 | Major | accepted-fix | codex | 06-run/04-r2-codex.reply.md (FINDING codex-5) | —
codex-6 | Major | open → fixed (awaiting finder) | codex | 06-run/04-r2-codex.reply.md (FINDING codex-6) | as claude-3
codex-7 | Major | open → fixed (awaiting finder) | codex | 06-run/04-r2-codex.reply.md (FINDING codex-7) | exemption checked with git diff --name-status, limited to the PR's debates folder (adds and updates)
codex-8 | Minor | open → fixed (awaiting finder) | codex | 06-run/04-r2-codex.reply.md (FINDING codex-8) | merge pins the new head (the transcript commit); 00-meta names the reviewed SHA
codex-9 | Major | open → fixed (awaiting finder) | codex | 06-run/04-r2-codex.reply.md (FINDING codex-9) | context and ledger snapshotted into OUT at start; prompts read the snapshots
codex-10 | Major | open → fixed (awaiting finder) | codex | 06-run/04-r2-codex.reply.md (FINDING codex-10) | FINDING syntax validated (ID, one scale, one of three statuses, reason)
CI | Major | fixed | owner | owner instruction (DECISIONS.md, verbatim) | quick checks failed after the force push (runs 43805282657, 43805282753, "bad object"); workflow falls back to HEAD
