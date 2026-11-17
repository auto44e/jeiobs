# Workspace snapshot format v1 (task 002, S1)

Status: **proposed**, 2026-11-16, S1-PR1. Code read at main `270f9d6c9` (OBS 33.0.1 base). This
fixes the v1 manifest, local store, export projection, capture allowlist, secret registry, import
limits and capture-coherence contract. PR1 implements only the inert parts (section 13), in
`shared/jeiobs-workspace/`, tested on Linux; nothing here runs against a live OBS. Facts come from the
[persistence audit](s1-persistence-audit.md), re-read in the code; where they differ, the citation
here wins. Integrity hashes detect corruption; they are not signatures and prove nothing about trust.

Settled by the two-round Claude/Codex design debate (its round-2 positions are this design) and by
the lead, who resolved what round 2 left open. Correspondence:
[debates/002-workspace-format](debates/002-workspace-format/). Each section gives the decision and
the main rejected alternative; section 14 records the owner's decisions.

## 1. Terms

- **Roots.** `config` is the OS config dir, or `CONFIG_PATH` in portable mode (`GetAppConfigPath`,
  `frontend/OBSApp.cpp:1702-1717`). `user`, `scenes`, `profiles` and `plugin_manager` come from
  `global.ini [Locations]` (`Configuration`, `SceneCollections`, `Profiles`,
  `PluginManagerSettings`). Each falls back to the default when its path does not exist, and
  portable mode ignores them (`OBSApp.cpp:577-613`).
- **App dir.** `obs-studio`, hard-coded (`OBSApp.cpp:420-469,550`); `jeiobs` after M0. Entry paths
  are relative to `<root>/<app dir>/`, so one manifest maps onto either.
- **Entry**: one file, or one directory excluded as a whole. **Blob**: an entry's stored bytes.
  **Revision**: a manifest and the blobs it names. Citations give file names under `frontend/`,
  `libobs/` and `plugins/<module>/`.

## 2. Local store

```text
<OS config dir>/jeiobs/
  machine-id                      random UUIDv4, created once, mode 0600
  workspaces/<workspace_id>/
    blobs/<2 hex>/<62 hex>        immutable; the name is the BLAKE2b-256 of the content
    revisions/<seq>-<revid>.json  immutable manifests; their rename is the commit
    HEAD                          hint only: "<seq>-<revid>\n"
    quarantine/                   manifests and temp files that failed recovery
    lock                          exclusive OS file lock, held by every writer
```

- `<OS config dir>` is `os_get_config_path` (`platform-nix.c:228`, `platform-cocoa.m:87`,
  `platform-windows.c:435-437`). Capture never lists `workspaces/` or `machine-id`.
- `seq` is a local counter: decimal, zero-padded to 10 digits, one above the highest in
  `revisions/`. The manifest repeats it and the two must match. Clocks never order revisions.
- On POSIX, directories are 0700 and files 0600. Windows relies on the per-user AppData ACL, which
  S2 checks. The store writer is S2; S1 capture ends at an in-memory `CaptureResult`.

## 3. Hashes and revision IDs

- **Hash**: plain BLAKE2b-256 (32-byte digest; no key, salt or personalization), written as 64
  lowercase hex digits, so `b2sum -l 256 FILE` prints the same value. PR1 tests the known answers
  `""` → `0e5751c026e543b2e8ab2eb06099daa1d1e5df47778f7787faab45cdf12fe3a8` and
  `"abc"` → `bddd813c634239723171ef3fee98579b94964e3bb1cb3e427262c8c068d52319`.
- **Code**: `OBS::blake2` (`deps/blake2/CMakeLists.txt:3-4`), streaming API at
  `deps/blake2/src/blake2.h:151-155`. On Linux the target exists only with `ENABLE_WHATSNEW`, which
  jeiobs turns off (`frontend/cmake/feature-whatsnew.cmake:5,14-15`). The library therefore adds
  `deps/blake2` behind the same `if(NOT TARGET OBS::blake2)` guard
  (`frontend/cmake/os-windows.cmake:1-2`). No file under `deps/` changes.
- **No domain separation.** The field that references a hash fixes the object type:
  `entries[].hash` names a document blob, and a revision file name or `parent` names a manifest.
  Readers parse a blob only as that type. Rejected: type-prefixed hashing. It breaks `b2sum`
  checks and adds nothing once types are enforced.
- **Revision ID** = the hash of the stored manifest bytes. It is never inside those bytes: it is
  the file name `revisions/<seq>-<revid>.json` and the child's `parent`. Readers check the bytes
  against the name before parsing. **No canonical serialization in v1**: stored manifests never
  change and are only hashed as stored. Rejected: canonical JSON. It is a second byte-stable
  serializer to keep identical across versions and OSes, for a check nobody makes. If S6 signs
  change sets, it signs its own envelope bytes.

## 4. Manifest v1

One UTF-8 JSON object, no BOM. Writers choose key order and whitespace; nothing re-derives them.

| Field | Content |
| --- | --- |
| `format`, `major`, `minor` | `"jeiobs-workspace"`, `1`, `0`; `minor` is the writer's own |
| `compat` | `{"incompat": [], "ro_compat": []}`: feature names (section 5) |
| `workspace_id`, `seq`, `parent` | UUIDv4 fixed at creation; section 2; the parent's revision ID, or `null`. One parent; merges would need `incompat: ["merge"]` |
| `created_utc`, `label`, `creator` | RFC 3339 UTC; the user's name for the snapshot; `{app, version, commit, obs_base}` |
| `active` | profile and collection UIDs, from `user.ini [Basic] ProfileDir` and `SceneCollectionFile` (`OBSApp.cpp:1127-1130`) |
| `target` | `os`, `arch`, `machine_id`, `app_dir`, `roots` (`default` or `custom` per root, never a path), `audio`, `devices` |
| `app_keys`, `entries`, `secret_refs` | reviewed `global.ini` keys (section 7); files (section 6); references to removed secrets (section 8) |
| `dropped_ext` | `[{"entry", "fields"}]`: unknown fields a writer could not carry over (section 5) |

- Every entry has `id` (unique), `status`, `root` and `path`. It also has `profile` or
  `collection` (a UID, section 9) when the file belongs to one. Pointers are RFC 6901 JSON
  Pointers; in INI files they are `/<section>/<key>`, with the same escaping, except that a section
  name over 1,024 bytes is written `/~section-<n>/<key>` (`n` the 0-based section ordinal; `~s`
  cannot occur in an escaped name).
- `"status": "stored"` adds `role`, `size`, `hash` and `fidelity`. Optional fields: `redactions`
  (`[{pointer, kind, ref}]`), `unreviewed` (`{rules, count}`), `parse` (`"invalid"` when the
  jeiobs parser rejects the file), `bak` (the id of the `<path>.bak` entry) and `load_rule`
  (section 7).
- `"status": "omitted"` adds `reason` and `size`. A directory excluded as a whole also gets
  `kind: "dir"`, and its `size` counts its regular files without following symlinks; a collapsed
  folder also gets `count` (its number of files, integer >= 1). Names that cannot be represented
  (not UTF-8, or breaking a path rule) are one entry per folder with `kind: "names"`, `reason:
  "unrepresentable"` and a `count`, and no names. A folder that cannot be listed is an entry with
  `reason: "unreadable"`; a symlink, junction or reparse point is `reason: "symlink"` and is never
  followed; a file over the blob limit is `reason: "too-large"`. `count` is allowed only on omitted
  entries of kind `dir` or `names`. An omitted entry never has a hash. A `collection` entry may carry
  `scene_uuids` (sorted, unique strings): the identity data of section 9, local only, dropped by the
  export projection.
