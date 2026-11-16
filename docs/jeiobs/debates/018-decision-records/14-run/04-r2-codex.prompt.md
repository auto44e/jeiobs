You are one of two independent reviewers (Claude and Codex, different model lineages) in a
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
For a new finding, use ID <your-name>-<n> (for example claude-1) and quote the evidence.

REVIEWED COMMIT: db2b88d342491ea758919d03783916ada7a80d1d

TOPIC: Review run 14 of PR #19: dispose of your findings; review the run-13 fixes.

SHARED CONTEXT:
# PR #19 review run 14 — head db2b88d34

PR #19 = the review-tooling commits after the owner's merge of #18 at 374f842c7 (`git log 270f9d6c9..HEAD`); owner-merged. Since run 12 (the last completed run, reviewed ce0e3c26b): `git diff ce0e3c26b HEAD`. Run 13 (reviewed 207875cad) failed closed after round 2 on a false positive: Claude's reply quoted the session-log search string with the live checkout path and Codex's next session log carried it; its findings claude-20 (narrowed), claude-21, codex-20 are answered in findings.md, and its transcripts are under 13-run/. Lead's stub tests of review.sh in a scratch repo, in addition to those of runs 12-13: a Codex session log containing the quoted string JSON-escaped (as a prompt carries it) passes, the engine's structure ('":"# AGENTS.md instructions for <tree>') still fails the run, and the strict pattern was checked against the real logs of the earlier live test (match) and of run 13 (no match); config.toml with a quoted 'mcp_servers'.x key, with developer_instructions, and with inherit = "all" are each refused (exit 2); an extra skills/evil folder is refused; a Codex stub that edits a .system skill fails the run; the four real review homes pass the allowlist. This run uses REVIEW_CODEX_HOME=~/.config/jeiobs-agent/codex-review.

Severity: Blocker = must not merge; Major = must be fixed before merge; Minor = follow-up. Other agent work runs on this host in separate worktrees; it does not touch this checkout.

FINDINGS LEDGER (kept by the lead):
# Findings ledger — PR #18 and its follow-up #19

Kept by the lead under DECISIONS gate item 4. Severity and status are each
finder's latest, taken from its own FINDING lines in the last completed review run (12-run) (the
"source" column points at them). Findings first raised in rounds 1–2 are quoted in
`01`, `02` and
`04`; later runs are in `05-final` to `12-run`. "open → fixed"
means the lead changed something and the finder has not yet accepted it.
Run 11 failed closed in round 1 because the lead ran `git fetch` in the
repository during the turn (the tamper check saw `refs/remotes/origin/main`
move). Run 13 failed closed after round 2: Claude's reply quoted the search
string with the live checkout path, Codex's next prompt carried that quote into
its session log, and the check matched it. The check now matches only the JSON
structure the engine writes. A failed run is not a review: its dispositions are
not counted, but the new findings it raised are listed.

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
claude-20 | Major | open → fixed (awaiting finder) | claude | 13-run/03-r2-claude.reply.md (FINDING claude-20) | config.toml is parsed as TOML (tomllib) against an exact key allowlist (quoted keys, developer_instructions, inherit elsewhere all refused; approvals_reviewer pinned to auto_review); hashed before the parse and re-checked; skills/ may hold only the engine .system set, pinned by content hash and checked after every turn; 00-meta names the home and both hashes. Run 13 failed closed (false positive, see below), so this is from its round-2 reply
claude-21 | Minor | open → fixed (awaiting finder) | claude | 13-run/03-r2-claude.reply.md (FINDING claude-21) | DECISIONS and README: 05-final reviewed #18 at ac008df4c, run 06 the merged head 374f842c7, runs 07 onwards the #19 commits
codex-19 | Major | open → fixed (awaiting finder) | codex | 12-run/04-r2-codex.reply.md (FINDING codex-19) | OUT must be new or empty (a stale 00-meta.md cannot be reused); the final append is checked and its last line verified before the file is pinned again
codex-20 | Major | open → fixed (awaiting finder) | codex | 13-run/04-r2-codex.reply.md (FINDING codex-20) | as claude-20: TOML parse with an exact allowlist instead of line regexes
CI | Major | fixed | owner | owner instruction (DECISIONS.md, verbatim) | quick checks failed after the force push (runs 43805282657, 43805282753, "bad object"); workflow falls back to HEAD

