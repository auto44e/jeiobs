# Findings ledger — PR #18 and its follow-up #19

Kept by the lead under DECISIONS gate item 4. Severity and status are each
finder's latest, taken from its own FINDING lines in the last completed review run (12-run) (the
"source" column points at them). Findings first raised in rounds 1–2 are quoted in
`01`, `02` and
`04`; later runs are in `05-final` to `12-run`. "open → fixed"
means the lead changed something and the finder has not yet accepted it.
Run 11 failed closed in round 1 because the lead ran `git fetch` in the
repository during the turn (the tamper check saw `refs/remotes/origin/main`
move). A failed run is not a review: its dispositions are not counted, but the
new findings it raised are listed.

ID | severity | status | finder | source | lead's note
--- | --- | --- | --- | --- | ---
k1 | Major | withdrawn | claude | 12-run/03-r2-claude.reply.md (FINDING k1) | —
k2 | Major | withdrawn | claude | 12-run/03-r2-claude.reply.md (FINDING k2) | —
k3 | Major | withdrawn | claude | 12-run/03-r2-claude.reply.md (FINDING k3) | —
k4 | Minor | accepted-fix | claude | 12-run/03-r2-claude.reply.md (FINDING k4) | —
k5 | Minor | accepted-fix | claude | 12-run/03-r2-claude.reply.md (FINDING k5) | —
k6 | Minor | accepted-fix | claude | 12-run/03-r2-claude.reply.md (FINDING k6) | —
k7 | Minor | withdrawn | claude | 12-run/03-r2-claude.reply.md (FINDING k7) | —
k8 | Minor | withdrawn | claude | 12-run/03-r2-claude.reply.md (FINDING k8) | —
k9 | Minor | accepted-fix | claude | 12-run/03-r2-claude.reply.md (FINDING k9) | —
k10 | Minor | withdrawn | claude | 12-run/03-r2-claude.reply.md (FINDING k10) | —
M1 | Major | accepted-fix | claude | 12-run/03-r2-claude.reply.md (FINDING M1) | —
M2 | Major | accepted-fix | claude | 12-run/03-r2-claude.reply.md (FINDING M2) | —
M3 | Major | accepted-fix | claude | 12-run/03-r2-claude.reply.md (FINDING M3) | —
N1 | Major | accepted-fix | claude | 12-run/03-r2-claude.reply.md (FINDING N1) | —
N2 | Major | accepted-fix | claude | 12-run/03-r2-claude.reply.md (FINDING N2) | —
N3 | Major | accepted-fix | claude | 12-run/03-r2-claude.reply.md (FINDING N3) | —
N4 | Major | accepted-fix | claude | 12-run/03-r2-claude.reply.md (FINDING N4) | —
N5 | Minor | accepted-fix | claude | 12-run/03-r2-claude.reply.md (FINDING N5) | —
claude-1 | Major | accepted-fix | claude | 12-run/03-r2-claude.reply.md (FINDING claude-1) | —
claude-2 | Major | accepted-fix | claude | 12-run/03-r2-claude.reply.md (FINDING claude-2) | —
claude-3 | Minor | accepted-fix | claude | 12-run/03-r2-claude.reply.md (FINDING claude-3) | —
claude-4 | Minor | accepted-fix | claude | 12-run/03-r2-claude.reply.md (FINDING claude-4) | —
claude-5 | Minor | accepted-fix | claude | 12-run/03-r2-claude.reply.md (FINDING claude-5) | —
claude-6 | Minor | accepted-fix | claude | 12-run/03-r2-claude.reply.md (FINDING claude-6) | —
claude-7 | Minor | accepted-fix | claude | 12-run/03-r2-claude.reply.md (FINDING claude-7) | —
claude-8 | Minor | accepted-fix | claude | 12-run/03-r2-claude.reply.md (FINDING claude-8) | —
claude-9 | Minor | accepted-fix | claude | 12-run/03-r2-claude.reply.md (FINDING claude-9) | —
claude-10 | Minor | accepted-fix | claude | 12-run/03-r2-claude.reply.md (FINDING claude-10) | —
claude-11 | Minor | accepted-fix | claude | 12-run/03-r2-claude.reply.md (FINDING claude-11) | —
claude-12 | Major | accepted-fix | claude | 12-run/03-r2-claude.reply.md (FINDING claude-12) | —
claude-13 | Minor | accepted-fix | claude | 12-run/03-r2-claude.reply.md (FINDING claude-13) | —
claude-14 | Minor | accepted-fix | claude | 12-run/03-r2-claude.reply.md (FINDING claude-14) | —
claude-15 | Major | accepted-fix | claude | 12-run/03-r2-claude.reply.md (FINDING claude-15) | —
claude-16 | Major | accepted-fix | claude | 12-run/03-r2-claude.reply.md (FINDING claude-16) | —
c1 | Blocker | withdrawn | codex | 12-run/04-r2-codex.reply.md (FINDING c1) | —
c2 | Major | withdrawn | codex | 12-run/04-r2-codex.reply.md (FINDING c2) | —
c3 | Major | withdrawn | codex | 12-run/04-r2-codex.reply.md (FINDING c3) | —
c4 | Major | withdrawn | codex | 12-run/04-r2-codex.reply.md (FINDING c4) | —
c5 | Minor | accepted-fix | codex | 12-run/04-r2-codex.reply.md (FINDING c5) | —
c6 | Major | withdrawn | codex | 12-run/04-r2-codex.reply.md (FINDING c6) | —
c7 | Minor | accepted-fix | codex | 12-run/04-r2-codex.reply.md (FINDING c7) | —
c8 | Major | withdrawn | codex | 12-run/04-r2-codex.reply.md (FINDING c8) | —
M1 | Major | accepted-fix | codex | 12-run/04-r2-codex.reply.md (FINDING M1) | —
M2 | Major | accepted-fix | codex | 12-run/04-r2-codex.reply.md (FINDING M2) | —
M3 | Major | accepted-fix | codex | 12-run/04-r2-codex.reply.md (FINDING M3) | —
N1 | Major | accepted-fix | codex | 12-run/04-r2-codex.reply.md (FINDING N1) | —
N2 | Major | accepted-fix | codex | 12-run/04-r2-codex.reply.md (FINDING N2) | —
N3 | Major | accepted-fix | codex | 12-run/04-r2-codex.reply.md (FINDING N3) | —
N4 | Major | accepted-fix | codex | 12-run/04-r2-codex.reply.md (FINDING N4) | —
N5 | Minor | accepted-fix | codex | 12-run/04-r2-codex.reply.md (FINDING N5) | —
N6 | Major | accepted-fix | codex | 12-run/04-r2-codex.reply.md (FINDING N6) | —
N7 | Major | accepted-fix | codex | 12-run/04-r2-codex.reply.md (FINDING N7) | —
codex-1 | Major | accepted-fix | codex | 12-run/04-r2-codex.reply.md (FINDING codex-1) | —
codex-2 | Major | accepted-fix | codex | 12-run/04-r2-codex.reply.md (FINDING codex-2) | —
codex-3 | Major | accepted-fix | codex | 12-run/04-r2-codex.reply.md (FINDING codex-3) | —
codex-4 | Minor | accepted-fix | codex | 12-run/04-r2-codex.reply.md (FINDING codex-4) | —
codex-5 | Major | accepted-fix | codex | 12-run/04-r2-codex.reply.md (FINDING codex-5) | —
codex-6 | Major | accepted-fix | codex | 12-run/04-r2-codex.reply.md (FINDING codex-6) | —
codex-7 | Major | accepted-fix | codex | 12-run/04-r2-codex.reply.md (FINDING codex-7) | —
codex-8 | Minor | accepted-fix | codex | 12-run/04-r2-codex.reply.md (FINDING codex-8) | —
codex-9 | Major | accepted-fix | codex | 12-run/04-r2-codex.reply.md (FINDING codex-9) | —
codex-10 | Major | accepted-fix | codex | 12-run/04-r2-codex.reply.md (FINDING codex-10) | —
codex-11 | Major | accepted-fix | codex | 12-run/04-r2-codex.reply.md (FINDING codex-11) | —
codex-12 | Minor | accepted-fix | codex | 12-run/04-r2-codex.reply.md (FINDING codex-12) | —
codex-13 | Major | accepted-fix | codex | 12-run/04-r2-codex.reply.md (FINDING codex-13) | —
codex-14 | Major | accepted-fix | codex | 12-run/04-r2-codex.reply.md (FINDING codex-14) | —
claude-17 | Major | accepted-fix | claude | 12-run/03-r2-claude.reply.md (FINDING claude-17) | —
codex-15 | Major | accepted-fix | codex | 12-run/04-r2-codex.reply.md (FINDING codex-15) | —
codex-16 | Minor | accepted-fix | codex | 12-run/04-r2-codex.reply.md (FINDING codex-16) | —
codex-17 | Major | accepted-fix | codex | 12-run/04-r2-codex.reply.md (FINDING codex-17) | —
claude-18 | Major | accepted-fix | claude | 12-run/03-r2-claude.reply.md (FINDING claude-18) | —
claude-19 | Minor | accepted-fix | claude | 12-run/03-r2-claude.reply.md (FINDING claude-19) | —
codex-18 | Major | accepted-fix | codex | 12-run/04-r2-codex.reply.md (FINDING codex-18) | —
claude-20 | Major | open → fixed (awaiting finder) | claude | 12-run/03-r2-claude.reply.md (FINDING claude-20) | workers now use their own CODEX_HOME (codex-worker); the runner refuses to start while another process uses the review CODEX_HOME and fails if one appears; config.toml hashed at start and checked after every turn; table-header allowlist and key denylist instead of one regex; wording fixed ("kept for review runs", telemetry sections described, system skills named)
claude-21 | Minor | open → fixed (awaiting finder) | claude | 12-run/03-r2-claude.reply.md (FINDING claude-21) | DECISIONS status, review record, README and ledger titles now say the owner merged #18 at 374f842c7 and runs 06-12 reviewed #19; DECISIONS item 4 lists the whitespace and link edits and their effect on hashes
codex-19 | Major | open → fixed (awaiting finder) | codex | 12-run/04-r2-codex.reply.md (FINDING codex-19) | OUT must be new or empty (a stale 00-meta.md cannot be reused); the final append is checked and its last line verified before the file is pinned again
CI | Major | fixed | owner | owner instruction (DECISIONS.md, verbatim) | quick checks failed after the force push (runs 43805282657, 43805282753, "bad object"); workflow falls back to HEAD
