# Review run

- Started: 2026-11-17T13:16:18Z
- Reviewed commit: `887847745cddbccee59a0cb470e15d138a4e58b4` (peers ran in a detached worktree at this commit)
- Author: codex; rounds: 2 (round 1 independent first pass, both at once, not isolated); cross-talk order: claude, codex; timeout per turn: 1800s
- Claude: claude 2.1.326.1734 (ASBX Claude Code, channel stable), model opus, effort max; tools Read, Glob, Grep; no MCP
- Codex: codex-cli 0.169.0 (engine started directly; the toolbox launcher runs only as Bedrock credential helper), effort xhigh; CODEX_HOME config without MCP/plugin/project sections, shell env "core", no project instructions (project_doc_max_bytes=0); --sandbox read-only, sandbox available
- Round 1 is an independent first pass, not isolated (see the script header)
- Script: `tools/jeiobs/peer-review/review.sh`, sha256 `63e7012b6f02f3151f72ee2ad2c1a6d849725472001d706e960af791c4eb99a4`
- Codex home: `~/.config/jeiobs-agent/codex-review-b`; config.toml sha256 `e3e76dcf155ea4674ca7ba34028a06eedba3e87ed3812d50da13ede1a7efbd44`; skills sha256 `253397a7192e7cf5c8088186e9a468f0696218ee6d62efe83d06c761e627bfeb`
- Context sha256 `12711e767b56ef7f92ee3065f73ae16f57d1972a780a4c69db6a05275ecb60f4`; ledger sha256 `b7a09f2c0929718cda7a0a83907eadea94fef339757e16e5cdb48f45e49f7b1e`
- stderr logs: kept locally (not committed), `03-run-20261117T131617Z`
- Topic: Review run 03 of pr33.

## CI state at the reviewed commit

```text
CI queued at review start for 887847745cddbccee59a0cb470e15d138a4e58b4
local: jc-unit, jc-cases (1128 lines) PASS
```
- Finished: 2026-11-17T13:33:44Z

## Files written by this run

The hash given for `00-meta.md` is of its header (the part before "Finished");
the runner prints the final hash of every file, this one included, when it exits.

- `00-context.md` sha256 `12711e767b56ef7f92ee3065f73ae16f57d1972a780a4c69db6a05275ecb60f4`
- `00-ledger-in.md` sha256 `b7a09f2c0929718cda7a0a83907eadea94fef339757e16e5cdb48f45e49f7b1e`
- `00-meta.md` sha256 `85e335ec1e1656b527612f8cb3db7cf14b62f5ec6522591b9029c9915933d29d`
- `01-r1-claude.prompt.md` sha256 `ef707dd9bb257f418b7f404ebdfa05362dec144ac066cc7d1d8eaf0a159c7c5f`
- `01-r1-claude.reply.md` sha256 `e088704ed38232e8d62023a6d017cc8b80eabd5adbc4330c5dcb8059b6b77ad4`
- `02-r1-codex.prompt.md` sha256 `ef707dd9bb257f418b7f404ebdfa05362dec144ac066cc7d1d8eaf0a159c7c5f`
- `02-r1-codex.reply.md` sha256 `a2ae874c1e0a43cc259101702837d660a43dec0802b8614e501faf1e8aa63ff9`
- `03-r2-claude.prompt.md` sha256 `0bf35d7b7c38d0009d7e716851e189bf65f5d8d13ac5cbe36cacfaac5c4dc573`
- `03-r2-claude.reply.md` sha256 `89669c3a9aed60f91cb7c1840ac3aa0004f93c7e6ad20e2753a9eba434a4befa`
- `04-r2-codex.prompt.md` sha256 `0dbfd317825114e528e80218a4650905fe63b99927f910e385340e521e33927b`
- `04-r2-codex.reply.md` sha256 `7e69f3e9f83f68794f7fd9c8fc9505292e912bb4cb6f001d462aed7cfe9db297`

