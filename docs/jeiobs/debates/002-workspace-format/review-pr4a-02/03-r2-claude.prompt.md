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

REVIEWED COMMIT: e3ecb4179924693626dcd59ecf8500ec7f287a46

TOPIC: Review run 02 of pr38.

SHARED CONTEXT:
# PR #38 review run 02 — S1-PR4a: config_lock() and config_unlock(), head e3ecb4179924693626dcd59ecf8500ec7f287a46

Run 01 (c23b46031): both peers requested changes (Claude: Major lock-order wording, Major config_save_safe race; Codex: Major config_save_safe race, plus Minors). The lead fixed everything (ledger and docs/jeiobs/debates/002-workspace-format/findings.md, "PR #38"). Your working directory is a checkout of the new head. Change since run 01: `git diff c23b46031 HEAD`; full change: `git diff f22e6274d HEAD`.
Verify each fix against the code, not the ledger: `config_save_safe` now reads `config->file` after taking the lock (is any other path still unlocked? config_save_safe's own error paths, the swap/restore under the lock, config_close); `config_num_sections` and `config_open_defaults` lock (any deadlock or lock-order risk, since config_get_section calls config_num_sections under the lock and config_open_defaults parses a file); header and sphinx wording (consistent with WORKSPACE_FORMAT section 10's persistMutex-then-config_lock order); `%ignore` in obslua.i and obspython.i (does SWIG accept it there; are other scripting bindings affected); test wiring and the new concurrent save_safe test (does it really fail without the fix; flakiness; portability); doc citations. Also look for anything the fixes broke or missed.
Lead's evidence (Linux, GCC, Debug, libobs-only build with -DENABLE_UNIT_TESTS=ON): test_config_lock (4 tests) passes 30 of 30 runs; the concurrent save_safe test fails 3 of 20 runs against the pre-fix config_save_safe and 0 of 60 after; no-op lock mutation fails the blocking tests. Not run: macOS, Windows, the scripting build (swig).
Severity: Blocker = must not merge; Major = must be fixed before a self-merge; Minor = follow-up. Owner (verbatim): "if you peers are more or less in agreement, no problem self merging".
Other agent work runs on this host in separate worktrees; it does not touch this checkout.

FINDINGS LEDGER (kept by the lead):
# Findings ledger

ID | severity | status | finder | source | lead's note
--- | --- | --- | --- | --- | ---
quick checks pending at start
local: test_config_lock 80/80 runs pass (Linux GCC Debug); no-op mutation fails 2 tests
claude-1, codex-4 | Major, Minor | fixed, awaiting finder | both | run 01 | lock-order wording rewritten
claude-3, codex-1 | Major | fixed, awaiting finder | both | run 01 | config_save_safe reads the file name under the lock; regression test
claude-2, codex-2, codex-6 | Minor | fixed, awaiting finder | both | run 01 | open_defaults and num_sections lock; config_close excluded
claude-4 | Minor | fixed, awaiting finder | claude | run 01 | %ignore in the SWIG interfaces
claude-5, codex-3, codex-7 | Minor | fixed, awaiting finder | both | run 01 | add_test NAME/COMMAND; atomic stop flag; unused event removed
claude-6, codex-5, codex-8 | Minor | fixed, awaiting finder | both | run 01 | stale citations replaced

YOUR OWN PREVIOUS REPLY:
**Approve.** All six of my run-01 findings are fixed in the code at e3ecb417, which I checked in the files rather than taking from the ledger. I found three new Minors, none of them blocking. I could only read files, so I built and ran nothing.

**Fixes checked**
- **claude-3 (save race):** fixed.
  - `config_save_safe` now reads `config->file` only after taking the lock (`config-file.c:429-433`).
  - The swap and restore (`:440-442`), the backup copy (`:453`), the replace (`:459`) and both exits through `cleanup` (`:462-466`) all run under the lock. The early return at `:423-427` touches nothing in the config.
  - Without the fix, glibc's tcache reuses the freed temp pointer, so the bug turns into a save failure, a leftover `.tmp` file or a double free in `config_close`. The test detects all three, but only sometimes; the lead's 3 of 20 is believable.
  - With the fix, every save is serialized and `os_safe_replace` is a plain rename (`platform-nix.c:576-581`), so I see no source of flakiness.
- **claude-1 (lock-order wording):** fixed. "One fixed order relative to this lock" plus "do not wait for another thread that may be using this config_t" (`h:56-58`, `rst:113-116`) is correct, and matches §10's `persistMutex`-then-`config_lock` order (`WORKSPACE_FORMAT.md:459-460`).
- **claude-2:** fixed.
  - The docs now exclude `config_close` (`h:54`, `rst:111`).
  - `config_open_defaults` (`c:330-332`) and `config_num_sections` (`c:495-497`) now take the lock.
  - No deadlock risk: every `config_t` mutex is created recursive (`c:81,280,308`), so `config_get_section` can call `config_num_sections` under the lock (`c:509-511`), and `config_parse_file` takes no other lock.
- **claude-4:** fixed. The `%ignore` lines come before `%include "util/config-file.h"` in both `obslua.i:56-57/109` and `obspython.i:68-69/131`. These are the only two `.i` files in the tree.
- **claude-5:** fixed.
  - The test is registered with `add_test(NAME … COMMAND test_config_lock)` (`cmocka/CMakeLists.txt:37`).
  - The stop flag now uses atomics (`test:69,220`), and the unused event is gone.
  - The check that the writer makes progress within 100 ms (`test:215-218`) is still there; I accept it.
