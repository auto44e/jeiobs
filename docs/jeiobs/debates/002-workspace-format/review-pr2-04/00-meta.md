# Review run

- Started: 2026-11-17T04:27:59Z
- Reviewed commit: `d610e03a14eeb00f2ca38a1c21f16ef86df0dd02` (peers ran in a detached worktree at this commit)
- Author: codex; rounds: 2 (round 1 independent first pass, both at once, not isolated); cross-talk order: claude, codex; timeout per turn: 1800s
- Claude: claude 2.1.326.1734 (ASBX Claude Code, channel stable), model opus, effort max; tools Read, Glob, Grep; no MCP
- Codex: codex-cli 0.169.0 (engine started directly; the toolbox launcher runs only as Bedrock credential helper), effort xhigh; CODEX_HOME config without MCP/plugin/project sections, shell env "core", no project instructions (project_doc_max_bytes=0); --sandbox read-only, sandbox available
- Round 1 is an independent first pass, not isolated (see the script header)
- Script: `tools/jeiobs/peer-review/review.sh`, sha256 `63e7012b6f02f3151f72ee2ad2c1a6d849725472001d706e960af791c4eb99a4`
- Codex home: `~/.config/jeiobs-agent/codex-review-b`; config.toml sha256 `e3e76dcf155ea4674ca7ba34028a06eedba3e87ed3812d50da13ede1a7efbd44`; skills sha256 `253397a7192e7cf5c8088186e9a468f0696218ee6d62efe83d06c761e627bfeb`
- Context sha256 `bdaf2d4ac01c1be13aa3daeb5273dcb04cee2efaced763e934a365a56053ae94`; ledger sha256 `7c87347747e4e2c1e7d97997e13f6681cbe0c9b501818a05e0d973d86e3e3423`
- stderr logs: kept locally (not committed), `04-run-20261117T042758Z`
- Topic: Review run 04 of PR #28 (S1-PR2).

## CI state at the reviewed commit

```text
CI at f7aa68ebe: jeiobs workspace library success on ubuntu-24.04, macos-15, windows-2022; quick checks success
head d610e03a14eeb00f2ca38a1c21f16ef86df0dd02: pushed at review start
local: all three ctest targets pass, also with -DJW_NO_FROM_CHARS
```
- Finished: 2026-11-17T04:47:16Z

## Files written by this run

The hash given for `00-meta.md` is of its header (the part before "Finished");
the runner prints the final hash of every file, this one included, when it exits.

- `00-context.md` sha256 `bdaf2d4ac01c1be13aa3daeb5273dcb04cee2efaced763e934a365a56053ae94`
- `00-ledger-in.md` sha256 `7c87347747e4e2c1e7d97997e13f6681cbe0c9b501818a05e0d973d86e3e3423`
- `00-meta.md` sha256 `5956fdf38ff67fa00b7ab7a58ca5cb7bf175036ca71c5c9bcbc805fb316b9915`
- `01-r1-claude.prompt.md` sha256 `ba9a1871fc213bb949ca9e592cb2385292238cc78eb1929abbf36796a8c63c94`
- `01-r1-claude.reply.md` sha256 `fdb3e18464c14699f040bdd9bf5f0124252cc63adc68f56774b0beab2641aaaa`
- `02-r1-codex.prompt.md` sha256 `ba9a1871fc213bb949ca9e592cb2385292238cc78eb1929abbf36796a8c63c94`
- `02-r1-codex.reply.md` sha256 `78548da2099fb3f4a15c6e24f56aee2d22b96f3a1d8e2816be2ec40861c7523c`
- `03-r2-claude.prompt.md` sha256 `a8250684be52735da850373e20dcd7a478289db64b31f3f37d3e47aff9aba79b`
- `03-r2-claude.reply.md` sha256 `f844cf7a5f3c699247bfde298991781e4950c8cbab2ab91fcd8fc91328a2de8f`
- `04-r2-codex.prompt.md` sha256 `9c025c707584ad11a46f7a0223f4a4e18a348a0f07326ba46ed708c021d38d0d`
- `04-r2-codex.reply.md` sha256 `e30c1e2ffcaf0d5f505a56ebd30dd8d6566c99fb31b73888d256c2e021d88799`

