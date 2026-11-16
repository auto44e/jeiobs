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
#   ROUNDS      at least 2 (default 2). Round 1 is an independent first pass:
#               both peers start together and their prompts do not include the
#               other's reply (not an isolation guarantee; see below).
#
# What is enforced, and what is not:
# - Claude gets only the Read, Glob and Grep tools, no MCP servers, and user
#   settings only (--setting-sources user): project settings, hooks and
#   CLAUDE.md in the reviewed checkout are not loaded.
# - Codex runs the codex-cli engine directly (the toolbox launcher in bin/
#   injects MCP servers into its config) with a CODEX_HOME kept for review runs:
#   the run refuses to start while another process uses it, and fails if one
#   appears or config.toml changes during the run. config.toml may only have
#   [shell_environment_policy], [analytics], [otel] (telemetry to a local
#   collector) and [model_providers.*] tables. It is parsed as TOML and every key
#   must be on an exact allowlist (anything else, such as MCP servers, plugins,
#   projects, profiles or instructions, refuses the run). Its skills/ folder may
#   hold only the engine's bundled .system set, pinned by hash. 00-meta.md names
#   the home and both hashes. Codex gets a "core" shell environment, --sandbox read-only, and
#   project_doc_max_bytes=0 so it reads no AGENTS.md or AGENTS.override.md from
#   the checkout. The run fails if a Codex session log from this run shows
#   project instructions for the checkout. The provider config runs the toolbox
#   launcher as Bedrock credential helper (credential-process) only; it does not
#   start Codex.
# - The run refuses a reviewed commit that contains .claude, CLAUDE.md,
#   AGENTS.override.md, .agents (Codex repo skills) or .codex anywhere, so a change
#   cannot configure its reviewer. Codex still loads the engine's bundled system
#   skills from CODEX_HOME/skills/.system.
# - Where Codex's sandbox cannot start (bubblewrap needs unprivileged user
#   namespaces; Ubuntu 24.04 AppArmor blocks them), Codex commands run
#   unsandboxed. 00-meta.md says so. Nothing then stops a peer from writing;
#   the runner only has tripwires:
#   - after every turn, the lead's repository (refs, HEAD, index entries and
#     flags, tracked diffs, untracked and ignored file contents, .git/config and
#     hooks, except OUTDIR) and the review checkout (HEAD, status, and index
#     flags: every entry must be plain "H") must be unchanged;
#   - the sha256 of the context and ledger snapshots, of 00-meta.md, of each
#     prompt (taken before its turn) and of each reply (taken when its turn
#     ends; in round 1, as soon as that peer exits) are held in the runner's
#     memory and verified before each prompt, after each turn, at the end and
#     in cleanup;
#   - each turn runs in its own process group, which is killed when the turn
#     ends.
#   These catch a change still present at the next check, until the run ends.
#   They miss reads, network use, writes elsewhere, changes undone before a
#   check, a reply changed before it is saved (during its turn), and changes
#   after the run by a process that left its process group (setsid). Only the
#   sandbox stops a peer that evades them. At exit the runner prints the sha256
#   of every file it wrote, including the final 00-meta.md (which cannot list
#   its own final hash); the lead checks the files against that output before
#   committing them. Hashes are of the files as written, before the transcript
#   edits in DECISIONS.md.
# - Round 1 is not isolated: a peer that can read the filesystem could find
#   the other's in-progress reply. Nothing relies on it being unseen.

