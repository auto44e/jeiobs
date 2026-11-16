The four prior Codex findings are fixed and accepted. One Minor schema/documentation mismatch remains.

FINDING codex-1 | Major | accepted-fix | Wait errors now leave entries unchanged; timeout requires a passed deadline and successful kill (vst3-scanner.cpp:1322-1329,1389-1391).
FINDING codex-2 | Major | accepted-fix | Exit 90 and `save-failed` now leave entries unchanged (vst3-scanner.cpp:1157-1160,1403-1405,1420-1423).
FINDING codex-3 | Minor | accepted-fix | Skip-list numeric fields and version now require JSON integer types (vst3-scanner.cpp:248-265,367-376).
FINDING codex-4 | Minor | accepted-fix | Setup marker publication and watchdog observation are mutex-atomic (vst3-scanner.cpp:1067-1078,1145-1149).
FINDING codex-5 | Minor | open | The spec permits “any non-empty string other than `timeout`” for failure kinds, but the loader accepts only `failure` or `timeout` (docs/jeiobs/tasks/005-vst3-skip-list.md:164-166; plugins/obs-vst3/vst3-scan/vst3-scanner.cpp:280-281).
VERDICT: approve