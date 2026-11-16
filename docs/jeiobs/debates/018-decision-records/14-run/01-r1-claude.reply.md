**Approve at `db2b88d34`.** claude-20's code items are fixed, so I'm lowering it to Minor. It stays open for three small items. I accept the claude-21 fix. No new findings.

**Checks (read-only):**
- The review worktree's HEAD is `db2b88d34`, with one reflog entry (`.git/worktrees/tree/logs/HEAD:1`). The memory dir is empty.
- The `codex-review` home has no `AGENTS.md`, `.env` or `rules/`. `skills/` holds only `.system`. Its `config.toml` (lines 1-33) fits the allowlist at :113-120.
- Session-log check (:234):
  - The engine's own instructions item (rollout `07-16-16 …b820:4`) starts its JSON string value with the prefix, so the pattern matches it.
  - This run's Codex logs carry the quoted pattern only JSON-escaped (`\":\"`), which the pattern cannot match.

**claude-20, fixed:**
- Item 1: TOML parse against an exact allowlist (:110-142), with `approvals_reviewer` pinned (:115). The config is hashed before the parse and re-checked after it (:109, :149) and after every turn (:241).
- Item 2: `skills/` may hold only `.system` (:145-146), and its hash is checked after every turn (:147-148, :242).
- Item 3: 00-meta names the home and both hashes (:260).

**claude-20, still open (Minor):**
- **a. `python3 -` runs without `-I` (:110).**
  - With `python3 -`, `sys.path[0]` is the cwd, which is the lead's checkout (usage at :5).
  - So a `tomllib.py` or `tomllib/` there replaces the stdlib parser, and the allowlist then passes anything. So do `PYTHONPATH` and user-site `.pth` files.
  - I asked for `python3 -I` in run 13 (13-run/01-r1-claude.reply.md:29). The repo already runs its Python heredocs that way (vst3-check/run.sh:135; results.py:1 "always run as python3 -I").
  - Nothing is planted now.
- **b. :153 still pipes `tr` into `grep -q` under pipefail (:80).** I flagged this in run 13 as old :114 (13-run/01-r1-claude.reply.md:20).
  - When grep exits on an early match, tr gets SIGPIPE, the pipeline returns 141, and the PID is never printed.
  - This always happens for an environment over 64 KiB. :230 states the rule this line breaks.
  - Fix: `grep -qzxF "CODEX_HOME=$REVIEW_CODEX_HOME" "$p/environ"`.
- **c. The item-4 wording is still missing** (header :24-72; DECISIONS:116-159). It should say that:
  - the runner checks only `config.toml` and `skills/` in the home;
  - any process on this account can change everything else, and nothing records or checks it beyond the CLI versions in 00-meta (:256-257). That covers:
    - the rest of the home; it holds the launcher's `.builder-mcp-setup-complete` marker;
    - `~/.claude`: `rules/*.md` and `outputStyle` (settings.json:59) are both in this turn's context, and every worktree shares one memory dir;
    - the launcher (config.toml:15, :22);
    - the engine and `claude` binaries (`REVIEW_CODEX_BIN` at :101; `claude` from PATH at :104).
- **Optional, along with c:**
  - The hash covers only regular files (`find . -type f`, :147), so a symlink added under `skills/` goes unseen after a turn. :146 runs only at start.
  - The otel exporter tables accept any table (`D`, :119), so "every key must be on an exact allowlist" (:33-34; DECISIONS:125-126) is not true for them.
  - The `.system` content is recorded by hash, not checked against the engine's bundled set.
- **Closes when** a–c are done.

**claude-21, accepted:** DECISIONS:3-6, :246-249 and README:3-4 now put 05-final and run 06 under #18 and runs 07 onwards under #19. Optional: findings.md still has no line naming the merge, and nothing mentions that both peers requested changes in run 06 (06-run/00-meta.md:44, :73).

**codex-19 and codex-20 (Codex decides):** the empty-OUT check (:160) and the checked final append (:407-408) look right. The TOML allowlist covers codex-20's cases; item a applies to it too.

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
FINDING N2 | Major | accepted-fix | engine (review.sh:101-102), config allowlist (:106-142), unsandboxed stated (DECISIONS:138-140); residue in claude-20
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
FINDING claude-14 | Minor | accepted-fix | launcher as credential helper stated (review.sh:40-42, :257; DECISIONS:132-134), matching codex-review config.toml:15, :22
FINDING claude-15 | Major | accepted-fix | process groups (:308-323); tripwire wording (:47-72; DECISIONS:138-153); final hashes printed (:410-413)
FINDING claude-16 | Major | accepted-fix | --setting-sources user (:313); refusal at any depth (:189-192)
FINDING claude-17 | Major | accepted-fix | project_doc_max_bytes=0 (:317); structural session-log match (:229-240) fits the engine's item
FINDING claude-18 | Major | accepted-fix | session-log hits in a variable, grep status 2 fails (:234-236); the :153 pipe is tracked in claude-20
FINDING claude-19 | Minor | accepted-fix | .agents/.codex refused at any depth (:189-192; DECISIONS:135-137); review home skills/ holds only .system
FINDING claude-20 | Minor | open | lowered: TOML allowlist, skills pin, 00-meta record done; left: python3 without -I (:110), tr piped into grep -q under pipefail (:153), wording on account-writable inputs (home, ~/.claude, launcher, binaries)
FINDING claude-21 | Minor | accepted-fix | DECISIONS:3-6, :246-249 and README:3-4 put run 06 under #18's merged head and runs 07+ under #19
VERDICT: approve
