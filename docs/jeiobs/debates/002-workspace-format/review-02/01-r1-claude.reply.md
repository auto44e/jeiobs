**Approve: no Blocker or Major is left.** I accept claude-2, claude-3, claude-5 and claude-7. claude-1, claude-4 and claude-6 each keep one leftover issue at Minor; claude-1 was Major and I'm lowering it. I'm adding two new Minors. This was a read-only review again: I had no shell, so I built and ran nothing.

## My findings
- **claude-1 (lowered to Minor):** the main fix is in.
  - Stored entries are now tied to the allowlist in both modes (`Manifest.cpp:493-549`, enforced at `:610`; `WORKSPACE_FORMAT.md:525-531`), and the store's own paths are refused (`cases.txt:356-357`).
  - Under the allowlist, stored paths in different roots can't overlap, and §12 gives the collision check on real destinations to S2/S3.
  - Two spec sentences are still missing:
    - My run-01 request to re-apply the role rules to imported blobs wasn't done. §8's websocket rule covers only S2 restore (`:362-363`), but §12 has S3 import writing to roots too (`:528-529`). An exported websocket `config.json` holds the exporter's 32-character placeholder (`:358-359`). If import writes it unchanged, the live password becomes one anyone with the ZIP knows, with auth on.
    - The fix has a side effect. §5 promises that readers accept every minor version (`:239`), but a v1.0 reader rejects any stored row a later minor adds (`E_SCHEMA role`). §5 should say that a new §7 row needs an `incompat` feature.
- **claude-2: fixed.**
  - Field checks: `Manifest.cpp:645-649,660-675`.
  - `now` is validated: `:1053-1080,1185`.
  - Catch-alls: `:1169`, `:1355` and `jw-check.cpp:234,260`.
- **claude-3: fixed** (`WORKSPACE_FORMAT.md:346,390-392`). The citations are accurate (`obs-ffmpeg-source.c:182,211,396-422`, `vlc-video-source.c:14,637`).
- **claude-4: partly fixed.** Objects are rebuilt, the include fidelity is derived and `--export` checks the export-only fields (`Manifest.cpp:727-756,1233-1290,1336-1339`). Two things remain:
  - The `restore` check on `app_keys` still fails open. An item whose `restore` is missing or unknown is exported (`:1278-1280`) and passes `--export` (`:750-752`). Only `same-os` items should be kept.
  - Input `dropped_ext` items are copied without any check (`:1292`).
- **claude-5: accepted** as a recorded PR3 open item (`WORKSPACE_FORMAT.md:641-647`).
- **claude-6: partly fixed.**
  - Fixed:
    - `revision` does one bounded read (`jw-check.cpp:126-144`).
    - `LoadedManifest` is gone.
    - Files are opened by UTF-8 path (`Utf8Path.hpp:17-24`).
    - `child` output is checked again before printing (`jw-check.cpp:182-187`).
  - Still open: `export` parses (`:194`) and hashes (`:209`) in two separate reads. If the file changes in between, `origin.revision_id` names bytes other than the ones exported. The fix is the same as for `revision`: read once with `ReadFile`, then run `HashBytes` and `ParseManifest` on that buffer.
- **claude-7: fixed** (`WORKSPACE_FORMAT.md:264-272`). Optional hardening: `parse` isn't limited to roles without secrets (`Manifest.cpp:672-675`), so an out-of-spec collection would export its exact size (`:1299-1310`).

## Codex's findings (for Codex to dispose of)
The codex-1 to codex-5 fixes are in the code:
- codex-1: `Manifest.cpp:660-671`.
- codex-2: `jw-check.cpp:199-208`, which now uses `Manifest.cpp:319-354`.
- codex-3: `PathRules.cpp:96-104,201-203`, with cases at `cases.txt:117-127`.
- codex-4: `jw-check.cpp:182-188,219-220`, plus the runner's one-line rule (`cases.txt:13`).
- codex-5: `jw-check.cpp:182-187`.

A related point I'm not raising as a finding: `export` output isn't checked again before printing (`jw-check.cpp:219-220`). Each entry with unknown fields adds a 3-value `dropped_ext` record (`Manifest.cpp:1298`), so a manifest near the limits can produce an export that `manifest --export` refuses. That fails safely.

## New findings
- **claude-8 (Minor): the spec wasn't updated for r2 and `wmain`.**
  - The §12 device rule still tests the part before the first dot as it is (`WORKSPACE_FORMAT.md:547-549`). By the spec, `CON .txt` is valid, but `PathRules.cpp:98-104` refuses it.
  - §15 still calls this an untested gap (`:635-636`), though cases exist at `cases.txt:117-119`.
  - §15 still says `jw-check` reads `argv` in the ANSI code page (`:632-634`), but it now has `wmain` (`jw-check.cpp:275`).
  - In §7, the collections row still lists only browser-source secrets (`:287`).
- **claude-9 (Minor): `url_credential` misses custom browser-dock URLs.**
  - Each custom dock's `url` is saved in `user.ini` under `[BasicWindow] ExtraBrowserDocks` (`OBSBasic_Browser.cpp:79-90`).
  - §8 notes that these are URLs (`:388-389`), but they aren't in the URL row (`:346`), and §7 lists no secret keys for `user.ini` (`:286`).
  - So a `?token=` in a dock URL is never redacted and never part of the live-value scan (`:269-271`). The same URL in a `browser_source` would be both.
  - It's Minor because export re-runs the current registry (`:395-396`), so the row can be added with PR2.

FINDING claude-1 | Minor | open | allowlist binding fixed; still open: §8 websocket rule binds only S2 though S3 import writes roots (placeholder known to the exporter would go live), and §5 lacks "a new §7 row needs an incompat feature"
FINDING claude-2 | Major | accepted-fix | unreviewed/omitted-field/now checks and catch-alls (Manifest.cpp:645-675,1053-1080,1169,1355)
FINDING claude-3 | Major | accepted-fix | ffmpeg_source input/ffmpeg_options and vlc playlist value added (WORKSPACE_FORMAT.md:346,390-392)
FINDING claude-4 | Minor | open | still open: app_keys with missing/unknown restore are exported and pass --export (Manifest.cpp:750-752,1278-1280); input dropped_ext items copied unchecked (:1292)
FINDING claude-5 | Minor | accepted-fix | recorded as a PR3 open item (WORKSPACE_FORMAT.md:641-647)
FINDING claude-6 | Minor | open | still open: export parses (jw-check.cpp:194) and hashes (:209) in two reads; other parts fixed
FINDING claude-7 | Minor | accepted-fix | 8-byte scan minimum and rounded unparseable sizes (WORKSPACE_FORMAT.md:264-272)
FINDING claude-8 | Minor | open | spec not updated: §12:547-549 device rule lacks the trailing-space strip; §15:632-636 out-of-date wmain and CON notes; §7:287 secret column
FINDING claude-9 | Minor | open | §8 url_credential misses user.ini [BasicWindow] ExtraBrowserDocks dock URLs (OBSBasic_Browser.cpp:79-90)
VERDICT: approve
