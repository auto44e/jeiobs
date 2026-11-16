FINDING codex-1 | Major | accepted-fix | Wait errors now remain unchanged unless the deadline passed and cleanup succeeded (vst3-scanner.cpp:1322-1332).
FINDING codex-2 | Major | accepted-fix | Exit 90 and save failures remain unrecorded (vst3-scanner.cpp:1157-1160,1407-1409).
FINDING codex-3 | Minor | accepted-fix | Skip-list numeric fields require JSON integers (vst3-scanner.cpp:248-265).
FINDING codex-4 | Minor | accepted-fix | Setup publication and watchdog observation are mutex-atomic (vst3-scanner.cpp:1067-1078,1145-1149).
FINDING codex-5 | Minor | accepted-fix | Documentation now states the loader accepts only failure and timeout kinds (docs/jeiobs/tasks/005-vst3-skip-list.md:164-166).
FINDING codex-6 | Minor | open | The 100 ms heuristic can classify a genuine false wait error as a timeout strike (vst3-scanner.cpp:1323-1332; libobs/util/pipe-posix.c:150-189; libobs/util/pipe-windows.c:207-209).
VERDICT: approve