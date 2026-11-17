**Approve.** The head is still e3ecb417, the commit I reviewed last round. I re-read the files, and my earlier conclusions all hold. Codex and I agree on everything that matters: neither of us has a Blocker or Major open, so the self-merge condition in `DECISIONS.md:187-196` is met. The three Minors left open are follow-ups. I could only read files, so I built and ran nothing.

**Codex's new findings repeat two of mine.** I checked both again:
- **codex-9 is the same as claude-8.** `config_save` reads `config->file` at `config-file.c:345`, before it takes the lock at `:351`. `config_save_safe` writes that field under the lock at `:440` and `:442`. It is harmless because `:353` reads the field again under the lock.
- **codex-10 is the same as claude-7.** `WORKSPACE_FORMAT.md:489` cites `config-file.c:347`, which is now a blank line; the `os_fopen(…, "wb")` call is at `:353`. `WORKSPACE_FORMAT.md:617` cites `:378-381`, which are now `dstr_cat` lines; the BOM write is at `:384-387`.

**Codex's accepted fixes check out.**
- The lock-order wording is at `h:54-59`.
- The cmocka and CI wording is at `WORKSPACE_FORMAT.md:610-611`.
- The locked `config_open_defaults` and `config_num_sections` are at `c:330-332` and `c:495-497`.

**claude-9 is still open; Codex did not comment on it.**
- The check `config_get_int(reopened, "A", "counter") >= 0` (`test:112`) also passes for an empty file.
- An empty file opens successfully (`c:256-257`), and a missing key returns 0 (`c:692-699`).
- The value 399 is guaranteed, so asserting it would be a real check:
  - Every set and save takes the same mutex (`c:547`, `c:429`), so they happen in a single order.
  - Each thread's last set is 399, and each thread saves after every set.
  - So the last save comes after the last set, and the file holds `counter=399` whenever there are no failures (asserted at `test:106`).

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
