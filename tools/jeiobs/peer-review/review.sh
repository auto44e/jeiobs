#!/usr/bin/env bash
# Two-lineage peer review with cross-talk, for the self-merge gate in
# docs/jeiobs/DECISIONS.md. Owner-only file: changing it changes the gate.
#
#   tools/jeiobs/peer-review/review.sh -c CONTEXT -l LEDGER -o OUTDIR -s SHA \
#       -a claude|codex|lead [-C CI_SUMMARY] [-r ROUNDS] [-t SECS] "TOPIC"
#
#   CONTEXT     shared brief (what to review and how)
#   LEDGER      findings ledger kept by the lead, a pipe table with rows
#               "ID | severity | status | finder | source | lead's note";
#               finder is claude or codex
#   OUTDIR      where every prompt and reply is saved (committed), normally
#               docs/jeiobs/debates/<pr-or-task>/<run-name>/
#   SHA         the exact commit under review; the peers run in a detached
#               worktree checked out at this commit, never in the lead's tree
#   -a          who wrote the change. The other lineage speaks first in the
#               cross-talk rounds; for "lead", Claude speaks first.
#   CI_SUMMARY  optional file with the CI state for the reviewed SHA; copied
#               into 00-meta.md
#   ROUNDS      at least 2 (default 2). Round 1 is blind: both peers run at the
#               same time and neither reply exists while the other works.
#
# What is enforced, and what is not:
# - Claude gets only the Read, Glob and Grep tools and no MCP servers.
# - Codex runs the codex-cli engine directly (the toolbox launcher in bin/
#   injects MCP servers into its config) with a dedicated CODEX_HOME whose
#   config.toml has no [mcp_servers] (the engine itself adds analytics/otel
#   telemetry sections), a "core" shell environment, and --sandbox read-only.
# - Where Codex's sandbox cannot start (bubblewrap needs unprivileged user
#   namespaces; Ubuntu 24.04 AppArmor blocks them), Codex commands run
#   unsandboxed. 00-meta.md says so. The tamper check is then the enforcement:
#   after every turn, the lead's repository (refs, HEAD, index flags, tracked
#   diffs, untracked and ignored file contents, .git/config and hooks, except
#   OUTDIR) and the review checkout (HEAD and status) must be unchanged.
# - Blindness in round 1 is by protocol and timing, not enforced: a peer that
#   can read the filesystem could look for the other's in-progress files.
#
# Fails closed (exit 3, writes FAILED) on a timeout, a non-zero exit, an empty
# reply, a missing or malformed VERDICT or FINDING line, a ledger row or earlier
# finding the peer did not dispose of, or a tamper-check failure. Exit 0 means
# every turn completed; it is not approval. stderr logs go to a local
# diagnostics folder (they contain tool output) and are not committed.
set -uo pipefail

ROUNDS=2 TIMEOUT=900 CTX="" LEDGER="" OUT="" SHA="" AUTHOR="" CI=""
while getopts "c:l:o:s:a:C:r:t:" o; do
  case $o in
    c) CTX=$OPTARG ;; l) LEDGER=$OPTARG ;; o) OUT=$OPTARG ;; s) SHA=$OPTARG ;;
    a) AUTHOR=$OPTARG ;; C) CI=$OPTARG ;; r) ROUNDS=$OPTARG ;; t) TIMEOUT=$OPTARG ;; *) exit 2 ;;
  esac
done
shift $((OPTIND - 1))
TOPIC="${1:-}"
for v in CTX LEDGER OUT SHA AUTHOR TOPIC; do
  [ -n "${!v}" ] || { echo "error: missing $v (see the header for usage)" >&2; exit 2; }
done
[ -f "$CTX" ] && [ -f "$LEDGER" ] || { echo "error: context or ledger file not found" >&2; exit 2; }
[ -z "$CI" ] || [ -f "$CI" ] || { echo "error: CI summary not found" >&2; exit 2; }
[ "$ROUNDS" -ge 2 ] 2>/dev/null || { echo "error: at least 2 rounds" >&2; exit 2; }
case $AUTHOR in
  claude) FIRST=codex SECOND=claude ;; codex | lead) FIRST=claude SECOND=codex ;;
  *) echo "error: -a claude|codex|lead" >&2; exit 2 ;;
