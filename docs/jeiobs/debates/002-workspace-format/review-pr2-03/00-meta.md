# Review run

- Started: 2026-11-17T03:12:28Z
- Reviewed commit: `19ae5d70d55056efaae28db3feb8123c79a93096` (peers ran in a detached worktree at this commit)
- Author: codex; rounds: 2 (round 1 independent first pass, both at once, not isolated); cross-talk order: claude, codex; timeout per turn: 1800s
- Claude: claude 2.1.326.1734 (ASBX Claude Code, channel stable), model opus, effort max; tools Read, Glob, Grep; no MCP
- Codex: codex-cli 0.169.0 (engine started directly; the toolbox launcher runs only as Bedrock credential helper), effort xhigh; CODEX_HOME config without MCP/plugin/project sections, shell env "core", no project instructions (project_doc_max_bytes=0); --sandbox read-only, sandbox available
- Round 1 is an independent first pass, not isolated (see the script header)
- Script: `tools/jeiobs/peer-review/review.sh`, sha256 `63e7012b6f02f3151f72ee2ad2c1a6d849725472001d706e960af791c4eb99a4`
- Codex home: `~/.config/jeiobs-agent/codex-review-b`; config.toml sha256 `e3e76dcf155ea4674ca7ba34028a06eedba3e87ed3812d50da13ede1a7efbd44`; skills sha256 `253397a7192e7cf5c8088186e9a468f0696218ee6d62efe83d06c761e627bfeb`
- Context sha256 `91e860828cc60015bc048961e09297b24cf5c7146822b555c83852678b9357c4`; ledger sha256 `0eaec56ff0185e3ed4f56a3931b880654e96ffc6cd9a84fc03824a85448a1c05`
- stderr logs: kept locally (not committed), `03-run-20261117T031227Z`
- Topic: Review run 03 of PR #28 (S1-PR2).

## CI state at the reviewed commit

```text
quick checks pending at start
local: jw-unit, jw-fuzz-smoke, jw-cases (994 lines) PASS on Linux, also under ASan+UBSan
```
- Finished: 2026-11-17T03:31:21Z

## Files written by this run

The hash given for `00-meta.md` is of its header (the part before "Finished");
the runner prints the final hash of every file, this one included, when it exits.

- `00-context.md` sha256 `91e860828cc60015bc048961e09297b24cf5c7146822b555c83852678b9357c4`
- `00-ledger-in.md` sha256 `0eaec56ff0185e3ed4f56a3931b880654e96ffc6cd9a84fc03824a85448a1c05`
- `00-meta.md` sha256 `389a3e5f456db69f80262a1118f165824402425573f41353fd00b6f3e0f300ea`
- `01-r1-claude.prompt.md` sha256 `aa5ad7658f7042764a20d623deb419beaa16c088e0018bfdfcab7736f5bbc733`
- `01-r1-claude.reply.md` sha256 `f1435f7f57b1a762199860666d2c1bd78f602e29f1e79b2c78cbe378e4e7c6e8`
- `02-r1-codex.prompt.md` sha256 `aa5ad7658f7042764a20d623deb419beaa16c088e0018bfdfcab7736f5bbc733`
- `02-r1-codex.reply.md` sha256 `a9f7f0bc130e313f7d181902a8d15c998986889870fd9070fc398bf6abe1a06d`
- `03-r2-claude.prompt.md` sha256 `0952b039c4f74b952a706776b443ac12acb1a078224c1dea3796afc32af84c9c`
- `03-r2-claude.reply.md` sha256 `d90ae70560fc03fc557284d82137ab2cbb3a6a4692ebf41911131cdcb6c3c3d1`
- `04-r2-codex.prompt.md` sha256 `f6552e3ab31070903679d8c688f25bfe00b3cc4fb0043ff5e630cca285195a7e`
- `04-r2-codex.reply.md` sha256 `1e8229668de7a0d962e4b5ea6104796926d4cf2cdedfc5a1b1b3d85e095a1423`

## Final dispositions

```text
FINDING claude-1 | Major | accepted-fix | IniScan follows config-file.c header/line/key rules; the r3 repros redact (jw_unit.cpp:483-493)
FINDING claude-2 | Major | accepted-fix | URL rule applied to ffmpeg_source input whatever is_local_file is (Redactor.cpp:737-738)
FINDING claude-3 | Major | accepted-fix | /~section-<n> for long names (Redactor.cpp:864-866); report capped (:38-53); escape scan limited to the value (:585-586); UrlRanges linear (:450-535)
FINDING claude-4 | Major | accepted-fix | app.modules accepts an array root (Redactor.cpp:1047-1053)
FINDING claude-5 | Minor | accepted-fix | overlapping and touching ranges merged (Redactor.cpp:975-981)
FINDING claude-6 | Minor | accepted-fix | groups items and six audio-device objects scanned (Redactor.cpp:799-813), as OBS loads them
FINDING claude-7 | Minor | accepted-fix | one leading BOM skipped in documents (JsonScan.cpp:646); manifests still reject it (Manifest.cpp:361-363)
FINDING claude-8 | Minor | accepted-fix | the four drift items fixed in run 02
FINDING claude-9 | Major | accepted-fix | query starts at the first '?' and splits on '&' only (Redactor.cpp:504-533)
FINDING claude-10 | Minor | accepted-fix | URL rule over the whole string; authority ends at / ? # (Redactor.cpp:466-501)
FINDING claude-11 | Minor | accepted-fix | number parsing is locale-independent; portability tracked as claude-13
FINDING claude-12 | Minor | accepted-fix | plan/apply failure gives omitted/unparseable for secret roles (Redactor.cpp:1071-1080,1097-1106); fuzz checks every range (jw_fuzz_smoke.cpp:59-94)
FINDING claude-13 | Major | open | "std::from_chars(token.data(), ..., number, std::chars_format::general)" (JsonScan.cpp:425-426) unchanged, no r3-item-10 fallback; macos-15 in matrix (jeiobs-workspace.yaml:21) never ran (WORKSPACE_FORMAT.md:681); withdrawn if macos-15 passes or a fallback lands
FINDING claude-14 | Minor | accepted-fix | §8:348, §8:355-363 and §12:534 match r3; later drift tracked as claude-16
FINDING claude-15 | Minor | accepted-fix | pre-header keys recorded (IniScan.cpp:247-249) and redacted under the fallback (Redactor.cpp:917)
FINDING claude-16 | Minor | open | spec unchanged: §8:361-363 lists a >1,024-byte-name fallback the code lacks; /~section-<n> and its counting missing from §4:86-87; §8:360 "malformed header is ignored" wrong; 4 MiB report omission undocumented and sizeof(Edit)-dependent (Redactor.cpp:46)
VERDICT: request changes
FINDING codex-1 | Major | accepted-fix | Overlapping/touching edits merge before application (`Redactor.cpp:965-982`).
FINDING codex-2 | Major | accepted-fix | String limits are checked before appending (`JsonScan.cpp:186-199`, `:313-317`).
FINDING codex-3 | Minor | accepted-fix | Export audio errors report `target.audio` (`Manifest.cpp:543-565`).
FINDING codex-4 | Major | accepted-fix | Malformed headers trigger fallback and pre-header keys are scanned (`IniScan.cpp:220-258`, `Redactor.cpp:907-920`).
FINDING codex-5 | Minor | accepted-fix | Documentation now requires ffmpeg input scanning unconditionally (`WORKSPACE_FORMAT.md:348`).
VERDICT: request changes
```
