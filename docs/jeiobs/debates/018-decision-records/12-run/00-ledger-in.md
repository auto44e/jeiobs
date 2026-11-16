# Findings ledger — PR #18

Kept by the lead under DECISIONS gate item 4. Severity and status are each
finder's latest, taken from its own FINDING lines in the last review run (10-run) (the
"source" column points at them). Findings first raised in rounds 1–2 are quoted in
`01`, `02` and
`04`; later runs are in `05-final` to `10-run`. "open → fixed"
means the lead changed something and the finder has not yet accepted it.
Run 11 failed closed in round 1 because the lead ran `git fetch` in the
repository during the turn (the tamper check saw `refs/remotes/origin/main`
move). A failed run is not a review: its dispositions are not counted, but the
new findings it raised are listed.

ID | severity | status | finder | source | lead's note
--- | --- | --- | --- | --- | ---
k1 | Major | withdrawn | claude | 10-run/03-r2-claude.reply.md (FINDING k1) | —
k2 | Major | withdrawn | claude | 10-run/03-r2-claude.reply.md (FINDING k2) | —
k3 | Major | withdrawn | claude | 10-run/03-r2-claude.reply.md (FINDING k3) | —
k4 | Minor | accepted-fix | claude | 10-run/03-r2-claude.reply.md (FINDING k4) | —
k5 | Minor | accepted-fix | claude | 10-run/03-r2-claude.reply.md (FINDING k5) | —
k6 | Minor | accepted-fix | claude | 10-run/03-r2-claude.reply.md (FINDING k6) | —
k7 | Minor | withdrawn | claude | 10-run/03-r2-claude.reply.md (FINDING k7) | —
k8 | Minor | withdrawn | claude | 10-run/03-r2-claude.reply.md (FINDING k8) | —
k9 | Minor | accepted-fix | claude | 10-run/03-r2-claude.reply.md (FINDING k9) | —
k10 | Minor | withdrawn | claude | 10-run/03-r2-claude.reply.md (FINDING k10) | finder withdrew in 10-run
M1 | Major | accepted-fix | claude | 10-run/03-r2-claude.reply.md (FINDING M1) | —
M2 | Major | accepted-fix | claude | 10-run/03-r2-claude.reply.md (FINDING M2) | —
M3 | Major | accepted-fix | claude | 10-run/03-r2-claude.reply.md (FINDING M3) | —
N1 | Major | accepted-fix | claude | 10-run/03-r2-claude.reply.md (FINDING N1) | —
N2 | Major | accepted-fix | claude | 10-run/03-r2-claude.reply.md (FINDING N2) | —
N3 | Major | accepted-fix | claude | 10-run/03-r2-claude.reply.md (FINDING N3) | —
N4 | Major | accepted-fix | claude | 10-run/03-r2-claude.reply.md (FINDING N4) | —
N5 | Minor | accepted-fix | claude | 10-run/03-r2-claude.reply.md (FINDING N5) | —
claude-1 | Major | accepted-fix | claude | 10-run/03-r2-claude.reply.md (FINDING claude-1) | —
claude-2 | Major | accepted-fix | claude | 10-run/03-r2-claude.reply.md (FINDING claude-2) | —
claude-3 | Minor | open → fixed (awaiting finder) | claude | 10-run/03-r2-claude.reply.md (FINDING claude-3) | round-1 comment rewritten to match the code (save and pin as each peer exits, then tamper check, then validate)
claude-4 | Minor | accepted-fix | claude | 10-run/03-r2-claude.reply.md (FINDING claude-4) | —
claude-5 | Minor | accepted-fix | claude | 10-run/03-r2-claude.reply.md (FINDING claude-5) | —
claude-6 | Minor | accepted-fix | claude | 10-run/03-r2-claude.reply.md (FINDING claude-6) | —
claude-7 | Minor | accepted-fix | claude | 10-run/03-r2-claude.reply.md (FINDING claude-7) | —
claude-8 | Minor | accepted-fix | claude | 10-run/03-r2-claude.reply.md (FINDING claude-8) | —
claude-9 | Minor | accepted-fix | claude | 10-run/03-r2-claude.reply.md (FINDING claude-9) | —
claude-10 | Minor | accepted-fix | claude | 10-run/03-r2-claude.reply.md (FINDING claude-10) | —
claude-11 | Minor | accepted-fix | claude | 10-run/03-r2-claude.reply.md (FINDING claude-11) | —
claude-12 | Major | accepted-fix | claude | 10-run/03-r2-claude.reply.md (FINDING claude-12) | —
claude-13 | Minor | accepted-fix | claude | 10-run/03-r2-claude.reply.md (FINDING claude-13) | —
claude-14 | Minor | open → fixed (awaiting finder) | claude | 10-run/03-r2-claude.reply.md (FINDING claude-14) | header, 00-meta and DECISIONS now say the toolbox launcher runs as Bedrock credential helper only
claude-15 | Major | open → fixed (awaiting finder) | claude | 10-run/03-r2-claude.reply.md (FINDING claude-15) | tripwire wording as proposed (header, DECISIONS, meta "tripwires only"); code for point 1 (round-1 replies pinned as each peer exits, wait -n -p) and point 3 (review-checkout index flags must all be H); per-turn process groups killed; final hashes printed at exit and checked by the lead
claude-16 | Major | accepted-fix | claude | 10-run/03-r2-claude.reply.md (FINDING claude-16) | nit fixed: .claude, CLAUDE.md and AGENTS.override.md refused at any depth
c1 | Blocker | withdrawn | codex | 10-run/04-r2-codex.reply.md (FINDING c1) | —
c2 | Major | withdrawn | codex | 10-run/04-r2-codex.reply.md (FINDING c2) | —
c3 | Major | withdrawn | codex | 10-run/04-r2-codex.reply.md (FINDING c3) | —
c4 | Major | withdrawn | codex | 10-run/04-r2-codex.reply.md (FINDING c4) | —
c5 | Minor | accepted-fix | codex | 10-run/04-r2-codex.reply.md (FINDING c5) | —
c6 | Major | withdrawn | codex | 10-run/04-r2-codex.reply.md (FINDING c6) | —
c7 | Minor | accepted-fix | codex | 10-run/04-r2-codex.reply.md (FINDING c7) | —
c8 | Minor | withdrawn | codex | 10-run/04-r2-codex.reply.md (FINDING c8) | —
M1 | Major | accepted-fix | codex | 10-run/04-r2-codex.reply.md (FINDING M1) | —
M2 | Major | accepted-fix | codex | 10-run/04-r2-codex.reply.md (FINDING M2) | —
M3 | Major | accepted-fix | codex | 10-run/04-r2-codex.reply.md (FINDING M3) | —
N1 | Major | accepted-fix | codex | 10-run/04-r2-codex.reply.md (FINDING N1) | —
N2 | Major | accepted-fix | codex | 10-run/04-r2-codex.reply.md (FINDING N2) | —
N3 | Major | accepted-fix | codex | 10-run/04-r2-codex.reply.md (FINDING N3) | —
N4 | Major | accepted-fix | codex | 10-run/04-r2-codex.reply.md (FINDING N4) | —
N5 | Minor | accepted-fix | codex | 10-run/04-r2-codex.reply.md (FINDING N5) | —
N6 | Major | accepted-fix | codex | 10-run/04-r2-codex.reply.md (FINDING N6) | —
N7 | Major | accepted-fix | codex | 10-run/04-r2-codex.reply.md (FINDING N7) | —
codex-1 | Major | accepted-fix | codex | 10-run/04-r2-codex.reply.md (FINDING codex-1) | —
codex-2 | Major | accepted-fix | codex | 10-run/04-r2-codex.reply.md (FINDING codex-2) | —
codex-3 | Major | accepted-fix | codex | 10-run/04-r2-codex.reply.md (FINDING codex-3) | —
codex-4 | Minor | accepted-fix | codex | 10-run/04-r2-codex.reply.md (FINDING codex-4) | —
codex-5 | Major | accepted-fix | codex | 10-run/04-r2-codex.reply.md (FINDING codex-5) | —
codex-6 | Major | accepted-fix | codex | 10-run/04-r2-codex.reply.md (FINDING codex-6) | —
codex-7 | Major | accepted-fix | codex | 10-run/04-r2-codex.reply.md (FINDING codex-7) | —
codex-8 | Minor | accepted-fix | codex | 10-run/04-r2-codex.reply.md (FINDING codex-8) | —
codex-9 | Major | accepted-fix | codex | 10-run/04-r2-codex.reply.md (FINDING codex-9) | —
codex-10 | Major | accepted-fix | codex | 10-run/04-r2-codex.reply.md (FINDING codex-10) | —
codex-11 | Major | accepted-fix | codex | 10-run/04-r2-codex.reply.md (FINDING codex-11) | —
codex-12 | Minor | accepted-fix | codex | 10-run/04-r2-codex.reply.md (FINDING codex-12) | —
codex-13 | Major | accepted-fix | codex | 10-run/04-r2-codex.reply.md (FINDING codex-13) | —
codex-14 | Major | accepted-fix | codex | 10-run/04-r2-codex.reply.md (FINDING codex-14) | —
claude-17 | Major | open → fixed (awaiting finder) | claude | 10-run/03-r2-claude.reply.md (FINDING claude-17) | Codex runs with project_doc_max_bytes=0 (tested: AGENTS.md and AGENTS.override.md no longer reach the session); run fails if a session log from the run shows checkout instructions; AGENTS.override.md refused anywhere; DECISIONS says what each peer reads
codex-15 | Major | open → fixed (awaiting finder) | codex | 10-run/04-r2-codex.reply.md (FINDING codex-15) | cleanup re-verifies all evidence after killing turn process groups; runner prints final hashes at exit; lead checks with sha256sum -c before committing; residual post-run gap stated
codex-16 | Minor | open → fixed (awaiting finder) | codex | 10-run/04-r2-codex.reply.md (FINDING codex-16) | 00-meta labels its listed hash as the header hash; final hash printed at exit
codex-17 | Major | open → fixed (awaiting finder) | codex | 10-run/04-r2-codex.reply.md (FINDING codex-17) | as claude-17
claude-18 | Major | open → fixed (awaiting finder) | claude | 11-run/01-r1-claude.reply.md (FINDING claude-18) | session-log matches collected into a variable and checked as non-empty (no pipeline into grep -q); grep errors fail the run; stub test with three matching logs fails the run
claude-19 | Minor | open → fixed (awaiting finder) | claude | 11-run/01-r1-claude.reply.md (FINDING claude-19) | .agents and .codex refused at any depth; CODEX_HOME holds only the engine's bundled system skills (disclosed)
codex-18 | Major | open → fixed (awaiting finder) | codex | 11-run/02-r1-codex.reply.md (FINDING codex-18) | exactly one VERDICT line, which must be the last non-empty line; a finding ID may appear once; stub tests for each
CI | Major | fixed | owner | owner instruction (DECISIONS.md, verbatim) | quick checks failed after the force push (runs 43805282657, 43805282753, "bad object"); workflow falls back to HEAD