YOUR OWN PREVIOUS REPLY:
Run-13 fixes accepted: codex-19 and codex-20 are resolved (`review.sh:160`, `407-409`, `110-141`).

Remaining:

- `codex-21` Major: `python3 - "$CFG"` imports `tomllib` without isolation; a reviewed checkout can shadow it with `tomllib.py` (`review.sh:110-112`).
- `codex-22` Major: `credential_export.command`/`args` and nested OTEL dictionaries are arbitrary, so the declared toolbox-only helper is not enforced (`review.sh:117-119`; `DECISIONS.md:133-134`).

FINDING c1 | Blocker | withdrawn | prior withdrawal stands (findings.md:53)
FINDING c2 | Major | withdrawn | prior withdrawal stands (findings.md:54)
FINDING c3 | Major | withdrawn | prior withdrawal stands (findings.md:55)
FINDING c4 | Major | withdrawn | prior withdrawal stands (findings.md:56)
FINDING c5 | Minor | accepted-fix | prior acceptance stands (findings.md:57)
FINDING c6 | Major | withdrawn | prior withdrawal stands (findings.md:58)
FINDING c7 | Minor | accepted-fix | prior acceptance stands (findings.md:59)
FINDING c8 | Major | withdrawn | prior withdrawal stands (findings.md:60)
FINDING M1 | Major | accepted-fix | prior acceptance stands (findings.md:61)
FINDING M2 | Major | accepted-fix | prior acceptance stands (findings.md:62)
FINDING M3 | Major | accepted-fix | prior acceptance stands (findings.md:63)
FINDING N1 | Major | accepted-fix | prior acceptance stands (findings.md:64)
FINDING N2 | Major | accepted-fix | prior acceptance stands (findings.md:65)
FINDING N3 | Major | accepted-fix | prior acceptance stands (findings.md:66)
FINDING N4 | Major | accepted-fix | prior acceptance stands (findings.md:67)
FINDING N5 | Minor | accepted-fix | prior acceptance stands (findings.md:68)
FINDING N6 | Major | accepted-fix | prior acceptance stands (findings.md:69)
FINDING N7 | Major | accepted-fix | prior acceptance stands (findings.md:70)
FINDING codex-1 | Major | accepted-fix | prior acceptance stands (findings.md:71)
FINDING codex-2 | Major | accepted-fix | prior acceptance stands (findings.md:72)
FINDING codex-3 | Major | accepted-fix | prior acceptance stands (findings.md:73)
FINDING codex-4 | Minor | accepted-fix | prior acceptance stands (findings.md:74)
FINDING codex-5 | Major | accepted-fix | prior acceptance stands (findings.md:75)
FINDING codex-6 | Major | accepted-fix | prior acceptance stands (findings.md:76)
FINDING codex-7 | Major | accepted-fix | prior acceptance stands (findings.md:77)
FINDING codex-8 | Minor | accepted-fix | prior acceptance stands (findings.md:78)
FINDING codex-9 | Major | accepted-fix | prior acceptance stands (findings.md:79)
FINDING codex-10 | Major | accepted-fix | prior acceptance stands (findings.md:80)
FINDING codex-11 | Major | accepted-fix | prior acceptance stands (findings.md:81)
FINDING codex-12 | Minor | accepted-fix | prior acceptance stands (findings.md:82)
FINDING codex-13 | Major | accepted-fix | prior acceptance stands (findings.md:83)
FINDING codex-14 | Major | accepted-fix | prior acceptance stands (findings.md:84)
FINDING codex-15 | Major | accepted-fix | prior acceptance stands (findings.md:86)
FINDING codex-16 | Minor | accepted-fix | prior acceptance stands (findings.md:87)
FINDING codex-17 | Major | accepted-fix | prior acceptance stands (findings.md:88)
FINDING codex-18 | Major | accepted-fix | prior acceptance stands (findings.md:91)
FINDING codex-19 | Major | accepted-fix | nonempty OUT and final metadata checks are enforced (review.sh:160, 407-409)
FINDING codex-20 | Major | accepted-fix | TOML parsing and exact top-level validation replace regex checks (review.sh:110-141)
FINDING codex-21 | Major | open | “python3 - "$CFG"” is non-isolated and permits reviewed-tree tomllib shadowing (review.sh:110-112)
FINDING codex-22 | Major | open | credential_export command/args and OTEL nested keys remain arbitrary (review.sh:117-119)
VERDICT: request changes
THE OTHER REVIEWER'S LATEST REPLY:
**Request changes at `db2b88d34` (round 2).** This is the same commit as round 1, but after checking Codex's two findings I agree with both. In round 1 I rated the `python3 -I` gap Minor; I now think codex-21 has to be fixed before merge. claude-20 stays Minor. I'm raising one new Minor, claude-22.

