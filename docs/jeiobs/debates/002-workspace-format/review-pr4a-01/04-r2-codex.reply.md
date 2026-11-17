No fixes are present at the reviewed commit; all prior findings remain open.

FINDING codex-1 | Major | open | `config_save_safe` snapshots `config->file` before locking (`libobs/util/config-file.c:414,423`), enabling a concurrent call to use a freed temporary at `:432,449,454`.
FINDING codex-2 | Minor | open | The docs claim the lock prevents all config changes and every getter locks (`libobs/util/config-file.h:52-54`), but `config_open_defaults` and `config_num_sections` access state unlocked (`libobs/util/config-file.c:323-329,481-484`).
FINDING codex-3 | Minor | open | The test’s `volatile int stop` is read and written across threads without synchronization (`test/cmocka/test_config_lock.c:50,68,171`).
FINDING codex-4 | Minor | open | The API says “Take it before any other lock” (`libobs/util/config-file.h:55-56`, `docs/sphinx/reference-libobs-util-config-file.rst:113-114`), conflicting with the required `persistMutex`-then-`config_lock` order (`docs/jeiobs/WORKSPACE_FORMAT.md:459-460`).
FINDING codex-5 | Minor | open | The docs still claim cmocka “never runs” (`docs/jeiobs/WORKSPACE_FORMAT.md:607-610`, `docs/jeiobs/tasks/002-workspace-saving.md:64-65`), contradicted by the new wiring (`CMakeLists.txt:35-37`).
FINDING codex-6 | Minor | open | The docs say the holder may call “any config_* function” (`libobs/util/config-file.h:54`), but `config_close` destroys the held mutex and frees the object (`libobs/util/config-file.c:459-478`).
FINDING codex-7 | Minor | open | The newly enabled tests use literal `add_test` binary paths (`test/cmocka/CMakeLists.txt:37`) that omit multi-config output directories/extensions under the Xcode and Visual Studio presets (`CMakePresets.json:90,161`).
FINDING codex-8 | Minor | open | Line citations became stale: `WORKSPACE_FORMAT.md:472-473` cites getters at `config-file.c:625-630`, which now contains default setters, and its save range ends before `os_safe_replace` at `:449`.
VERDICT: request changes
