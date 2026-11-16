# Review run

- Started: 2026-11-16T08:53:57Z
- Reviewed commit: `6c8269596926acf9e01bd398073a8aa23cbd0d86` (peers ran in a detached worktree at this commit)
- Author: codex; rounds: 2 (round 1 independent first pass, both at once, not isolated); cross-talk order: claude, codex; timeout per turn: 1800s
- Claude: claude 2.1.325.1734 (ASBX Claude Code, channel stable), model opus, effort max; tools Read, Glob, Grep; no MCP
- Codex: codex-cli 0.169.0 (engine started directly; the toolbox launcher runs only as Bedrock credential helper), effort xhigh; CODEX_HOME config without MCP/plugin/project sections, shell env "core", no project instructions (project_doc_max_bytes=0); --sandbox read-only, sandbox UNAVAILABLE on this host (bubblewrap cannot create user namespaces); Codex commands ran unsandboxed, tripwires only
- Round 1 is an independent first pass, not isolated (see the script header)
- Script: `tools/jeiobs/peer-review/review.sh`, sha256 `598e463168267d25240404cc43ef709c60b50d2fb158c14e14b72da163741068`
- Context sha256 `e96a6baccb4e9b442180eba5bcec0d285c730bac6d76e08abf0ecb17be17c7a7`; ledger sha256 `a74c6d55cba0983b5c69c55c7aba0872043088939d09a95a641e0fe84473dbb7`
- stderr logs: kept locally (not committed), `01-run-20261116T085356Z`
- Topic: Review run 01 of PR #21 (S1-PR1).

## CI state at the reviewed commit

```text
quick checks for 6c8269596: pending at start (push just made)
local: jw-unit PASS, jw-cases 328/328 PASS (Linux); check_scaffold.py PASS
```
- Finished: 2026-11-16T09:24:43Z

## Files written by this run

The hash given for `00-meta.md` is of its header (the part before "Finished");
the runner prints the final hash of every file, this one included, when it exits.

- `00-context.md` sha256 `e96a6baccb4e9b442180eba5bcec0d285c730bac6d76e08abf0ecb17be17c7a7`
- `00-ledger-in.md` sha256 `a74c6d55cba0983b5c69c55c7aba0872043088939d09a95a641e0fe84473dbb7`
- `00-meta.md` sha256 `32fa7213371f9a127b10e26ae142f28901533702668529de4d38d8686ccd4c0a`
- `01-r1-claude.prompt.md` sha256 `b6f142f0ea85557e6b56527af02e6cbb4e6257987636288773d481627bc42e76`
- `01-r1-claude.reply.md` sha256 `192d85059141559735911cdc44ae88318b06ab1d7449d9bcf4d85e0321969913`
- `02-r1-codex.prompt.md` sha256 `b6f142f0ea85557e6b56527af02e6cbb4e6257987636288773d481627bc42e76`
- `02-r1-codex.reply.md` sha256 `fffdcaab6333526aa8bb1d383718ef801cfe24043319c13cd60b1383d8b18782`
- `03-r2-claude.prompt.md` sha256 `af5a10d3980fe4675e45b8a2093cfefc601403c7b05e100ff2e7fbfe1c3cd657`
- `03-r2-claude.reply.md` sha256 `cbb45e213f5a43fb4011aabeb9a4360f54dab3630e9b8cebc8c4ae09947b4c0f`
- `04-r2-codex.prompt.md` sha256 `ffd54077880fa853134d7eb4f9643fbac2b35d601ef6fd7e4e638dffc3b044f3`
- `04-r2-codex.reply.md` sha256 `1fcef1e91110745219fe9cfbd44a89ad14360ca5030c8250d558f967d1604ba6`

## Final dispositions

```text
FINDING claude-1 | Major | open | restore/import write gate is PathRules only (post-M0 this includes the store and machine-id); bind (root,path,role) to the §7 allowlist and check collisions on resolved destinations
FINDING claude-2 | Major | open | ProjectForExport throws on validator-accepted input (Manifest.cpp:1060; unreviewed optional at :537-545); omitted-entry fidelity and `now` unvalidated
FINDING claude-3 | Major | open | secret registry misses ffmpeg_source input/ffmpeg_options and vlc_source playlist URLs despite "lists every secret key"
FINDING claude-4 | Minor | open | export copies unknown content (creator/active/compat/dropped_ext/app_keys), --export accepts local-only fields, include fidelity not derived
FINDING claude-5 | Minor | open | live OBS states PathRules rejects (Aux, Main+main on Linux, stray reserved-char/non-UTF-8 names) have no capture outcome; 8.3, HFS+, file-vs-directory gaps
FINDING claude-6 | Minor | open | hash and parse in separate reads (revision hashes unbounded input before the 4 MiB check), unused LoadedManifest, narrow-path ifstream on Windows, CarryOver output not re-validated
FINDING claude-7 | Minor | open | live-value scan false-positives on short usernames; unparseable omitted size reveals secret length
VERDICT: request changes
FINDING codex-1 | Major | open | Export include dereferences optional `unreviewed` data accepted by validation (Manifest.cpp:504-545,1056-1061).
FINDING codex-2 | Major | open | Decisions input uses unrestricted `resize`/`Json::parse`, bypassing limits (jw-check.cpp:56-74,212-223).
FINDING codex-3 | Major | open | `CON .txt` passes device-name validation despite Windows trimming spaces (PathRules.cpp:96-103,193-207; WORKSPACE_FORMAT.md:624-625).
FINDING codex-4 | Minor | open | `PrintError` writes export errors to stdout despite the contract requiring nothing there (jw-check.cpp:32-38; 04-pr1-contract.md:45-46).
FINDING codex-5 | Minor | open | `CarryOver` copies parent extensions without rechecking output limits, so valid inputs can produce an unreadable child (Manifest.cpp:871-875,893-900;821-824).
VERDICT: request changes
```
