# Task 004: upstream services and remaining identity (needs owner decisions)

Status: **Proposed**. The display-only renames (window title, `--version`, tray,
English dialog text, startup log notice) are done on `jeiobs/identity-ui`. What
remains either sends data to the OBS Project, sends users to OBS Project
support, or is an identifier that M0 must change together with config paths.

Line numbers are from main `a88ba0bbd`.

## Sends user data to obsproject.com (decide first)

| Feature | Where | Platforms |
| --- | --- | --- |
| Log upload: Help > Log Files > Upload. POSTs the whole log (system, GPU, plugin and path details) to `https://obsproject.com/logs/upload`. | `frontend/widgets/OBSBasic_MainControls.cpp:290-299` | all |
| Log analyzer link: opens `https://obsproject.com/tools/analyzer?log_url=…` | `frontend/dialogs/LogUploadDialog.cpp:144` | all |
| Crash report upload to the same endpoint | `frontend/utility/CrashHandler.cpp:38` | Windows, macOS (Linux finds no crash log) |

**Decided (owner, 2026-11-15): no uploads; save the file locally.** Done on
`jeiobs/local-logs`:

- Help > Log Files > "Save a Copy of the Current/Previous Log" writes the log,
  with a one-line JeiOBS header, to `<Documents>/JeiOBS/logs/<log name>`
  without asking where (home directory if there is no Documents folder). An
  existing copy is never replaced; a second save becomes `<log name> (2)`. A
  message shows the path, offers "Show in Folder", and reminds the user to check
  the file for personal details before sharing it. Logs are still written to the
  normal log folder as they happen.
- The upload dialog (`LogUploadDialog`), the analyzer link, the crash-report
  upload checkbox on the unclean-shutdown prompt, the "Upload Previous Crash
  Report" menu item and all crash-upload code in `CrashHandler` are removed.
  Crash reports stay in the local crash folder (Help > Crash Reports > Show
  Crash Reports, Windows and macOS).
- Fixed on the way: on a first run there is no previous log, and the old code
  read the log folder itself as a file, which trapped in libobs. The new code
  shows "No log file was found" instead.

## Opens OBS Project pages or fetches from them

- **Done (owner agreed 2026-11-15, `jeiobs/help-links`):** Help menu
  "JeiOBS Repository" and "JeiOBS Release Notes" open
  `github.com/auto44e/jeiobs` and its releases page. Discord is removed. The help
  portal stays and is labelled "OBS Studio Help Portal (upstream)". New en-US keys
  are used for the relabelled items, so other languages show the English label
  instead of a translation that names the wrong product.
- **Done:** What's New (`frontend/utility/WhatsNewInfoThread.cpp:17-35`)
  fetched `obsproject.com/update_studio/whatsnew.json`. `ENABLE_WHATSNEW` now
  defaults to OFF on every platform; the menu item is removed when it is off. It
  was already absent from the documented Linux build, which has no browser
  panels.
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