# Fails closed (exit 3, writes FAILED) on a timeout, a non-zero exit, an empty
# reply, a missing, repeated or not-last VERDICT line, a malformed or repeated
# FINDING line, a ledger row or earlier
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
# config.toml is parsed as TOML (python3 tomllib) and must match this exact allowlist: anything
# else (MCP servers, plugins, projects, profiles, instructions, any unknown key) refuses the run.
CFG="$REVIEW_CODEX_HOME/config.toml"
CFG_HASH="$(sha256sum < "$CFG")"   # hashed before the parse; re-checked after it and after every turn
python3 - "$CFG" <<'PYCFG' || exit 2
import sys, tomllib
S, B, L, D = str, bool, list, dict
ALLOWED = {
    "model_provider": S, "model_reasoning_effort": S, "model": S, "forced_login_method": S,
    "check_for_update_on_startup": B, "chatgpt_base_url": S, "approvals_reviewer": ("auto_review",),
    "shell_environment_policy": {"inherit": ("core",)},
    "model_providers": {"*": {"aws": {"region": S, "credential_export": {"command": S, "args": L}}}},
    "analytics": {"enabled": B},
    "otel": {"log_user_prompt": B, "exporter": D, "metrics_exporter": D, "trace_exporter": D, "environment": S},
}
def check(value, rule, path):
    if isinstance(rule, tuple):
        if value not in rule:
            sys.exit(f"error: {path} must be one of {rule}")
    elif isinstance(rule, dict):
        if not isinstance(value, dict):
            sys.exit(f"error: {path} must be a table")
        for key, item in value.items():
            sub = rule.get(key, rule.get("*"))
            if sub is None:
                sys.exit(f"error: {path}.{key} is not an allowed review config key")
            check(item, sub, f"{path}.{key}")
    elif not isinstance(value, rule):
        sys.exit(f"error: {path} has the wrong type")
try:
    config = tomllib.load(open(sys.argv[1], "rb"))
except Exception as error:
    sys.exit(f"error: cannot parse {sys.argv[1]}: {error}")
check(config, ALLOWED, "config")
if config.get("shell_environment_policy", {}).get("inherit") != "core":
    sys.exit('error: config needs shell_environment_policy.inherit = "core"')
PYCFG
# The engine installs its bundled system skills once per CODEX_HOME; nothing else may be there.
SKILLS="$REVIEW_CODEX_HOME/skills"
[ -f "$SKILLS/.system/.codex-system-skills.marker" ] || { echo "error: $SKILLS/.system is not initialized; run one Codex session with this CODEX_HOME first" >&2; exit 2; }
[ -z "$(find "$SKILLS" -mindepth 1 -maxdepth 1 ! -name .system -print -quit)" ] || { echo "error: $SKILLS has skills other than the engine's .system set" >&2; exit 2; }
skills_hash() { (cd "$SKILLS" && find . -type f -print0 | LC_ALL=C sort -z | xargs -0 sha256sum) | sha256sum; }
SKILLS_HASH="$(skills_hash)"
[ "$(sha256sum < "$CFG")" = "$CFG_HASH" ] || { echo "error: $CFG changed while it was checked" >&2; exit 2; }
codex_home_users() { # PIDs whose environment uses REVIEW_CODEX_HOME (checked when no turn is running)
  local p
  for p in /proc/[0-9]*; do
    tr '\0' '\n' 2>/dev/null < "$p/environ" | grep -qxF "CODEX_HOME=$REVIEW_CODEX_HOME" && echo "${p#/proc/}"
  done
}
[ -z "$(codex_home_users)" ] || { echo "error: another process uses $REVIEW_CODEX_HOME; review runs need it to themselves" >&2; exit 2; }
mkdir -p "$REVIEW_CODEX_HOME/sessions" || exit 2   # searched by the tamper check
REPO="$(git rev-parse --show-toplevel)" || exit 2
FULLSHA="$(git -C "$REPO" rev-parse --verify "$SHA^{commit}")" || { echo "error: $SHA is not a commit" >&2; exit 2; }
[ ! -e "$OUT" ] || [ -z "$(ls -A "$OUT" 2>/dev/null)" ] || { echo "error: $OUT must be new or empty" >&2; exit 2; }
mkdir -p "$OUT" || exit 2
OUT="$(cd "$OUT" && pwd)"
DIAG="${REVIEW_DIAG_DIR:-$HOME/.cache/jeiobs-review}/$(basename "$OUT")-$(date -u +%Y%m%dT%H%M%SZ)"
mkdir -p "$DIAG" || exit 2

