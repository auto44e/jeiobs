# S1 persistence audit (task 002)

Status: audit, 2026-11-16, at main `b76d7cdb1` (OBS 33.0.1 base). Read from the
code by four parallel read-only investigations (context-gatherer subagents); not
run. File:line references are approximate where marked "~". This document records
facts the snapshot format must respect; it does not choose the format (see the
S1 design record).

## 1. Roots

| Root | Default | Set by | Notes |
| --- | --- | --- | --- |
| config root | OS config dir + `obs-studio/` (`~/.config/obs-studio` on Linux) | `GetAppConfigPath` `frontend/OBSApp.cpp:1702-1717` | portable mode: `CONFIG_PATH` |
| `userConfigLocation` (U) | config root | `global.ini [Locations] Configuration` (`OBSApp.cpp:548-615`) | falls back to default if missing; ignored in portable mode |
| `userScenesLocation` (S) | config root | `[Locations] SceneCollections` | same |
| `userProfilesLocation` (P) | config root | `[Locations] Profiles` | same |
| `userPluginManagerSettingsLocation` | config root | `[Locations] PluginManagerSettings` | same |

`global.ini` always lives at the real config root. A snapshot must resolve paths
through these variables, never by assuming one root.

## 2. Files and their writers

| File | Format | Writer and trigger | Atomic? |
| --- | --- | --- | --- |
| `obs-studio/global.ini` | ini (`appConfig`) | `saveAll` at exit (`OBSBasic.cpp:1759`), beta opt-in, crash handler, migrations | tmp + rename, no `.bak` |
| `<U>/obs-studio/user.ini` | ini (`userConfig`) | many: settings OK, profile/collection switch, docks, volume, exit (`OBSBasic.cpp:1769`). **Geometry and DockState are set in memory and only flushed at exit** | tmp + rename, no `.bak` |
| `<P>/obs-studio/basic/profiles/<dir>/basic.ini` | ini (`activeConfiguration`) | settings apply (`OBSBasicSettings.cpp:3788`), profile switch (`OBSBasic_Profiles.cpp:677,695,713`), OAuth (`oauth/Auth.cpp:79,86`), output setup, hotkey rename (`OBSBasic_Hotkeys.cpp:122`, **plain, non-atomic**) | tmp + rename (one plain writer) |
| `<P>/.../service.json` | obs_data JSON `{type, settings}` | `SaveService` (`OBSBasic_Service.cpp:24-42`) on settings apply / API | tmp + `.bak` |
| `<P>/.../streamEncoder.json`, `recordEncoder.json` | obs_data JSON | `WriteJsonData` (`OBSBasicSettings.cpp:3384-3401`) when changed | tmp + `.bak` |
| `<S>/obs-studio/basic/scenes/<name>.json` | obs_data JSON (pretty) | `OBSBasic::Save` (`OBSBasic_SceneCollections.cpp:806-990`) via coalesced `SaveProject` on every change, `SaveProjectNow` before switch/duplicate/rename, and at exit. **No autosave timer.** | tmp + `.json.bak` |
| `<S>/.../<name>.json.v1` | JSON | one-time backup before v1→v2 coordinate migration (`:1307-1320`) | tmp |
| `<plugin_manager>/modules.json` | JSON array | `PluginManager::saveModules_` (`plugin-manager/PluginManager.cpp:190-208`) every startup | **plain** |
| `obs-studio/plugin_config/<module>/…` | per module | plugins via `obs_module_config_path` (section 4) | per plugin |
| `obs-studio/logs/*.txt`, `crashes/`, `profiler_data/`, `updates/`, `.sentinel/` | text/binary | runtime only | plain |
| `obs-studio/themes/` | user files | read only | — |

Profile operations (`OBSBasic_Profiles.cpp`): duplicate and rename copy the whole
directory recursively (rename = copy + delete, no rollback); import/export copy
only `basic.ini`, `service.json` and the two encoder files. Collection rename leaves
the old `.json.bak`/`.json.v1` orphaned. Corrupt collections are renamed to
`.json.invalid` and replaced by a default scene.

**Restore hazard:** `obs_data_create_from_json_file_safe(path, "bak")` renames a
`.bak` over an unparseable file. Restoring a `.json` while a stale `.bak` sits next
to it can silently replace the restored file. Restore must write or remove both.

## 3. Scene collection document

Top-level keys, in write order (`OBSBasic_SceneCollections.cpp:806-990`): `name`;
`DesktopAudioDevice1..2`, `AuxAudioDevice1..4` (full source objects for output
channels 1–6, absent = disabled); `sources`; `groups`; `scene_order`;
`current_scene`, `current_program_scene` (names, not UUIDs); `canvases`;
`current_transition`, `transition_duration`, `transitions`, `quick_transitions`;
`saved_projectors`; preview UI state (`preview_locked`, `scaling_*`);
`virtual-camera`; `modules`; `resolution`, `migration_resolution`; `version`
(2 = relative coordinates, 1 = absolute). Plugins may also add top-level keys
through save callbacks (`scripts-tool`, `auto-scene-switcher`, `captions`,
`output-timer`, `decklink_captions`).

Per source (`libobs/obs.c:2443-2545`): `prev_ver`, `name`, `uuid`, `id`,
`versioned_id`, `settings`, `mixers` (bitmask, default `0x3F`), `sync`, `flags`,
`volume`, `balance`, `enabled`, `muted`, push-to-mute/talk and delays, `hotkeys`,
`deinterlace_*`, `monitoring_type`, `monitoring_enabled`, `canvas_uuid` (scenes,
groups), `private_settings`, transition keys, `filters` (recursive, reverse order).
Scene items (`libobs/obs-scene.c:1308-1433`) reference sources by `source_uuid`,
falling back to `name`.