esac
CODEX_BIN="${REVIEW_CODEX_BIN:-$(ls -d "$HOME"/.toolbox/tools/codex/*/codex 2>/dev/null | sort -V | tail -1)}"
"$CODEX_BIN" --version 2>/dev/null | grep -q '^codex-cli ' || { echo "error: $CODEX_BIN is not the codex-cli engine" >&2; exit 2; }
REVIEW_CODEX_HOME="${REVIEW_CODEX_HOME:-$HOME/.config/jeiobs-agent/codex-review}"
command -v claude >/dev/null && [ -x "$CODEX_BIN" ] || { echo "error: claude and codex needed" >&2; exit 2; }
[ -f "$REVIEW_CODEX_HOME/config.toml" ] || { echo "error: no $REVIEW_CODEX_HOME/config.toml (model/provider only)" >&2; exit 2; }
if grep -qE '^\[(mcp_servers|plugins|marketplaces|projects)' "$REVIEW_CODEX_HOME/config.toml" \
  || ! grep -qE '^inherit = "core"' "$REVIEW_CODEX_HOME/config.toml"; then
  echo "error: $REVIEW_CODEX_HOME/config.toml must have no MCP/plugin/project sections and inherit = \"core\"" >&2; exit 2
fi
REPO="$(git rev-parse --show-toplevel)" || exit 2
FULLSHA="$(git -C "$REPO" rev-parse --verify "$SHA^{commit}")" || { echo "error: $SHA is not a commit" >&2; exit 2; }
mkdir -p "$OUT" || exit 2
OUT="$(cd "$OUT" && pwd)"
DIAG="${REVIEW_DIAG_DIR:-$HOME/.cache/jeiobs-review}/$(basename "$OUT")-$(date -u +%Y%m%dT%H%M%SZ)"
mkdir -p "$DIAG" || exit 2

TMP="$(mktemp -d)"
TREE="$TMP/tree"
fail() { echo "FAILED: $1" | tee "$OUT/FAILED" >&2; exit 3; }
cleanup() {
  if [ -d "$TREE" ] && [ -n "$(git -C "$TREE" status --porcelain --untracked-files=all --ignored=matching 2>/dev/null)" ]; then
    echo "FAILED: the review checkout was modified" | tee -a "$OUT/FAILED" >&2
  fi
  git -C "$REPO" worktree remove --force "$TREE" >/dev/null 2>&1; rm -rf "$TMP"
}
trap cleanup EXIT
git -C "$REPO" worktree add --detach "$TREE" "$FULLSHA" >/dev/null 2>&1 || { echo "error: cannot check out $FULLSHA" >&2; exit 2; }

# Snapshots: peers always see the context and ledger as they were at the start.
cp "$CTX" "$OUT/00-context.md"; cp "$LEDGER" "$OUT/00-ledger-in.md"
CTX="$OUT/00-context.md"; LEDGER="$OUT/00-ledger-in.md"