**What I re-checked (read-only):**
- The worktree's HEAD is `db2b88d34`, with one reflog entry (`.git/worktrees/tree/logs/HEAD:1`). The memory dir is empty.
- Nothing is planted at the main checkout's root: no `tomllib*`, `sitecustomize*` or `*.pth`.
- This run's Codex log lists one skill root, `skills/.system` (09-44-59 …df9a:3).
- No session log has an instructions item for this run's checkout.

**codex-21: I agree, and it should block the merge.**
- One correction to where the shadow comes from:
  - At :110 the cwd is the lead's checkout; :158 derives REPO from it.
  - The review worktree doesn't exist until :188.
  - The lead's checkout normally contains the change under review.
- A `tomllib.py` at the repo root is outside the owner-only path (DECISIONS:88), so any self-merged change could add one. From then on, every run started from a checkout that contains it is affected:
  - The file runs before the first turn and outside every tripwire.
  - The script hash in 00-meta (:259) does not change.
  - It can make :139 pass any config.
  - It can change `.system` before :148 pins it.
  - It can write reviewer inputs that are never checked, such as `~/.claude/rules/*.md`, which are in this turn's context.
- Fix: `python3 -I -`. On Python 3.11+ that implies -E, -P and -s. The repo already does this in vst3-check/run.sh:135.

**codex-22: I agree.**
- :117 accepts any string for `command` and any list for `args`, and doesn't check the list's elements.
  - config.toml:15-16 and :22-23 do name the launcher, but only the config hash records that.
- DECISIONS:116 introduces this text as "exactly as enforced". Two of its claims are not enforced:
  - "runs the toolbox launcher … only" (:133-134; review.sh:40-42);
  - "telemetry to a local collector" (:124; review.sh:32-33).
- The otel `D` tables (:119) also make "every key must be on an exact allowlist" untrue (:33-34; DECISIONS:125-126). I listed that as optional in round 1; it belongs under codex-22.
- Either fix works:
  - pin the values: `command` = the launcher path, `args` = `["amzn","credential-process"]`, exporter endpoints on 127.0.0.1;
  - or reword those lines to say the values are only recorded by hash, not checked.

**claude-20 (Minor, open):** items b and c are unchanged from round 1.
- b: :153 still pipes `tr` into `grep -q` under pipefail.
- c: the item-4 wording is still missing.
- Item a is the same defect as codex-21 and closes when it does.
- The optional notes stand:
  - symlinks are not hashed (`find . -type f`, :147);
  - `.system` is not compared with the engine's bundled set.

**claude-22 (new, Minor): the session-log check can still fail a run by mistake.**
- The engine's item is `"type":"input_text","text":"# AGENTS.md instructions for <tree>` (07-16-16 …b820:4).
- But :234 matches any JSON value that starts with that prefix, not just the engine's item.
- Exec records store raw command output as such a value: `"aggregated_output":"db2b88d34 (HEAD, …` (df9a:30).
- So suppose one reply quotes the prefix with the live checkout path, as happened in run 13. If the other peer then runs `grep -o` on its logs, the output starts with the prefix and the run fails. It fails closed, but the run is lost.
- Fix: match on `"type":"input_text","text":"` before the prefix, and update the comment at :231-233.

