**Approve.** The head is still 4f5738264, the same commit as my last reply, so my dispositions stand. I re-checked every citation in the files. This was read-only again: I had no shell, so I built and ran nothing.

## codex-6 (Codex sets the severity)
The problem is real:
- Local mode only checks that `target` is an object with a string `os` (`Manifest.cpp:788-793`). It never looks at `audio`.
- The projection copies whichever of `sample_rate`/`channels` are present (`:1263-1272`). So `audio:{}` or `{"explicit":true}` exports as `audio:{}`, and nothing is listed in `dropped_ext` because `explicit` and `src` are known fields (`:949-950`).
- `--export` accepts this, because it only checks key names (`:739-747`).
- No case tests a partial `audio`. Every fixture has the full one (`cases.txt:404-409,540-551`).

I'd rate it Minor. An empty object carries no data, and until S2 writes the first store, the validator can be tightened at no cost.

The cause is wider than `audio`. Neither mode type-checks any value under a known key:
- `:783-799` only checks that `creator`, `active` and `target` are objects, that `os` is a string, and that `app_keys` and `dropped_ext` are arrays.
- So any JSON tree under `creator.app`, `active.profile`, `target.audio.sample_rate` or `app_keys[].value` survives the rebuild (`:1243-1254,1263-1272,1282-1287`) and passes `--export`.

That is unknown content carried into the export, which is what claude-4 was about, so I'm adding it to claude-4's leftover issues instead of opening a new ID. One fix covers both: check the known shapes (`WORKSPACE_FORMAT.md:95`, `:321-329`) in local mode, and require the full shape in `--export`.

## My findings (unchanged since my last reply)
- **Accepted:** claude-2, claude-3, claude-5 and claude-7. Evidence as in my last reply.
- **Still open, all Minor:**
  - **claude-1:** §8 keeps authentication on only for S2 restore (`:362-363`), but S3 import also writes roots (`:528-529`). §5 still promises that readers accept every minor (`:239`).
  - **claude-4:** `app_keys` items are still exported when `restore` is missing or unknown (`Manifest.cpp:750-752,1278-1280`), and input `dropped_ext` items are copied unchecked (`:1292`). Now also the unchecked values above.
  - **claude-6:** `export` still parses (`jw-check.cpp:194`) and hashes (`:209`) in two separate reads.
  - **claude-8:** spec text is out of date. The device rule (`WORKSPACE_FORMAT.md:547-549`) doesn't strip trailing spaces, but the code does (`PathRules.cpp:98-104`). The §15 notes are outdated (`:632-636`). The §7 collections row lists only browser-source secrets (`:287`).
  - **claude-9:** custom browser-dock URLs are saved in `user.ini` (`OBSBasic_Browser.cpp:73-90`), but they are not in the URL secret row (`:346`), and §7 lists no secret keys for `user.ini` (`:286`).

Self-merge still waits on Codex closing codex-6.

FINDING claude-1 | Minor | open | §8:362-363 keeps auth on only for S2 though S3 import writes roots (:528-529), so the exporter's known placeholder could go live; §5:239 lacks "a new §7 row needs an incompat feature"
FINDING claude-2 | Major | accepted-fix | unreviewed/omitted-field/now checks and catch-alls (Manifest.cpp:645-675,1053-1080,1169,1355)
FINDING claude-3 | Major | accepted-fix | ffmpeg_source input/ffmpeg_options and vlc playlist value added (WORKSPACE_FORMAT.md:346,390-392)
FINDING claude-4 | Minor | open | app_keys with missing/unknown restore exported and pass --export (Manifest.cpp:750-752,1278-1280); input dropped_ext copied unchecked (:1292); known values never type-checked, so nested data under creator/active/target.audio/app_keys keys survives rebuild and --export (:783-799,1243-1287)
FINDING claude-5 | Minor | accepted-fix | recorded as a PR3 open item (WORKSPACE_FORMAT.md:641-647)
FINDING claude-6 | Minor | open | export parses (jw-check.cpp:194) and hashes (:209) in two reads; other parts fixed
FINDING claude-7 | Minor | accepted-fix | 8-byte scan minimum and rounded unparseable sizes (WORKSPACE_FORMAT.md:264-272)
FINDING claude-8 | Minor | open | spec not updated: §12:547-549 device rule lacks the trailing-space strip; §15:632-636 out-of-date wmain and CON notes; §7:287 secret column
FINDING claude-9 | Minor | open | §8 url_credential misses user.ini [BasicWindow] ExtraBrowserDocks dock URLs (OBSBasic_Browser.cpp:73-90)
VERDICT: approve
