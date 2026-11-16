# S1 design brief: JeiOBS workspace snapshot format and inert document model

Repo: ~/oss/jeiobs (OBS Studio 33.0.1 fork, C/C++, Qt6 frontend, CMake, libobs util APIs,
nlohmann_json available to the frontend). Read first:
- /tmp/jeiobs-s1/s1-persistence-audit.md (what OBS persists, where, writers, secrets, coherence, devices) — facts.
- docs/jeiobs/WORKSPACE_SAVING.md (requirements S1-S6, offline editor, portability, security bounds).
- docs/jeiobs/ROADMAP.md M1 and C1-C3 (OBS-compatible legacy saves; sidecars; never write live OBS data).
- docs/jeiobs/tasks/002-workspace-saving.md (S1-S6 deliverables).

S1 deliverable (this design decides it; implementation follows in small PRs):
"audit actual persistence writers and define versioned schema, secret boundaries, coherent capture
and inert document model; add synthetic round-trip fixtures."

Decide, concretely:
1. Snapshot container on disk (local history) vs portable export: a directory of content-addressed
   blobs + manifest (git-like), a zip, or both (store vs export). Atomicity on Windows/macOS/Linux.
2. Manifest schema v1: workspace ID, revision ID, parent, timestamps, creator app/version, target
   machine profile (OS/arch/sample rate/channel layout/device inventory) kept separate from editor
   overlays; per-file entries (logical role, original relative path + which root (U/S/P/config),
   size, hash, secret-class); omitted/placeholder entries; schema versioning and forward-compat
   (unknown fields preserved; read-only fallback for unknown major version).
3. Which files/roots are captured by default (allowlist): collections, profiles, user.ini/global.ini
   parts, plugin_manager, module configs (which ones?), and what is never captured (logs, crashes,
   caches, CEF profile, updates).
4. Secret boundary: structural exclusion by file+key (service.json key/bearer_token/password,
   basic.ini OAuth Token/RefreshToken, websocket ServerPassword, CEF profile). Store a machine-local
   reference so restore on the same machine can re-attach; portable export never contains them.
   Unreviewed opaque payloads (plugin state blobs, script settings, browser URLs): how flagged.
5. Lossless storage: store files byte-exact (OBS re-save drops unknown keys) and ALSO a parsed index
   for the offline editor? Where does the parsed inert model live: a new C++ library
   (e.g. `shared/jeiobs-workspace/`, no Qt, no libobs runtime state, only libobs util + json) with
   unit tests runnable in CI (OBS uses cmocka for libobs tests; what test harness?).
6. Stable object identity for later change sets (S6): source/scene UUIDs exist; transitions are
   name-keyed; profiles/collections are name+dir keyed. Propose IDs.
7. Coherent capture API (UI thread): DeferSaveBegin -> SaveProjectNow (refuse if disableSaving) ->
   flush profile/user config -> copy -> DeferSaveEnd. Where is it implemented (frontend module) and
   how is it tested without a real OBS session?
8. Hashing: which algorithm (libobs has only crc32/sha via mbedtls in frontend? check what is linked;
   blake2 is vendored in deps/blake2 for the updater). Integrity, not trust.
9. Bounded parsing rules for import (sizes, counts, path safety) — S3 uses them; define now.
10. First implementation slice (PR-sized): what exactly goes in S1-PR1, S1-PR2 ...

Be concrete (file layout, JSON examples, function names). Cite file:line from the audit/code
where it matters. Max ~900 words. Do not edit files.