FINDING k1 | Major | withdrawn | finder-only closure holds (DECISIONS:188-192)
FINDING k2 | Major | withdrawn | tests pass or owner accepts (DECISIONS:100-104)
FINDING k3 | Major | withdrawn | owner-only classes and owner FLEX (DECISIONS:75-93, :253-255)
FINDING k4 | Minor | accepted-fix | review pinned (DECISIONS:105-107), merge pinned to head (:180-182)
FINDING k5 | Minor | accepted-fix | verbatim owner quotes, terms, merge marker (DECISIONS:10-43, :200-202)
FINDING k6 | Minor | accepted-fix | CONTRIBUTING:14-16; lead's own PR defined (DECISIONS:42-43)
FINDING k7 | Minor | withdrawn | withdrawn in r2 (04-r2-crosstalk.md:118)
FINDING k8 | Minor | withdrawn | memory dir still empty (re-checked); shared-dir wording tracked in claude-20
FINDING k9 | Minor | accepted-fix | 018 README:26-27 points to the SHA table
FINDING k10 | Minor | withdrawn | deferred to an owner-merged dependency PR (HANDOFF:110-111); findings.md:28 reads withdrawn
FINDING M1 | Major | accepted-fix | pins, submodules, deps/ owner-only (DECISIONS:83-87)
FINDING M2 | Major | accepted-fix | one scale, FINDING-line sources (DECISIONS:183-187)
FINDING M3 | Major | accepted-fix | runs committed and linked (DECISIONS:244-249)
FINDING N1 | Major | accepted-fix | owner instruction and rewrite recorded (DECISIONS:20-30)
FINDING N2 | Major | accepted-fix | engine (review.sh:101-102), config allowlist (:106-142), unsandboxed stated (DECISIONS:138-140); residue in claude-20, codex-21, codex-22
FINDING N3 | Major | accepted-fix | admin actions owner-only (DECISIONS:90-93)
FINDING N4 | Major | accepted-fix | owner decides first (DECISIONS:95-99)
FINDING N5 | Minor | accepted-fix | owner-hold rule (DECISIONS:75-76)
FINDING claude-1 | Major | accepted-fix | review worktree HEAD db2b88d34, one reflog entry (.git/worktrees/tree/logs/HEAD:1)
FINDING claude-2 | Major | accepted-fix | transcript-only exemption (DECISIONS:172-177)
FINDING claude-3 | Minor | accepted-fix | round-1 comment (review.sh:355-357) matches code (:358-376)
FINDING claude-4 | Minor | accepted-fix | stderr local, CLI versions, full SHA (review.sh:163-164, :254-257)
FINDING claude-5 | Minor | accepted-fix | CI runs recorded (history :41-44)
FINDING claude-6 | Minor | accepted-fix | inconclusive defined (DECISIONS:193-196)
FINDING claude-7 | Minor | accepted-fix | merge pins head (DECISIONS:180-182); exemption (:172-177); peers wording (:35-38)
FINDING claude-8 | Minor | accepted-fix | CI state passed via -C into 00-meta (review.sh:264)
FINDING claude-9 | Minor | accepted-fix | 018 README:11-12 says stderr stays local, run 06 trimmed
FINDING claude-10 | Minor | accepted-fix | raised IDs become required (review.sh:341)
FINDING claude-11 | Minor | accepted-fix | saved and pinned before tamper check and validation (review.sh:370-376, :384-387)
FINDING claude-12 | Major | accepted-fix | in-turn writes to snapshots and saved replies fail the run (:196-206, :224, :346); cleanup exits 3 (:172-186)
FINDING claude-13 | Minor | accepted-fix | engine binary (:101-102), core env (:116, :140-141), HANDOFF:106-109
FINDING claude-14 | Minor | accepted-fix | launcher as credential helper stated (review.sh:40-42, :257; DECISIONS:132-134), matching config.toml:15, :22; enforcement gap is codex-22
FINDING claude-15 | Major | accepted-fix | process groups (:308-323); tripwire wording (:47-72; DECISIONS:138-153); final hashes printed (:410-413)
FINDING claude-16 | Major | accepted-fix | --setting-sources user (:313); refusal at any depth (:189-192)
FINDING claude-17 | Major | accepted-fix | project_doc_max_bytes=0 (:317); structural match (:229-240) fits the engine's item; residual false positive in claude-22
FINDING claude-18 | Major | accepted-fix | session-log hits in a variable, grep status 2 fails (:234-236); the :153 pipe is tracked in claude-20
FINDING claude-19 | Minor | accepted-fix | .agents/.codex refused at any depth (:189-192; DECISIONS:135-137); this run's only skill root is skills/.system (df9a:3)
FINDING claude-20 | Minor | open | left: tr piped into grep -q under pipefail (:153); wording on account-writable inputs (home, ~/.claude, launcher, binaries); python3 without -I (:110) closes with codex-21
FINDING claude-21 | Minor | accepted-fix | DECISIONS:3-6, :246-249 and README:3-4 put run 06 under #18's merged head and runs 07+ under #19
FINDING claude-22 | Minor | open | :234 also matches raw command output ("aggregated_output":"…, df9a:30), not only the engine's input_text item (b820:4); a peer's grep -o can fail the run
VERDICT: request changes