## Final dispositions

```text
FINDING claude-1 | Major | accepted-fix | rounds then clamps (Value.cpp:602-681), FitsSigned requires sign extension (:317-334), Relative kept
FINDING claude-2 | Major | accepted-fix | one strict parser for ParseMapping/LoadMapping/CompileMapping(bytes)/ParseEvent (Mapping.cpp:257-287,1583-1603,1712-1719,1730-1735)
FINDING claude-3 | Major | accepted-fix | hotkey always output-control (Mapping.cpp:1377-1378), wildcards refused (:1633-1641)
FINDING claude-4 | Major | accepted-fix | url/input/ffmpeg_options/playlist denied (jc_unit.cpp:164-171); PR4 must keep field a single key
FINDING claude-5 | Major | accepted-fix | nlohmann::json removes the quadratic insert; flooding of the dup-key set is claude-14
FINDING claude-6 | Major | accepted-fix | segment and key escaping injective (Mapping.cpp:1037-1073,1435-1447), merge groups by escaped key
FINDING claude-7 | Minor | open | r4.5 docs absent: 006:31,318-323,332-333, README:12,15 stale; E_SCHEMA routes for enabled (Mapping.cpp:884-885) vs r1.3 and caps Limits.hpp:30-31 unrecorded
FINDING claude-8 | Minor | open | not in r4 (07:160-168): contract (07:65,85-88) doesn't fix an emitted event's slot; edge rule Mapping.cpp:1227-1230 relies on it; emit=Meta (:1388-1389) unrecorded; PR3 follow-up
FINDING claude-9 | Minor | accepted-fix | lazy cursor over emit-only first-segment index (Mapping.cpp:1165-1255), no stored edges, O(n) memory
FINDING claude-10 | Minor | accepted-fix | wmain converts UTF-16 arguments as jw-check does (jc-check.cpp:633-655)
FINDING claude-11 | Major | accepted-fix | reserve only for elements != size_t(-1) && <= 1024 (Mapping.cpp:192-194); start_array ignores count (:223); Windows run on this head must be cited, none in .github
FINDING claude-12 | Minor | accepted-fix | MakeInt stores SignExtended raw (Value.cpp:467); MakeInt(w, v.intRaw()) round-trips int8 -128 (jc_unit.cpp:66-74)
FINDING claude-13 | Minor | accepted-fix | expected file without effect, cases.txt:913 accepts only it; nits: comment :904-905 stale, expected/compile/key-hotkey.json orphaned
FINDING claude-14 | Minor | open | dup-key "std::unordered_set<std::string> keys" (Mapping.cpp:93,205) hash-floodable on MSVC (unseeded FNV-1a, low-bit buckets); use frame.value.contains (std::map)
VERDICT: approve
FINDING codex-1 | Major | accepted-fix | Strict parsing is unified across mapping and event entry points (Mapping.cpp:1509-1513,1592-1603,1712-1737).
FINDING codex-2 | Minor | accepted-fix | Requirements enforce ASCII syntax (Mapping.cpp:338-349).
FINDING codex-3 | Major | accepted-fix | Address and target keys escape all separators injectively (Mapping.cpp:1037-1072,1435-1502).
FINDING codex-4 | Major | accepted-fix | Invalid integer and IEEE32 raw values are rejected (Value.cpp:441-477).
FINDING codex-5 | Minor | accepted-fix | Hotkey expected output omits effect while the fixture supplies it (key-hotkey-with-effect.json:1; key-hotkey.json:9; cases.txt:913).
FINDING codex-6 | Minor | accepted-fix | Unused bytes local was removed (jc-check.cpp:162-167).
FINDING codex-7 | Minor | open | Duplicate detection uses attacker-controlled `"std::unordered_set<std::string> keys"` and can be hash-flooded within the 4 MiB input cap (Mapping.cpp:93,199-206; Limits.hpp:17).
VERDICT: approve
```