- `target.audio` = `{sample_rate, channels, explicit, src: {entry}}`. The values come from the
  active profile's `basic.ini [Audio] SampleRate` and `ChannelSetup`
  (`OBSBasicSettings.cpp:3646,3650`). `explicit` is false when OBS's defaults apply (48000,
  `Stereo`; `OBSBasic.cpp:758-759`). The validator checks the shape of `target.audio` (PR2); comparing it with the blob needs the INI scan and arrives with PR3.
- `target.devices` lists the devices present at capture and stays local. Bindings inside documents
  (`device_id` in source settings, `[Audio] MonitoringDeviceId`) are document content and travel
  unchanged.

### Example: local revision

Stored as `revisions/0000000007-3e8a1f5c0b7d2e94c6f1a0835d9e2b7c14a6f0d8e2c93b57a0f4d16e8b3c7259.json`.

```json
{
  "format": "jeiobs-workspace", "major": 1, "minor": 0, "compat": {"incompat": [], "ro_compat": []},
  "workspace_id": "0d6f3c1e-8b2a-4c47-9e15-6a0b7d93f2c4", "seq": 7,
  "parent": "5c0e7a913b2d4f68a1c9e2b7d4058c6e1f2a9b3c7d8e4f5a6b1c2d3e4f5a6b7c",
  "created_utc": "2026-11-16T18:04:05Z", "label": "Before master FX",
  "creator": {"app": "jeiobs", "version": "0.0.0-dev", "commit": "b76d7cdb1", "obs_base": "33.0.1"},
  "active": {"profile": "7b1e4c2a-90d3-4f6e-8a5b-2c1d0e9f8a7b", "collection": "c3d2e1f0-4a5b-4c6d-9e8f-0a1b2c3d4e5f"},
  "target": {"os": "windows", "arch": "x86_64", "machine_id": "3a9e2b70-51c4-4f0e-8d6a-1c2b3d4e5f60",
    "app_dir": "obs-studio",
    "roots": {"config": "default", "user": "default", "scenes": "custom", "profiles": "default", "plugin_manager": "default"},
    "audio": {"sample_rate": 48000, "channels": "Stereo", "explicit": true, "src": {"entry": "e4"}},
    "devices": [{"api": "wasapi", "io": "input", "name": "Chat Mic (RØDECaster Pro II Chat)",
                 "id": "{0.0.1.00000000}.{8f2b6c1d-3e4a-4b5c-9d6e-7f8091a2b3c4}"}]},
  "app_keys": [
    {"root": "config", "file": "global.ini", "section": "Video", "key": "Renderer", "value": "Direct3D 11", "restore": "same-os"},
    {"root": "config", "file": "global.ini", "section": "General", "key": "LastVersion", "value": "553648129", "restore": "never"}],
  "entries": [
    {"id": "e1", "status": "stored", "root": "user", "path": "user.ini", "role": "app.user", "fidelity": "exact",
     "size": 3120, "hash": "0b8c4d2a9e7f1c3566d2a0b4c8e19f732a5d7c0e91b3f48a6c2e0d17b5a39f84"},
    {"id": "e2", "status": "stored", "root": "scenes", "path": "basic/scenes/Main.json", "role": "collection",
     "collection": "c3d2e1f0-4a5b-4c6d-9e8f-0a1b2c3d4e5f", "fidelity": "opaque-unreviewed",
     "unreviewed": {"rules": "u1", "count": 3}, "bak": "e3", "load_rule": "bak-fallback",
     "size": 48211, "hash": "7e14a9c352b0d8f6e3a7c1d94f6b2e801d9c5a37b8e2f064a3c7d9152e6f0b48"},
    {"id": "e3", "status": "stored", "root": "scenes", "path": "basic/scenes/Main.json.bak", "role": "collection.bak",
     "collection": "c3d2e1f0-4a5b-4c6d-9e8f-0a1b2c3d4e5f", "fidelity": "opaque-unreviewed",
     "unreviewed": {"rules": "u1", "count": 3}, "size": 48190,
     "hash": "c41f8e07a2d95b630e7c3f91d8b46a25f17e0c3b9a5d2e846b0f1c7a3d8e9b52"},
    {"id": "e4", "status": "stored", "root": "profiles", "path": "basic/profiles/Main/basic.ini", "role": "profile.basic",
     "profile": "7b1e4c2a-90d3-4f6e-8a5b-2c1d0e9f8a7b", "fidelity": "redacted",
     "redactions": [{"pointer": "/Twitch/Token", "kind": "oauth_token", "ref": "s1"},
                    {"pointer": "/Twitch/RefreshToken", "kind": "oauth_token", "ref": "s2"}],
     "size": 2210, "hash": "2d9a6f13e8c04b75b1f73a9e05c6d82b4e9f1a60c3b7d528f0a14e9c76d2b3e1"},
    {"id": "e5", "status": "stored", "root": "profiles", "path": "basic/profiles/Main/service.json",
     "role": "profile.service", "profile": "7b1e4c2a-90d3-4f6e-8a5b-2c1d0e9f8a7b", "fidelity": "redacted",
     "redactions": [{"pointer": "/settings/key", "kind": "stream_key", "ref": "s3"}],
     "size": 118, "hash": "9b3e7d05f2a81c643c6d0e9fa7b42e18d05f9c3a81e6b72d4a0c9f5ee7d31b86"},
    {"id": "e6", "status": "stored", "root": "config", "path": "plugin_config/obs-websocket/config.json",
     "role": "module.obs-websocket", "fidelity": "redacted",
     "redactions": [{"pointer": "/server_password", "kind": "ws_password", "ref": "s4"},
                    {"pointer": "/auth_required", "kind": "forced", "was": false}],
     "size": 164, "hash": "e05b3a9d74c1f826a9d03e5b1f8c7e426b2a0d95c7e4f13a8d5b6e01f39a2c74"},
    {"id": "e7", "status": "omitted", "root": "config", "path": "plugin_config/obs-websocket/persistent_data.json",
     "size": 913, "reason": "unknown"},
    {"id": "e8", "status": "omitted", "root": "config", "path": "plugin_config/obs-browser", "kind": "dir",
     "size": 18874368, "reason": "secret-store"}],
  "secret_refs": [
    {"id": "s1", "entry": "e4", "pointer": "/Twitch/Token", "kind": "oauth_token", "bind": {"auth_type": "Twitch"},
     "machine_id": "3a9e2b70-51c4-4f0e-8d6a-1c2b3d4e5f60"},
    {"id": "s2", "entry": "e4", "pointer": "/Twitch/RefreshToken", "kind": "oauth_token", "bind": {"auth_type": "Twitch"},
     "machine_id": "3a9e2b70-51c4-4f0e-8d6a-1c2b3d4e5f60"},
    {"id": "s3", "entry": "e5", "pointer": "/settings/key", "kind": "stream_key",
     "bind": {"type": "rtmp_common", "service": "Twitch", "server": "auto"}, "machine_id": "3a9e2b70-51c4-4f0e-8d6a-1c2b3d4e5f60"},
    {"id": "s4", "entry": "e6", "pointer": "/server_password", "kind": "ws_password", "bind": {},
     "machine_id": "3a9e2b70-51c4-4f0e-8d6a-1c2b3d4e5f60"}],
  "dropped_ext": []
}
```

