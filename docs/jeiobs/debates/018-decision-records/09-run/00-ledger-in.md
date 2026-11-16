# Findings ledger — PR #18

Kept by the lead under DECISIONS gate item 4. Severity and status are each
finder's latest, taken from its own FINDING lines in the last review run (08-run) (the
"source" column points at them). Findings first raised in rounds 1–2 are quoted in
`01`, `02` and
`04`; later runs are in `05-final` to `08-run`. "open → fixed"
means the lead changed something and the finder has not yet accepted it.

ID | severity | status | finder | source | lead's note
--- | --- | --- | --- | --- | ---
k1 | Major | withdrawn | claude | 08-run/03-r2-claude.reply.md (FINDING k1) | —
k2 | Major | withdrawn | claude | 08-run/03-r2-claude.reply.md (FINDING k2) | —
k3 | Major | withdrawn | claude | 08-run/03-r2-claude.reply.md (FINDING k3) | —
k4 | Minor | accepted-fix | claude | 08-run/03-r2-claude.reply.md (FINDING k4) | —
k5 | Minor | accepted-fix | claude | 08-run/03-r2-claude.reply.md (FINDING k5) | —
k6 | Minor | accepted-fix | claude | 08-run/03-r2-claude.reply.md (FINDING k6) | —
k7 | Minor | withdrawn | claude | 08-run/03-r2-claude.reply.md (FINDING k7) | —
k8 | Minor | withdrawn | claude | 08-run/03-r2-claude.reply.md (FINDING k8) | —
k9 | Minor | accepted-fix | claude | 08-run/03-r2-claude.reply.md (FINDING k9) | —
k10 | Minor | open (deferred) | claude | 08-run/03-r2-claude.reply.md (FINDING k10) | deferred: dependency change (owner-only class); in HANDOFF
M1 | Major | accepted-fix | claude | 08-run/03-r2-claude.reply.md (FINDING M1) | —
M2 | Major | accepted-fix | claude | 08-run/03-r2-claude.reply.md (FINDING M2) | —
M3 | Major | accepted-fix | claude | 08-run/03-r2-claude.reply.md (FINDING M3) | —
N1 | Major | accepted-fix | claude | 08-run/03-r2-claude.reply.md (FINDING N1) | —
N2 | Major | accepted-fix | claude | 08-run/03-r2-claude.reply.md (FINDING N2) | —
N3 | Major | accepted-fix | claude | 08-run/03-r2-claude.reply.md (FINDING N3) | —
N4 | Major | accepted-fix | claude | 08-run/03-r2-claude.reply.md (FINDING N4) | —
N5 | Minor | accepted-fix | claude | 08-run/03-r2-claude.reply.md (FINDING N5) | —
claude-1 | Major | accepted-fix | claude | 08-run/03-r2-claude.reply.md (FINDING claude-1) | —
claude-2 | Major | accepted-fix | claude | 08-run/03-r2-claude.reply.md (FINDING claude-2) | —
claude-3 | Minor | open → fixed (awaiting finder) | claude | 08-run/03-r2-claude.reply.md (FINDING claude-3) | header no longer claims isolation; round 1 is an independent first pass and nothing relies on it being unseen (DECISIONS item 3)
claude-4 | Minor | accepted-fix | claude | 08-run/03-r2-claude.reply.md (FINDING claude-4) | —
claude-5 | Minor | accepted-fix | claude | 08-run/03-r2-claude.reply.md (FINDING claude-5) | —
claude-6 | Minor | accepted-fix | claude | 08-run/03-r2-claude.reply.md (FINDING claude-6) | —
claude-7 | Minor | accepted-fix | claude | 08-run/03-r2-claude.reply.md (FINDING claude-7) | —
claude-8 | Minor | accepted-fix | claude | 08-run/03-r2-claude.reply.md (FINDING claude-8) | —
claude-9 | Minor | accepted-fix | claude | 08-run/03-r2-claude.reply.md (FINDING claude-9) | —
claude-10 | Minor | accepted-fix | claude | 08-run/03-r2-claude.reply.md (FINDING claude-10) | —
claude-11 | Minor | accepted-fix | claude | 08-run/03-r2-claude.reply.md (FINDING claude-11) | —
claude-12 | Major | open → fixed (awaiting finder) | claude | 08-run/03-r2-claude.reply.md (FINDING claude-12) | snapshots and every saved prompt/reply are hashed into the runner's memory and verified before each prompt and after each turn; cleanup fails (exit 3) if the checkout changed; DECISIONS item 3 says so
claude-13 | Minor | accepted-fix | claude | 08-run/03-r2-claude.reply.md (FINDING claude-13) | —
claude-14 | Minor | open → fixed (awaiting finder) | claude | 08-run/03-r2-claude.reply.md (FINDING claude-14) | Minor: telemetry sections and the credential helper are disclosed in the script header and DECISIONS; 06/07 meta files are historical records of those runs
c1 | Blocker | withdrawn | codex | 08-run/04-r2-codex.reply.md (FINDING c1) | —
c2 | Major | withdrawn | codex | 08-run/04-r2-codex.reply.md (FINDING c2) | —
c3 | Major | withdrawn | codex | 08-run/04-r2-codex.reply.md (FINDING c3) | —
c4 | Major | withdrawn | codex | 08-run/04-r2-codex.reply.md (FINDING c4) | —
c5 | Minor | accepted-fix | codex | 08-run/04-r2-codex.reply.md (FINDING c5) | —
c6 | Major | withdrawn | codex | 08-run/04-r2-codex.reply.md (FINDING c6) | —
c7 | Minor | accepted-fix | codex | 08-run/04-r2-codex.reply.md (FINDING c7) | —
c8 | Minor | withdrawn | codex | 08-run/04-r2-codex.reply.md (FINDING c8) | —
M1 | Major | accepted-fix | codex | 08-run/04-r2-codex.reply.md (FINDING M1) | —
M2 | Major | accepted-fix | codex | 08-run/04-r2-codex.reply.md (FINDING M2) | —
M3 | Major | accepted-fix | codex | 08-run/04-r2-codex.reply.md (FINDING M3) | —
N1 | Major | accepted-fix | codex | 08-run/04-r2-codex.reply.md (FINDING N1) | —
N2 | Major | accepted-fix | codex | 08-run/04-r2-codex.reply.md (FINDING N2) | —
N3 | Major | accepted-fix | codex | 08-run/04-r2-codex.reply.md (FINDING N3) | —
N4 | Major | accepted-fix | codex | 08-run/04-r2-codex.reply.md (FINDING N4) | —
N5 | Minor | accepted-fix | codex | 08-run/04-r2-codex.reply.md (FINDING N5) | —
N6 | Major | accepted-fix | codex | 08-run/04-r2-codex.reply.md (FINDING N6) | —
N7 | Major | accepted-fix | codex | 08-run/04-r2-codex.reply.md (FINDING N7) | —
codex-1 | Major | accepted-fix | codex | 08-run/04-r2-codex.reply.md (FINDING codex-1) | —
codex-2 | Major | accepted-fix | codex | 08-run/04-r2-codex.reply.md (FINDING codex-2) | —
codex-3 | Major | accepted-fix | codex | 08-run/04-r2-codex.reply.md (FINDING codex-3) | —
codex-4 | Minor | accepted-fix | codex | 08-run/04-r2-codex.reply.md (FINDING codex-4) | —
codex-5 | Major | accepted-fix | codex | 08-run/04-r2-codex.reply.md (FINDING codex-5) | —
codex-6 | Major | open → fixed (awaiting finder) | codex | 08-run/04-r2-codex.reply.md (FINDING codex-6) | as claude-3: blindness is no longer claimed or relied on
codex-7 | Major | accepted-fix | codex | 08-run/04-r2-codex.reply.md (FINDING codex-7) | —
codex-8 | Minor | accepted-fix | codex | 08-run/04-r2-codex.reply.md (FINDING codex-8) | —
codex-9 | Major | accepted-fix | codex | 08-run/04-r2-codex.reply.md (FINDING codex-9) | —
codex-10 | Major | accepted-fix | codex | 08-run/04-r2-codex.reply.md (FINDING codex-10) | —
codex-11 | Major | open → fixed (awaiting finder) | codex | 08-run/04-r2-codex.reply.md (FINDING codex-11) | as claude-12
codex-12 | Minor | accepted-fix | codex | 08-run/04-r2-codex.reply.md (FINDING codex-12) | —
codex-13 | Major | open → fixed (awaiting finder) | codex | 08-run/04-r2-codex.reply.md (FINDING codex-13) | cleanup now exits 3 when the review checkout changed (also checks its HEAD)
CI | Major | fixed | owner | owner instruction (DECISIONS.md, verbatim) | quick checks failed after the force push (runs 43805282657, 43805282753, "bad object"); workflow falls back to HEAD
