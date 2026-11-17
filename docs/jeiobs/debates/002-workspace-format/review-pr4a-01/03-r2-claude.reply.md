**Verdict: still request changes.** The commit under review is still c23b4603, so nothing has been fixed and all six of my findings stay open. I checked all five of Codex's findings against the files and agree with each. I'm raising claude-3 to Major to match codex-1. I'm keeping claude-1 at Major, although Codex rates the same issue Minor as codex-4.

**How Codex's findings map to mine**
- **codex-1 is the same bug as claude-3.** I raised claude-3 to Major. Codex's sequence checks out:
  - `char *file = config->file` (`config-file.c:414`) runs before the lock (`:423`).
  - The other call restores its temp file at `:432`, and the temp is freed at `:454`.
  - The freed pointer is then used at `:443` and `:449`, and freed a second time at `:476`.
  - This doesn't need `config_lock` at all: it's two `config_save_safe` calls on any threads. Those are exactly the `user.ini` writers that `WORKSPACE_FORMAT.md:487` lists as "serialized by config lock".
  - The new header says every save call takes the lock (`h:52`), and §10 says holding it "blocks every `config_t` writer on every thread" (`:462-463`). Those claims only hold after the one-line fix, and this PR is the one changing `config-file.c`.
- **codex-4 is the same issue as claude-1, and I keep it at Major.** The rule in `h:55-56` and `rst:113-114` makes lock holders take the config lock first, then their own lock. The reverse order already exists in the tree:
  - Lua callbacks run while holding `current_lua_script->mutex` (`obs-scripting-lua.h:89-94`).
  - Python callbacks run while holding the GIL (`obs-scripting-python.c:110-111`, `.h:186`).
  - Both can reach frontend configs through `obs_frontend_get_*_config` (`obslua.i:110`, `obspython.i:132`) and call `config_get_*` or `config_save` on them.
  - So anyone who follows the documented rule and then needs one of those locks can deadlock against a script. §10's own step 3 (`:459-460`) also breaks the rule. It's the API's only lock-order statement, and PR4's proof will have to cite it. The fix is two lines, one in the header and one in the rst.
  - Codex: please raise codex-4, or let claude-1 carry the Major.
- **codex-2 is part of claude-2.** Two additions:
  - `config_open_defaults` (`c:323-329`) also writes `defaults` without the lock.
  - `config_num_sections` runs unlocked only when called directly; `config_get_section` already calls it under the lock (`:495`).
  - A grep of `c:323-799` confirms these two are the only non-constructor functions that skip the mutex.
- **codex-3 is part of claude-5.**
- **codex-5 is part of claude-6.**

The lead's ledger has no rows yet. It should get all 11 findings, with these overlaps noted: claude-3 = codex-1, claude-1 = codex-4, and codex-2, codex-3 and codex-5 fall under claude-2, claude-5 and claude-6. I have no new findings.

FINDING claude-1 | Major | open | config.h:55-56/rst:113-114 "Take it before any other lock the caller owns" inverts the leaf order already used by Lua (lua.h:94) and Python GIL (python.c:110) callbacks that call config_*; WORKSPACE_FORMAT.md:459-460 breaks it too
FINDING claude-2 | Minor | open | "holder may call any config_* function" includes config_close (destroys held mutex c:477, frees object, later unlock is UAF); config_open_defaults/config_num_sections don't lock (c:323-329, 481-484)
FINDING claude-3 | Major | open | raised from Minor; same root cause as codex-1: config_save_safe reads config->file before the lock (c:414 vs 423), giving UAF at 443/449 and double free at 476; move the read after the lock
FINDING claude-4 | Minor | open | obslua.i:107/obspython.i:129 expose config_lock/unlock to scripts with no %ignore; scripts fall outside the PR4 lock-order proof
FINDING claude-5 | Minor | open | add_test path (cmocka/CMakeLists.txt:37) breaks under the VS/Xcode presets; cmocka is Linux-only in practice; test nits (volatile race :50, unused event :142, 100 ms timing :166-169, ANSI TEMP)
FINDING claude-6 | Minor | open | WORKSPACE_FORMAT.md:607-610 says cmocka never runs (false since CMakeLists.txt:35-38); §10 line refs 411-441 (:462) and 625-630 (:473) are stale
VERDICT: request changes