### Export projection

`ProjectForExport(manifest, decisions)` is a pure function in PR1.

- **Drops:** `workspace_id`, `seq`, `parent`, `secret_refs`, `redactions[].ref` and `.was`,
  `app_keys` with `"restore": "never"`, and all of `target` except `os` and the audio
  `sample_rate` and `channels`.
- **Adds:** `origin: {workspace_id, revision_id}` (the exported revision) and `exported_utc`.
- **Decisions:** every `opaque-unreviewed` entry needs one. `include`, given after user review,
  sets `fidelity` to `exact` or `redacted` and adds `review`. `omit` turns the entry into an
  omitted entry with `reason: "unreviewed"`. A missing decision is an error; there is never a
  partial result. Entries with `parse: "invalid"` are exported only as omitted.
- **Unknown input:** the function keeps only fields it knows and lists the rest in `dropped_ext`.
  It refuses an unknown major and any unknown `incompat` or `ro_compat` feature.

Audition overlays (S4) live outside revisions. If a later minor records them, the projection drops
them. Rejected: exporting the device inventory, because names, endpoint IDs and serials identify
hardware and people. Flex: S5 may need `arch` for plugin binary checks; adding it is a minor change.

```json
{
  "format": "jeiobs-workspace", "major": 1, "minor": 0, "compat": {"incompat": [], "ro_compat": []},
  "origin": {"workspace_id": "0d6f3c1e-8b2a-4c47-9e15-6a0b7d93f2c4",
             "revision_id": "3e8a1f5c0b7d2e94c6f1a0835d9e2b7c14a6f0d8e2c93b57a0f4d16e8b3c7259"},
  "created_utc": "2026-11-16T18:04:05Z", "exported_utc": "2026-11-16T19:10:42Z", "label": "Before master FX",
  "creator": {"app": "jeiobs", "version": "0.0.0-dev", "commit": "b76d7cdb1", "obs_base": "33.0.1"},
  "active": {"profile": "7b1e4c2a-90d3-4f6e-8a5b-2c1d0e9f8a7b", "collection": "c3d2e1f0-4a5b-4c6d-9e8f-0a1b2c3d4e5f"},
  "target": {"os": "windows", "audio": {"sample_rate": 48000, "channels": "Stereo"}},
  "app_keys": [
    {"root": "config", "file": "global.ini", "section": "Video", "key": "Renderer", "value": "Direct3D 11", "restore": "same-os"}],
  "entries": [
    {"id": "e1", "status": "stored", "root": "user", "path": "user.ini", "role": "app.user", "fidelity": "exact",
     "size": 3120, "hash": "0b8c4d2a9e7f1c3566d2a0b4c8e19f732a5d7c0e91b3f48a6c2e0d17b5a39f84"},
    {"id": "e2", "status": "stored", "root": "scenes", "path": "basic/scenes/Main.json", "role": "collection",
     "collection": "c3d2e1f0-4a5b-4c6d-9e8f-0a1b2c3d4e5f", "fidelity": "exact",
     "review": {"rules": "u1", "decision": "include"}, "bak": "e3", "load_rule": "bak-fallback",
     "size": 48211, "hash": "7e14a9c352b0d8f6e3a7c1d94f6b2e801d9c5a37b8e2f064a3c7d9152e6f0b48"},
    {"id": "e3", "status": "omitted", "root": "scenes", "path": "basic/scenes/Main.json.bak", "role": "collection.bak",
     "collection": "c3d2e1f0-4a5b-4c6d-9e8f-0a1b2c3d4e5f", "size": 48190, "reason": "unreviewed"},
    {"id": "e4", "status": "stored", "root": "profiles", "path": "basic/profiles/Main/basic.ini", "role": "profile.basic",
     "profile": "7b1e4c2a-90d3-4f6e-8a5b-2c1d0e9f8a7b", "fidelity": "redacted",
     "redactions": [{"pointer": "/Twitch/Token", "kind": "oauth_token"}, {"pointer": "/Twitch/RefreshToken", "kind": "oauth_token"}],
     "size": 2210, "hash": "2d9a6f13e8c04b75b1f73a9e05c6d82b4e9f1a60c3b7d528f0a14e9c76d2b3e1"},
    {"id": "e5", "status": "stored", "root": "profiles", "path": "basic/profiles/Main/service.json",
     "role": "profile.service", "profile": "7b1e4c2a-90d3-4f6e-8a5b-2c1d0e9f8a7b", "fidelity": "redacted",
     "redactions": [{"pointer": "/settings/key", "kind": "stream_key"}],
     "size": 118, "hash": "9b3e7d05f2a81c643c6d0e9fa7b42e18d05f9c3a81e6b72d4a0c9f5ee7d31b86"},
    {"id": "e6", "status": "stored", "root": "config", "path": "plugin_config/obs-websocket/config.json",
     "role": "module.obs-websocket", "fidelity": "redacted",
     "redactions": [{"pointer": "/server_password", "kind": "ws_password"}, {"pointer": "/auth_required", "kind": "forced"}],
     "size": 164, "hash": "e05b3a9d74c1f826a9d03e5b1f8c7e426b2a0d95c7e4f13a8d5b6e01f39a2c74"},
    {"id": "e7", "status": "omitted", "root": "config", "path": "plugin_config/obs-websocket/persistent_data.json",
     "size": 913, "reason": "unknown"},
    {"id": "e8", "status": "omitted", "root": "config", "path": "plugin_config/obs-browser", "kind": "dir",
     "size": 18874368, "reason": "secret-store"}],
  "dropped_ext": []
}
```

The export container is a ZIP. `manifest.json` (the projection) comes first, then
`blobs/<2>/<62>` for each stored entry, named as in the store. The ZIP reader and writer are S3
work with their own design record; S1 fixes the limits and zip-slip rules now (section 12).
Rejected: exporting a directory tree, which gives no single object to stage, scan and commit
atomically.

## 5. Compatibility

- Readers ignore unknown fields.
- `incompat`: a reader that does not know every listed feature refuses the manifest.
- `ro_compat`: a reader that does not know a listed feature may read, display and restore the
  revision. It must not write a child of the revision or export it.
- Unknown `major`: no writes of any kind (child, export, restore). The reader shows the revision
  read-only if it still validates under the v1 rules, and refuses it otherwise.
- A minor only adds optional fields or features. Readers accept every minor of their major. A new
  row in the section 7 allowlist (a new restorable file) is not optional for a reader that
  would restore the revision: it needs an `incompat` or `ro_compat` feature name.
- A writer creating a child copies the parent's unknown top-level fields as JSON values. It copies
  an entry's unknown fields only if the child entry has the same `root`, `path` and `hash` as the
  parent entry; otherwise it drops them and lists them in `dropped_ext`.
- Unknown fields inside other objects (`target`, `creator`, `compat`, `active`, and items of
  `secret_refs`, `app_keys` and `redactions`) are dropped, because the writer rebuilds those
  objects. A minor that adds a field there must also add an `ro_compat` feature. So must a minor
  whose new top-level field an older writer's edits could make wrong.
- Rejected: carrying extensions as a byte-exact raw tree (debate R2). The child's bytes are new and
  hashed on their own, and readers consume values, not formatting.

## 6. Fidelity, omitted files and the export gate