TMP="$(mktemp -d)"
TREE="$TMP/tree"
fail() { echo "FAILED: $1" | tee "$OUT/FAILED" >&2; exit 3; }
declare -A EVIDENCE=()      # path -> sha256, held only in this process
declare -A PGID=()          # peer -> process group of its running turn
reap() { local g; for g in "${PGID[@]}"; do kill -KILL -- "-$g" 2>/dev/null; done; PGID=(); }
cleanup() {
  local rc=$? f
  reap
  for f in "${!EVIDENCE[@]}"; do
    [ "$(sha256sum < "$f" 2>/dev/null)" = "${EVIDENCE[$f]}" ] || { echo "FAILED: $(basename "$f") changed after it was saved" | tee -a "$OUT/FAILED" >&2; rc=3; }
  done
  if [ -d "$TREE" ] && { [ "$(git -C "$TREE" rev-parse HEAD 2>/dev/null)" != "${FULLSHA:-}" ] ||
    [ -n "$(git -C "$TREE" status --porcelain --untracked-files=all --ignored=matching 2>/dev/null)" ] ||
    [ -n "$(git -C "$TREE" ls-files -v 2>/dev/null | grep -v '^H ')" ]; }; then
    echo "FAILED: the review checkout was modified" | tee -a "$OUT/FAILED" >&2
    rc=3
  fi
  git -C "$REPO" worktree remove --force "$TREE" >/dev/null 2>&1; rm -rf "$TMP"
  exit $rc
}
trap cleanup EXIT
git -C "$REPO" worktree add --detach "$TREE" "$FULLSHA" >/dev/null 2>&1 || { echo "error: cannot check out $FULLSHA" >&2; exit 2; }
if [ -n "$(find "$TREE" -path "$TREE/.git" -prune -o \( -name .claude -o -name CLAUDE.md -o -name AGENTS.override.md \
    -o -name .agents -o -name .codex \) -print -quit)" ]; then
  echo "error: the reviewed commit contains .claude, CLAUDE.md, AGENTS.override.md, .agents or .codex; refusing to review (a change must not configure its reviewer)" >&2; exit 2
fi
touch "$TMP/started"   # Codex session logs newer than this belong to this run

