You are one of two independent reviewers (Claude and Codex, different model lineages) in a
cross-talk review for the JeiOBS repository. Your working directory is a checkout of exactly the
commit under review. The lead agent wrote or integrated the change and must NOT decide on its own
whether your findings are resolved: under docs/jeiobs/DECISIONS.md a finding is closed only when the
reviewer who raised it accepts the fix or withdraws it, in writing, in a later round. Use one
severity scale: Blocker, Major, Minor. Only the finder may lower a finding's severity. You are
read-only: never modify, create or delete files, and never run commands that change state.
Verify claims in the files rather than trusting the context. Be terse; cite file:line.

End your reply with one line per finding YOU raised (every ledger row with you as finder, every
finding you raised in an earlier round, plus new ones), then the verdict. If you raised none, write
FINDING none. Use exactly this syntax:
FINDING <ID> | <Blocker|Major|Minor> | <open|accepted-fix|withdrawn> | <one-line reason>
VERDICT: approve
or
VERDICT: request changes
For a new finding, use ID <your-name>-<n> (for example claude-1) and quote the evidence.

REVIEWED COMMIT: c23b4603170555429450eb07d2a4fa6b21cc8732

TOPIC: Review run 01 of pr38.

SHARED CONTEXT:
# PR #38 review run 01 — S1-PR4a: config_lock() and config_unlock() in libobs util, head c23b4603170555429450eb07d2a4fa6b21cc8732

Your working directory is a checkout of this commit. Change: `git diff f22e6274d HEAD`. Read libobs/util/config-file.{h,c} (the existing mutex use: `config_save`, `config_save_safe` hold it across the write and the replace; getters take it), test/cmocka/test_config_lock.c, the CMake wiring (top-level CMakeLists.txt `ENABLE_UNIT_TESTS`, test/cmocka/CMakeLists.txt), docs/sphinx/reference-libobs-util-config-file.rst, and docs/jeiobs/WORKSPACE_FORMAT.md sections 10 and 14 (decision 1) and task 002.
Authorship: the lead wrote everything (about 40 lines of C over an existing recursive mutex). No worker.
Lead's evidence (Linux, GCC, Debug, libobs-only build with -DENABLE_UNIT_TESTS=ON): test_config_lock passes 80 of 80 runs; making config_lock a no-op fails both blocking tests; test_os_path and test_darray still build and pass. Not run: macOS, Windows, any CI for libobs unit tests (none exists).
Review for: correctness and safety of the API (recursive lock semantics, NULL handling, pairing, interaction with config_save_safe swapping config->file while holding the mutex, config_close destroying a locked mutex, lock-order hazards with other libobs locks, ABI/export), documentation accuracy, the test's validity and portability (w32-pthreads, os_event, timing assumptions, flakiness), the CMake change (does ENABLE_UNIT_TESTS=OFF leave normal builds unchanged; does test/cmocka build on all three platforms' toolchains with the options as wired), and the doc claims in WORKSPACE_FORMAT/task 002.
Severity: Blocker = must not merge; Major = must be fixed before a self-merge; Minor = follow-up. Owner (verbatim): "if you peers are more or less in agreement, no problem self merging".
Other agent work runs on this host in separate worktrees; it does not touch this checkout.

FINDINGS LEDGER (kept by the lead):
# Findings ledger

ID | severity | status | finder | source | lead's note
--- | --- | --- | --- | --- | ---
quick checks pending at start
local: test_config_lock 80/80 runs pass (Linux GCC Debug); no-op mutation fails 2 tests

YOUR OWN PREVIOUS REPLY:
FINDING codex-1 | Major | open | `"char *file = config->file"` precedes locking at `libobs/util/config-file.c:414`; concurrent `config_save_safe` calls can capture another call’s temporary, restore a freed pointer at `:432`, and pass it to `os_safe_replace` at `:449` after `dstr_free` at `:454`.
FINDING codex-2 | Minor | open | The API claims every getter takes the lock (`libobs/util/config-file.h:52-54`), but `config_num_sections` reads the hash unlocked at `libobs/util/config-file.c:481-484`.
FINDING codex-3 | Minor | open | The test has an unsynchronized cross-thread stop flag: `volatile int stop` (`test/cmocka/test_config_lock.c:50`) is read at `:68` and written at `:171`; `volatile` is not C thread synchronization.
FINDING codex-4 | Minor | open | Lock-order documentation conflicts: the API requires `config_lock` before other locks (`libobs/util/config-file.h:55-56`), while the workspace protocol requires `persistMutex` before `config_lock` (`docs/jeiobs/WORKSPACE_FORMAT.md:459-464`).
FINDING codex-5 | Minor | open | `WORKSPACE_FORMAT.md` still says cmocka “never runs” because only `test/test-input` is added (`docs/jeiobs/WORKSPACE_FORMAT.md:607-610`), contradicted by this commit’s conditional wiring (`CMakeLists.txt:35-37`).
VERDICT: request changes
THE OTHER REVIEWER'S LATEST REPLY:
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