- `exact`: the stored bytes are the bytes read at capture.
- `redacted`: registry secret values (section 8) are replaced by deterministic byte-range edits.
  Every other byte is unchanged. No original secret byte, length or hash is kept. A redacted file
  is not byte-exact, and the entry says so.
- `opaque-unreviewed`: the file holds fields the review rules cannot classify (section 8).
  Registry secrets in it are still redacted, and the rest is exact. These bytes stay in local
  history only, in the 0700/0600 store (owner decision 2).
- **Unknown files are omitted by default.** This covers extra files in profile folders,
  `persistent_data.json` and third-party module folders. Each becomes
  `{status: "omitted", path, size, reason}` with no hash, because a hash of a small file lets
  anyone test guesses offline. A directory with more than 100 unknown files becomes one
  `kind: "dir"` entry.
- A file the jeiobs parser rejects is stored only if its role has no registry secrets, as
  `opaque-unreviewed` with `parse: "invalid"`. Otherwise it is omitted with
  `reason: "unparseable"`, because its secrets cannot be located; its `size` is rounded up to a
  multiple of 65,536 so that it does not reveal a secret's length.
- **Export fails closed** while any `opaque-unreviewed` entry has no decision. S3 stages the ZIP
  in a private (0700) directory on the destination volume. It re-runs the current registry and
  review rules on every staged blob, and searches every staged byte for the live value of each
  registry secret on this machine that is at least 8 bytes long (shorter values, such as a short
  RTMP username, would match ordinary text; they are covered by the structural redaction alone). The ZIP is renamed into place only when every check passes; any
  failure deletes the staging directory.
- Copying `blobs/` is not an export path. jeiobs never syncs, uploads or offers to copy the store.
- Rejected: capturing unknown files as unreviewed (debate R1). The store would hold plugin tokens
  and caches of unknown size before anyone reviewed them.

## 7. Allowlist

Paths are under `<root>/<app dir>/`. `<f>` is a collection file and `<d>` a profile folder.
Section 8 lists every secret key.

| File | Root | Role | Captured | Secret keys | Fidelity |
| --- | --- | --- | --- | --- | --- |
| `global.ini` | config | — | reviewed keys only, as `app_keys` (below) | legacy `[OBSWebSocket] ServerPassword` | — |
| `user.ini` | user | `app.user` | whole file | dock URLs in `ExtraBrowserDocks` (flagged, not redacted) | `exact`; `opaque-unreviewed` with a non-empty `[BasicWindow] ExtraBrowserDocks` (PR2). Unknown sections follow in PR3 with the reviewed section list |
| `basic/scenes/<f>.json`, `<f>.json.bak` | scenes | `collection`, `collection.bak` | whole file | browser-source, media-source and VLC playlist URL credentials | `opaque-unreviewed` until every field is reviewed |
| `basic/profiles/<d>/basic.ini` | profiles | `profile.basic` | whole file | OAuth `Token`, `RefreshToken`; `[AdvOut] FFURL` credentials | `exact` or `redacted`; `opaque-unreviewed` with unknown sections or a set `FFURL` |
| `basic/profiles/<d>/service.json`, `.bak` | profiles | `profile.service`, `profile.service.bak` | whole file | `key`, `bearer_token`, `username`, `password`; `server` credentials | `exact` or `redacted`; `opaque-unreviewed` for `rtmp_custom` and `whip_custom` (typed URL) and third-party types |
| `basic/profiles/<d>/streamEncoder.json`, `recordEncoder.json`, `.bak` | profiles | `profile.encoder.stream`, `profile.encoder.record`, `.bak` | whole file | — | `exact` |
| `plugin_manager/modules.json` | plugin_manager | `app.modules` | whole file | — | `exact` |
| `plugin_config/obs-websocket/config.json` | config | `module.obs-websocket` | whole file | `server_password` | `redacted` |
| `plugin_config/decklink-output-ui/decklink{,Preview}OutputProps.json` | config | `module.decklink-output-ui` | whole file | — | `exact` (the redactor has no role for it until PR3; `jw-check redact` answers `E_SCHEMA role`) |
| `plugin_config/aja-output-ui/aja{Output,PreviewOutput,Misc}Props.json` | config | `module.aja-output-ui` | whole file | — | `exact` |

| Omitted, `reason` | Files |
| --- | --- |
| `unknown` | any other file in a profile folder, in `basic/scenes/` or in a listed module folder, including `obs-websocket/persistent_data.json` (client data written from obs-websocket worker threads, `RequestHandler_Config.cpp:109`); every other module folder, as one `kind: "dir"` entry |
| `out-of-process-writer` | `obs-vst3/vst3list.json` and its `.bak`, written by the separate `obs-vst3-scanner` process (`vst3-scanner.cpp:646`), which no in-process lock can serialize; S5 owns the plugin inventory |
| `secret-store` | `obs-browser/`, the CEF profile with cookies and sessions |
| `cache`, `runtime` | the `rtmp-services`, `text-freetype2` and `win-capture` module folders; `logs/`, `crashes/`, `profiler_data/`, `updates/`, `.sentinel/`. `themes/` is referenced, not captured |
| `not-loaded` | `*.json.v1`, `*.json.invalid`, and the decklink and AJA `.bak` files, which OBS never reads (`decklink-ui-main.cpp:44-57`, `aja-ui-main.cpp:44-57`) |
| `transient`, `symlink` | `*.tmp`; any symlink below a resolved root, which capture never follows |

**`.bak` pairs.** A captured JSON file and its `.bak` are two entries. The main entry names the
`.bak` in `bak` and records `load_rule`. jeiobs does not predict which of the two OBS loads.
- `bak-fallback` (collections, `service.json`): `obs_data_create_from_json_file_safe` parses
  `<f>`. If that fails (missing, unreadable, invalid JSON or a duplicate key) and `<f>.bak` exists,
  it renames `<f>.bak` over `<f>` and parses it (`libobs/obs-data.c:641-668`). OBS runs this on
  every collection when it lists them at startup (`OBSBasic_SceneCollections.cpp:449-450`) and
  again on load (`:1135`). If the file still fails, it is renamed to `.json.invalid` and OBS
  creates a default scene (`:1137-1157`). `service.json` loads the same way
  (`OBSBasic_Service.cpp:55`).
- `mixed` (encoder JSON): the settings dialog falls back to the `.bak`
  (`OBSBasicSettings.cpp:1939`); output setup does not (`AdvancedOutput.cpp:11-31`).

S2 restore writes both files as stored, or removes a live `.bak` when the revision stores none.
OBS's own rule then picks the file it would have picked at capture. Rejected: storing only the file
jeiobs expects OBS to load (debate R2); a wrong guess restores stale state.

**Reviewed `global.ini` keys.** Each `app_keys` item records `root`, `file`, `section`, `key`, the
raw value and `restore`. Absent keys are not recorded; OBS's default applies.

| Keys | Written at | `restore` |
| --- | --- | --- |
| `[Video] Renderer`, `DisableOSXVSync`, `ResetOSXVSyncOnExit`; `[General] ProcessPriority`, `BrowserHWAccel`; `[Audio] DisableAudioDucking` | `OBSBasicSettings.cpp:3269,3297,3301,3276,3286,3318` | `same-os`: S2 offers them only when restoring on `target.os` |
| `[PropertiesWindow]` and `[InteractionWindow]` `cx`, `cy` | `OBSBasicProperties.cpp:417-418`, `OBSBasicInteraction.cpp:163-164` | `same-os` |
| `[General] LastVersion`, `Pre31Migrated` | `OBSBasic.cpp:1758`, `OBSApp.cpp:569` | `never`: migration state of the build that wrote them (`OBSApp.cpp:561-572`) |
| `[Locations]` | — | `never`; only `default` or `custom` per root goes into `target.roots` |