# Snapshots: peers always see the context and ledger as they were at the start.
cp "$CTX" "$OUT/00-context.md"; cp "$LEDGER" "$OUT/00-ledger-in.md"
CTX="$OUT/00-context.md"; LEDGER="$OUT/00-ledger-in.md"
declare -A PROMPT_HASH=()   # "<peer>-r<round>" -> sha256 of the prompt before the turn
pin() { [ -f "$1" ] || fail "cannot pin missing file $(basename "$1")"; EVIDENCE["$1"]="$(sha256sum < "$1")"; }
verify_evidence() { # label
  local f
  for f in "${!EVIDENCE[@]}"; do
    [ "$(sha256sum < "$f" 2>/dev/null)" = "${EVIDENCE[$f]}" ] || fail "$1: $(basename "$f") changed after it was saved"
  done
}
pin "$CTX"; pin "$LEDGER"

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
  verify_evidence "$1"
  [ "$(repo_state)" = "$BASE_STATE" ] || fail "$1: the lead's repository changed during the turn"
  [ "$(git -C "$TREE" rev-parse HEAD)" = "$FULLSHA" ] || fail "$1: the review checkout's HEAD moved"
  [ -z "$(git -C "$TREE" status --porcelain --untracked-files=all --ignored=matching)" ] || fail "$1: the review checkout was modified"
  [ -z "$(git -C "$TREE" ls-files -v | grep -v '^H ')" ] || fail "$1: index flags were set in the review checkout"
  local logs f hits=""
  # Collected into variables (no pipeline into grep -q), so pipefail cannot turn a hit into a pass.
  # The engine writes project docs as a JSON string value that starts with
  # "# AGENTS.md instructions for <dir>". Matching the unescaped '":"' before it means a peer
  # that quotes this text in a reply (JSON-escaped inside a later prompt) cannot trip the check.
  logs="$(grep -rlE "\":\"# [A-Za-z._-]+ instructions for ${TREE//./\\.}([^A-Za-z0-9._/-]|\$)" \
    --include='*.jsonl' "$REVIEW_CODEX_HOME/sessions" 2>/dev/null)"
  case $? in 0 | 1) ;; *) fail "$1: cannot search the Codex session logs" ;; esac
  while IFS= read -r f; do
    [ -n "$f" ] && [ "$f" -nt "$TMP/started" ] && hits="$hits $f"
  done <<< "$logs"
  [ -z "$hits" ] || fail "$1: Codex loaded project instructions from the review checkout"
  [ "$(sha256sum < "$CFG" 2>/dev/null)" = "$CFG_HASH" ] || fail "$1: $CFG changed"
  [ "$(skills_hash)" = "$SKILLS_HASH" ] || fail "$1: $SKILLS changed"
  [ -z "$(codex_home_users)" ] || fail "$1: another process uses $REVIEW_CODEX_HOME"
}
BASE_STATE="$(repo_state)"
if bwrap --ro-bind / / true >/dev/null 2>&1; then SANDBOX="available"
else SANDBOX="UNAVAILABLE on this host (bubblewrap cannot create user namespaces); Codex commands ran unsandboxed, tripwires only"; fi

