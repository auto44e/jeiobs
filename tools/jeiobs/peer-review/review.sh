#!/usr/bin/env bash
# Two-lineage peer review with cross-talk, for the self-merge gate in
# docs/jeiobs/DECISIONS.md. Owner-only file: changing it changes the gate.
#
#   tools/jeiobs/peer-review/review.sh -c CONTEXT -l LEDGER -o OUTDIR -s SHA \
#       [-f claude|codex] [-r ROUNDS] [-t SECS] "TOPIC"
#
#   CONTEXT  shared brief (what to review and how)
#   LEDGER   findings ledger kept by the lead, a pipe table with rows
#            "ID | severity | status | finder | source | lead's note"
#            severity: Blocker, Major or Minor; status: open, fixed, withdrawn,
#            accepted-fix; finder: claude or codex; source: file:line of the
#            finding's verbatim text in a committed transcript
#   OUTDIR   where every prompt, reply and stderr log is saved, normally
#            docs/jeiobs/debates/<pr-or-task>/<run-name>/
#   SHA      the exact commit under review. The peers run in a detached
#            worktree checked out at this commit, never in the lead's tree.
#   -f       who speaks first in cross-talk rounds: the lineage that did NOT
#            write the change (default claude)
#   ROUNDS   rounds, at least 2 (default 2). Round 1 is blind: both peers
#            review independently. Later rounds are cross-talk.
#
# Both peers are confined: Claude gets only the Read, Glob and Grep tools and no
# MCP servers; Codex runs with --sandbox read-only and no MCP servers.
#
# Fails closed (exit 3, writes FAILED) when a peer times out, exits non-zero,
# returns an empty reply, omits the VERDICT line, or omits a FINDING line for a
# ledger row it owns. Exit 0 means every turn completed; it is not approval.
set -uo pipefail

ROUNDS=2 TIMEOUT=900 CTX="" LEDGER="" OUT="" SHA="" FIRST=claude
while getopts "c:l:o:s:f:r:t:" o; do
  case $o in
    c) CTX=$OPTARG ;; l) LEDGER=$OPTARG ;; o) OUT=$OPTARG ;; s) SHA=$OPTARG ;;
    f) FIRST=$OPTARG ;; r) ROUNDS=$OPTARG ;; t) TIMEOUT=$OPTARG ;; *) exit 2 ;;
  esac
done
shift $((OPTIND - 1))
TOPIC="${1:-}"
for v in CTX LEDGER OUT SHA TOPIC; do
  [ -n "${!v}" ] || { echo "error: missing $v (see the header for usage)" >&2; exit 2; }
done
[ -f "$CTX" ] && [ -f "$LEDGER" ] || { echo "error: context or ledger file not found" >&2; exit 2; }
[ "$ROUNDS" -ge 2 ] 2>/dev/null || { echo "error: at least 2 rounds" >&2; exit 2; }
case $FIRST in claude) SECOND=codex ;; codex) SECOND=claude ;; *) echo "error: -f claude|codex" >&2; exit 2 ;; esac
command -v claude >/dev/null && command -v codex >/dev/null || { echo "error: claude and codex CLIs needed" >&2; exit 2; }
REPO="$(git rev-parse --show-toplevel)" || exit 2
FULLSHA="$(git -C "$REPO" rev-parse --verify "$SHA^{commit}")" || { echo "error: $SHA is not a commit" >&2; exit 2; }
mkdir -p "$OUT" || exit 2
OUT="$(cd "$OUT" && pwd)"

TMP="$(mktemp -d)"
TREE="$TMP/tree"
cleanup() { git -C "$REPO" worktree remove --force "$TREE" >/dev/null 2>&1; rm -rf "$TMP"; }
trap cleanup EXIT
git -C "$REPO" worktree add --detach "$TREE" "$FULLSHA" >/dev/null 2>&1 || { echo "error: cannot check out $FULLSHA" >&2; exit 2; }

SELF="$(readlink -f "$0")"
{
  echo "# Review run"
  echo
  echo "- Started: $(date -u +%FT%TZ)"
  echo "- Reviewed commit: \`$FULLSHA\` (peers ran in a detached worktree at this commit)"
  echo "- Rounds: $ROUNDS (round 1 blind), first cross-talk speaker: $FIRST, timeout per turn: ${TIMEOUT}s"
  echo "- Claude: $(claude --version 2>/dev/null | head -1), model ${PR_CLAUDE_MODEL:-opus}, effort ${PR_CLAUDE_EFFORT:-max}; tools Read,Glob,Grep; no MCP"
  echo "- Codex: $(codex --version 2>/dev/null | head -1), effort ${PR_CODEX_EFFORT:-xhigh}; sandbox read-only; no MCP"
  echo "- Script: \`tools/jeiobs/peer-review/review.sh\`, sha256 \`$(sha256sum "$SELF" | cut -c1-64)\`"
  echo "- Context: \`00-context.md\`, sha256 \`$(sha256sum "$CTX" | cut -c1-64)\`"
  echo "- Topic: $TOPIC"
} > "$OUT/00-meta.md"
cp "$CTX" "$OUT/00-context.md"
cp "$LEDGER" "$OUT/00-ledger-in.md"

