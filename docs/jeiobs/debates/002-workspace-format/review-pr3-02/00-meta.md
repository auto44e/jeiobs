# Review run

- Started: 2026-11-17T12:16:56Z
- Reviewed commit: `5827f23dca0be20e9d6348c2dc7e0d717fa112e7` (peers ran in a detached worktree at this commit)
- Author: codex; rounds: 2 (round 1 independent first pass, both at once, not isolated); cross-talk order: claude, codex; timeout per turn: 1800s
- Claude: claude 2.1.326.1734 (ASBX Claude Code, channel stable), model opus, effort max; tools Read, Glob, Grep; no MCP
- Codex: codex-cli 0.169.0 (engine started directly; the toolbox launcher runs only as Bedrock credential helper), effort xhigh; CODEX_HOME config without MCP/plugin/project sections, shell env "core", no project instructions (project_doc_max_bytes=0); --sandbox read-only, sandbox available
- Round 1 is an independent first pass, not isolated (see the script header)
- Script: `tools/jeiobs/peer-review/review.sh`, sha256 `63e7012b6f02f3151f72ee2ad2c1a6d849725472001d706e960af791c4eb99a4`
- Codex home: `~/.config/jeiobs-agent/codex-review`; config.toml sha256 `e3e76dcf155ea4674ca7ba34028a06eedba3e87ed3812d50da13ede1a7efbd44`; skills sha256 `253397a7192e7cf5c8088186e9a468f0696218ee6d62efe83d06c761e627bfeb`
- Context sha256 `a8a1591568c898d6a2c7bb8cd974cce9dea90945bc988858ade6bb8b2d8e36cf`; ledger sha256 `22388c280774f674ae95d7a064948eff9f0b716284730cfe985d0d4aaada8f50`
- stderr logs: kept locally (not committed), `02-run-20261117T121655Z`
- Topic: Review run 02 of pr32.

## CI state at the reviewed commit

```text
quick checks pending at start
local: jw-unit, jw-fuzz-smoke, jw-cases (1134 lines) PASS
```
- Finished: 2026-11-17T12:38:39Z

## Files written by this run

The hash given for `00-meta.md` is of its header (the part before "Finished");
the runner prints the final hash of every file, this one included, when it exits.

- `00-context.md` sha256 `a8a1591568c898d6a2c7bb8cd974cce9dea90945bc988858ade6bb8b2d8e36cf`
- `00-ledger-in.md` sha256 `22388c280774f674ae95d7a064948eff9f0b716284730cfe985d0d4aaada8f50`
- `00-meta.md` sha256 `bca9be1c1a71089464a43dfa9cf3beff907365c43ff5251cc1f0350b17c7f44b`
- `01-r1-claude.prompt.md` sha256 `088c21d345d1c3fb9b891fd02288a1cd231da519be1692a003dbf4b418a91b2c`
- `01-r1-claude.reply.md` sha256 `14bd83c0ec50bd5203e41ec60245ec4ce7a088faa35a077b621c2079a62e1224`
- `02-r1-codex.prompt.md` sha256 `088c21d345d1c3fb9b891fd02288a1cd231da519be1692a003dbf4b418a91b2c`
- `02-r1-codex.reply.md` sha256 `32e7497edc71e4a7cf9a94ddeb04dfd6371c3c8360e042fc23ed271bd1f9baf7`
- `03-r2-claude.prompt.md` sha256 `fb670cd55dfad18e7dbb934ba1ce3e91d6d5584d52f682e26eea4a02a7110cde`
- `03-r2-claude.reply.md` sha256 `190b357e6744f76776f5990e228fdfc394274a46ed697b347c94044519a4afbd`
- `04-r2-codex.prompt.md` sha256 `10e5a873b2e8481157361f17d2624e90460bed1a54d1da5e1f9bc2e1a386ecd8`
- `04-r2-codex.reply.md` sha256 `2f05f30456780bbf4af46f83b76b62b1123deabfa8cd7ce3942e195106e53047`

## Final dispositions

```text
FINDING claude-1 | Major | accepted-fix | two-pass identity (CapturePlanner.cpp:1134-1173); pass 2 sees only unmatched collections; order cases pass
FINDING claude-2 | Major | accepted-fix | themes skip removed from ProcessDirectory; only the config root ignores unowned names (CapturePlanner.cpp:642-663)
FINDING claude-3 | Major | accepted-fix | linked app dir and owned subpaths give a symlink entry and root-symlink warning; reparse points map to Symlink (Fs.cpp:52-75); Windows build break tracked as claude-12
FINDING claude-4 | Major | accepted-fix | open, entry and increment errors fail the folder as unreadable with RestoreState; per-name conversion (Fs.cpp:267-294); residuals in claude-14
FINDING claude-5 | Major | accepted-fix | count validated (Manifest.cpp:853-857) and known to export and CarryOver (:1131, :1259); doc gap in claude-9
FINDING claude-6 | Minor | accepted-fix | ReviewRules linear with a discriminating test; collections and profiles counted; maps replace find_if and per-pair strings
FINDING claude-7 | Minor | open | sample capped at 101 (CapturePlanner.cpp:487) while the collapse counts regular files only (:296): over 101 empty subfolders or FIFOs drop silently, against r2.7
FINDING claude-8 | Minor | accepted-fix | basic/profiles folders without basic.ini are one unknown dir entry with no UID (CapturePlanner.cpp:954-973)
FINDING claude-9 | Minor | open | WORKSPACE_FORMAT.md sections 4, 7, 8, 13 still stale (scene_uuids, count, names, unrepresentable, unreadable, too-large); contract r1.2 (:103) still denies the profile UID on a lone .bak
FINDING claude-10 | Minor | accepted-fix | cases.txt:1342 accepts only bak-alone.json
FINDING claude-11 | Minor | accepted-fix | CanStore runs before any read (CapturePlanner.cpp:378,684)
FINDING claude-12 | Major | open | Fs.cpp:23 includes windows.h without NOMINMAX, so numeric_limits max() at Fs.cpp:84,317 fails under MSVC /W4 /WX in the standalone Windows CI
FINDING claude-13 | Minor | open | a symlinked basic.ini turns the profile into one unknown dir entry with the link unreported (CapturePlanner.cpp:962-973); OBS follows the link
FINDING claude-14 | Minor | open | every lstat error on an app dir counts as missing and is dropped silently (Fs.cpp:257-258, CapturePlanner.cpp:337,580); an entry vanishing mid-listing fails the whole folder (Fs.cpp:275-278)
FINDING claude-15 | Minor | open | ProcessOwnedRoot lists unrepresentable names under the app dir and basic/ (CapturePlanner.cpp:571-583), against r1.4; up to five entries with path obs-studio in the default layout
FINDING claude-16 | Minor | open | AddOmitted merge scans are O(n^2) (CapturePlanner.cpp:245-266, r2.6); only basic/scenes is sorted (:841), so the 256 MiB budget follows directory order for profiles and modules (r3)
VERDICT: request changes
FINDING codex-1 | Major | withdrawn | Lone `.bak` has no collection UID (CapturePlanner.cpp:1253-1260).
FINDING codex-2 | Major | open | `Summary.error` is ignored, producing a partial `unrepresentable/names` entry without `listing-error` (CapturePlanner.cpp:306-325; 31-pr3-contract.md:131-134).
FINDING codex-3 | Major | accepted-fix | Scene UUID extraction is indexed and capped at 100,000 (CapturePlanner.cpp:744-803).
FINDING codex-4 | Minor | withdrawn | Invalid-name symlink handling is a contract ambiguity (CapturePlanner.cpp:306-325).
VERDICT: request changes
```
