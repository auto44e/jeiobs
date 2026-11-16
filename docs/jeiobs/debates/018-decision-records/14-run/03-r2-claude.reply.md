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
