# Task 004: upstream services and remaining identity (needs owner decisions)

Status: **Proposed**. The display-only renames (window title, `--version`, tray,
English dialog text, startup log notice) are done on `jeiobs/identity-ui`. What
remains either sends data to the OBS Project, sends users to OBS Project
support, or is an identifier that M0 must change together with config paths.

Line numbers are from main `8c382452d`.

## Sends user data to obsproject.com (decide first)

| Feature | Where | Platforms |
| --- | --- | --- |
| Log upload: Help > Log Files > Upload. POSTs the whole log (system, GPU, plugin and path details) to `https://obsproject.com/logs/upload`. | `frontend/widgets/OBSBasic_MainControls.cpp:290-299` | all |
| Log analyzer link: opens `https://obsproject.com/tools/analyzer?log_url=…` | `frontend/dialogs/LogUploadDialog.cpp:144` | all |
| Crash report upload to the same endpoint | `frontend/utility/CrashHandler.cpp:38` | Windows, macOS (Linux finds no crash log) |

The OBS Project's support service should not receive JeiOBS logs. Options:
remove or hide the upload actions and keep "Show/View log files"; or point them
at a JeiOBS endpoint, which does not exist. Proposed: hide upload and analyzer,
keep local log access.

## Opens OBS Project pages or fetches from them

- Help menu (`OBSBasic_MainControls.cpp:418-461`): Help Portal, Website, Discord
  (obsproject.com) and Release Notes (github.com/obsproject releases for this
  version). Proposed: Website and Release Notes go to the JeiOBS repository;
  remove Discord; keep Help Portal but label it as upstream documentation.
- What's New (`frontend/utility/WhatsNewInfoThread.cpp:17-35`) fetches
  `obsproject.com/update_studio/whatsnew.json`. It is only built with browser
  panels (CEF), which the documented Linux build disables. Proposed:
  `ENABLE_WHATSNEW=OFF` by default.
- Updater (Windows and macOS only; none on Linux). Covered by M0.
- Missing-encoder help link `https://obsproject.com/kb/encoder-missing`
  (`OBSBasic_Profiles.cpp:1065`), the legacy plugin locations link
  (`plugin-manager/InstalledPluginRow.cpp:25`), the Windows VC runtime link
  (`obs-main.cpp:824`), the YouTube chat placeholder page
  (`oauth/YoutubeAuth.cpp:25`) and the OAuth callback page logo
  (`oauth/AuthListener.cpp:13`). These are content links. They are useful as-is
  but point users at upstream.
- OAuth (`frontend/CMakeLists.txt:63`, `auth.obsproject.com`) is only active
  when built with upstream client IDs, which a fork does not have.

## Names that are identifiers or sent to third parties

These are left alone on purpose until M0, because changing one without the
others breaks things:

- `com.obsproject.Studio` desktop ID (`OBSApp.cpp:1008`), the `.desktop` and
  AppStream files, macOS bundle IDs, the D-Bus path, the `obs-studio` config
  paths, and the `OBS_*` product constants in `cmake/common/bootstrap.cmake`.
- The startup log keeps `OBS <version>` and the upload headers keep
  `OBS <version> log file uploaded…`, so log tools can still parse them.
- Host names sent to plugins and services: the VST2 host vendor "OBS Studio"
  (`plugins/obs-vst/VSTPlugin.cpp:50`; some plugins check it), the PipeWire
  stream name (`plugins/linux-pipewire/screencast-portal.c:187`), and the
  YouTube default stream titles (`OBSYoutubeActions.cpp:407,456`,
  `AutoConfigStreamPage.cpp:302`).
- The logo, the 59 non-English locales, and the unused `OBSStudio` ini key.
