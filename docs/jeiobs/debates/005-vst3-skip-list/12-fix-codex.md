Implemented and committed as `13b6bdbe2`.

- Wait errors now leave entries unchanged; timeout strikes require a passed monotonic deadline and successful kill (`vst3-scanner.cpp:1311-1329,1388-1390`).
- Added downgrade-only exit codes 90/91 (`:71-76,1068-1078,1157-1160,1419-1429`).
- Valid nonce-matching results are accepted before marker handling (`:1381-1386`).
- Setup marker publication is mutex-atomic with watchdog observation (`:1023-1032,1070-1075,1146-1149`).
- Skip-list/version numeric fields require integer JSON types (`:248-265,284-300,369-376`).
- Linux binary resolution now uses `uname().machine + "-linux"` (`:587-595`).

Verified:

- `cmake --build _build -j8` passed.
- Scanner and all 16 D6 skip-list checks passed.
- Full script: 26 checks passed; 6 OBS host/audio checks were blocked by missing Xvfb/X display.
- Worktree is clean; only the requested scanner file changed.
