# Review run

- Started: 2026-11-17T09:51:50Z
- Reviewed commit: `1a8bd1eb4cbb54afb16eeb18d6b14a34ab82d9ad` (peers ran in a detached worktree at this commit)
- Author: codex; rounds: 2 (round 1 independent first pass, both at once, not isolated); cross-talk order: claude, codex; timeout per turn: 1800s
- Claude: claude 2.1.326.1734 (ASBX Claude Code, channel stable), model opus, effort max; tools Read, Glob, Grep; no MCP
- Codex: codex-cli 0.169.0 (engine started directly; the toolbox launcher runs only as Bedrock credential helper), effort xhigh; CODEX_HOME config without MCP/plugin/project sections, shell env "core", no project instructions (project_doc_max_bytes=0); --sandbox read-only, sandbox available
- Round 1 is an independent first pass, not isolated (see the script header)
- Script: `tools/jeiobs/peer-review/review.sh`, sha256 `63e7012b6f02f3151f72ee2ad2c1a6d849725472001d706e960af791c4eb99a4`
- Codex home: `~/.config/jeiobs-agent/codex-review-b`; config.toml sha256 `e3e76dcf155ea4674ca7ba34028a06eedba3e87ed3812d50da13ede1a7efbd44`; skills sha256 `253397a7192e7cf5c8088186e9a468f0696218ee6d62efe83d06c761e627bfeb`
- Context sha256 `aa8635f3838dbd2a2541744908f6d488dbb5ec6377afbda94f0dd92e2345cb1c`; ledger sha256 `0e7944720746fcce027449df5aed0d9aae079aed74a565ae4b5f397cc6e9223a`
- stderr logs: kept locally (not committed), `01-run-20261117T095149Z`
- Topic: Review run 01 of pr33.

## CI state at the reviewed commit

```text
quick checks pending at start
local: jc-unit, jc-cases (916 lines) PASS on Linux, also under ASan+UBSan
```
- Finished: 2026-11-17T10:19:20Z

## Files written by this run

The hash given for `00-meta.md` is of its header (the part before "Finished");
the runner prints the final hash of every file, this one included, when it exits.

- `00-context.md` sha256 `aa8635f3838dbd2a2541744908f6d488dbb5ec6377afbda94f0dd92e2345cb1c`
- `00-ledger-in.md` sha256 `0e7944720746fcce027449df5aed0d9aae079aed74a565ae4b5f397cc6e9223a`
- `00-meta.md` sha256 `2089ec42db419e8b61b72b58cf51630020588fc1017cd608ab26b30cf368ae39`
- `01-r1-claude.prompt.md` sha256 `5fee5ec168b2aaa0c4ba3a5723f5b39e4567252e282b1c9ea7203d429a81ecfa`
- `01-r1-claude.reply.md` sha256 `16947b99a10847e755be5385bad4401234344a3aa0ff483ac340d65ccbc72ef5`
- `02-r1-codex.prompt.md` sha256 `5fee5ec168b2aaa0c4ba3a5723f5b39e4567252e282b1c9ea7203d429a81ecfa`
- `02-r1-codex.reply.md` sha256 `5dd3c4cacef960a4c6d191b13ec3be63f7ac42bfbbe194cdb859b9145599fb69`
- `03-r2-claude.prompt.md` sha256 `cebbc82e8cb3469af28723e677cdf0a0737ec8c3867ef1bd3aafd876e81e62d3`
- `03-r2-claude.reply.md` sha256 `4bcfd970edadf1c4c3adb2065f17ae79f39eeb5817d4e5d226304be73a568939`
- `04-r2-codex.prompt.md` sha256 `7777113e91a465c901fbd97e3b525e0f1983e73a05eb8b2d4cf7a1481e3e1e94`
- `04-r2-codex.reply.md` sha256 `91c149f2363f28182ece6b8ef405c52e5ffa981ff04cc3f7f9603849730d9495`

## Final dispositions

```text
FINDING claude-1 | Major | open | FromDoubleInCodeSpace wraps instead of clamping (uint7 127.5→0, int8 127.5→−128, Value.cpp:576-601); FitsSigned never rejects (MakeInt(8,300)→44); relative_ dropped; no fractional-input tests
FINDING claude-2 | Major | open | ParseMapping/CompileMapping(bytes) use the lax pre-r1 schema (Mapping.cpp:862-888,1455,1473,1593); only LoadMapping is strict; jc-unit tests only the lax path (same defect as codex-1)
FINDING claude-3 | Major | open | hotkey class comes from the file's declared effect (Mapping.cpp:1255-1265), which is outside target_key (:1355-1358): OBSBasic.StartStreaming with effect "scene" passes a wildcard, against A2/A13
FINDING claude-4 | Major | open | hard deny omits section 8 source secrets (browser_source url, ffmpeg_source input/ffmpeg_options, vlc_source playlist value; WORKSPACE_FORMAT.md:350) that A1/item 9 require; cases.txt:605 pins url as allowed
FINDING claude-5 | Major | open | ordered_json member insert is a linear scan (Mapping.cpp:34,147): parsing a ~500k-key object that fits in 4 MiB takes Θ(n²) time; nlohmann::json is a drop-in fix
FINDING claude-6 | Major | open | raised from Minor, same defect as codex-3: emit target_key not injective (unescaped "/" in AddressKey, Mapping.cpp:1054-1064), "\" unescaped against r1.5 (:1328), and merge groups by this string (:1535-1539)
FINDING claude-7 | Minor | open | r1.3/r1.5/r1.8 and Progress disagree with code and cases (incl. codex-2's café); unrecorded caps, check order, A11 prefix/inert-denied deviations; stale 006:31, 006:318
FINDING claude-8 | Minor | open | cycle edge rule is sound only if an emitted event keeps the emitting route's slot; load-bearing emit=meta unrecorded (12:52-53)
FINDING claude-9 | Minor | open | merge check O(g²) with set inserts (Mapping.cpp:1541-1562); cycle graph keeps edges to non-emit routes, 25M edges from ~1.3 MB (:1121-1136)
FINDING claude-10 | Minor | open | jc-check takes ANSI argv on Windows (jc-check.cpp:411), unlike jw-check's wmain (jw-check.cpp:409-441); io-validate-utf8-path cannot pass there
VERDICT: request changes
FINDING codex-1 | Major | open | Public ParseMapping/CompileMapping(bytes) use lax parsing: missing enabled/requires and empty addresses are accepted (`Mapping.cpp:878-912,1455,1470-1473`; contract:115-118).
FINDING codex-2 | Minor | open | IsRequirement accepts non-ASCII bytes, so `requires:["café"]` violates the required ASCII regex (`Mapping.cpp:332-350`; contract:115-116).
FINDING codex-3 | Major | open | Emit target keys are non-injective: `/` is unescaped and `\` is omitted, so distinct addresses collide (`Mapping.cpp:1041-1063,1324-1333,1378-1380`; contract:119).
FINDING codex-4 | Major | open | MakeInt masks invalid high bits after ineffective validation, and MakeFloatBits truncates oversized IEEE32 raw values (`Value.cpp:286-310,424-446`; contract:17-24).
VERDICT: request changes
```
