# Review run

- Started: 2026-11-17T01:20:12Z
- Reviewed commit: `e9de5fb2836bc49a0aef887c2b134d3e8e9d60b3` (peers ran in a detached worktree at this commit)
- Author: codex; rounds: 2 (round 1 independent first pass, both at once, not isolated); cross-talk order: claude, codex; timeout per turn: 1800s
- Claude: claude 2.1.326.1734 (ASBX Claude Code, channel stable), model opus, effort max; tools Read, Glob, Grep; no MCP
- Codex: codex-cli 0.169.0 (engine started directly; the toolbox launcher runs only as Bedrock credential helper), effort xhigh; CODEX_HOME config without MCP/plugin/project sections, shell env "core", no project instructions (project_doc_max_bytes=0); --sandbox read-only, sandbox available
- Round 1 is an independent first pass, not isolated (see the script header)
- Script: `tools/jeiobs/peer-review/review.sh`, sha256 `63e7012b6f02f3151f72ee2ad2c1a6d849725472001d706e960af791c4eb99a4`
- Codex home: `~/.config/jeiobs-agent/codex-review-b`; config.toml sha256 `e3e76dcf155ea4674ca7ba34028a06eedba3e87ed3812d50da13ede1a7efbd44`; skills sha256 `253397a7192e7cf5c8088186e9a468f0696218ee6d62efe83d06c761e627bfeb`
- Context sha256 `a5b12d2e9f8864388be7c6126b17ef5c121b7f5e8e46f855734dee50757bd915`; ledger sha256 `b55980b7cea13d91419d6d7e8f3aeac2d94745914ab0003e1e061913ba78329f`
- stderr logs: kept locally (not committed), `02-run-20261117T012011Z`
- Topic: Review run 02 of PR #28 (S1-PR2).

## CI state at the reviewed commit

```text
quick checks pending at start
local: jw-unit, jw-fuzz-smoke, jw-cases (968 lines) PASS on Linux, also under ASan+UBSan
```
- Finished: 2026-11-17T01:46:41Z

## Files written by this run

The hash given for `00-meta.md` is of its header (the part before "Finished");
the runner prints the final hash of every file, this one included, when it exits.

- `00-context.md` sha256 `a5b12d2e9f8864388be7c6126b17ef5c121b7f5e8e46f855734dee50757bd915`
- `00-ledger-in.md` sha256 `b55980b7cea13d91419d6d7e8f3aeac2d94745914ab0003e1e061913ba78329f`
- `00-meta.md` sha256 `68a702d26ea688810fc7eae72b5081bae49ac7a328134e2533c638a2fe3b0f91`
- `01-r1-claude.prompt.md` sha256 `224b4086a0b9e65c6b42a57d76a183437cde3ef38dd4c43ea2307abf10b746bd`
- `01-r1-claude.reply.md` sha256 `98ef6f66c675cc303871a495fc56540421e3c93c1cdf5f9c430bab2eb2afe73b`
- `02-r1-codex.prompt.md` sha256 `224b4086a0b9e65c6b42a57d76a183437cde3ef38dd4c43ea2307abf10b746bd`
- `02-r1-codex.reply.md` sha256 `145c704d01bae7bc7791d82f96c24ec41c73e2fb903104b0d385658ea8f366b4`
- `03-r2-claude.prompt.md` sha256 `327e4fdde9bbed52c8e29d6deffc1be9cc8c5cbeb72d487df3669806f993398c`
- `03-r2-claude.reply.md` sha256 `69c80a534ea6c8ce8249b80af1b08098a6b61a8225d8ebb4011d8e32c1fe04fb`
- `04-r2-codex.prompt.md` sha256 `19bbebd6d6be0d8fd33924b604ebb7f7013646cc87fd5bed5e7b5671bb7596ea`
- `04-r2-codex.reply.md` sha256 `89cee323d8b054a922e5a9a8d50787c0078d754bf68b5f6ff12a0d16e687f3c6`

## Final dispositions

```text
FINDING claude-1 | Major | accepted-fix | IniScan follows config-file.c header/line/key rules; the five repros redact (jw_unit.cpp:474-483, cases 843-849)
FINDING claude-2 | Major | accepted-fix | URL rule applied to input regardless of is_local_file (Redactor.cpp:696); cases 914-920
FINDING claude-3 | Major | open | INI redactions copy the section name into each pointer (Redactor.cpp:824,543,893: 1 MiB header + 10^6 "Token=" → ~1 TiB); find('\\') scans to the document end per URL value (:564: 64 MiB collection → ~1.4e13 bytes)
FINDING claude-4 | Major | accepted-fix | app.modules accepts a top-level array (Redactor.cpp:946-951); cases 958-969
FINDING claude-5 | Minor | accepted-fix | overlapping and touching ranges merged (Redactor.cpp:875-879); cases 789-793
FINDING claude-6 | Minor | open | versioned_id fixed; groups (OBSBasic_SceneCollections.cpp:1270-1274) and the audio-device objects (:1259-1264) still unscanned and not deferred in writing
FINDING claude-7 | Minor | accepted-fix | one leading BOM skipped in documents (JsonScan.cpp:646-649); manifests still E_BOM
FINDING claude-8 | Minor | accepted-fix | the four drift items fixed (WORKSPACE_FORMAT.md:98,288,295; case 943)
FINDING claude-9 | Major | accepted-fix | query runs from the first '?', values to '&' or end (Redactor.cpp:487-516); SRT cases 783-788
FINDING claude-10 | Minor | accepted-fix | whole-string URL rule; authority ends only at / ? # (Redactor.cpp:449,480-485)
FINDING claude-11 | Minor | accepted-fix | number parsing is locale-independent (JsonScan.cpp:425); portability tracked as claude-13
FINDING claude-12 | Minor | open | an ApplyEdits failure stores the unredacted input as Stored "redacted"/"exact" (Redactor.cpp:972-975,994-997); fuzz CheckResult passes it (jw_fuzz_smoke.cpp:59-72)
FINDING claude-13 | Major | open | floating-point std::from_chars (JsonScan.cpp:425-426) is missing from libc++ before LLVM 20, so the macos-15 job likely fails to compile; r3 item 10 required a fallback; withdrawn if macos-15 passes
FINDING claude-14 | Minor | open | spec lags r3: §8:348 still limits input to is_local_file false; §12 lacks the INI key/section limits; query rule and 1b fallback only in the contract
FINDING claude-15 | Minor | open | 1b fallback never sees Token/RefreshToken lines before the first header (IniScan.cpp:163-165): "Token=S\n[Twitch] ;x\n" is uncertain yet keeps S
VERDICT: request changes
FINDING codex-1 | Major | accepted-fix | Overlapping edits are merged before application (Redactor.cpp:864-879).
FINDING codex-2 | Major | accepted-fix | String limits are checked before appending (JsonScan.cpp:186-199,314-317).
FINDING codex-3 | Minor | accepted-fix | Export audio errors return `target.audio` (Manifest.cpp:543-565).
FINDING codex-4 | Major | open | Empty/lone `[` headers return without fail-closed fallback; later tokens survive (IniScan.cpp:163-176; 19-pr2-contract-r3.md:68-71).
FINDING codex-5 | Minor | open | Format docs still limit ffmpeg input redaction to `is_local_file` false, contradicting r3 (WORKSPACE_FORMAT.md:348; 19-pr2-contract-r3.md:19-21).
VERDICT: request changes
```
