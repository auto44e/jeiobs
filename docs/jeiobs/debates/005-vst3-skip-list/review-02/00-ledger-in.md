# Findings ledger — PR #20 (task 005)

Kept by the lead under DECISIONS gate item 4. Severity and status are each finder's latest, from
its own FINDING lines in the last review run (`review-01`). "open → fixed" means the lead's
workers changed something and the finder has not yet accepted it. Fixes: `13b6bdbe2` (Codex
worker), checks 12–17 `13649f70b` (Claude worker, written without reading the implementation).

ID | severity | status | finder | source | lead's note
--- | --- | --- | --- | --- | ---
claude-1 | Minor | open → fixed (awaiting finder) | claude | review-01/03-r2-claude.reply.md (FINDING claude-1) | a valid result with the matching nonce is accepted before any marker logic; check #17 (no marker beside a valid result)
claude-2 | Minor | open → fixed (awaiting finder) | claude | review-01/03-r2-claude.reply.md (FINDING claude-2) | downgrade-only exit codes: 90 (save-failed marker not written) → unchanged, 91 (watchdog timeout marker not written) → timeout strike; check #15 simulates both exits
claude-3 | Minor | open → fixed (awaiting finder) | claude | review-01/03-r2-claude.reply.md (FINDING claude-3) | Linux arch directory is `uname().machine + "-linux"`, as the SDK's module_linux.cpp; not testable on x86_64 (listed in Not done)
claude-4 | Minor | open → fixed (awaiting finder) | claude | review-01/03-r2-claude.reply.md (FINDING claude-4) | checks #12 load-failed, #13 setup + clean exit, #14 coordinator deadline kill → timeout strike; remaining gaps listed in task 005 Not done
codex-1 | Major | open → fixed (awaiting finder) | codex | review-01/04-r2-codex.reply.md (FINDING codex-1) | a timeout strike needs the coordinator's own monotonic deadline to have passed and its kill after setup; a wait error leaves the entry unchanged (not covered by a check: needs fault injection, listed in Not done)
codex-2 | Major | open → fixed (awaiting finder) | codex | review-01/04-r2-codex.reply.md (FINDING codex-2) | as claude-2
codex-3 | Minor | open → fixed (awaiting finder) | codex | review-01/04-r2-codex.reply.md (FINDING codex-3) | numeric fields must be JSON integers, else the entry is ignored; a non-integer version ignores the file; check #16
codex-4 | Minor | open → fixed (awaiting finder) | codex | review-01/04-r2-codex.reply.md (FINDING codex-4) | the setup marker rename and the flag the watchdog reads are under one mutex, which the watchdog takes; no check (race)
