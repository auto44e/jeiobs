# Peer debate — 2026-11-16T05:20:45Z
Topic: Design S1 (snapshot format, secret boundary, inert model, capture API, first PR slices). Round 1 propose/critique; round 2 converge; list remaining disagreements.
Rounds: 2   Roles: claude=implementor codex=adversarial-reviewer
Context file: /tmp/jeiobs-s1/brief.md


[R1 CLAUDE] I'm the implementor. My position: store local history as a content-addressed directory, export it as a ZIP, and use one manifest schema for both. Stored files stay byte-exact, secrets are blanked in place at their exact byte positions, and the inert model is a C++ library with no Qt and no libobs. Four things I checked in the code change the brief's plan.

## Corrections to the brief and audit
- **The brief's capture order never saves the collection.** `DeferSaveBegin()` increments `disableSaving` (`OBSBasic_SceneCollections.cpp:992-995`), and `SaveProjectNow()` returns early while that counter is non-zero (`:1518-1522`). We would copy a stale file while a save is still queued (`:1528-1535`). `Save()` also only logs a failure; it returns nothing (`:985-989`).
- **Audit §6 is wrong about layout.** Window geometry and DockState are only written into memory by `saveAll()` (`OBSBasic.cpp:1740-1749`, run once at exit) and `closeEvent` (`:1574`). Calling `config_save_safe` alone writes an old layout.
- **cmocka isn't wired up.** The root `CMakeLists.txt:33` adds only `test/test-input`, so the `ENABLE_UNIT_TESTS` branch in `test/CMakeLists.txt:13` never runs, and no jeiobs workflow runs tests.
- **The fork still writes to `obs-studio/`** (`COEXISTENCE.md:8`, `OBSApp.cpp:420-457`). S1 must stay read-only, apart from the saves OBS already does. S2 restore has to wait until the fork has its own config identity.

## 1. Container
- **Location:** `<OS config>/jeiobs/workspaces/<workspace_id>/`, holding `objects/ab/<hash>`, `revisions/<rev>.json`, `HEAD`, `lock` and `overlays/`. It sits outside every U/S/P root.
- **Write order:**
  1. Blobs: temp file, then `fsync` / `FlushFileBuffers`, then rename. On macOS, one `F_FULLFSYNC` barrier. A blob that already exists is verified and skipped.
  2. The manifest.
  3. `HEAD`, written to a temp file and renamed over the old one (`MoveFileExW(REPLACE_EXISTING|WRITE_THROUGH)` on Windows).
- **Not `os_safe_replace`:** on POSIX with a backup it leaves a moment with no target file, and it never fsyncs (`platform-nix.c:576-581`).
- **Recovery doesn't trust `HEAD`:** the newest manifest whose blobs all verify wins. Orphan blobs are cleaned up under `lock`. This needs no atomic directory rename, so it behaves the same on all three OSes.
- **Unchanged files share blobs across revisions,** so history is cheap.
- **Export:** a `.jeiobsws` ZIP (zlib is already required, `libobs/CMakeLists.txt:17`), with `manifest.json` first and the same object names. Import unpacks into a staging store and runs the same validator.

## 2. Manifest v1
```json
{"format":"jeiobs.workspace","version":{"major":1,"minor":0},
 "features":{"ro_compat":[],"incompat":[]},
 "workspace_id":"<uuid4>","revision_id":"<uuid7>",
 "parents":[{"revision_id":"…","manifest_hash":"b2b256:…"}],
 "created_utc":"…","creator":{"app":"jeiobs","version":"…","commit":"…","obs_base":"33.0.1"},
 "target":{"os":"windows","arch":"x86_64","machine_id":"<local uuid>",
   "roots":{"S":"custom"},"active":{"profile":"prof:…","collection":"coll:…"},"devices":null},
 "entries":[
  {"id":"e3","role":"collection","uid":"coll:…","root":"S","path":"basic/scenes/Main.json",
   "size":48211,"hash":"b2b256:…","class":"private"},
  {"id":"e7","role":"profile.service","uid":"prof:…","root":"P","path":"basic/profiles/Main/service.json",
   "size":198,"hash":"b2b256:…","class":"private","redactions":[{"ptr":"/settings/key","ref":"sec:1"}]},
  {"id":"e9","role":"module.config","module":"obs-browser","status":"omitted","reason":"deny.cef-profile"}],
 "secret_refs":[{"id":"sec:1","kind":"stream_key","bind":{"type":"rtmp_common","service":"Twitch","server":"auto"}}]}
```
- Paths are relative to `<root>/<appdir>/`, so both OBS-named and jeiobs-named roots map. No absolute paths are stored.
- Unknown fields round-trip.
- An unknown major version or `ro_compat` feature opens read-only; an unknown `incompat` feature is refused.
- Sample rate and channel layout are not copied into the manifest; they are read from `basic.ini [Audio]`, so there is one source of truth.
- Editor audition overlays live in `overlays/` and are never part of a revision, a hash or an export.

