No prior Codex finding is fixed; one additional race is open.

FINDING codex-1 | Major | open | `os_process_pipe_wait` errors are treated as deadline kills and persisted as timeouts without checking the deadline (`vst3-scanner.cpp:1278-1281,1361-1363`).
FINDING codex-2 | Major | open | A failed `save-failed` marker write falls through from `setup` + no result to a remembered failure (`vst3-scanner.cpp:1150-1154,1367-1368`).
FINDING codex-3 | Minor | open | `obs_data_get_int` truncates doubles and returns zero for nonnumbers, allowing malformed skip-list fields/version values (`vst3-scanner.cpp:267-275,345-367`; `libobs/obs-data.c:1676-1689`).
FINDING codex-4 | Minor | open | The watchdog can observe `setupWritten == false` after the setup rename but before the atomic store, then persist failure instead of timeout (`vst3-scanner.cpp:1034-1038,1106-1107,1367-1368`).
VERDICT: request changes