Not recorded: every other key. This includes `[General] InstallGUID` (a machine identifier,
`WhatsNewInfoThread.cpp:154`), the updater keys and the legacy `[OBSWebSocket]` section, which
obs-websocket moves into its `config.json` (`Config.cpp:155-193`). Rejected: capturing
`global.ini` whole as machine data (debate R1). It holds paths, migration state and possibly a
plaintext websocket password, and nothing restores it as a file.

## 8. Secrets

| File | Key or pointer | Kind | Placeholder |
| --- | --- | --- | --- |
| `service.json`, `.bak` | `/settings/key` (`rtmp-common.c:154`, `rtmp-custom.c:26`) | `stream_key` | `""` |
| `service.json`, `.bak` | `/settings/bearer_token` (`whip-service.cpp:14`) | `bearer_token` | `""` |
| `service.json`, `.bak` | `/settings/username`, `/settings/password` (`rtmp-custom.c:28-29`) | `rtmp_auth` | `""` |
| `basic.ini` | `Token`, `RefreshToken` in `[Twitch]`, `[Restream]`, `[YouTube]` (`OAuth.cpp:52-53`, `YoutubeAuth.cpp:84-85`; sections: `TwitchAuth.cpp:31`, `RestreamAuth.cpp:29`, `YoutubeAuth.cpp:23`) | `oauth_token` | `""` |
| `obs-websocket/config.json` | `/server_password` (`Config.cpp:43`) | `ws_password` | 32 random characters (below) |
| `user.ini` `[BasicWindow] ExtraBrowserDocks` (dock URLs, `OBSBasic_Browser.cpp:73-90`: not redacted, a non-empty value makes the entry `opaque-unreviewed`); `service.json` `/settings/server`; `basic.ini` `/AdvOut/FFURL` (`OBSBasicSettings.cpp:3557`); collection `browser_source` `/settings/url`; `ffmpeg_source` `/settings/input` always (OBS opens it for any `is_local_file` that is not a true boolean, `obs-ffmpeg-source.c:182,396-422`; a local path has no `scheme://`) and `/settings/ffmpeg_options` (`:211`, the URL rule runs over the whole string); `vlc_source` `/settings/playlist/*/value` (`vlc-video-source.c:14,637`) | URL userinfo, and the values of query keys `key`, `token`, `access_token`, `auth`, `password`, `passphrase`, `secret`, `streamid` (any case) | `url_credential` | `""` for that byte range |
| `global.ini` | `[OBSWebSocket] ServerPassword` (`Config.cpp:29,35`) | `ws_password` | never read; `global.ini` is not captured |
| `plugin_config/obs-browser/` | the CEF profile | session | never captured |

- Every occurrence of a registry key is redacted, duplicates included. An edit covers only the
  value: the bytes between the quotes of a JSON string, or from after the first `=` to the end of
  an INI line.
- **URL rule.** For every `scheme://` in the value: the userinfo (the bytes up to the last `@` of the
  authority, the `@` stays) and, after the FIRST `?` anywhere in the string (a `#` does not end the
  query, because OBS's SRT code reads `streamid` and `passphrase` up to `&` or the end), the value
  of each listed parameter up to the next `&`. Overlapping ranges are merged.
- **INI files follow libobs's parser** (`config-file.c`, `lexer.c`: CR, LF, CRLF and LFCR are line
  breaks; a header runs to the first `]` or line break, so `[Twitch` and `[Twitch]]` both name
  section `Twitch`, and the rest of the line after a `]` is read as keys). Where the scanner is not certain (a CR not followed by LF, a `[` line that is not
  `[name]` plus trailing blanks, a section name over 1,024 bytes) it redacts `Token` and
  `RefreshToken` in EVERY section and position, and the file becomes `opaque-unreviewed`
  (count +1). A redaction report that would exceed 4 MiB (64 bytes per item plus its pointer, kind
  and placeholder) makes the file `omitted` with reason `unparseable`.
- **Placeholders fail closed.** An empty key, token or password makes the output or the login fail
  and opens nothing.
- **obs-websocket is the exception.** Its auth secret is derived from whatever password is stored
  (`WebSocketServer.cpp:85-86`), and `auth_required` is a separate flag (`:238`). An empty password
  would therefore leave a remote-control server open.
  - The placeholder is 32 random `[A-Za-z0-9]` characters, new for each capture. It is the only
    random input to redaction.
  - An explicit `"auth_required": false` becomes `true`, recorded as a `forced` redaction with
    `was: false`. An absent key already means `true` (`Config.h:42`).
  - Restore (S2) and import (S3) never write `""` or the stored placeholder. They re-attach the
    live password or generate a new one, and keep authentication on. An exported placeholder is a
    known value in a public file; it is therefore never used as a live password.
  - Rejected: blanking the password (debate R1).
- Identity values are not secrets and stay: `[Twitch] Name` and `UUID` (`TwitchAuth.cpp:164-165`),
  `[YouTube] ChannelName` (`YoutubeAuth.cpp:299`). The export review lists them, with local paths.

**Secret references** are `{id, entry, pointer, kind, bind, machine_id}`. `bind` holds, as stored,
the non-secret values the secret belongs to: `type`, `service` and `server` for a stream key;
`[Auth] Type` for OAuth (`Auth.cpp:84`); the source `uuid` for a browser URL. S2 re-attaches a
secret only when all of these hold:
- the `machine_id` is the same;
- the live file at the same root and path has the same `bind` values;
- the live value is non-empty.

Otherwise the placeholder stays unresolved and the restore report lists it. v1 uses no OS
keystore. Export drops every reference. Rejected: a fingerprint derived from the value, which would
let anyone with the manifest test guesses offline.

**Review rules `u1`.** Any field these rules cannot classify makes its entry `opaque-unreviewed`,
with `unreviewed: {rules: "u1", count}`. The rules cover:
- collections: `settings` and `private_settings` of source types outside the reviewed first-party
  list (PR3); `browser_source` `url` and `css`; VST2 `chunk_data`; VST3 `vst3_state` and
  `vst3_ctrl_state`; `scripts-tool` settings; `auto-scene-switcher` window titles; unknown
  top-level keys;
- INI files: sections or keys outside the reviewed list for that file. Third-party plugins write
  there through `obs_frontend_get_user_config()` and `obs_frontend_get_profile_config()`
  (`obs-frontend-api.h:210-213`). `[BasicWindow] ExtraBrowserDocks` holds dock URLs
  (`OBSBasic_Browser.cpp:73-90`);
- URLs and option strings the user typed (custom `server`, `FFURL`, browser `url`, media source
  `input` and `ffmpeg_options`, VLC playlist `value`), even after the URL rule, because a token
  inside a URL path or an option string cannot be found by structure;
- `service.json` files with any other `type`.

Entries name their rules by ID. Export re-runs the current rules, so they can get stricter without
rewriting history.

## 9. Identity