- **claude-6:** fixed (`WORKSPACE_FORMAT.md:459-463,610`).

**New findings**
- **claude-7 (Minor): the fixes made two line citations stale.**
  - Locking `config_open_defaults` added 6 lines, which shifted everything below it.
  - `WORKSPACE_FORMAT.md:489` cites "truncates in place, `config-file.c:347`". Line 347 is now blank; the `os_fopen(config->file, "wb")` call is at `:353`.
  - `:617` cites the BOM write as `config-file.c:378-381`. Those lines are now `dstr_cat` calls inside the item loop; the BOM write is at `:384-387`.
  - §10 itself says "line numbers drift, so they are not cited" (`:462`).
- **claude-8 (Minor, also in upstream): `config_save` reads `config->file` before taking the lock.**
  - The `if (!config->file)` check (`c:345`) runs before the lock at `c:351`, while `config_save_safe` writes that field under the lock (`c:440,442`).
  - Example: obs-websocket's worker calls `config_save` on basic.ini while the UI calls `config_save_safe` on it (`WORKSPACE_FORMAT.md:489`).
  - Formally this is a data race that ThreadSanitizer would report. In practice it is harmless: the value is only compared with NULL, and `:353` reads it again under the lock (`os_fopen(NULL)` returns NULL, `platform.c:61,69`).
  - Fix: move the check after the lock.
- **claude-9 (Minor): the new test's file-content check proves nothing.**
  - `test:109` says "The real file holds a complete config", but `config_get_int(reopened, "A", "counter") >= 0` (`test:112`) also passes for an empty file: a missing key returns 0 (`c:692-699`), and an empty file opens with success (`c:256-257`).
  - The final value is always 399, because every thread's last write sets 399 before its last save. So `assert_int_equal(..., 399)` would be a real check.

FINDING claude-1 | Major | accepted-fix | h:56-58/rst:113-116 now say one fixed order relative to this lock and no waiting on another user of the config_t; consistent with WORKSPACE_FORMAT.md:459-460
FINDING claude-2 | Minor | accepted-fix | config_close excluded (h:54, rst:111); open_defaults (c:330-332) and num_sections (c:495-497) lock; mutexes recursive (c:81,280,308) so config_get_section cannot self-deadlock
FINDING claude-3 | Major | accepted-fix | config->file read after the lock (c:429-433); swap/restore, backup copy, replace and both exits all under the lock (c:440-466)
FINDING claude-4 | Minor | accepted-fix | %ignore before %include "util/config-file.h" in obslua.i:56-57/109 and obspython.i:68-69/131; no other .i files
FINDING claude-5 | Minor | accepted-fix | add_test NAME/COMMAND (cmocka/CMakeLists.txt:37), atomic stop flag (test:69,220), unused event removed; 100 ms progress check (test:215-218) accepted
FINDING claude-6 | Minor | accepted-fix | WORKSPACE_FORMAT.md:610 and §10 :459-463 updated
FINDING claude-7 | Minor | open | WORKSPACE_FORMAT.md:489 "config-file.c:347" (now a blank line; fopen "wb" at :353) and :617 "config-file.c:378-381" (BOM now :384-387) became stale when config_open_defaults grew by 6 lines
FINDING claude-8 | Minor | open | config_save's `if (!config->file)` (c:345) runs before the lock (c:351) while config_save_safe writes config->file under it (c:440,442): formal data race, harmless (NULL test only, read again under the lock at :353); move it after the lock
FINDING claude-9 | Minor | open | test:112 `config_get_int(reopened,"A","counter") >= 0` also passes for an empty file (c:256-257, 692-699), contrary to the comment at :109; assert == 399
VERDICT: approve

THE OTHER REVIEWER'S LATEST REPLY:
FINDING codex-1 | Major | accepted-fix | `config_save_safe` snapshots `config->file` under the mutex (`libobs/util/config-file.c:429-463`).
FINDING codex-2 | Minor | accepted-fix | `config_open_defaults` and `config_num_sections` now lock (`libobs/util/config-file.c:330-332,491-499`).
FINDING codex-3 | Minor | accepted-fix | Stop flag uses atomic load/store (`test/cmocka/test_config_lock.c:69,220`).
FINDING codex-4 | Minor | accepted-fix | Lock-order wording now permits the documented persist-mutex-then-config-lock order (`libobs/util/config-file.h:57-59`).
FINDING codex-5 | Minor | accepted-fix | Documentation now records conditional cmocka wiring and absent CI coverage (`docs/jeiobs/WORKSPACE_FORMAT.md:608-611`).
FINDING codex-6 | Minor | accepted-fix | Documentation excludes `config_close` and warns against concurrent close (`libobs/util/config-file.h:54-59`).
FINDING codex-7 | Minor | accepted-fix | Test registration uses target-aware CTest syntax (`test/cmocka/CMakeLists.txt:37`).
FINDING codex-8 | Minor | accepted-fix | Stale section-10 line citations were replaced with function references (`docs/jeiobs/WORKSPACE_FORMAT.md:460-475`).
FINDING codex-9 | Minor | open | `config_save` still reads `config->file` before locking (`libobs/util/config-file.c:343-351`), racing `config_save_safe`’s swap/restore (`:440-442`).
FINDING codex-10 | Minor | open | Documentation citations remain stale: `config-file.c:347` and `:378-381` no longer identify truncation/BOM writes (`docs/jeiobs/WORKSPACE_FORMAT.md:489,617`).
VERDICT: approve
