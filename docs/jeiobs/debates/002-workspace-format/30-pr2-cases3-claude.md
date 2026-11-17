I committed `581f3a812` on the detached HEAD, with no branch and no push. It touches 59 files, all under `tests/cases/`, and takes the suite from 968 to 994 cases.

**Test runs (Linux only, memory capped at 2.5 GB):**
- **Codex's r3.2 build** (`/tmp/jeiobs-s2/build-r32/jw-check` from 02:23, sha256 `37216dd2…`): 994 of 994 pass. It may not be their final build.
- **The PR #28 head, built here:** 974 pass and 20 fail. Every failure is one of the new r3.2 cases, and all 968 existing cases still pass with my runner changes. Its 64 MiB collection case times out, which confirms claude-3's per-URL scan to the end of the document.
- **A fake `jw-check`** that serves the expected outputs: every alternative report passes. Broken outputs (a range left in, no redactions, a dropped PLAINvalue, one byte changed late in a 64 MiB file) all fail with clear messages and nothing hangs.
- I did not run Windows, macOS or ASan.

## New cases
- **Item 1, long section names:**
  - A committed fixture pins `/~section-1/` (the long section is not first), a 1,024-byte name that keeps its pointer, and a section named `~section-1` escaped to `/~0section-1/`. A second fixture has two long names.
  - Generated per the brief: a 1 MiB header over 10^5 `Token` lines at `/~section-0/Token`, plus a 20,000-line version of the same file.
  - Linear URL scanning: a 64 MiB collection with 114,001 sources (about 54 MiB of trailing blanks), and an INI file with 900,000 `FFURL` values.
- **Item 2:** skipped. It needs a fault inside the library that no CLI input can reach; a comment in `cases.txt` says so.
- **Item 3:** the brief's `Token=S\n[Twitch] ;x\nRefreshToken=T\n`, plus:
  - `[]`, `[`, `[` at end of file, and `[ ]`, each followed by Token lines in later sections;
  - the codex-4 repro `[Twitch]\n[]\nToken=…`;
  - a key on the same line as `[]`;
  - keys before the first header, including one ended by a bare CR;
  - a file with no fallback trigger, where a Token before the first header is kept.
- **Item 4:**
  - `DesktopAudioDevice1`, `AuxAudioDevice1`, all six device names, `groups[]` items, `groups` without `sources`, and a `.bak` variant;
  - negatives that must be kept: `DesktopAudioDevice3`, `AuxAudioDevice0`/`5`, a lower-case name, nested objects, and device or `groups` values of the wrong type.
- **Existing reports changed:** only the two cases on the 1 MiB-header input, which now accept the long-name count reading.
- **Runner:** the marker count is now linear even when many secrets survive, and stored files over 4 MiB are compared 1 MiB at a time instead of being read whole. WORK_DIR now needs about 700 MB.

## Ambiguities (each accepted either way; the PASS line names the report that matched)
- **Long-name +1:** I accept it as an extra count or as the section's existing unknown-section count. Codex's build uses the second, so the lead may want to pin it.
- **The 10^5-token report** is about 8 MB. I accept stored or omitted (a size cap would omit it), and Codex's build omits it. The 20,000-token case is pinned to stored.
- **Pointer of a Token right after `[]`:** `/Twitch/Token` or `//Token`. I also accept count 1 or 2 when the empty section counts as an unknown section.
- **Collection count:** only `sources` items, or devices and groups items counted too. Codex's build counts them; I designed the fixtures so each has only two readings.
- **Pinned, not ambiguous:** libobs reads `[ ]` as section `" "`, so the pointer is `/ /Token` and the count is 2.

## OBS code I checked for item 4
- The six device names are defined at `OBSBasic_SceneCollections.cpp:798-803`. Each is loaded with `obs_load_source` (`:1007-1014`, `:1259-1264`), which types the source by `versioned_id` if it is non-empty, else `id` (`obs.c:2263-2282`).
- `groups` is read at `:1222` and appended to `sources`, or used as `sources` when that is missing (`:1270-1274`).

## Not covered
- **Two more load paths, outside r3.2:** `transitions[]` items are created with any `id` and their settings (`OBSBasic_Transitions.cpp:1466`), and so are filters (`obs.c:2376`). An existing case keeps filter URLs unredacted.
- **Memory hazard:** against a library without the fixes, the long-section inputs need 20–100 GiB. The runner header now says to cap memory when running such a build.

The Python I used to write the fixtures, the fake `jw-check` and the libobs oracle are in `/tmp/jeiobs-s2/cases-r32-tools/` and are not committed.
