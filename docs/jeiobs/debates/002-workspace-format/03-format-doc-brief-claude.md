# Drafting brief: docs/jeiobs/WORKSPACE_FORMAT.md (JeiOBS S1-PR1)

You are drafting a design/format document for JeiOBS (public OBS Studio 33.0.1 fork) at
~/oss/jeiobs (read-only for you; do NOT edit any file). Print ONLY the finished Markdown document
on stdout (no preamble, no closing remarks). It will be committed as docs/jeiobs/WORKSPACE_FORMAT.md.

Read first (facts and decisions):
- /tmp/jeiobs-s1/s1-persistence-audit.md — what OBS persists, writers, secrets (facts, with file:line).
- /tmp/jeiobs-s1/design-debate.md — the 2-round Claude/Codex design debate. Its R2 sections are the
  converged design; the lead resolves the remaining disagreements below.
- ~/oss/jeiobs/docs/jeiobs/WORKSPACE_SAVING.md, docs/jeiobs/tasks/002-workspace-saving.md,
  docs/jeiobs/ROADMAP.md (M0/M1, C1-C3), docs/jeiobs/DECISIONS.md, docs/jeiobs/tasks/003-*.md
  (house style: plain, specific, cite file:line, no marketing words, short sentences).

Lead's resolutions of the open points (write them in as decisions, with the rejected alternative):
1. Revision ID = lowercase hex BLAKE2b-256 of the stored manifest bytes. The hash is never inside
   the bytes it hashes; it is the file name `revisions/<seq>-<revid>.json` and the child's `parent`.
   No canonical serialization in v1 (rejected: canonical JSON; reason: stored bytes are immutable
   and only ever hashed as stored).
2. Forward compatibility: readers ignore unknown fields. A writer creating a child revision copies
   unknown top-level fields and unknown per-entry fields structurally (as JSON values) from the
   parent; per-entry unknown fields are copied only when that entry's blob hash is unchanged, else
   dropped and listed in `dropped_ext`. `compat.incompat[]` (reader must refuse) and
   `compat.ro_compat[]` (reader may read, must not write children). Unknown major `format` = refuse
   writes, read-only display if the manifest still validates.
3. Plain BLAKE2b-256 (no domain separation), so `b2sum -l 256` verifies blobs. Object type comes from
   the referencing field; readers enforce expected type. Uses `deps/blake2` target `OBS::blake2`.
   Known-answer vectors: "" -> 0e5751c026e543b2e8ab2eb06099daa1d1e5df47778f7787faab45cdf12fe3a8,
   "abc" -> bddd813c634239723171ef3fee98579b94964e3bb1cb3e427262c8c068d52319.
4. Entry fidelity: `exact` (original bytes), `redacted` (deterministic byte-range replacement of
   secret values only; no original secret bytes retained), `opaque-unreviewed` (exact bytes kept in
   local history only, store perms 0700/0600; export FAILS CLOSED while any such entry exists —
   export is staged privately, scanned, and committed atomically only when every opaque entry is
   reviewed or omitted; copying `blobs/` is not an export path). Unknown FILES (extra files in
   profile dirs, persistent_data.json, third-party module dirs) are omitted by default:
   `{status:"omitted", path, size, reason}`, no hash.
5. Placeholders per secret kind: stream key / bearer token / OAuth -> empty (fails closed);
   obs-websocket `server_password` -> random per-capture value AND `auth_required` forced true;
   S2 restore must never write "". `secret_ref = {entry, pointer, kind, bind, machine_id}`, no
   value-derived fingerprint; mismatch -> unresolved placeholder. Export drops secret_refs.
6. `global.ini` is not captured; only listed reviewed keys, each recorded with file/section/key
   provenance and restore semantics. Legacy `[OBSWebSocket] ServerPassword` is a secret.
7. `.bak`: capture both the collection and its `.bak`, record the loader-selection rule OBS uses
   (cite code) instead of claiming which one loads.
8. Identity: each collection/profile gets a generated UID stored in the revision. Matching against
   the parent: same path, then identical scene-UUID set — only a UNIQUE match counts, otherwise new
   UID. Transitions are name-keyed; rename = delete + add.
9. Capture coherence (implemented in S1-PR4, not PR1): correctness comes from serializing every
   in-scope writer through a documented coordinator, reading from handles opened while serialized;
   capture refuses if a writer outside the protocol may run. Re-stat / generation counters are
   diagnostics only, never correctness gates. Writer inventory must be listed (cite audit). The
   libobs `config_lock()` API is an OWNER DECISION (new public libobs API) — say so. Windows: open
   with FILE_SHARE_READ|WRITE|DELETE so ReplaceFileW in os_safe_replace still succeeds.
   Capture refuses on a shared `obs-studio/` config root (needs M0 separate root) unless it is a
   test root.
10. Durability: POSIX fsync file, rename, fsync dir (F_FULLFSYNC on macOS at commit). Windows:
   FlushFileBuffers then MoveFileExW(MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH). Commit point
   = the revision manifest rename; HEAD is a hint. Recovery picks highest seq that fully verifies
   (manifest, blobs, parent chain); others to quarantine/. No GC before S2. CI cannot prove
   power-loss durability — say so.
11. Portable export container = ZIP via ProjectForExport() (drops machine_id, custom-root info,
   secret_refs, overlays, parent -> origin{workspace_id, revision_id}). ZIP reader/writer is S3 with
   its own design record; S1 fixes the limits and zip-slip rules now.
12. Device inventory / target machine: local only, export carries only sample rate / channel layout
   / OS family; device names, IDs, serials omitted from export (privacy).
13. Import limits — give concrete numbers (pick conservative values and justify briefly): manifest
   bytes, single file bytes, total bytes, entry count, JSON nesting depth, string length, path
   length/components, parent-chain length. Parser rejects duplicate keys (OBS does too,
   obs-data.c JSON_REJECT_DUPLICATES), invalid UTF-8, NUL, non-finite numbers. Path rules: relative
   only, no `..`, no NUL/control chars, no drive letters / UNC / leading slash/backslash, no
   Windows reserved names, no trailing dot/space, no symlinks, collision check after Unicode NFC +
   case fold.
14. Store layout under `<OS config dir>/jeiobs/workspaces/<workspace_id>/` with `blobs/<2>/<62>`,
   `revisions/`, `HEAD`, `quarantine/`, `lock`.
15. PR slices: PR1 = this doc + `shared/jeiobs-workspace/` (C++17, no Qt, no libobs runtime) with
   Limits, PathRules, Hash, Manifest (+ProjectForExport, validation) and synthetic fixtures, tested
   by ctest. The 3-OS CI workflow is a `.github` change -> separate owner-merged PR. PR2 parsers +
   redactor; PR3 CapturePlanner/identity; PR4 coherent capture (owner decision on config_lock).
   Store writer moves to S2.

Include a full manifest v1 JSON example (local) and its export projection, the allowlist table
(file, root, role, captured keys/whole file, secret keys, fidelity), the secret registry table
(file, key/pointer, kind, placeholder), and an "Open owner decisions" section (config_lock API;
keeping opaque-unreviewed data in local history). Verify every file:line you cite by reading the
code. Keep it under ~450 lines.