SELF="$(readlink -f "$0")"
{
  echo "# Review run"
  echo
  echo "- Started: $(date -u +%FT%TZ)"
  echo "- Reviewed commit: \`$FULLSHA\` (peers ran in a detached worktree at this commit)"
  echo "- Author: $AUTHOR; rounds: $ROUNDS (round 1 independent first pass, both at once, not isolated); cross-talk order: $FIRST, $SECOND; timeout per turn: ${TIMEOUT}s"
  echo "- Claude: $(claude --version 2>/dev/null | head -1), model ${PR_CLAUDE_MODEL:-opus}, effort ${PR_CLAUDE_EFFORT:-max}; tools Read, Glob, Grep; no MCP"
  echo "- Codex: $("$CODEX_BIN" --version 2>/dev/null | head -1) (engine started directly; the toolbox launcher runs only as Bedrock credential helper), effort ${PR_CODEX_EFFORT:-xhigh}; CODEX_HOME config without MCP/plugin/project sections, shell env \"core\", no project instructions (project_doc_max_bytes=0); --sandbox read-only, sandbox $SANDBOX"
  echo "- Round 1 is an independent first pass, not isolated (see the script header)"
  echo "- Script: \`tools/jeiobs/peer-review/review.sh\`, sha256 \`$(sha256sum "$SELF" | cut -c1-64)\`"
  echo "- Codex home: \`${REVIEW_CODEX_HOME/#$HOME/\~}\`; config.toml sha256 \`${CFG_HASH%% *}\`; skills sha256 \`${SKILLS_HASH%% *}\`"
  echo "- Context sha256 \`$(sha256sum "$CTX" | cut -c1-64)\`; ledger sha256 \`$(sha256sum "$LEDGER" | cut -c1-64)\`"
  echo "- stderr logs: kept locally (not committed), \`$(basename "$DIAG")\`"
  echo "- Topic: $TOPIC"
  if [ -n "$CI" ]; then echo; echo "## CI state at the reviewed commit"; echo; echo '```text'; cat "$CI"; echo '```'; fi
} > "$OUT/00-meta.md"
pin "$OUT/00-meta.md"

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
  verify_evidence "prompt $peer r$r"
  p="$TMP/$peer-r$r.prompt"
  {
    echo "$PREAMBLE"; echo; echo "REVIEWED COMMIT: $FULLSHA"; echo; echo "TOPIC: $TOPIC"
    echo; echo "SHARED CONTEXT:"; cat "$CTX"
    echo; echo "FINDINGS LEDGER (kept by the lead):"; cat "$LEDGER"
    echo; echo "YOUR OWN PREVIOUS REPLY:"; [ -n "$own" ] && cat "$own" || echo "(none: first round)"
    echo; echo "THE OTHER REVIEWER'S LATEST REPLY:"
    if [ "$r" -eq 1 ]; then echo "(not shown in round 1)"; else cat "$other"; fi
  } > "$p" || fail "cannot write the $peer r$r prompt"
  echo "$p"
}
start_peer() { # peer prompt reply stderr  (background; sets PGID[peer])
  # timeout puts itself in a new process group (it is not --foreground), so the
  # whole turn can be killed when it ends.
  if [ "$1" = claude ]; then
    (cd "$TREE" && exec timeout "$TIMEOUT" claude -p --model "${PR_CLAUDE_MODEL:-opus}" --effort "${PR_CLAUDE_EFFORT:-max}" \
      --tools "Read,Glob,Grep" --allowedTools "Read,Glob,Grep" --strict-mcp-config --setting-sources user \
      < "$2" > "$3" 2> "$4") &
  else
    CODEX_HOME="$REVIEW_CODEX_HOME" timeout "$TIMEOUT" "$CODEX_BIN" exec --sandbox read-only --skip-git-repo-check \
      -C "$TREE" -c project_doc_max_bytes=0 -c model_reasoning_effort="${PR_CODEX_EFFORT:-xhigh}" -o "$3" - < "$2" > "$4" 2>&1 &
  fi
  PGID[$1]=$!
}
end_peer() { # peer  (kills what is left of the turn's process group)
  kill -KILL -- "-${PGID[$1]}" 2>/dev/null; unset "PGID[$1]"
}
validate() { # peer round reply
  local peer=$1 r=$2 f=$3 id bad
  [ -s "$f" ] || fail "$peer r$r returned an empty reply"
  # Exactly one VERDICT line, and it is the last non-empty line; each finding ID at most once.
  [ "$(grep -c '^VERDICT' "$f")" = 1 ] || fail "$peer r$r must have exactly one VERDICT line"
  grep -v '^[[:space:]]*$' "$f" | tail -n 1 | grep -qE '^VERDICT: (approve|request changes)[[:space:]]*$' \
    || fail "$peer r$r: the last line must be a valid VERDICT line"
  bad=$(grep -oE "^FINDING $ID_RE \|" "$f" | sort | uniq -d | head -1)
  [ -z "$bad" ] || fail "$peer r$r lists a finding more than once: $bad"
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
  cp "$TMP/$1-r$2.prompt" "$OUT/$n-r$2-$1.prompt.md" || fail "cannot save the $1 r$2 prompt"
  [ "$(sha256sum < "$OUT/$n-r$2-$1.prompt.md")" = "${PROMPT_HASH[$1-r$2]}" ] || fail "$1 r$2 prompt changed during the turn"
  cp "$TMP/$1-r$2.reply" "$OUT/$n-r$2-$1.reply.md" || fail "cannot save the $1 r$2 reply"
  echo "$OUT/$n-r$2-$1.reply.md"
}
pin_saved() { # peer round slot
  local n; n=$(printf '%02d' $(( ($2 - 1) * 2 + $3 )))
  pin "$OUT/$n-r$2-$1.prompt.md"; pin "$OUT/$n-r$2-$1.reply.md"
}