## 3. What is captured by default
- **Always:**
  - `user.ini`.
  - Every `basic/scenes/*.json`.
  - Each profile's `basic.ini`, `service.json`, `streamEncoder.json` and `recordEncoder.json`. Any other file in a profile folder is captured as `unreviewed`.
  - `modules.json`.
- **`global.ini`:** captured as `app.machine` but never auto-restored or exported. It holds `[Locations]`, and `LastVersion` (`OBSBasic.cpp:1758`) drives migrations.
- **First-party module configs:**
  - websocket `config.json`, with the password redacted.
  - `persistent_data.json`, as `unreviewed`.
  - The decklink/aja output settings (`DecklinkOutputUI.cpp:55,91`).
  - `vst3list.json` (`obs-vst3.cpp:192`), as the target machine's plugin inventory.
- **Third-party module folders:** listed in the inventory only, with a per-module opt-in. They can hold caches or tokens of unknown size.
- **Never captured:**
  - logs, crashes, profiler_data, updates, `.sentinel`.
  - The browser (CEF) profile.
  - Service, font and capture caches, and `probe-*.json`.
  - `.bak`, `.v1`, `.invalid` and `.tmp` files. Their existence is recorded because S2 must deal with the `.bak` restore hazard.
  - Themes (referenced only).
  - The store itself.
- **Symlinks:** never followed; recorded as omitted.

## 4. Secrets
- **Registry:**
  - `service.json` `/settings/{key,bearer_token,password}`.
  - In `basic.ini`, the keys `Token` and `RefreshToken` in any section. The section name varies by service: `OAuth.cpp:52-53`, `YoutubeAuth.cpp:84-85`.
  - The websocket `ServerPassword`.
  - Names, UUIDs, channel names and usernames are classed private.
- **Redaction:** replace only the value's bytes with an empty value; every other byte stays the same.
- **Tests:** put the raw bytes back from the live file and get the original exactly; no secret bytes appear anywhere in the store.
- **Hashes:** they cover redacted bytes only. A plain hash of a file holding a weak `ServerPassword` would let someone test password guesses offline.
- **`secret_refs`:** no value and no hash.
- **Re-attach (S2):** needs the same `machine_id` and a matching `bind`. Otherwise a live Twitch key could be put into a restored custom-RTMP profile that points at a different server. No OS keystore in v1.
- **Unreviewed fields:** third-party settings, `private_settings`, VST state blobs, browser `url`/`css`, script settings and switcher window titles. They are flagged at export time by versioned rules, not stored in the manifest, so the rules can get stricter without rewriting history.

## 5. Inert model
- **Rule:** blobs are the source of truth. Capture and restore never re-serialize a file.
- **Library:** `shared/jeiobs-workspace/`, C++17, using nlohmann_json and the `deps/blake2` sources, and buildable on its own.
- **Components:**
  - `BoundedJsonScanner`: records byte positions and enforces limits before nlohmann sees the input.
  - `IniDocument`: preserves lines, including CRLF and BOM.
  - `CollectionIndex`: read-only view, with placeholders for missing devices and plugins.
  - `Manifest`.
- **S4 edits:** patch only the touched files.
- **Tests:** ctest with a ~60-line `jw_test.hpp`. A new `jeiobs-workspace-tests.yaml` workflow builds just this library on ubuntu, macos and windows, which gives real native CI on all three.

## 6. Stable IDs
- **Collections and profiles:** `coll:` and `prof:` UUIDs in a store sidecar, updated on the `*_RENAMED` events (`obs-frontend-api.h:61-62`). If a rename wasn't tracked, match the collection by its set of scene UUIDs.
- **Objects:** `(coll, uuid)` for sources, scenes, groups, filters and canvases (filters need the collection prefix because duplicated collections keep filter UUIDs).
- **Scene items:** `(coll, scene_uuid, id)` (`obs-scene.c:1338`).
- **Transitions:** `(coll, name)`.
- **INI values:** `(prof, file, section, key)`.
- **Fallback:** a JSON pointer. Pointers that use array indexes conflict with any concurrent edit to that array.