- Each collection and profile gets a generated UID (UUIDv4), stored in the revision. Capture
  matches it against the parent revision in this order:
  1. same `root` and `path`;
  2. collections only: the same non-empty set of scene UUIDs (the `uuid` of top-level `sources`
     items whose `id` is `scene`);
  3. otherwise, a new UID.

  Only a unique match counts: exactly one parent collection, claimed by no other current
  collection. Profiles have no UUIDs, so a renamed profile gets a new UID (OBS copies the folder and
  deletes the old one, audit section 2).
- Objects inside a collection are keyed as follows, all scoped by the collection UID because a
  duplicated collection keeps its filter UUIDs (audit section 3):
  - sources, scenes, groups, filters and canvases by `uuid` (`obs.c:2483`);
  - scene items by scene UUID and `id` (`obs-scene.c:1338`);
  - transitions by name, because they get new UUIDs on every load. A transition rename is a delete
    plus an add.
- Rejected: a sidecar updated on rename events (debate R1). A missed event or a crash silently
  assigns the wrong identity. Matching can only cost identity, never data.

## 10. Capture coherence (S1-PR4)

Correctness comes from serializing every in-scope writer through one documented coordinator. Files
are read from handles opened while writers are serialized, and capture refuses if a writer outside
the protocol may run. Re-stat checks and generation counters are diagnostics only. Rejected as
correctness gates: read-twice, re-stat and a `persistGen` counter (debate R1, R2). An overwrite can
keep size and mtime, and a counter covers only the writers that bump it.

The capture order in the audit (its section 6) never saves anything. `DeferSaveBegin()` raises
`disableSaving` (`OBSBasic_SceneCollections.cpp:992-995`), and `SaveProjectNow()` returns while it
is raised (`:1518-1522`). Geometry and `DockState` reach the config only in `saveAll()` at exit
(`OBSBasic.cpp:1740-1749`). `Save()` only logs a failure (`OBSBasic_SceneCollections.cpp:985-989`).

1. Run capture as a posted UI-thread task, never inside a frontend-event handler: switches emit
   `*_CHANGING` before they save. Refuse when saving is disabled or the root gate fails.
2. `SaveProjectNow()` returns `Saved`, `Refused` or `Failed` (a fork-local change). Only `Saved`
   continues.
3. Take the locks in this order: a new frontend `persistMutex`, which `SaveService()` also takes;
   then `config_lock` on the user, profile and app configs. `config_save` and `config_save_safe`
   hold the recursive `config->mutex` (`struct config_data` in `config-file.c`) across the write and
   the replace (functions `config_save` and `config_save_safe`; line numbers drift, so they are not
   cited). Holding it therefore blocks every `config_t` writer on every
   thread, including obs-websocket and plugins.
4. Stage the window state (code extracted from `saveAll`), then `config_save_safe` the user and
   profile configs.
5. Open and read every captured file. On Windows, open with `FILE_SHARE_READ`, `FILE_SHARE_WRITE`
   and `FILE_SHARE_DELETE`. `os_safe_replace` calls `ReplaceFileW` (`platform-windows.c:767`), which
   fails while a reader lacks delete sharing, and OBS only logs that failure
   (`OBSBasic_SceneCollections.cpp:987-988`).
6. Release the locks in reverse order. Redaction, hashing and the manifest are built on a worker.

PR4 must show that each writer takes at most one of these locks. Getters also take `config->mutex`
(`config_get_string` and the other `config_get_*` functions), so PR4 must also show that no audio or video thread reads these configs,
and it reports the hold time.

**Root gate.** Steps 2 and 4 write live config. While the fork shares `obs-studio/` with OBS
(`OBSApp.cpp:420-469,550`; [COEXISTENCE](COEXISTENCE.md)), capture refuses unless one of these
holds:
- the app dir belongs to jeiobs (M0);
- the root is a test root: one the test harness created under the temp directory and marked with
  `.jeiobs-test-root`. Release builds ignore that marker.

Writer inventory (audit section 2, re-checked):

| File | Writers (thread) | Primitive | Serialized by |
| --- | --- | --- | --- |
| `user.ini` | settings, docks, exit (UI; `OBSBasicSettings.cpp:3789`, `OBSBasic.cpp:1769`); plugins through `obs_frontend_get_user_config()` (any thread) | `config_save_safe` | config lock |
| `basic.ini` | settings (UI, `OBSBasicSettings.cpp:3788`); OAuth (UI, `Auth.cpp:79,86`); hotkey rename (UI, `OBSBasic_Hotkeys.cpp:122`); obs-websocket `SetProfileParameter`, `SetVideoSettings`, `SetRecordDirectory` (worker pool, `WebSocketServer.cpp:352`; `RequestHandler_Config.cpp:417,513,655`); plugins through `obs_frontend_get_profile_config()` | `config_save` (truncates in place, `config-file.c:347`), `config_save_safe` | config lock |
| `service.json` | `SaveService()` (`OBSBasic_Service.cpp:24-43`), called from the UI (`OBSBasicSettings_Stream.cpp:333`) and from obs-websocket `SetStreamServiceSettings` on a worker (`RequestHandler_Config.cpp:603` → `OBSStudioAPI.cpp:534-536`) | tmp + `.bak` | `persistMutex` |
| encoder JSON, collections, `modules.json`, websocket `config.json`, decklink and AJA props | UI thread only: `WriteJsonData` (`OBSBasicSettings.cpp:3384-3398`), `OBSBasic::Save` (`OBSBasic_SceneCollections.cpp:806-990`), `saveModules_` (`PluginManager.cpp:190-210`), `Config::Save` (`SettingsDialog.cpp:210`; at startup, `Config.cpp:87,92`), output dialogs (`DecklinkOutputUI.cpp:59,95`, `AJAOutputUI.cpp:82`) | tmp + `.bak`; plain `ofstream` for `modules.json` and the websocket `config.json` (`Json.cpp:214`) | UI turn |
| `global.ini` | exit and migrations (UI; `OBSBasic.cpp:1759`, `OBSApp.cpp:570`); websocket migration (`Config.cpp:189`) | `config_save_safe`, `config_save` | config lock (keys only) |
| `persistent_data.json`, `vst3list.json` | obs-websocket worker (`RequestHandler_Config.cpp:109`); scanner process (`vst3-scanner.cpp:646`) | plain; tmp + `.bak` | not captured |

A writer found later that bypasses `config_t` and `SaveService()` either joins the protocol or
makes capture refuse while it can run. `config_lock` (owner decision 1) exists in libobs since S1-PR4a; without it capture would have to refuse
while obs-websocket's server is enabled.

## 11. Durability and recovery (S2 implements; fixed now)

- **POSIX:** write `<name>.tmp` (`O_CREAT|O_EXCL`, 0600) in the target directory, `fsync` it,
  `rename` it, then `fsync` the directory. macOS uses `fcntl(F_FULLFSYNC)` at the commit (on the
  manifest and on `revisions/`), because its `fsync` does not flush the drive cache.
- **Windows:** `CreateFileW(CREATE_NEW)`, write, `FlushFileBuffers`, close, then `MoveFileExW`
  with `MOVEFILE_REPLACE_EXISTING` and `MOVEFILE_WRITE_THROUGH`. The call returns only after the
  move is on disk.
- **Order:** blobs first (an existing blob is re-hashed and skipped), then the manifest, then
  `HEAD` (temp file and rename). The commit point is the manifest rename. A stale or missing
  `HEAD` is not an error.