### What a stock OBS re-save loses (important for C1–C3 and for snapshots)

- **Unknown top-level keys are dropped**, except `modules` (kept verbatim from
  load, so data from unloaded plugins survives) and keys owned by loaded save
  callbacks.
- **Unknown per-source and per-scene-item keys are dropped.** Unknown keys
  *inside* `settings`, `private_settings` and `hotkeys` survive.
- Defaults are materialized, `prev_ver` is rewritten, `monitoring_type` collapses
  to 0 or 2, relative/absolute coordinates are recomputed, `virtual-camera` legacy
  keys are normalized.
- **Sources whose plugin is missing survive** with id, uuid and settings.
  **Transitions whose plugin is missing are dropped** unless safe mode is on.
  Scene items whose source cannot be resolved are dropped.
- UUIDs are stable for sources, scenes, groups, filters and canvases; transitions
  get new UUIDs on each load and are referenced by name. Duplicate UUIDs are
  replaced on load. Duplicating a collection regenerates `sources` UUIDs only.

Consequence: a jeiobs snapshot must store the files as written (lossless), not a
re-serialization through libobs. jeiobs-only data must not be embedded in OBS
documents where OBS drops it (sidecars, per ROADMAP C2).

## 4. Plugin-owned state

| Owner | Where | Secret? |
| --- | --- | --- |
| obs-vst3 | filter `settings`: `vst3_plugin` (class ID), `vst3_path`, `vst3_name`, `vst3_state` and `vst3_ctrl_state` (**hex**), `sidechain_source`; module config `vst3list.json`, transient `probe-*.json` | path; opaque state may hold licence data |
| obs-vst (VST2) | filter `settings`: `plugin_path`, `chunk_data` (base64), `chunk_hash` (MD5 of the plugin binary) | path; opaque state |
| obs-websocket | module config `config.json` (`ServerPassword` **plaintext**, port, flags); `persistent_data.json` (client-defined) | **yes** |
| scripts (frontend-tools) | collection key `scripts-tool[] {path, settings}`; Python path in user config | paths; script settings may hold tokens |
| auto scene switcher, captions, output timer | collection keys | window titles (private) |
| decklink/aja output UI | module config JSON | no |
| obs-browser | module config dir is the CEF profile: cookies, `Local State`, cache, `obs_profile_cookies*` | **yes** (sessions) |
| rtmp-services, text-freetype2, win-capture | module config caches | no |

## 5. Secrets and private data (snapshot boundary)

Plaintext secrets: `service.json` `key` (stream key), `bearer_token` (WHIP),
`username`/`password` (custom RTMP); `basic.ini` `[Twitch]/[Restream]/[YouTube]`
`Token` and `RefreshToken`; obs-websocket `config.json` `ServerPassword`; the
obs-browser CEF profile. Identity: `basic.ini` Twitch `Name`/`UUID`, YouTube
`ChannelName`. Private: local paths everywhere (media, VST, scripts, recording
output), window titles, browser-source URLs (may carry tokens), opaque plugin
state, script settings.

The scene collection JSON contains no stream key, OAuth token or websocket
password, but it can contain anything a user typed into source properties.
Therefore: a snapshot must exclude the listed secrets structurally (by key and
file, not by heuristics) and treat all other opaque payloads as *unreviewed*.

## 6. Coherence

- All frontend writers run on the Qt UI thread; `obs_frontend_save()` only queues.
- `disableSaving` (counter) suppresses saves during load and clear; if clearing
  fails it stays raised for the rest of the run. Frontend events are dropped while
  it is raised, except cleanup and exit.
- A coherent capture on the UI thread is: `DeferSaveBegin` → `SaveProjectNow`
  (refuse and report "busy" if saving is disabled) → `config_save_safe` of the
  profile and user configs (this also flushes in-memory geometry/DockState) →
  copy the files → `DeferSaveEnd`. Snapshots must never run on audio threads.
- Profile and collection switches emit `*_CHANGING` before the old file is saved;
  a snapshot taken in that handler can be stale.

## 7. Device bindings and routing

- Global audio devices are full source objects in the collection (section 3).
  Source IDs per OS: `wasapi_*_capture` (Windows), `coreaudio_*_capture` (macOS),
  `pulse_*_capture` (Linux; ALSA/JACK/PipeWire sources can also appear).
- `device_id` formats: `"default"`, WASAPI endpoint IDs, CoreAudio UIDs, Pulse
  source/sink names, ALSA PCM names, V4L2 node paths (`/dev/video0`), DirectShow
  `name:path`, AVFoundation `uniqueID`.
- **OBS never falls back to the default device for a missing explicit ID**: WASAPI
  retries every 3 s, CoreAudio/Pulse fail, the properties UI shows the stale ID.
  The ID round-trips unchanged, which is what offline editing needs; remapping on
  another machine must be an explicit, recorded user choice.
- Monitoring device: `basic.ini [Audio] MonitoringDeviceId/Name`. Engine format:
  `[Audio] SampleRate` (44100/48000), `ChannelSetup` (Mono…7.1), restart required.
- Mixer routing: per-source `mixers` bitmask (tracks 1–6).

## Open questions for the S1 design

1. Container: a directory snapshot vs a single archive; content addressing.
2. Which roots/files are in a snapshot by default; module config per plugin
   (allowlist) vs everything.
3. Secret handling: drop, replace with a machine-local reference, or keep in an
   OS keystore reference.
4. Where the inert document model lives (C++ library without Qt/libobs, so it can
   be unit-tested and used by the offline editor) and what it parses vs keeps
   opaque.
5. How to identify a workspace and revision (UUID, parent, content hash).
