# PR #18 peer correspondence

- `01`, `02`: round-1 reviews (Codex, Claude), prompt and answer verbatim.
- `03`, `04`: round-2 cross-talk with `~/.kiro/bin/peer-debate.sh`. Its context
  is verbatim; that helper did not save per-turn prompts (each turn's prompt was
  the context plus the other peer's previous reply, which is in `04`).
- `05-final/`, `06-…/`: review runs with `tools/jeiobs/peer-review/review.sh`.
  Every prompt, reply and stderr log is kept; `00-meta.md` names the reviewed
  commit and the final dispositions.
- [findings.md](findings.md): the ledger.

Transcripts are verbatim except that trailing whitespace was stripped (the
repository's whitespace check rejects it), the home directory is shown as `~`,
and relative markdown links inside run folders are shown as plain text.
SHAs before `f4e2e6d35` are pre-rewrite; translate them with
[history/2026-11-16-history-rewrite.md](../../history/2026-11-16-history-rewrite.md).
