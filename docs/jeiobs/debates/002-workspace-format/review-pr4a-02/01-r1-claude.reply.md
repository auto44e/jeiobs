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
