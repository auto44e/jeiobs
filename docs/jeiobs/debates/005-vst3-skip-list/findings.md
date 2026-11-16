# Findings ledger — PR #20 (task 005)

Kept by the lead under DECISIONS gate item 4. Severity and status are each finder's latest, from
its own FINDING lines in the last review run (`review-02`). "open → fixed" means the lead's
workers changed something and the finder has not yet accepted it. Fixes: `13b6bdbe2` (Codex
worker), checks 12–17 `13649f70b` (Claude worker, written without reading the implementation).

ID | severity | status | finder | source | lead's note
--- | --- | --- | --- | --- | ---
claude-1 | Minor | accepted-fix | claude | review-02/03-r2-claude.reply.md (FINDING claude-1) | —
claude-2 | Minor | accepted-fix | claude | review-02/03-r2-claude.reply.md (FINDING claude-2) | —
claude-3 | Minor | accepted-fix | claude | review-02/03-r2-claude.reply.md (FINDING claude-3) | —
claude-4 | Minor | accepted-fix | claude | review-02/03-r2-claude.reply.md (FINDING claude-4) | —
codex-1 | Major | accepted-fix | codex | review-02/04-r2-codex.reply.md (FINDING codex-1) | —
codex-2 | Major | accepted-fix | codex | review-02/04-r2-codex.reply.md (FINDING codex-2) | —
codex-3 | Minor | accepted-fix | codex | review-02/04-r2-codex.reply.md (FINDING codex-3) | —
codex-4 | Minor | accepted-fix | codex | review-02/04-r2-codex.reply.md (FINDING codex-4) | —
claude-5 | Minor | open | claude | review-02/03-r2-claude.reply.md (FINDING claude-5) | follow-up (Minor, less sticky direction): use the same clock as the libobs pipe wait for the deadline decision
codex-5 | Minor | open | codex | review-02/04-r2-codex.reply.md (FINDING codex-5) | follow-up (Minor): say in the task doc that the loader accepts only kind failure or timeout

## PR #22 (follow-ups), review `followup-review-01` on `7adc69a8b`

Both peers approved; codex-1 to codex-5 and claude-1 to claude-5 accepted.

ID | severity | status | finder | source | lead's note
--- | --- | --- | --- | --- | ---
claude-6 | Minor | open → fixed (awaiting finder) | claude | followup-review-01/03-r2-claude.reply.md (FINDING claude-6) | Outcomes table and Not done now state the 100 ms slack
codex-6 | Minor | open (accepted trade-off, documented) | codex | followup-review-01/04-r2-codex.reply.md (FINDING codex-6) | a wait error within 100 ms of the deadline after a successful kill counts as a strike; cost at most one extra backoff; written in task 005 Not done