- **Not `os_safe_replace`:** with a backup it leaves a moment with no target on POSIX
  (`platform-nix.c:576-581`), and OBS's write path never syncs (`platform.c:261-284`).
- **Recovery** runs under `lock` and starts at the highest `seq`. That manifest is current only if
  all of these hold:
  - its bytes hash to its file name;
  - it validates and contains the same `seq`;
  - every stored blob exists and hashes correctly;
  - every ancestor manifest exists, hashes correctly and has a lower `seq`. Ancestor blobs are not
    re-hashed.

  Otherwise the manifest moves to `quarantine/` with a reason file, and recovery tries the next
  one. Stray `*.tmp` files also go to `quarantine/`. So do both manifests when two share a `seq`,
  which only copying files in by hand can cause.
- No garbage collection before S2. S2 must define how pruning interacts with the ancestor rule
  before it deletes anything.
- S2 tests inject a failure at every step through an `Fs` interface. CI cannot prove durability
  across power loss: that needs real power cuts on real disks, and the results will say so.
- Rejected: trusting `HEAD` or the newest timestamp (debate R1). Clocks go backwards, and an orphan
  manifest can be newer but unrelated.

## 12. Limits, path rules and ZIP rules

Limits apply to everything jeiobs reads from a store or an import, and are checked before the
allocation or inflation they bound. A minor may raise a limit; an older reader then refuses the
larger input, which is the safe failure.

| Limit | v1 | Basis |
| --- | --- | --- |
| Manifest | 4 MiB | a large real setup (50 collections, 20 profiles) is about 150 entries, ~100 KiB |
| Entries | 10,000 per manifest | omitted entries count; large unknown directories collapse into one entry (section 6) |
| One blob | 64 MiB | the biggest files are collections with hex VST state; OBS reads whole files into memory (`os_quick_read_utf8_file`, `obs-data.c:630`) |
| One revision | 256 MiB of blobs | four maximal files; configuration only (S3 sets asset limits) |
| JSON depth | 64 | OBS documents nest about 10 levels |
| JSON string | 16 MiB in documents, 64 KiB in manifests | 8 MiB of plugin state as hex |
| JSON values | 1,000,000 per document, 250,000 per manifest | bounds parser memory |
| INI line | 1 MiB | the longest real values are base64 `DockState` and `ExtraBrowserDocks` |
| INI keys, sections | 1,000,000 keys, 100,000 sections | bounds scanner memory; OBS writes a few hundred keys |
| Path | 512 bytes, 8 components, 255 bytes per component | OBS builds its paths in 512-byte buffers (`OBSApp.cpp:418,548`); its layout is at most 4 components deep |
| Parent chain | 10,000 revisions | recovery walks the chain; nothing is pruned before S2 |

**Stored entries are bound to the allowlist.** A `stored` entry's `root`, `role` and `path` must
match a row of the section 7 table (`<f>` and `<d>` are one path component); anything else is
invalid, so a manifest cannot name the store (`workspaces/`, `machine-id`, which after M0 sit
under the same app dir) or any other file as a restore target. Restore (S2) and import (S3) write
only such entries, resolve each destination under its root, and check collisions there with
`O_EXCL`/`CREATE_NEW` in staging, because the destination filesystem decides what collides.
Omitted entries are never written.

**Parser** (manifests in PR1, documents in PR2). It rejects the following while parsing:
- duplicate keys (OBS does too: `JSON_REJECT_DUPLICATES` at `obs-data.c:611`);
- invalid UTF-8;
- NUL, raw or as `\u0000`;
- non-finite numbers such as `1e400`;
- a BOM in a manifest;
- anything past a limit.

**`PathRules`** (PR1) apply to every manifest path:
- relative, `/`-separated UTF-8;
- no empty, `.` or `..` component and no leading `/`;
- no `\` or `:` anywhere (drive letters, UNC paths, NTFS streams);
- no control characters (U+0000–U+001F, U+007F) and none of `< > " | ? *`;
- no component that ends in a dot or a space;
- no component whose part before the first dot, with trailing spaces removed, is a Windows device
  name, in any case (so `CON .txt` and `nul  .json` are rejected): `CON`,
  `PRN`, `AUX`, `NUL`, `COM0`–`COM9`, `LPT0`–`LPT9`, their superscript-digit forms, `CONIN$`,
  `CONOUT$`;
- no two paths in one manifest equal under Unicode canonical caseless matching,
  `NFD(toCasefold(NFD(p)))` (Unicode D145; owner decision 3);
- no symlinks: capture never follows one, and S2 and S3 never create one.

Stored paths keep the bytes the filesystem returned. S2 and S3 also create files with
`O_EXCL`/`CREATE_NEW` in staging on the destination filesystem, which catches collisions specific
to that filesystem. Rejected: ASCII-only case folding (debate R1), because APFS and NTFS also fold
non-ASCII letters.

**ZIP rules** (S3 implements them):
- `manifest.json` comes first, followed only by the `blobs/<2>/<62>` names of the manifest's
  stored entries, each once. Anything else, or a missing blob, rejects the archive.
- Regular files only: no symlink, directory, device or hard-link attributes.
- Stored or deflate only; no encryption, ZIP64 or multi-disk archives.
- Local headers must match the central directory, and there are no bytes outside the listed
  entries.
- Declared sizes must be within the limits. Inflation stops at the declared size, a size or CRC
  mismatch rejects the archive, and the compression ratio is at most 100:1 per entry.
- **Zip-slip:** no filesystem path is ever derived from a ZIP name. Each blob is written to private
  staging under its verified hash, and files reach a root only through manifest paths that passed
  `PathRules`.
- A blob is hashed before it is parsed. Nothing an import references is fetched, loaded or executed.

## 13. Library and PR slices

`shared/jeiobs-workspace/` is C++17, with no Qt and no libobs runtime, and builds on its own
(`cmake -S shared/jeiobs-workspace`). It uses only dependencies already in the tree:
- `OBS::blake2` (section 3);
- nlohmann_json 3.11, which the frontend already requires
  (`frontend/cmake/feature-plugin-manager.cmake:1`). Its SAX interface lets the manifest parser
  enforce section 12 during the parse.

Tests run under ctest with a small in-tree `jw_test.hpp`. A test framework would be a new
third-party dependency, which is owner-merged ([DECISIONS](DECISIONS.md) gate item 1). The
inherited cmocka setup was never built until S1-PR4a added `test/cmocka` behind `ENABLE_UNIT_TESTS`;
no CI job runs it yet.

