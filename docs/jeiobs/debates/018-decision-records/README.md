# PR #18 and #19 peer correspondence

The owner merged #18 at `374f842c7` (run 06 reviewed that head). Runs 07
onwards reviewed the commits after it, which are PR #19.

- `01`, `02`: round-1 reviews (Codex, Claude), prompt and answer verbatim.
- `03`, `04`: round-2 cross-talk with `~/.kiro/bin/peer-debate.sh`. Its context
  is verbatim; that helper did not save per-turn prompts (each turn's prompt was
  the context plus the other peer's previous reply, which is in `04`).
- `05-final/`, `06-…/`: review runs with `tools/jeiobs/peer-review/review.sh`.
  Every prompt and reply is kept. stderr logs are kept locally, not here
  (they contain tool output); run 06 was trimmed to match. `00-meta.md` names
  the reviewed commit and the final dispositions.
- `11-run/` failed closed in round 1 (the lead fetched during the turn and the
  tamper check caught the moved ref); it has no evidence list, and its files are
  kept as written apart from the edits below.
- `13-run/` failed closed after round 2 on a false positive of the session-log
  check (a peer quoted the search string); it has no evidence list.
- [findings.md](findings.md): the ledger.

Transcripts are verbatim except that trailing whitespace was stripped (the
repository's whitespace check rejects it), the home directory is shown as `~`,
and relative markdown links inside run folders are shown as plain text. These
edits change file hashes, so the hashes in each `00-meta.md` are of the files
as the runner wrote them.
SHAs before `f4e2e6d35` are pre-rewrite; translate them with
[history/2026-11-16-history-rewrite.md](../../history/2026-11-16-history-rewrite.md).