# Round 1: independent first pass. Both peers start together. Each reply is saved
# and pinned as soon as its peer exits; after both have exited, the tamper check
# runs, then both replies are validated.
pa=$(prompt "$FIRST" 1 "" "") || exit 3; pb=$(prompt "$SECOND" 1 "" "") || exit 3
PROMPT_HASH["$FIRST-r1"]="$(sha256sum < "$pa")"; PROMPT_HASH["$SECOND-r1"]="$(sha256sum < "$pb")"
start_peer "$FIRST" "$pa" "$TMP/$FIRST-r1.reply" "$DIAG/r1-$FIRST.stderr.txt"
start_peer "$SECOND" "$pb" "$TMP/$SECOND-r1.reply" "$DIAG/r1-$SECOND.stderr.txt"
declare -A RC=()
ja=${PGID[$FIRST]} jb=${PGID[$SECOND]} left="${PGID[$FIRST]} ${PGID[$SECOND]}"
for _ in 1 2; do
  done=""; wait -n -p done $left; rc=$?
  if [ "$done" = "$ja" ]; then peer=$FIRST slot=1 left=$jb
  elif [ "$done" = "$jb" ]; then peer=$SECOND slot=2 left=$ja
  else fail "round 1: lost track of a peer (wait returned $rc)"; fi
  RC[$peer]=$rc; end_peer "$peer"
  touch "$TMP/$peer-r1.reply"; f=$(save "$peer" 1 $slot) || exit 3; pin_saved "$peer" 1 $slot
  if [ $slot = 1 ]; then a=$f; else b=$f; fi
done
tamper_check "round 1"
[ "${RC[$FIRST]}" -eq 0 ] || fail "$FIRST r1 exited ${RC[$FIRST]}"
[ "${RC[$SECOND]}" -eq 0 ] || fail "$SECOND r1 exited ${RC[$SECOND]}"
validate "$FIRST" 1 "$a"; validate "$SECOND" 1 "$b"
for ((r = 2; r <= ROUNDS; r++)); do
  for side in 1 2; do
    if [ $side = 1 ]; then peer=$FIRST own=$a other=$b; else peer=$SECOND own=$b other=$a; fi
    p=$(prompt "$peer" "$r" "$own" "$other") || exit 3
    PROMPT_HASH["$peer-r$r"]="$(sha256sum < "$p")"
    start_peer "$peer" "$p" "$TMP/$peer-r$r.reply" "$DIAG/r$r-$peer.stderr.txt"
    wait "${PGID[$peer]}"; rc=$?; end_peer "$peer"
    touch "$TMP/$peer-r$r.reply"; f=$(save "$peer" "$r" $side) || exit 3; pin_saved "$peer" "$r" $side
    tamper_check "round $r $peer"
    [ $rc -eq 0 ] || fail "$peer r$r exited $rc"
    validate "$peer" "$r" "$f"
    if [ $side = 1 ]; then a=$f; else b=$f; fi
  done
done
verify_evidence "end of run"
{
  echo "- Finished: $(date -u +%FT%TZ)"
  echo
  echo "## Files written by this run"
  echo
  echo "The hash given for \`00-meta.md\` is of its header (the part before \"Finished\");"
  echo "the runner prints the final hash of every file, this one included, when it exits."
  echo
  for f in "${!EVIDENCE[@]}"; do echo "- \`$(basename "$f")\` sha256 \`${EVIDENCE[$f]%% *}\`"; done | sort
  echo
  echo "## Final dispositions"
  echo
  echo '```text'
  grep -hE '^(FINDING|VERDICT)' "$a" "$b"
  echo '```'
} >> "$OUT/00-meta.md" || fail "cannot append to 00-meta.md"
tail -n 1 "$OUT/00-meta.md" | grep -qxF '```' || fail "00-meta.md did not get its final section"
pin "$OUT/00-meta.md"
grep -hE '^(FINDING|VERDICT)' "$a" "$b"
echo
echo "Evidence (check with: sha256sum -c, from $OUT, before committing):"
for f in "${!EVIDENCE[@]}"; do echo "${EVIDENCE[$f]%% *}  $(basename "$f")"; done | sort -k2