| Slice | Contents | Merge |
| --- | --- | --- |
| PR1 | this document; `Limits`, `PathRules`, `Hash`, `Manifest` (parse, validate, child-writer extension rules, `ProjectForExport`). Synthetic fixtures: valid local and export manifests, unknown fields at each level, future minor, future major (read-only), unknown `incompat` and `ro_compat`, duplicate key, invalid UTF-8, NUL, `1e400`, depth 65, oversize input, every path rule, an undecided `opaque-unreviewed` entry | self-merge gate; persistence format, so review rounds repeat until one finds no blocker or major |
| CI | `.github/workflows/jeiobs-workspace.yaml`: build and ctest on ubuntu, macos, windows | owner (`.github/`) |
| PR2 | bounded JSON scanner with byte positions; INI reader that keeps BOM (written on Windows, `config-file.c:378-381`), CRLF and duplicates; redactor with per-kind placeholders. Fixtures: rtmp_common, custom RTMP with auth, WHIP, Twitch/YouTube/Restream tokens, websocket placeholder with forced auth, URL rule; a scan proving no secret bytes remain; a fuzz target | self-merge gate |
| PR3 | `Fs` (`MemFs`, `RealFs`), `CapturePlanner` (allowlist, omit-by-default, `.bak` pairs, symlinks, unrepresentable names, collisions, limits), identity against a parent revision, `ReviewRules` (table starts with `scene` and `group`), `jw-check plan`; synthetic root trees with custom locations. Not yet: capture of file contents, real Windows/macOS trees (CI runs the synthetic ones) | self-merge gate |
| PR4 | frontend capture host: save result, `StageWindowState`, `persistMutex`, root gate, Windows sharing, coordinator. Tests: an off-thread `config_save` blocks instead of tearing a read; an out-of-protocol writer makes capture refuse; `ReplaceFileW` succeeds during a capture read; a failed save yields no result | self-merge gate with decision 1 already made; it changes libobs public API, so review rounds repeat until one finds no Blocker or Major |
| S2 | store writer, recovery, history, restore, and the purge (delete a revision or a workspace; remove unreferenced blobs) that decision 2 depends on | — |
| Unicode tables | owner-merged: pin the Unicode Character Database, a committed generator script and the generated normalization and case-fold tables (decision 3); needed before S2 or S3 may write from a manifest with non-ASCII paths | owner (third-party data) |

## 14. Owner decisions

Decided 2026-11-16. The owner's words, verbatim: "will go with all you recs for all 3".

- **Decision 1: add `config_lock()` / `config_unlock()` to libobs util.** Approved. Added in
  S1-PR4a (`libobs/util/config-file.{c,h}`, a recursive lock over the config's own mutex, with a
  cmocka test that an off-thread writer blocks and that the file is not rewritten while the lock is
  held). It is a new public libobs API, so PR4 (the capture host) must still show the lock-order
  proof and hold times described in section 10.
  - Why: it is the only mechanism found that serializes obs-websocket's pool writes and
    third-party `config_t` writers without changing them.
  - Flex: reopen if the lock-order proof fails, if hold times starve the audio or video threads,
    or if upstream OBS adds its own config lock; the fallback is the "if declined" behaviour below.
- **Decision 2: keep `opaque-unreviewed` data in local history**, under the stated cost (no
  encryption; kept after the user removes it from OBS until S2 adds a purge) and the fail-closed
  export gate of section 6. S2 therefore includes the purge (section 13).
  - Why: restoring the user's own setup must not lose browser, script or VST state.
  - Cost, as accepted: history keeps the data until the S2 purge removes it. S2's scope includes
    that purge (delete a revision, delete a workspace, and remove blobs no revision references).
  - Flex: reopen if a purge cannot be built, or if a leak of local history is shown to be
    realistic for this store; the alternative is stripping at capture (below).
- **Decision 3: generate the Unicode tables from a pinned Unicode Character Database with a
  committed script** (the first option below), not a vendored `utf8proc`. This item had no
  recommendation written in this section; that pick is the lead's, taken under the owner's "all you
  recs". It adds third-party data (Unicode License v3), so the PR that adds the pin and the
  generated tables is owner-merged. Until it lands, PR1's ASCII-only fold and the "unchecked"
  rule stay.
  - Why generated tables over `utf8proc`: no vendored code and no new runtime dependency, the
    data is pinned and reproducible from one script, and only the two functions needed (NFD and
    case fold) are built.
  - If `utf8proc` had been chosen: a maintained library and fewer bugs in our own folding, at the
    cost of vendored code under `deps/` and its update burden.
  - Flex: reopen if the generator proves hard to keep correct across Unicode releases; then
    vendor `utf8proc`. The owner may also pick it now.

The options and costs as they were put to the owner. The "Decided:" labels were added after the owner's answer:

1. **`config_lock()` and `config_unlock()` in libobs util (needed by PR4). Decided: add it.** This is a new public
   libobs API around the existing recursive `config->mutex`. It is the only mechanism found that
   serializes obs-websocket's pool writes (`RequestHandler_Config.cpp:417,513,655`) and third-party
   `config_t` writers without changing them.
   - Recommendation: add it. It is additive, like the exports in task 003's D3.
   - Cost: a public API with a lock-order obligation, and getters block while capture holds the
     lock.
   - If declined: capture refuses while obs-websocket's server is enabled. Third-party `config_t`
     writers on other threads stay an undetected gap that the capture UI must state. Patching
     obs-websocket instead moves a submodule pointer (also owner-only) and covers only websocket.
2. **Keeping `opaque-unreviewed` data in local history. Decided: keep it.**
   - Recommendation: keep it, so that restoring the user's own setup is lossless (WORKSPACE_SAVING:
     "Preserve unknown fields and opaque state").
   - Cost: history keeps whatever users or plugins put into source, script or plugin state,
     including URL-path tokens the URL rule cannot find. It keeps them after the user removes them
     from OBS, until S2 adds a purge. The only protection is the 0700/0600 store; there is no
     encryption.
   - If declined: capture strips unreviewed fields, and restoring one's own collection loses
     browser, script and VST state.
3. **Unicode data for the path collision key. Decided: generated tables.** Canonical caseless matching needs normalization and
   case-folding tables, and nothing usable is in the tree (no ICU or utf8proc; the library excludes
   Qt).
   - Options: tables generated by a committed script from a pinned Unicode Character Database
     (Unicode License v3), or utf8proc (MIT) vendored under `deps/`. Either one adds third-party
     material, so it is owner-merged.
   - Until then: PR1 folds ASCII only and reports non-ASCII paths as unchecked. S2 and S3 must not
     write files from a manifest with unchecked paths.
   - Such paths do occur: `GetFileSafeName` keeps every character `iswalnum` accepts
     (`OBSApp.cpp:1748-1774`).

## 15. Not done

- Built so far in `shared/jeiobs-workspace/`: limits, path rules, hashing, manifests and export
  projection (PR1), the JSON and INI scanners and the redactor (PR2), the capture planner and
  identity (PR3). Sections 2, 6 (capture), 8's live capture hooks, 10 and 11 (store, coherence,
  durability) and the ZIP rules are design only.
- The library builds and passes its tests in CI on Ubuntu, macOS and Windows
  (`.github/workflows/jeiobs-workspace.yaml`). Apple's libc++ differs from libstdc++ (no
  floating-point `std::from_chars` for older deployment targets, O(n) insertion of equal keys into
  `unordered_multimap`): both bit PR2, so new code avoids those.
- Power-loss durability cannot be shown in CI.
- Every real collection is `opaque-unreviewed`: the reviewed first-party source table starts with
  `scene` and `group` only. Adding a source type means enumerating its settings keys and showing that
  none is a secret, URL, path or user text.
- Two cases are not detected: tokens inside URL paths, and plugins that write OBS files directly
  instead of through `config_t` or `SaveService()`.
- Live OBS states that PathRules rejects are handled by the planner: names that are not UTF-8 or
  break a rule become one omitted `kind: "names"` entry per folder with a count and no names;
  case-only clashes (profiles `Main` and `main` on Linux, `OBSBasic_Profiles.cpp:146-160`) are
  planned with `collision: true` and a warning, and capture (PR4) blocks restore onto a
  case-insensitive filesystem instead of rejecting the manifest. Still open: 8.3 short names, HFS+
  normalization and file-versus-folder clashes.