REL_OUT="${OUT#"$REPO"/}"
repo_state() {
  local gd; gd="$(git -C "$REPO" rev-parse --git-common-dir)"; case $gd in /*) ;; *) gd="$REPO/$gd" ;; esac
  {
    git -C "$REPO" for-each-ref --format='%(refname) %(objectname)'
    git -C "$REPO" rev-parse HEAD
    git -C "$REPO" ls-files -v | grep -v " $REL_OUT/"          # index entries with flags
    git -C "$REPO" diff --binary; git -C "$REPO" diff --cached --binary
    { git -C "$REPO" ls-files -o -z --exclude-standard; git -C "$REPO" ls-files -o -z --ignored --exclude-standard; } \
      | tr '\0' '\n' | grep -v "^$REL_OUT/" | sort -u | while IFS= read -r f; do
        printf '%s ' "$f"; sha256sum < "$REPO/$f" 2>/dev/null || echo missing; done
    sha256sum "$gd/config" "$gd"/hooks/* 2>/dev/null
  } 2>/dev/null | sha256sum
}
TREE_HEAD=""
tamper_check() { # label
  [ "$(repo_state)" = "$BASE_STATE" ] || fail "$1: the lead's repository changed during the turn"
  [ "$(git -C "$TREE" rev-parse HEAD)" = "$FULLSHA" ] || fail "$1: the review checkout's HEAD moved"
  [ -z "$(git -C "$TREE" status --porcelain --untracked-files=all --ignored=matching)" ] || fail "$1: the review checkout was modified"
}
BASE_STATE="$(repo_state)"
if bwrap --ro-bind / / true >/dev/null 2>&1; then SANDBOX="available"
else SANDBOX="UNAVAILABLE on this host (bubblewrap cannot create user namespaces); tamper check enforced"; fi

SELF="$(readlink -f "$0")"
{
  echo "# Review run"
  echo
  echo "- Started: $(date -u +%FT%TZ)"
  echo "- Reviewed commit: \`$FULLSHA\` (peers ran in a detached worktree at this commit)"
  echo "- Author: $AUTHOR; rounds: $ROUNDS (round 1 blind, both at once); cross-talk order: $FIRST, $SECOND; timeout per turn: ${TIMEOUT}s"
  echo "- Claude: $(claude --version 2>/dev/null | head -1), model ${PR_CLAUDE_MODEL:-opus}, effort ${PR_CLAUDE_EFFORT:-max}; tools Read, Glob, Grep; no MCP"
  echo "- Codex: $("$CODEX_BIN" --version 2>/dev/null | head -1) (engine, not the toolbox launcher), effort ${PR_CODEX_EFFORT:-xhigh}; CODEX_HOME config without MCP/plugin/project sections, shell env \"core\"; --sandbox read-only, sandbox $SANDBOX"
  echo "- Round 1 is blind by protocol and timing, not enforced (see the script header)"
  echo "- Script: \`tools/jeiobs/peer-review/review.sh\`, sha256 \`$(sha256sum "$SELF" | cut -c1-64)\`"
  echo "- Context sha256 \`$(sha256sum "$CTX" | cut -c1-64)\`; ledger sha256 \`$(sha256sum "$LEDGER" | cut -c1-64)\`"
  echo "- stderr logs: kept locally (not committed), \`$(basename "$DIAG")\`"
  echo "- Topic: $TOPIC"
  if [ -n "$CI" ]; then echo; echo "## CI state at the reviewed commit"; echo; echo '```text'; cat "$CI"; echo '```'; fi
} > "$OUT/00-meta.md"

PREAMBLE="You are one of two independent reviewers (Claude and Codex, different model lineages) in a
cross-talk review for the JeiOBS repository. Your working directory is a checkout of exactly the
commit under review. The lead agent wrote or integrated the change and must NOT decide on its own
whether your findings are resolved: under docs/jeiobs/DECISIONS.md a finding is closed only when the
reviewer who raised it accepts the fix or withdraws it, in writing, in a later round. Use one
severity scale: Blocker, Major, Minor. Only the finder may lower a finding's severity. You are
read-only: never modify, create or delete files, and never run commands that change state.
Verify claims in the files rather than trusting the context. Be terse; cite file:line.

End your reply with one line per finding YOU raised (every ledger row with you as finder, every
finding you raised in an earlier round, plus new ones), then the verdict. If you raised none, write
FINDING none. Use exactly this syntax:
FINDING <ID> | <Blocker|Major|Minor> | <open|accepted-fix|withdrawn> | <one-line reason>
VERDICT: approve
or
VERDICT: request changes
For a new finding, use ID <your-name>-<n> (for example claude-1) and quote the evidence."

ID_RE='[A-Za-z0-9][A-Za-z0-9._-]*'
owned_ids() { # ledger rows with this finder
  awk -F'|' -v p="$1" 'NF >= 5 {
      id=$1; f=$4; gsub(/^[ \t]+|[ \t]+$/, "", id); gsub(/^[ \t]+|[ \t]+$/, "", f)
      if (id != "" && id != "ID" && id !~ /^-+$/ && index(f, p) > 0) print id }' "$LEDGER"
}
declare -A REQ=([claude]="$(owned_ids claude)" [codex]="$(owned_ids codex)")

prompt() { # peer round own_prev other_latest -> path
  local peer=$1 r=$2 own=$3 other=$4 p
  p="$TMP/$peer-r$r.prompt"
  {
    echo "$PREAMBLE"; echo; echo "REVIEWED COMMIT: $FULLSHA"; echo; echo "TOPIC: $TOPIC"
    echo; echo "SHARED CONTEXT:"; cat "$CTX"
    echo; echo "FINDINGS LEDGER (kept by the lead):"; cat "$LEDGER"
    echo; echo "YOUR OWN PREVIOUS REPLY:"; [ -n "$own" ] && cat "$own" || echo "(none: first round)"
    echo; echo "THE OTHER REVIEWER'S LATEST REPLY:"
    if [ "$r" -eq 1 ]; then echo "(not shown: round 1 is blind)"; else cat "$other"; fi
  } > "$p"
  echo "$p"
}
run_peer() { # peer prompt reply stderr
  if [ "$1" = claude ]; then
    (cd "$TREE" && timeout "$TIMEOUT" claude -p --model "${PR_CLAUDE_MODEL:-opus}" --effort "${PR_CLAUDE_EFFORT:-max}" \
      --tools "Read,Glob,Grep" --allowedTools "Read,Glob,Grep" --strict-mcp-config < "$2" > "$3" 2> "$4")
  else
    CODEX_HOME="$REVIEW_CODEX_HOME" timeout "$TIMEOUT" "$CODEX_BIN" exec --sandbox read-only --skip-git-repo-check \
      -C "$TREE" -c model_reasoning_effort="${PR_CODEX_EFFORT:-xhigh}" -o "$3" - < "$2" > "$4" 2>&1
  fi
}
validate() { # peer round reply
  local peer=$1 r=$2 f=$3 id bad
  [ -s "$f" ] || fail "$peer r$r returned an empty reply"
  grep -qE '^VERDICT: (approve|request changes)[[:space:]]*$' "$f" || fail "$peer r$r gave no valid VERDICT line"
  grep -q '^FINDING ' "$f" || fail "$peer r$r gave no FINDING line"
  bad=$(grep '^FINDING ' "$f" | grep -vE "^FINDING none[[:space:]]*$" \
    | grep -vE "^FINDING $ID_RE \| (Blocker|Major|Minor) \| (open|accepted-fix|withdrawn) \| .+" || true)
  [ -z "$bad" ] || fail "$peer r$r has a malformed FINDING line: $(echo "$bad" | head -1)"
  for id in ${REQ[$peer]}; do
    grep -qE "^FINDING $id \|" "$f" || fail "$peer r$r did not dispose of its finding $id"
  done
  # Findings raised now must be disposed of in later rounds.
  REQ[$peer]="$(printf '%s\n' ${REQ[$peer]} $(grep -oE "^FINDING $ID_RE \|" "$f" | awk '{print $2}') | sort -u)"
}
save() { # peer round slot  (copies the prompt and reply into OUT)
  local n; n=$(printf '%02d' $(( ($2 - 1) * 2 + $3 )))
  cp "$TMP/$1-r$2.prompt" "$OUT/$n-r$2-$1.prompt.md"
  cp "$TMP/$1-r$2.reply" "$OUT/$n-r$2-$1.reply.md"
  echo "$OUT/$n-r$2-$1.reply.md"
}

# Round 1: blind. Both peers start together; both are waited for even if one
# fails, then the tamper check runs, then the files are saved, then validated.
pa=$(prompt "$FIRST" 1 "" ""); pb=$(prompt "$SECOND" 1 "" "")
run_peer "$FIRST" "$pa" "$TMP/$FIRST-r1.reply" "$DIAG/r1-$FIRST.stderr.txt" & ja=$!
run_peer "$SECOND" "$pb" "$TMP/$SECOND-r1.reply" "$DIAG/r1-$SECOND.stderr.txt" & jb=$!
wait $ja; ra=$?; wait $jb; rb=$?
touch "$TMP/$FIRST-r1.reply" "$TMP/$SECOND-r1.reply"
a=$(save "$FIRST" 1 1); b=$(save "$SECOND" 1 2)
tamper_check "round 1"
[ $ra -eq 0 ] || fail "$FIRST r1 exited $ra"
[ $rb -eq 0 ] || fail "$SECOND r1 exited $rb"
validate "$FIRST" 1 "$a"; validate "$SECOND" 1 "$b"
for ((r = 2; r <= ROUNDS; r++)); do
  for side in 1 2; do
    if [ $side = 1 ]; then peer=$FIRST own=$a other=$b; else peer=$SECOND own=$b other=$a; fi
    p=$(prompt "$peer" "$r" "$own" "$other")
    run_peer "$peer" "$p" "$TMP/$peer-r$r.reply" "$DIAG/r$r-$peer.stderr.txt"; rc=$?
    touch "$TMP/$peer-r$r.reply"; f=$(save "$peer" "$r" $side)
    tamper_check "round $r $peer"
    [ $rc -eq 0 ] || fail "$peer r$r exited $rc"
    validate "$peer" "$r" "$f"
    if [ $side = 1 ]; then a=$f; else b=$f; fi
  done
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