PREAMBLE="You are one of two independent reviewers (Claude and Codex, different model lineages) in a
cross-talk review for the JeiOBS repository. Your working directory is a checkout of exactly the
commit under review. The lead agent wrote or integrated the change and must NOT decide on its own
whether your findings are resolved: under docs/jeiobs/DECISIONS.md a finding is closed only when the
reviewer who raised it accepts the fix or withdraws it, in writing, in a later round. Use one
severity scale: Blocker, Major, Minor. Only the finder may lower a finding's severity. You are
read-only. Verify claims in the files rather than trusting the context. Be terse; cite file:line.

End your reply with one line per finding YOU raised (every ledger row with you as finder, plus new
ones), then the verdict. If you raised none, write FINDING none.
FINDING <ID> | <Blocker|Major|Minor> | <open|accepted-fix|withdrawn> | <one-line reason>
VERDICT: approve | request changes
For a new finding, use ID <your-name>-<n> (for example claude-1) and quote the evidence."

fail() { echo "FAILED: $1" | tee "$OUT/FAILED" >&2; exit 3; }

# Ledger IDs owned by a peer: rows "ID | sev | status | finder | ..." with that finder.
owned_ids() {
  awk -F'|' -v p="$1" 'NF >= 5 {
      id=$1; f=$4; gsub(/^[ \t]+|[ \t]+$/, "", id); gsub(/^[ \t]+|[ \t]+$/, "", f)
      if (id != "" && id != "ID" && id !~ /^-+$/ && index(f, p) > 0) print id }' "$LEDGER"
}

turn() { # peer round slot own_prev other_latest
  local peer=$1 r=$2 slot=$3 own=$4 other=$5 n p f e id
  n=$(printf '%02d' $(( (r - 1) * 2 + slot )))
  p="$OUT/$n-r$r-$peer.prompt.md"; f="$OUT/$n-r$r-$peer.reply.md"; e="$OUT/$n-r$r-$peer.stderr.txt"
  {
    echo "$PREAMBLE"; echo; echo "REVIEWED COMMIT: $FULLSHA"; echo; echo "TOPIC: $TOPIC"
    echo; echo "SHARED CONTEXT:"; cat "$CTX"
    echo; echo "FINDINGS LEDGER (kept by the lead; check it against the raw replies in $OUT):"; cat "$LEDGER"
    echo; echo "YOUR OWN PREVIOUS REPLY:"; [ -n "$own" ] && cat "$own" || echo "(none: first round)"
    echo; echo "THE OTHER REVIEWER'S LATEST REPLY:"
    if [ "$r" -eq 1 ]; then echo "(not shown: round 1 is blind)"; elif [ -n "$other" ]; then cat "$other"; else echo "(none yet)"; fi
  } > "$p"
  if [ "$peer" = claude ]; then
    (cd "$TREE" && timeout "$TIMEOUT" claude -p --model "${PR_CLAUDE_MODEL:-opus}" --effort "${PR_CLAUDE_EFFORT:-max}" \
      --tools "Read,Glob,Grep" --allowedTools "Read,Glob,Grep" --strict-mcp-config --add-dir "$OUT" \
      < "$p" > "$f" 2> "$e") || fail "claude r$r exited $? (see $(basename "$e"))"
  else
    timeout "$TIMEOUT" codex exec --sandbox read-only --skip-git-repo-check -C "$TREE" -c 'mcp_servers={}' \
      -c model_reasoning_effort="${PR_CODEX_EFFORT:-xhigh}" -o "$f" - < "$p" > "$e" 2>&1 \
      || fail "codex r$r exited $? (see $(basename "$e"))"
  fi
  [ -s "$f" ] || fail "$peer r$r returned an empty reply"
  grep -q '^VERDICT:' "$f" || fail "$peer r$r gave no VERDICT line"
  grep -q '^FINDING ' "$f" || fail "$peer r$r gave no FINDING line"
  for id in $(owned_ids "$peer"); do
    grep -qE "^FINDING $id( |\|)" "$f" || fail "$peer r$r did not dispose of its ledger finding $id"
  done
  echo "$f"
}

# Round 1: blind, both independently. Later rounds: cross-talk in the chosen order.
a=$(turn "$FIRST" 1 1 "" "") || exit 3
b=$(turn "$SECOND" 1 2 "" "") || exit 3
for ((r = 2; r <= ROUNDS; r++)); do
  a=$(turn "$FIRST" "$r" 1 "$a" "$b") || exit 3
  b=$(turn "$SECOND" "$r" 2 "$b" "$a") || exit 3
done
{
  echo "- Finished: $(date -u +%FT%TZ)"
  echo
  echo "## Final dispositions"
  echo
  echo '```text'
  grep -hE '^(FINDING|VERDICT)' "$a" "$b"
  echo '```'
} >> "$OUT/00-meta.md"
grep -hE '^(FINDING|VERDICT)' "$a" "$b"