## 7. Capture
- **Where:** the logic is `jw::CaptureSession` in the library; `frontend/jeiobs/WorkspaceCapture.cpp` implements the `jw::CaptureHost` interface.
- **Sequence:**
  1. Post the capture through the event queue, never from inside a frontend-event handler, which avoids the stale `*_CHANGING` case.
  2. If `SavingDisabled()`, report busy.
  3. `SaveProjectNow()`, with a small fork-local change so it reports success or failure.
  4. `StageWindowState()`, extracted from `saveAll`, then `config_save_safe` for the user and profile configs.
  5. Read every file into memory in that same UI step. With no return to the event loop, no UI writer can interleave, so DeferSave isn't needed.
- **Other writers:** websocket `SetPersistentData` writes `persistent_data.json` (`RequestHandler_Config.cpp:109`) off the UI thread (`WebSocketServer.cpp:123`). Such files are read twice and retried up to three times, then recorded as a warning.
- **Worker thread:** redacts, hashes and writes the store.
- **Testing without OBS:** a fake host, temporary root trees, and an injected writer that changes a file mid-capture.

## 8. Hash
BLAKE2b-256 using the streaming API in `deps/blake2/src/blake2.h:151-155`, tagged `b2b256:`. SHA-256 would pull mbedtls into the inert library. It is for integrity only.