## Final dispositions

```text
FINDING claude-1 | Major | accepted-fix | IniScan follows config-file.c header/line/key rules; the r3 repros redact (jw_unit.cpp:483-493)
FINDING claude-2 | Major | accepted-fix | URL rule applied to ffmpeg_source input whatever is_local_file is (Redactor.cpp:736-737)
FINDING claude-3 | Major | accepted-fix | /~section-<n> for long names (Redactor.cpp:863-867); report capped, platform-independent (:38-55); escape scan limited to the value (:584-585); UrlRanges linear (:449-534)
FINDING claude-4 | Major | accepted-fix | app.modules accepts an array root (Redactor.cpp:1046-1052)
FINDING claude-5 | Minor | accepted-fix | overlapping and touching ranges merged (Redactor.cpp:975-980)
FINDING claude-6 | Minor | accepted-fix | groups items and six audio-device objects scanned (Redactor.cpp:798-812), as OBS loads them
FINDING claude-7 | Minor | accepted-fix | one leading BOM skipped in documents (JsonScan.cpp:748-751); manifests still reject it
FINDING claude-8 | Minor | accepted-fix | the four drift items fixed in run 02
FINDING claude-9 | Major | accepted-fix | query starts at the first '?' and splits on '&' only (Redactor.cpp:503-532)
FINDING claude-10 | Minor | accepted-fix | URL rule over the whole string; authority ends at / ? # (Redactor.cpp:465-501)
FINDING claude-11 | Minor | accepted-fix | number check is locale-free: manual fast path, from_chars or classic-locale stream (JsonScan.cpp:101-131)
FINDING claude-12 | Minor | accepted-fix | plan/apply failure gives omitted/unparseable for secret roles (Redactor.cpp:1070-1079,1096-1105); fuzz checks every range
FINDING claude-13 | Major | accepted-fix | fallback landed: from_chars only under __cpp_lib_to_chars, else classic-locale stream (JsonScan.cpp:111-131); underflow accepted on both paths; residual gaps filed as claude-17/18
FINDING claude-14 | Minor | accepted-fix | §8:348, §8:355-363 and §12:534 match r3; later drift tracked as claude-16
FINDING claude-15 | Minor | accepted-fix | pre-header keys recorded (IniScan.cpp:248-249) and redacted under the fallback (Redactor.cpp:916)
FINDING claude-16 | Minor | accepted-fix | long names set uncertain (IniScan.cpp:231-234); /~section-<n> in §4:86-89; header rule and 64-byte/4 MiB budget in §8:361-367 match Redactor.cpp:46-53
FINDING claude-17 | Minor | open | exponent cut short at 10^6, not clamped (JsonScan.cpp:77-79), no number length limit: 0.<999,999 zeros>1e10000000 (=10^9,000,000) gives magnitude 1 and the fast path accepts it (:108) against §12:554; 1<1,000,400 zeros>e-10000000 (underflow) gives 401, E_NONFINITE on from_chars/libc++ (:118-119,:127-128)
FINDING claude-18 | Minor | open | locale test scans "1.5" (jw_unit.cpp:521), fast path, no conversion; no underflow case (numbers.json 1.5e-300 is fast path); no test isolates the IniScan.cpp:231 length trigger (jw_unit.cpp:495 starts with Twitch; long-name fixtures also have [Video] ;x or a CR without LF); WORKSPACE_FORMAT.md:682,685 stale ("only on Linux", "macOS runner has not run yet")
VERDICT: approve
FINDING codex-1 | Major | accepted-fix | Overlapping edits are normalized before application (Redactor.cpp:965-982).
FINDING codex-2 | Major | accepted-fix | String limits are checked before appending (JsonScan.cpp:292-304).
FINDING codex-3 | Minor | accepted-fix | Audio schema errors report target.audio (Manifest.cpp:543-565).
FINDING codex-4 | Major | accepted-fix | Malformed headers trigger fallback and pre-header keys are scanned (IniScan.cpp:220-258; Redactor.cpp:906-920).
FINDING codex-5 | Minor | accepted-fix | Documentation requires unconditional ffmpeg input scanning (WORKSPACE_FORMAT.md:350).
VERDICT: approve
```
