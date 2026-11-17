# Review run

- Started: 2026-11-17T19:43:43Z
- Reviewed commit: `e3ecb4179924693626dcd59ecf8500ec7f287a46` (peers ran in a detached worktree at this commit)
- Author: lead; rounds: 2 (round 1 independent first pass, both at once, not isolated); cross-talk order: claude, codex; timeout per turn: 1800s
- Claude: claude 2.1.326.1734 (ASBX Claude Code, channel stable), model opus, effort max; tools Read, Glob, Grep; no MCP
- Codex: codex-cli 0.169.0 (engine started directly; the toolbox launcher runs only as Bedrock credential helper), effort xhigh; CODEX_HOME config without MCP/plugin/project sections, shell env "core", no project instructions (project_doc_max_bytes=0); --sandbox read-only, sandbox available
- Round 1 is an independent first pass, not isolated (see the script header)
- Script: `tools/jeiobs/peer-review/review.sh`, sha256 `63e7012b6f02f3151f72ee2ad2c1a6d849725472001d706e960af791c4eb99a4`
- Codex home: `~/.config/jeiobs-agent/codex-review-c`; config.toml sha256 `e3e76dcf155ea4674ca7ba34028a06eedba3e87ed3812d50da13ede1a7efbd44`; skills sha256 `253397a7192e7cf5c8088186e9a468f0696218ee6d62efe83d06c761e627bfeb`
- Context sha256 `042a2d4616139f7a1c6218365d392088a3595ca5981f61ede79664841b56097d`; ledger sha256 `a79cae6601b841240ba0fa64aa2642b9d28c96915486a4911adbcb2f817d0ee9`
- stderr logs: kept locally (not committed), `02-run-20261117T194342Z`
- Topic: Review run 02 of pr38.

## CI state at the reviewed commit

```text
CI: only the scaffold quick check applies to this change; no CI job builds libobs unit tests
```
- Finished: 2026-11-17T19:52:28Z

## Files written by this run

The hash given for `00-meta.md` is of its header (the part before "Finished");
the runner prints the final hash of every file, this one included, when it exits.

- `00-context.md` sha256 `042a2d4616139f7a1c6218365d392088a3595ca5981f61ede79664841b56097d`
- `00-ledger-in.md` sha256 `a79cae6601b841240ba0fa64aa2642b9d28c96915486a4911adbcb2f817d0ee9`
- `00-meta.md` sha256 `f21a0372155e1081b91e6302dfeddd6c3fec7068be3d980cfcbf9e69d27c079f`
- `01-r1-claude.prompt.md` sha256 `d886229e97de99b6b642d58b5d4f3dea65ba57dffaa8734190b8a491a0121b4f`
- `01-r1-claude.reply.md` sha256 `e64901a821892faa1cd1101952d1a3c63f3da3e04556cd47754bc445e79e4136`
- `02-r1-codex.prompt.md` sha256 `d886229e97de99b6b642d58b5d4f3dea65ba57dffaa8734190b8a491a0121b4f`
- `02-r1-codex.reply.md` sha256 `93d5033e30548fbfb597367a15085b821ba00059d5a47739dcacc7488e78417f`
- `03-r2-claude.prompt.md` sha256 `fab415b81313258c6f139abb52590282874f460b358780cc1e5dd95ec52ce85b`
- `03-r2-claude.reply.md` sha256 `fe7a823e0219133f9b5076b37f052e3369f84e68b1241614e4f4ed1c9352340e`
- `04-r2-codex.prompt.md` sha256 `ed980e0828c32d8ec21fee04f563a34db70e4614bb80557ec2b3fe1195b34bb8`
- `04-r2-codex.reply.md` sha256 `0457a3af685acf8d8ef6eb6b8764814d2d8e9596728de95bb57c799ffebee9da`

## Final dispositions

```text
FINDING claude-1 | Major | accepted-fix | h:56-58/rst:113-116 now say one fixed order relative to this lock and no waiting on another user of the config_t; consistent with WORKSPACE_FORMAT.md:459-460
FINDING claude-2 | Minor | accepted-fix | config_close excluded (h:54,59; rst:111); open_defaults (c:330-332) and num_sections (c:495-497) lock; mutexes recursive (c:81,280,308) so config_get_section (c:509-511) cannot self-deadlock
FINDING claude-3 | Major | accepted-fix | config->file read after the lock (c:429-433); swap/restore, backup copy, replace and both exits under the lock (c:440-466)
FINDING claude-4 | Minor | accepted-fix | %ignore before %include "util/config-file.h" in obslua.i and obspython.i; no other .i files
FINDING claude-5 | Minor | accepted-fix | add_test NAME/COMMAND (cmocka/CMakeLists.txt:37), atomic stop flag (test:69,220), unused event removed
FINDING claude-6 | Minor | accepted-fix | WORKSPACE_FORMAT.md:459-463 and :608-611 updated
FINDING claude-7 | Minor | open | WORKSPACE_FORMAT.md:489 "config-file.c:347" (blank; fopen "wb" at :353) and :617 "config-file.c:378-381" (BOM at :384-387) are stale; same as codex-10
FINDING claude-8 | Minor | open | config_save `if (!config->file)` (c:345) runs before the lock (c:351) while config_save_safe writes config->file under it (c:440,442); harmless, move after the lock; same as codex-9
FINDING claude-9 | Minor | open | test:112 `config_get_int(reopened,"A","counter") >= 0` passes for an empty file (c:256-257, 692-699), contrary to test:109; the final value is always 399, so assert == 399
VERDICT: approve
FINDING codex-1 | Major | accepted-fix | `config_save_safe` snapshots `config->file` under the lock (`libobs/util/config-file.c:429-442`).
FINDING codex-2 | Minor | accepted-fix | `config_open_defaults` and `config_num_sections` now lock (`libobs/util/config-file.c:330-332,495-497`).
FINDING codex-3 | Minor | accepted-fix | Stop flag uses atomic access (`test/cmocka/test_config_lock.c:69,220`).
FINDING codex-4 | Minor | accepted-fix | Lock-order wording now permits persistMutex-then-config_lock (`libobs/util/config-file.h:55-59`).
FINDING codex-5 | Minor | accepted-fix | Documentation records conditional cmocka wiring and absent CI coverage (`docs/jeiobs/WORKSPACE_FORMAT.md:608-611`).
FINDING codex-6 | Minor | accepted-fix | Documentation excludes concurrent `config_close` (`libobs/util/config-file.h:54-59`).
FINDING codex-7 | Minor | accepted-fix | Test registration uses target-aware CTest syntax (`test/cmocka/CMakeLists.txt:37`).
FINDING codex-8 | Minor | accepted-fix | Stale section-10 citations were replaced with function references (`docs/jeiobs/WORKSPACE_FORMAT.md:459-475`).
FINDING codex-9 | Minor | open | `config_save` reads `config->file` before locking (`libobs/util/config-file.c:345,351`), racing `config_save_safe`’s swap/restore (`:440-442`).
FINDING codex-10 | Minor | open | `WORKSPACE_FORMAT.md:489` still cites `config-file.c:347`; the truncating `os_fopen` is at `:353`.
FINDING codex-11 | Minor | open | `"config_get_int(...) >= 0"` (`test/cmocka/test_config_lock.c:112`) accepts an empty file or missing key (`libobs/util/config-file.c:256-257,692-699`).
VERDICT: approve
```