## 9. Import limits (`limits.hpp`)
- **Sizes:** manifest ≤ 4 MiB and 20k entries; each document ≤ 64 MiB; all documents ≤ 512 MiB.
- **JSON:** depth ≤ 64, strings ≤ 16 MiB, ≤ 1M values. INI lines ≤ 1 MiB.
- **Paths:** relative, UTF-8, `/` only. Reject empty, `.`, `..`, absolute, drive or UNC paths, `\`, `:`, control characters, a trailing dot or space, and Windows device names. Components ≤ 255 bytes, paths ≤ 512 bytes.
- **Duplicates:** reject exact and ASCII case-insensitive duplicates, and create files with `O_EXCL`/`CREATE_NEW` to catch collisions specific to the filesystem.
- **ZIP:** no symlink, hardlink, device or encrypted entries; declared size must equal actual size; compression ratio ≤ 100:1; no entries the manifest doesn't list.
- **Always:** verify the hash before parsing; never fetch or execute anything.

## 10. PR slices
1. **PR1:** `WORKSPACE_FORMAT.md`, `limits`, `PathRules`, `Hash`, `Manifest` and their fixtures (unknown fields, future minor and major, incompat, malformed), plus 3-OS CI.
2. **PR2:** scanner, INI parser and redactor, with secret fixtures (rtmp_common, custom RTMP with auth, WHIP, Twitch/YouTube, websocket), byte-exact and no-leak tests, and a fuzz target.
3. **PR3:** `CapturePlanner` and `CollectionIndex`, with synthetic Windows/macOS/Linux root trees: custom locations, portable mode, RØDECaster-like WASAPI IDs, CoreAudio, Pulse, VST3 hex state, a missing plugin, `.bak` noise and a symlink.
4. **PR4:** the frontend host, `StageWindowState()` and the save-result change. No UI.

## Where I expect pushback
I'll defend in-place byte redaction over re-rendering the file, having no keystore, making third-party modules opt-in, and not using cmocka.
[R1 CODEX] I agree with the CAS local store, ZIP export, inert no-Qt/no-runtime library, and the correction that `DeferSaveBegin()` cannot precede `SaveProjectNow()`: `OBSBasic_SceneCollections.cpp:992-995` increments `disableSaving`, while `SaveProjectNow()` exits early at `:1518-1522`. Extracting window staging is also necessary because `saveAll()` only persists geometry/DockState at `OBSBasic.cpp:1740-1749`.

The proposal still has several correctness holes.

1. Atomicity and recovery

`fsync` plus rename is insufficient on POSIX unless the containing directories are also fsynced. Windows replacement needs explicit crash-recovery testing, not just `WRITE_THROUGH`. `HEAD` recovery should not choose the newest timestamp: clocks can regress, and an orphan manifest may be newer but unrelated. Validate a commit marker, manifest hash, referenced blobs, and parent lineage. Defer garbage collection; “unreferenced” does not mean unreachable if branches or interrupted writes exist.

Store and export cannot literally share identical metadata. Portable export must strip `machine_id`, local root identities, secret references, and local overlays. Use the same schema with an explicit export projection.

2. Manifest problems

The manifest omits the required normalized target sample rate and channel layout, then treats `basic.ini` as the sole source. That fails when profiles differ. Record target audio fields explicitly, with a source-entry reference.

Capturing all of `global.ini` as `app.machine` is questionable. It contains machine paths and migration state (`OBSBasic.cpp:1758`); if it is never restored or exported, capture only reviewed keys or represent them as target metadata.

Unknown-field preservation needs a defined mechanism. Normal `nlohmann::json` parse/serialize does not preserve formatting or necessarily key order. Store the original manifest bytes or an extension DOM, and define canonical bytes for `manifest_hash`. Also clarify whether `parents` is intentionally a DAG rather than the brief’s singular parent.

3. Allowlist and secrets

“Any other file in a profile folder is captured as `unreviewed`” violates the security boundary. Unknown opaque files must be omitted by default, represented by placeholders, and enabled only through reviewed module rules. Export-time flagging is too late: the local store already contains the payload.

Key-based redaction is necessary but insufficient. Credentials can appear in URL userinfo/query strings, script settings, plugin blobs, `private_settings`, and nested module data. Those must be structurally redacted or omitted at capture. The websocket path also needs the exact key/path rule; `RequestHandler_Config.cpp:109` confirms an off-thread persistence writer, which makes broad opaque capture particularly risky.

“In-place redaction” is not byte-exact storage. The artifact is a sanitized projection; only non-secret files can claim byte-exact preservation. A `secret_ref` must identify the source file/key, a non-secret source fingerprint, binding constraints, and a machine-local handle. Without a keystore, reattachment can only be attempted against the same live location and must become an unresolved placeholder on mismatch. Portable exports should omit the references entirely.

4. Capture coherence

Posting work through the event queue does not create a writer barrier. `persistent_data.json` can be modified off-thread, so “read twice and warn” produces a potentially mixed revision, not a coherent snapshot. The capture API needs a frontend persistence epoch/barrier that serializes all known writers. If an injected writer changes a file during capture, tests should require rejection or explicit non-coherent status—not silently accept it.

Reading every file into memory on the UI thread also risks long stalls. Stage bounded files or use a writer-quiesced snapshot mechanism. `SaveProjectNow()` must return a real success/failure result; its current logging-only behavior at `:985-989` is inadequate.

5. Identity and parsing

Rename-event sidecars can lose updates on crashes or missed events. Persist the mapping atomically in the workspace store and use scene/source UUID matching as fallback. Transition identity remains name-scoped and should explicitly treat rename as delete/add.

The parser limits should additionally reject duplicate JSON keys, invalid UTF-8, embedded NULs, non-finite numbers, and Unicode-normalization/case collisions. ASCII case-folding is insufficient for Windows/macOS filesystem behavior; validate against the destination filesystem as well.

BLAKE2b-256 is acceptable only after confirming the updater copy is actually linkable by this library. “Present in `deps/blake2`” is not a build contract. Add known-vector tests and define lowercase encoding/domain separation.

PR1 should include the audited allowlist, export projection, placeholder semantics, and canonical manifest rules—not only the data structures. PR2 should make unknown opaque data fail closed. PR4 must test the real off-thread-writer race and save-result failure.

Remaining disagreements are therefore: in-place redaction versus sanitized projection, default treatment of opaque module files, whether `global.ini` is captured at all, the required capture barrier, keystore/reference semantics, and whether BLAKE2 is truly a supported dependency.
[R2 CLAUDE] I accept most of Codex's corrections. I dispute three points: canonical manifest bytes, domain-separated hashing, and stripping unreviewed data out of collections at capture. Codex and I both understated the coherence problem: obs-websocket writes the active profile's files from worker threads, not just `persistent_data.json`.

**Accepted into v1**
- **Durability.** On POSIX: fsync the file, rename, then fsync the parent directory (`F_FULLFSYNC` at commit on macOS).
  - The commit point is the rename of `revisions/<seq>-<rev>.json`. `HEAD` is only a hint.
  - Recovery ignores clocks and uses a local, ever-increasing `seq`. It picks the highest `seq` whose manifest and blobs verify and whose parent chain verifies; anything else moves to `quarantine/`.
  - No garbage collection until S2.
  - Crash consistency is tested by injecting failures at every write step through an `Fs` interface. CI cannot prove power-loss durability; the format doc will say so.
- **One `parent` per revision**, matching WORKSPACE_SAVING's "parent revision". Merges come later behind `incompat:["merge"]`.
- **`ProjectForExport()`** drops `machine_id`, custom-root info, `secret_refs`, overlays and `parent` (replaced by `origin:{workspace_id,revision_id}`). Known secrets are already removed at capture, so export never rewrites a blob for secrets.
- **`target.audio`** holds `{sample_rate, channels, explicit, src:{entry,key}}` from the active profile. The validator rejects it if it disagrees with the referenced blob.
- **`global.ini` is not captured; only reviewed keys go into `target.app`.** New evidence: obs-websocket migrates from `global.ini [OBSWebSocket] ServerPassword` (`Config.cpp:35,155-190`), so a shared or older config root can hold a plaintext password there.
- **Unknown data is omitted by default.** This covers extra files in profile folders, `persistent_data.json` and third-party module folders. Each is recorded as `{status:"omitted",path,size,reason}` with no hash, because a hash of a small file lets someone test guesses offline.
- **Each entry carries `fidelity: exact|redacted`.** I keep editing only the secret value's bytes rather than re-rendering the file: re-rendering changes float formatting, escapes and key order, which breaks S4's patch-only edits.
- **`secret_ref` = {entry, pointer, kind, bind, machine_id}.** It carries no fingerprint derived from the value. On a mismatch it becomes an unresolved placeholder.
- **Parser** rejects duplicate keys (OBS already does: `obs-data.c:611`, `JSON_REJECT_DUPLICATES`), invalid UTF-8, NULs and non-finite numbers.
  - On capture, a file OBS cannot load is stored as `parse:"invalid"`, and its `.bak` is captured as `collection.fallback`, since that is what OBS actually loads.
  - On import, such a file is rejected.
- **Identity drops the event sidecar.** Capture resolves each collection against the parent revision: same path, then same set of scene UUIDs, then a new uid. The result is stored in the revision. A missed event can only cost identity, never data. A transition rename is delete plus add.
- **`SaveProjectNow` returns `Saved|Refused|Failed`** (`:1518-1526`, `:985-988`).

**Disputed**
1. **Canonical manifest bytes aren't needed.** Revisions never change, and `manifest_hash` hashes the stored bytes; nothing re-hashes a parsed copy. Rules for unknown fields:
   - Readers ignore them.
   - A child revision copies an entry's unknown fields only if that entry's blob hash is unchanged.
   - Any field that is unsafe to drop must set `ro_compat`, which stops older writers from creating children.
2. **No domain separation in the hash.** Plain BLAKE2b-256 lets `b2sum -l 256` verify exports; the object type comes from the field that references it.
   - BLAKE2 is a real build contract: the `OBS::blake2` target (`deps/blake2/CMakeLists.txt:3-4`) is already pulled in behind guards at `os-windows.cmake:1-2`, `feature-whatsnew.cmake:14-15` and `feature-macos-update.cmake:5-6`. License is CC0/OpenSSL/Apache-2.0.
   - I checked the known-answer vectors: `""`→`0e5751c0…e3a8`, `"abc"`→`bddd813c…2319`.
3. **Unreviewed data inside collections stays in local history.** The spec calls opaque state "unreviewed, not automatically sanitized" and has export block it until reviewed. Stripping browser, script or VST state would make restoring the user's own collection lossy.
   - It is stored with 0700/0600 permissions, flagged at capture as `unreviewed:{rules,count}`, and re-checked at export.
   - The spec does exclude "authenticated URLs" from snapshots, so URL userinfo and a reviewed list of query keys are treated as secrets at capture. Tokens embedded in URL paths can't be found structurally; they stay flagged and blocked at export.

**Missed by both of us**
1. **The active profile's files are written off the UI thread.** obs-websocket handles requests on a thread pool (`WebSocketServer.cpp:352`).
   - `SetProfileParameter` and `SetRecordDirectory` call plain `config_save` (`RequestHandler_Config.cpp:417,655`). It truncates the file before writing (`config-file.c:347-382`), so a reader can see an empty `basic.ini` that still parses.
   - `SetStreamServiceSettings` runs `SaveService()` on the worker thread (`:603` → `OBSStudioAPI.cpp:534-537`). Its POSIX replace-with-backup leaves a moment where the file doesn't exist (`platform-nix.c:578-580`).

   obs-websocket is an upstream submodule, so the barrier has to use locks these writers already take. A generic save counter does not serialize writers that don't take part.
   - Add `config_lock()/config_unlock()` to libobs util. `config_save` and `config_save_safe` already hold `config->mutex` across the write and replace (`:345-390,411-441`), so this blocks every `config_t` writer on every thread, third-party plugins included. Take one lock at a time.
   - Add a frontend `persistMutex` that `SaveService()` holds.
   - Read the files writers can touch right now (active collection, `user.ini`, active profile) in a single UI turn. Other collections and profiles change only through UI-thread operations, so a worker reads them while a `persistGen` counter, bumped at those code paths, detects interference. A counter only works for writers we own, and this is where we own them.
   - If a file changed underneath us (detected by re-stat), refuse the capture.
2. **Blanking the websocket password opens the server.**
   - The JSON key is `server_password` (`Config.cpp:43`); `ServerPassword` is the legacy INI key.
   - The auth secret is derived from whatever password is stored (`WebSocketServer.cpp:86`). `AuthRequired` is a separate flag (`:238`).
   - So `""` lets anyone log in to a network-reachable remote-control server.
   - Placeholders must differ by kind: stream key, bearer token and OAuth go to empty, which fails closed. The websocket password gets a random value generated per capture. S2 must never write `""`.
3. **On Windows, our reads can make OBS's saves fail.** `os_safe_replace` uses `ReplaceFileW` (`platform-windows.c:767-770`), which fails while any reader holds the file without `FILE_SHARE_DELETE`. CRT and `ifstream` readers don't set that flag, and OBS only logs the failure (`:988`). Capture must open files with `CreateFileW(READ|WRITE|DELETE)` sharing, with a test.
4. **There is no ZIP reader or writer in the tree.** zlib only compresses. The ZIP reader is S3's main attack surface and needs its own design record; S1 fixes only the limits.
5. **Capture needs a scope gate.** Capture writes live config (`SaveProjectNow`, `config_save_safe`), and task 002 forbids live OBS writes. While the fork still shares `obs-studio/`, the frontend capture code must refuse to run unless the config root belongs to jeiobs or is a test root.

**Revised PR slices.** The store writer moves to S2; S1 capture ends at an in-memory `CaptureResult`.
- **PR1:** `WORKSPACE_FORMAT.md` (allowlist, secret registry, placeholders, export projection, compatibility rules, recovery spec), plus `limits`, `PathRules`, `Hash`, `Manifest` with `ProjectForExport`, fixtures, and CI on all three OSes.
- **PR2:** JSON scanner, INI parser, and redactor with per-kind placeholders. Fixtures cover the websocket placeholder, the legacy `[OBSWebSocket]` section, URL userinfo, a scan proving no secret bytes remain, and fuzzing.
- **PR3:** `CapturePlanner` (omit-by-default, `.bak` fallback), `CollectionIndex`, identity resolution.
- **PR4:** `config_lock`, `persistMutex`, the three-state save result, `StageWindowState`, and the gated capture code. Tests: an injected off-thread `config_save` is blocked (no torn read), an outside change is refused, Windows sharing mode, and a failed save produces no result.

**For the human to decide:** the new libobs `config_lock` API, and keeping unreviewed collection data in local history.

**Still disagreeing with Codex:** keeping unreviewed collection data locally, canonical manifest bytes, and hash domain separation.
[R2 CODEX] Claude’s worker-thread finding is material and correct: `SaveProjectNow` plus a UI-thread generation counter cannot establish coherence while obs-websocket calls `config_save` from its pool (`WebSocketServer.cpp:352`; `RequestHandler_Config.cpp:417,655`) and `SaveService()` replaces files asynchronously (`OBSStudioAPI.cpp:534-537`). A pre/post `stat` is only detection, not serialization: an overwrite can preserve size/mtime, and Windows replacement can invalidate the pathname between open and read.

I agree with:

- Fail-closed websocket placeholders. Blanking `server_password` is unsafe because auth derives from it (`WebSocketServer.cpp:86`), and `AuthRequired` is separate (`:238`). However, the random placeholder must also force `AuthRequired=true`; otherwise a restored snapshot can remain unauthenticated.
- Omitting `global.ini` wholesale while allowlisting reviewed keys, given the legacy plaintext password path (`Config.cpp:35,155-190`). Those keys need explicit provenance and restore semantics; putting them only in `target.app` loses file-level fidelity.
- `.bak` capture, bounded parsing, duplicate-key rejection, and an in-memory `CaptureResult` before implementing the store.
- Plain BLAKE2b-256 is acceptable. Domain separation is defense-in-depth, not required if blob namespaces and expected object types are enforced. External `b2sum` compatibility is useful.

I dispute or qualify three points.

1. `config_lock` is not yet a coherence solution.

Exposing `config_t`’s mutex is risky: it creates lock-order/API compatibility obligations and only covers writers using that exact `config_t`. It does not cover `SaveService()`, direct file writes, scripts, or plugins that bypass config APIs. A global frontend `persistMutex` also works only if every relevant writer takes it.

The contract should be:

- enumerate every writer;
- make each in-scope writer acquire one documented coordinator lock;
- capture from already-open handles while the lock is held;
- reject capture if any writer is outside the protocol.

`persistGen` and re-stat may remain diagnostics, but must not be correctness gates. On Windows, use a handle opened with `FILE_SHARE_DELETE`; test that `ReplaceFileW` can proceed and that the capture reads a stable handle, not a subsequently replaced pathname.

2. “Unknown fields are copied if the entry blob is unchanged” is underspecified.

Unknown fields may exist at top level, nested objects, and entries. An older writer can silently drop top-level extensions even when every blob is unchanged. `ro_compat` only helps if every unknown field is known to require it.

Use one of these explicit rules:

- preserve an opaque raw-extension tree byte-for-byte; or
- define canonical serialization and preserve unknown fields structurally.

The current hybrid rule is not sufficient for the stated forward-compatibility requirement. Also, `manifest_hash` cannot be inside the bytes it hashes; define an external hash record or hash the manifest excluding a specified integrity field.

3. Keeping unreviewed opaque data locally is acceptable only with a hard export gate.

It conflicts with “portable export never contains secrets” unless export fails closed whenever any `unreviewed` entry exists. “Re-checked at export” is too weak: the export must be staged privately, scanned, and atomically committed only after all opaque entries are either reviewed or omitted. A copied `blobs/` directory must not constitute an export path.

Also distinguish:

- `exact`: original bytes retained;
- `redacted`: deterministic byte-range replacement;
- `opaque-unreviewed`: exact bytes retained locally but never exportable.

Redaction means the result is no longer byte-exact. The schema must say that explicitly and retain no original secret bytes. For websocket placeholders, preserve a machine-local `secret_ref`, generate a random nonempty password, and force authentication on restore.

Important omissions:

- The `.bak` file is not automatically “what OBS loads.” Capture both files and record the loader-selection rule; otherwise restore may select stale state.
- The allowlist needs exact file/key paths, required/optional status, and restore behavior for omitted placeholders. “Module configs” is still unresolved.
- Import limits need concrete values: maximum manifest bytes, file bytes, total bytes, entry count, nesting depth, path length, and parent-chain length. Path rules must reject absolute paths, `..`, NULs, drive/UNC paths, symlink escapes, and case-fold collisions.
- Target device inventory needs privacy rules; device names, serials, and endpoint paths can themselves be sensitive.
- Windows durability is incomplete. POSIX directory fsync has no direct equivalent; specify `FlushFileBuffers`, replacement semantics, and recovery behavior.
- Portable export’s actual container is still undecided. `ProjectForExport()` defines projection, not whether export is a directory, zip, or another format. ZIP limits and zip-slip defenses belong in S1 even if implementation is S2.
- Identity matching by path, then scene-UUID set, can produce false matches after path reuse. Require a unique match and retain an explicit generated collection UID; otherwise create a new identity.

Remaining disagreements:

- I still reject relying on `persistGen`/re-stat as correctness mechanisms.
- I require opaque-data export to fail closed, not merely be “re-checked.”
- Canonical bytes are optional, but raw extension preservation is mandatory.
- `config_lock` needs a writer inventory and lock-order proof before becoming a public libobs API.
