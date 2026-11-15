# Initial coexistence audit

Inspected OBS 33.0.1 at 65c320b46e1af5c7a555161aea6182cdf7a8c524,
2026-11-15. This is a source inventory, not a completed isolation implementation.

| Surface | Verified integration point | Next work |
| --- | --- | --- |
| Config, profiles, scenes, logs, updates | frontend/OBSApp.cpp hard-codes obs-studio paths | Centralize fork identity and audit every reader/writer, including custom paths |
| Linux desktop identity | frontend/OBSApp.cpp; frontend/cmake/linux/com.obsproject.Studio.desktop | Independent desktop ID, binary, icon, MIME/protocol integration |
| Product/package | cmake/common/bootstrap.cmake; cmake/common/cpackconfig_common.cmake | jeiobs product and package naming; no OBS package replacement |
| macOS bundle | cmake/macos/helpers.cmake | Independent app/bundle identity, data, entitlements and update feed |
| Windows install layout | cmake/windows/helpers.cmake | Executable/package/uninstaller/registry and DLL search audit |
| Updates | frontend/updater; frontend/utility/AutoUpdateThread.cpp; frontend/utility/MacUpdateThread.cpp | Disable upstream updater in fork builds until own channel exists |
| Master mix candidate | libobs/obs-audio.c: audio_callback and mix_audio | Trace delivery to encoder; no hook chosen yet |

Still to inspect: WebSocket submodule/default port, virtual camera registrations,
plugin manager/core/legacy paths, IPC instance locks, crash/log upload destinations,
scripting, shared device access, and update/uninstaller ownership boundaries.

Acceptance matrix: Windows x64; macOS arm64 and x86_64; Linux x86_64. For each,
create sentinel OBS profiles/plugins, snapshot hashes, install OBS then jeiobs and
reverse the order, launch both, save separate scenes, attempt updates, uninstall
each in both orders and compare hashes. Include non-ASCII paths and clean users.
Tests must use disposable VMs/accounts until isolation is implemented.
The intended config root is jeiobs; other identifiers are design proposals until
recorded and implemented. Renaming the repository alone changes none of these.
