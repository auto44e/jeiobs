#!/usr/bin/env bash
# Two-lineage peer review with cross-talk, for the self-merge gate in
# docs/jeiobs/DECISIONS.md. Owner-only file: changing it changes the gate.
#
#   tools/jeiobs/peer-review/review.sh -c CONTEXT -l LEDGER -o OUTDIR -s SHA [-r ROUNDS] [-t SECS] "TOPIC"
#
#   CONTEXT  shared brief (what to review, where, the diff or how to get it)
#   LEDGER   findings ledger kept by the lead: one line per finding,
#            "<ID> | <severity> | <status> | <finder> | <verbatim text>"
#            (severity: Blocker, Major or Minor; status: open, fixed, withdrawn)
#   OUTDIR   where every prompt and reply is saved, normally
#            docs/jeiobs/debates/<pr-or-task>/<round-name>/
#   SHA      the exact commit under review (recorded, and shown to the peers)
#   ROUNDS   cross-talk rounds, at least 2 (default 2)
#
# Each round, Claude then Codex. Both run read-only (Claude with only Read, Glob
# and Grep; Codex with --sandbox read-only). Each peer receives the context, the
# ledger, its OWN previous reply and the other peer's latest reply, so it can
# dispose of its own findings explicitly.
#
# Fails closed: if a peer times out, exits non-zero or returns an empty reply,
# the run stops with exit 3 and writes FAILED in OUTDIR. A failed run is not a
# review. Exit 0 means every turn completed; it does not mean approval: read the
# VERDICT and FINDING lines.
set -uo pipefail

ROUNDS=2 TIMEOUT=900 CTX="" LEDGER="" OUT="" SHA=""
while getopts "c:l:o:s:r:t:" o; do
  case $o in
    c) CTX=$OPTARG ;; l) LEDGER=$OPTARG ;; o) OUT=$OPTARG ;; s) SHA=$OPTARG ;;
    r) ROUNDS=$OPTARG ;; t) TIMEOUT=$OPTARG ;; *) exit 2 ;;
  esac
done
shift $((OPTIND - 1))
TOPIC="${1:-}"
for v in CTX LEDGER OUT SHA TOPIC; do
  [ -n "${!v}" ] || { echo "error: missing $v (see the header for usage)" >&2; exit 2; }
done
[ -f "$CTX" ] && [ -f "$LEDGER" ] || { echo "error: context or ledger file not found" >&2; exit 2; }
[ "$ROUNDS" -ge 2 ] 2>/dev/null || { echo "error: at least 2 rounds" >&2; exit 2; }
command -v claude >/dev/null && command -v codex >/dev/null || { echo "error: claude and codex CLIs needed" >&2; exit 2; }
mkdir -p "$OUT" || exit 2

SELF="$(readlink -f "$0")"
{
  echo "# Review run"
  echo
  echo "- Started: $(date -u +%FT%TZ)"
  echo "- Reviewed commit: \`$SHA\`"
  echo "- Rounds: $ROUNDS, timeout per turn: ${TIMEOUT}s"
  echo "- Script: \`tools/jeiobs/peer-review/review.sh\`, sha256 \`$(sha256sum "$SELF" | cut -c1-64)\`"
  echo "- Context: \`00-context.md\`, sha256 \`$(sha256sum "$CTX" | cut -c1-64)\`"
  echo "- Topic: $TOPIC"
} > "$OUT/00-meta.md"
cp "$CTX" "$OUT/00-context.md"
cp "$LEDGER" "$OUT/00-ledger-in.md"

PREAMBLE="You are one of two independent reviewers (Claude and Codex, different model lineages) in a
cross-talk review for the JeiOBS repository. The lead agent wrote or integrated the change and must
NOT decide on its own whether your findings are resolved: under docs/jeiobs/DECISIONS.md a finding is
closed only when the reviewer who raised it accepts the fix or withdraws it, in writing, in a later
round. Use one severity scale: Blocker, Major, Minor. Only the finder may lower a finding's severity.
You are read-only. Verify claims in the files rather than trusting the context. Be terse; cite file:line.

End your reply with these lines, one per finding YOU raised (from the ledger or new), then the verdict:
FINDING <ID> | <Blocker|Major|Minor> | <open|accepted-fix|withdrawn> | <one-line reason>
VERDICT: approve | request changes
For a new finding, use ID <your-name>-<n> (for example claude-1) and quote the evidence."

TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT
fail() { echo "FAILED: $1" | tee "$OUT/FAILED" >&2; exit 3; }

turn() { # peer round own_prev other_latest
  local peer=$1 r=$2 own=$3 other=$4 n p f
  local slot=1; [ "$peer" = codex ] && slot=2
  n=$(printf '%02d' $(( (r - 1) * 2 + slot )))
  p="$OUT/$n-r$r-$peer.prompt.md"; f="$OUT/$n-r$r-$peer.reply.md"
  {
    echo "$PREAMBLE"; echo; echo "REVIEWED COMMIT: $SHA"; echo; echo "TOPIC: $TOPIC"
    echo; echo "SHARED CONTEXT: see the file 00-context.md in $OUT (read it)."
    echo; echo "FINDINGS LEDGER (kept by the lead; check it against the raw replies in $OUT):"; cat "$LEDGER"
    echo; echo "YOUR OWN PREVIOUS REPLY:"; [ -n "$own" ] && cat "$own" || echo "(none: first round)"
    echo; echo "THE OTHER REVIEWER'S LATEST REPLY:"; [ -n "$other" ] && cat "$other" || echo "(none yet)"
  } > "$p"
  if [ "$peer" = claude ]; then
    timeout "$TIMEOUT" claude -p --model "${PR_CLAUDE_MODEL:-opus}" --effort "${PR_CLAUDE_EFFORT:-max}" \
      --allowedTools "Read,Glob,Grep" --add-dir "$OUT" < "$p" > "$f" 2> "$TMP/err" || fail "claude r$r exited $?"
  else
    timeout "$TIMEOUT" codex exec --sandbox read-only --skip-git-repo-check \
      -c model_reasoning_effort="${PR_CODEX_EFFORT:-xhigh}" -o "$f" - < "$p" > "$TMP/log" 2>&1 || fail "codex r$r exited $?"
  fi
  [ -s "$f" ] || fail "$peer r$r returned an empty reply"
  grep -q '^VERDICT:' "$f" || fail "$peer r$r gave no VERDICT line"
  echo "$f"
}

cprev="" xprev=""
for ((r = 1; r <= ROUNDS; r++)); do
  cprev=$(turn claude "$r" "$cprev" "$xprev") || exit 3
  xprev=$(turn codex "$r" "$xprev" "$cprev") || exit 3
done
{
  echo "- Finished: $(date -u +%FT%TZ)"
  echo
  echo "## Final dispositions"
  echo
  echo '```text'
  grep -hE '^(FINDING|VERDICT)' "$cprev" "$xprev"
  echo '```'
} >> "$OUT/00-meta.md"
grep -hE '^(FINDING|VERDICT)' "$cprev" "$xprev"
