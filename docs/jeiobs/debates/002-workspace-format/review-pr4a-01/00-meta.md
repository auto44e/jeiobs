# Review run

- Started: 2026-11-17T19:20:38Z
- Reviewed commit: `c23b4603170555429450eb07d2a4fa6b21cc8732` (peers ran in a detached worktree at this commit)
- Author: lead; rounds: 2 (round 1 independent first pass, both at once, not isolated); cross-talk order: claude, codex; timeout per turn: 1800s
- Claude: claude 2.1.326.1734 (ASBX Claude Code, channel stable), model opus, effort max; tools Read, Glob, Grep; no MCP
- Codex: codex-cli 0.169.0 (engine started directly; the toolbox launcher runs only as Bedrock credential helper), effort xhigh; CODEX_HOME config without MCP/plugin/project sections, shell env "core", no project instructions (project_doc_max_bytes=0); --sandbox read-only, sandbox available
- Round 1 is an independent first pass, not isolated (see the script header)
- Script: `tools/jeiobs/peer-review/review.sh`, sha256 `63e7012b6f02f3151f72ee2ad2c1a6d849725472001d706e960af791c4eb99a4`
- Codex home: `~/.config/jeiobs-agent/codex-review-c`; config.toml sha256 `e3e76dcf155ea4674ca7ba34028a06eedba3e87ed3812d50da13ede1a7efbd44`; skills sha256 `253397a7192e7cf5c8088186e9a468f0696218ee6d62efe83d06c761e627bfeb`
- Context sha256 `a84d53d0006404ab93ba5bd56233a08c6693ee212817bc34bbc0291234a40097`; ledger sha256 `e7c5b7dd9301f35be1586a05559ca13fc097fd6abde56ac4184a8c9666466c38`
- stderr logs: kept locally (not committed), `01-run-20261117T192037Z`
- Topic: Review run 01 of pr38.

## CI state at the reviewed commit

```text
CI: only the scaffold quick check applies to this change; no CI job builds libobs unit tests
```
- Finished: 2026-11-17T19:38:05Z

## Files written by this run

The hash given for `00-meta.md` is of its header (the part before "Finished");
the runner prints the final hash of every file, this one included, when it exits.

- `00-context.md` sha256 `a84d53d0006404ab93ba5bd56233a08c6693ee212817bc34bbc0291234a40097`
- `00-ledger-in.md` sha256 `e7c5b7dd9301f35be1586a05559ca13fc097fd6abde56ac4184a8c9666466c38`
- `00-meta.md` sha256 `5da2cd74780585aef348b6c6c9ccdf9a744e4976a9d75649c23576172eebdfd7`
- `01-r1-claude.prompt.md` sha256 `1770d613e57ff5b54005eb3359c2917342b819ad4d47df5dc65e2587547df4f4`
- `01-r1-claude.reply.md` sha256 `999e02c371bc1902b71331db63b51a77167d6d6acc46a699a62d466265134790`
- `02-r1-codex.prompt.md` sha256 `1770d613e57ff5b54005eb3359c2917342b819ad4d47df5dc65e2587547df4f4`
- `02-r1-codex.reply.md` sha256 `0af2e3f09c1afab1c9fb28096b5548bd2f42a1a60ab0ff82e6090fc3010bc72d`
- `03-r2-claude.prompt.md` sha256 `05f6c6ee5556eee2be4ab0d4038ccc862c82121f755ad245009fa3e665737611`
- `03-r2-claude.reply.md` sha256 `859523afed2bb9d649202c0409b64e7d8bbaeca154449262e61255ed0c0cb2ab`
- `04-r2-codex.prompt.md` sha256 `18dfddc6608fe6e60713beec21c940bb2379d9524b14206504a0436f52f11706`
- `04-r2-codex.reply.md` sha256 `79c33598671bcaecbd95bb91fac5ef089f036fd651334b62be4af1a8a29a6b2c`

## Final dispositions

```text
FINDING claude-1 | Major | open | config.h:55-56/rst:113-114 "Take it before any other lock the caller owns" inverts the leaf order already used by Lua (lua.h:94) and Python GIL (python.c:110) callbacks that call config_*; WORKSPACE_FORMAT.md:459-460 breaks it too
FINDING claude-2 | Minor | open | "holder may call any config_* function" includes config_close (destroys held mutex c:477, frees object, later unlock is UAF); config_open_defaults/config_num_sections don't lock (c:323-329, 481-484)
FINDING claude-3 | Major | open | raised from Minor; same root cause as codex-1: config_save_safe reads config->file before the lock (c:414 vs 423), giving UAF at 443/449 and double free at 476; move the read after the lock
FINDING claude-4 | Minor | open | obslua.i:107/obspython.i:129 expose config_lock/unlock to scripts with no %ignore; scripts fall outside the PR4 lock-order proof
FINDING claude-5 | Minor | open | add_test path (cmocka/CMakeLists.txt:37) breaks under the VS/Xcode presets; cmocka is Linux-only in practice; test nits (volatile race :50, unused event :142, 100 ms timing :166-169, ANSI TEMP)
FINDING claude-6 | Minor | open | WORKSPACE_FORMAT.md:607-610 says cmocka never runs (false since CMakeLists.txt:35-38); §10 line refs 411-441 (:462) and 625-630 (:473) are stale
VERDICT: request changes
FINDING codex-1 | Major | open | `config_save_safe` snapshots `config->file` before locking (`libobs/util/config-file.c:414,423`), enabling a concurrent call to use a freed temporary at `:432,449,454`.
FINDING codex-2 | Minor | open | The docs claim the lock prevents all config changes and every getter locks (`libobs/util/config-file.h:52-54`), but `config_open_defaults` and `config_num_sections` access state unlocked (`libobs/util/config-file.c:323-329,481-484`).
FINDING codex-3 | Minor | open | The test’s `volatile int stop` is read and written across threads without synchronization (`test/cmocka/test_config_lock.c:50,68,171`).
FINDING codex-4 | Minor | open | The API says “Take it before any other lock” (`libobs/util/config-file.h:55-56`, `docs/sphinx/reference-libobs-util-config-file.rst:113-114`), conflicting with the required `persistMutex`-then-`config_lock` order (`docs/jeiobs/WORKSPACE_FORMAT.md:459-460`).
FINDING codex-5 | Minor | open | The docs still claim cmocka “never runs” (`docs/jeiobs/WORKSPACE_FORMAT.md:607-610`, `docs/jeiobs/tasks/002-workspace-saving.md:64-65`), contradicted by the new wiring (`CMakeLists.txt:35-37`).
FINDING codex-6 | Minor | open | The docs say the holder may call “any config_* function” (`libobs/util/config-file.h:54`), but `config_close` destroys the held mutex and frees the object (`libobs/util/config-file.c:459-478`).
FINDING codex-7 | Minor | open | The newly enabled tests use literal `add_test` binary paths (`test/cmocka/CMakeLists.txt:37`) that omit multi-config output directories/extensions under the Xcode and Visual Studio presets (`CMakePresets.json:90,161`).
FINDING codex-8 | Minor | open | Line citations became stale: `WORKSPACE_FORMAT.md:472-473` cites getters at `config-file.c:625-630`, which now contains default setters, and its save range ends before `os_safe_replace` at `:449`.
VERDICT: request changes
```
