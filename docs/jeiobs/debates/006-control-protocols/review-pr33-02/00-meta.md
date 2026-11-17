# Review run

- Started: 2026-11-17T12:16:56Z
- Reviewed commit: `80ceecf629e0834153a9256fa390eb38fbf4d6d8` (peers ran in a detached worktree at this commit)
- Author: codex; rounds: 2 (round 1 independent first pass, both at once, not isolated); cross-talk order: claude, codex; timeout per turn: 1800s
- Claude: claude 2.1.326.1734 (ASBX Claude Code, channel stable), model opus, effort max; tools Read, Glob, Grep; no MCP
- Codex: codex-cli 0.169.0 (engine started directly; the toolbox launcher runs only as Bedrock credential helper), effort xhigh; CODEX_HOME config without MCP/plugin/project sections, shell env "core", no project instructions (project_doc_max_bytes=0); --sandbox read-only, sandbox available
- Round 1 is an independent first pass, not isolated (see the script header)
- Script: `tools/jeiobs/peer-review/review.sh`, sha256 `63e7012b6f02f3151f72ee2ad2c1a6d849725472001d706e960af791c4eb99a4`
- Codex home: `~/.config/jeiobs-agent/codex-review-b`; config.toml sha256 `e3e76dcf155ea4674ca7ba34028a06eedba3e87ed3812d50da13ede1a7efbd44`; skills sha256 `253397a7192e7cf5c8088186e9a468f0696218ee6d62efe83d06c761e627bfeb`
- Context sha256 `1bca7c9bdd5517992438e27573d2a7e35509c6b5577fe335e54c04fb35e4dd04`; ledger sha256 `01947da3b74db94616d87ea00267ea25b875e053f808ec5bcf58337aa6f4e921`
- stderr logs: kept locally (not committed), `02-run-20261117T121655Z`
- Topic: Review run 02 of pr33.

## CI state at the reviewed commit

```text
quick checks pending at start
local: jc-unit, jc-cases (1128 lines) PASS
```
- Finished: 2026-11-17T12:37:07Z

## Files written by this run

The hash given for `00-meta.md` is of its header (the part before "Finished");
the runner prints the final hash of every file, this one included, when it exits.

- `00-context.md` sha256 `1bca7c9bdd5517992438e27573d2a7e35509c6b5577fe335e54c04fb35e4dd04`
- `00-ledger-in.md` sha256 `01947da3b74db94616d87ea00267ea25b875e053f808ec5bcf58337aa6f4e921`
- `00-meta.md` sha256 `435bbde2a1d9eb79e83bce8457c3010e278165e58f2868a5501a364874298378`
- `01-r1-claude.prompt.md` sha256 `2ac2019913f7dec2cf0462c081b485cbcbf9eb4f5d2ea215f0ce2b1387118a07`
- `01-r1-claude.reply.md` sha256 `a3648587db6261a8b884e8c9fd957d754a66324ca42807ad0909820b2eda6eab`
- `02-r1-codex.prompt.md` sha256 `2ac2019913f7dec2cf0462c081b485cbcbf9eb4f5d2ea215f0ce2b1387118a07`
- `02-r1-codex.reply.md` sha256 `6a2d41b1b46e8e6625ee7e870ccd5db7c97dfdf11fe4dbe51974badc6ebda604`
- `03-r2-claude.prompt.md` sha256 `987b47b786c0cb9a15f48cdaa327019c29b937d0de8c614abaae30ed2b231503`
- `03-r2-claude.reply.md` sha256 `46ce9d5ee08c9be48b2f0a07d7d50ee0231afa80222f8d91c998985207b3bcb4`
- `04-r2-codex.prompt.md` sha256 `4ac307fa936d845c673e9a92e9f30f74b095288814be2e3d9f8287c0bd4112db`
- `04-r2-codex.reply.md` sha256 `65f43675962d25f27865f8694be546bb16bcc7da17640d67a62d606778d2ff3a`

## Final dispositions

```text
FINDING claude-1 | Major | accepted-fix | rounds then clamps (Value.cpp:602-641), FitsSigned requires sign extension (:306-323), Relative kept; stored form split out as claude-12
FINDING claude-2 | Major | accepted-fix | one strict parser for ParseMapping/LoadMapping/CompileMapping(bytes) (Mapping.cpp:1546,1559,1674); events same rules; byte API unit-tested
FINDING claude-3 | Major | accepted-fix | hotkey always output-control (Mapping.cpp:1337-1338), wildcards refused; r3.4 test gap is claude-13
FINDING claude-4 | Major | accepted-fix | url/input/ffmpeg_options/playlist denied (Mapping.cpp:972-974, cases.txt:648-679); PR4 must keep field a single key
FINDING claude-5 | Major | accepted-fix | nlohmann::json removes the quadratic insert; the reserve regression is claude-11
FINDING claude-6 | Major | accepted-fix | segment and key escaping injective (Mapping.cpp:1033-1044,1397-1407), merge groups by escaped key
FINDING claude-7 | Minor | open | not covered by r2/r3: E_SCHEMA routes vs r1.3 (Mapping.cpp:880-881), emit-empty E_SCHEMA target vs r1.4, caps/order/A11 unrecorded, r2.3 not in A2/A15, stale 006:31,318-323,332-333, README:15
FINDING claude-8 | Minor | open | unchanged: cycle soundness depends on the unrecorded slot of an emitted event; emit=meta unrecorded (Mapping.cpp:1210-1212,1348-1349)
FINDING claude-9 | Minor | open | merge now O(n log n); emit-to-emit edges still Θ(n²): x_i→[e], y_j emitting [s:f],[i:j] stores 25M edges (Mapping.cpp:1160,1216)
FINDING claude-10 | Minor | accepted-fix | wmain converts UTF-16 arguments as jw-check does (jc-check.cpp:625-656)
FINDING claude-11 | Major | open | reserve(size_t(-1)) per object (Mapping.cpp:190; parser.hpp:197; hashtable_policy.h:557-558): UB float-to-size_t; on AArch64 rehash throws, every mapping/event E_JSON
FINDING claude-12 | Minor | open | intRaw() width-masked (Value.cpp:456) but MakeInt requires sign-extended raw (:306-323); MakeInt(w, v.intRaw()) fails for negatives
FINDING claude-13 | Minor | open | same defect as codex-5: key-hotkey-with-effect.json:1 still has effect=none against r3.4 (07:156-158); cases.txt:913 accepts both; library correct (Mapping.cpp:1428-1431)
VERDICT: request changes
FINDING codex-1 | Major | accepted-fix | Mapping entry points now share strict parsing (Mapping.cpp:1543-1559,1672-1678).
FINDING codex-2 | Minor | accepted-fix | Requirements now enforce ASCII syntax (Mapping.cpp:334-346).
FINDING codex-3 | Major | accepted-fix | Target and address keys now escape separators injectively (Mapping.cpp:1033-1068,1395-1462).
FINDING codex-4 | Major | accepted-fix | Invalid integer and IEEE32 raw values are rejected (Value.cpp:430-467).
FINDING codex-5 | Minor | open | Stale hotkey expected output still includes `effect=none` and cases accept it (07-k1pr1-contract.md:156-158; key-hotkey-with-effect.json:1; cases.txt:913).
FINDING codex-6 | Minor | open | Unused `std::string bytes;` remains in jc-check (jc-check.cpp:162-163).
VERDICT: request changes
```
