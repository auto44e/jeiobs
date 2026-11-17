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
**Verdict: request changes.** One Major (the lock-order rule in the new API's docs) and five Minor. I could only read files, so I built and ran nothing; everything below comes from reading the code at c23b4603.

The C code is fine. `config-file.c:398-408` uses the same recursive mutex that every locked path takes (it is created at 81, 280 and 308), and NULL does nothing. `EXPORT` resolves to dllexport or default visibility, so the change only adds symbols and breaks no ABI. When `ENABLE_UNIT_TESTS` is unset or OFF, `CMakeLists.txt:35-38` is skipped and normal builds don't change.

**claude-1 (Major): the lock-order rule is backwards and contradicts the only planned caller.**
- `config-file.h:55-56` and `rst:113-114` say: "Take it before any other lock the caller owns."
- `WORKSPACE_FORMAT.md:459-460` takes `persistMutex` first, then three `config_lock`s one after another. The rule forbids both: `persistMutex` is a caller lock, and each config lock is "another lock the caller owns" when the next one is taken.
- `config->mutex` has been a leaf lock until now, because no config function takes another lock while holding it. So any thread may already call `config_*` while holding its own lock M (order M → cfg). The rule pushes holders to do cfg → M, which closes a deadlock cycle.
- The safe rule is the reverse: config locks are innermost. While holding one, take no other lock except more config locks in one fixed order. Don't wait for it while holding a lock that audio or video threads need.

**claude-2 (Minor): the claim that "the holder may call any config_* function" (`h:54`, `rst:110-111`) also covers `config_close`.**
- `config_close` destroys the held mutex (`c:477`; undefined in POSIX, glibc returns EBUSY) and frees the object. The matching `config_unlock`, or a C++ guard's destructor, is then a use-after-free.
- "The one every getter, setter and save call takes" is also not quite true: `config_open_defaults` (`c:323-329`) and `config_num_sections` (`c:481-484`) don't take the lock.
- The header's "a file … is not rewritten" should say "through this config_t", as the rst already does ("by it").

**claude-3 (Minor): `config_save_safe` reads `config->file` before taking the lock (`c:414` vs `c:423`).**
- Sequence: the holder's `config_save_safe` swaps in the temp path (`c:430-431`). Thread B enters and keeps that pointer. The holder frees it (`c:454`). B then writes it back (`c:432`).
- Result: use-after-free at `c:443` and `c:449`, a dangling `config->file`, and a double free at `c:476`.
- The bug is in upstream OBS too. But §10 step 4 calls `config_save_safe` under the lock, and the `user.ini` row (`WORKSPACE_FORMAT.md:487`) counts plugins' any-thread `config_save_safe` as "serialized by config lock".
- One-line fix: read `config->file` after taking the mutex.

**claude-4 (Minor): Lua and Python scripts get `config_lock` and `config_unlock` automatically.**
- `obslua.i:107` and `obspython.i:129` include `util/config-file.h` with no `%ignore` for the new functions.
- Scripts can take config locks in any order, which PR4's proof ("each writer takes at most one", `WORKSPACE_FORMAT.md:472`) doesn't cover. A script error between the two calls leaves the lock held and hangs the UI.
- Either add `%ignore` in both files or document the exposure as intended.

**claude-5 (Minor): the test only works on Linux in practice.**
- `test/cmocka/CMakeLists.txt:37` registers the test with a hard-coded binary path. The macOS and Windows presets use Xcode and Visual Studio (`CMakePresets.json:90,161,190`), which put the binary under `<Config>/`. Use `add_test(NAME test_config_lock COMMAND test_config_lock)`.
- `find_package(CMocka CONFIG REQUIRED)` (line 3) has no source for cmocka on Windows or macOS. Task 002 (lines 65-67) should say ON is Linux-only.
- Test nits:
  - `volatile int stop` (test:50) is a data race; use `os_atomic_*`.
  - The `finished` event in test 3 is unused (:142).
  - The check at :166-169 needs the writer to progress within 100 ms. It is the only assertion that could flake.
  - On Windows, `getenv("TEMP")` returns the ANSI code page, but `os_fopen` expects UTF-8.
  - Temp files are left behind when an assertion fails.

**claude-6 (Minor): stale text in the spec.**
- `WORKSPACE_FORMAT.md:609-610` still says the cmocka setup never runs. That stopped being true with `CMakeLists.txt:35-38`.
- `:607-608` calls a test framework a new dependency that the owner must merge. Reconcile this: cmocka is upstream's existing optional dependency, already installed by `setup_ubuntu:86` and `cleanroom/Dockerfile:19`, and no pins changed.
- Line references in §10 moved with this PR: `411-441` (:462) is now 423-453, and `625-630` (:473) is now 642-650.

FINDING claude-1 | Major | open | config.h:55-56/rst:113 "Take it before any other lock the caller owns" is backwards for a leaf mutex and contradicts WORKSPACE_FORMAT.md:459-460 (persistMutex, then 3 config locks)
FINDING claude-2 | Minor | open | "holder may call any config_* function" includes config_close (destroys held mutex, frees object, later unlock is UAF); open_defaults/num_sections don't lock
FINDING claude-3 | Minor | open | config_save_safe reads config->file unlocked (c:414); racing a holder's swap (430-432) gives UAF and double free (476); move after lock
FINDING claude-4 | Minor | open | obslua.i:107/obspython.i:129 expose config_lock/unlock to scripts with no %ignore; outside the PR4 lock-order proof
FINDING claude-5 | Minor | open | add_test path breaks under VS/Xcode presets; cmocka Linux-only; test nits (volatile race, unused event, 100 ms timing, ANSI TEMP)
FINDING claude-6 | Minor | open | WORKSPACE_FORMAT.md:607-610 says cmocka never runs (now false); §10 line refs 411-441/625-630 stale
VERDICT: request changes

THE OTHER REVIEWER'S LATEST REPLY:
FINDING codex-1 | Major | open | `"char *file = config->file"` precedes locking at `libobs/util/config-file.c:414`; concurrent `config_save_safe` calls can capture another call’s temporary, restore a freed pointer at `:432`, and pass it to `os_safe_replace` at `:449` after `dstr_free` at `:454`.
FINDING codex-2 | Minor | open | The API claims every getter takes the lock (`libobs/util/config-file.h:52-54`), but `config_num_sections` reads the hash unlocked at `libobs/util/config-file.c:481-484`.
FINDING codex-3 | Minor | open | The test has an unsynchronized cross-thread stop flag: `volatile int stop` (`test/cmocka/test_config_lock.c:50`) is read at `:68` and written at `:171`; `volatile` is not C thread synchronization.
FINDING codex-4 | Minor | open | Lock-order documentation conflicts: the API requires `config_lock` before other locks (`libobs/util/config-file.h:55-56`), while the workspace protocol requires `persistMutex` before `config_lock` (`docs/jeiobs/WORKSPACE_FORMAT.md:459-464`).
FINDING codex-5 | Minor | open | `WORKSPACE_FORMAT.md` still says cmocka “never runs” because only `test/test-input` is added (`docs/jeiobs/WORKSPACE_FORMAT.md:607-610`), contradicted by this commit’s conditional wiring (`CMakeLists.txt:35-37`).
VERDICT: request changes
