# S1-PR1 contract, addendum r2 (after review run 01). Wins over the contract where they differ.

1. Stored-entry allowlist (finding claude-1). A `status:"stored"` entry must have one of these
   exact (root, role, path) shapes, where `<f>` and `<d>` are ONE path component (no `/`) that
   itself passes PathRules; anything else → `ERR E_SCHEMA role`:
   | root | role | path |
   | --- | --- | --- |
   | user | app.user | `user.ini` |
   | scenes | collection | `basic/scenes/<f>.json` |
   | scenes | collection.bak | `basic/scenes/<f>.json.bak` |
   | profiles | profile.basic | `basic/profiles/<d>/basic.ini` |
   | profiles | profile.service | `basic/profiles/<d>/service.json` |
   | profiles | profile.service.bak | `basic/profiles/<d>/service.json.bak` |
   | profiles | profile.encoder.stream | `basic/profiles/<d>/streamEncoder.json` |
   | profiles | profile.encoder.stream.bak | `basic/profiles/<d>/streamEncoder.json.bak` |
   | profiles | profile.encoder.record | `basic/profiles/<d>/recordEncoder.json` |
   | profiles | profile.encoder.record.bak | `basic/profiles/<d>/recordEncoder.json.bak` |
   | plugin_manager | app.modules | `plugin_manager/modules.json` |
   | config | module.obs-websocket | `plugin_config/obs-websocket/config.json` |
   | config | module.decklink-output-ui | `plugin_config/decklink-output-ui/decklinkOutputProps.json` or `.../decklinkPreviewOutputProps.json` |
   | config | module.aja-output-ui | `plugin_config/aja-output-ui/ajaOutputProps.json`, `.../ajaPreviewOutputProps.json` or `.../ajaMiscProps.json` |
   Path comparison is exact (case-sensitive). Omitted entries keep the old rules (any role or none).
2. Entry field consistency (claude-2, codex-1):
   - `fidelity:"opaque-unreviewed"` requires `unreviewed` = object with `rules` (non-empty string)
     and `count` (integer ≥ 1); missing or malformed → `E_SCHEMA unreviewed`.
   - `unreviewed` on a stored entry with another fidelity → `E_SCHEMA unreviewed`.
   - `parse` may only be `"invalid"` and only on an `opaque-unreviewed` entry → else `E_SCHEMA parse`.
   - An omitted entry must not have `fidelity`, `hash`, `redactions`, `unreviewed`, `parse`
     → `E_SCHEMA <that field>` (first in that order).
   - `ProjectForExport` and every CLI command must never throw or crash on any input the
     validator accepts, nor on any input at all (catch → `ERR` with a code, never abort).
3. `export --now` must be RFC 3339 UTC exactly `YYYY-MM-DDTHH:MM:SSZ` with a valid calendar date
   and time → else `ERR E_SCHEMA now` (exit 1).
4. DECISIONS file (codex-2): read with the same bounded reader and parser as manifests (4 MiB
   limit before reading, `E_DUPKEY`, `E_UTF8`, `E_NUL`, `E_NONFINITE`, `E_BOM`, `E_JSON`, depth
   and string limits); a non-regular file or unreadable path → `E_IO`. It must be an object; each
   value an object with `decision` = `"include"` or `"omit"`; `include` may carry `fidelity`,
   anything else → `E_SCHEMA decisions`. A decision for an id that is not an entry →
   `E_SCHEMA decisions`.
5. Include fidelity is derived (claude-4): the exported fidelity is `"redacted"` if the entry has
   a non-empty `redactions` array, else `"exact"`. If the decision gives `fidelity` and it differs
   → `E_SCHEMA decisions`.
6. Export rebuilds known objects only (claude-4): `creator` keeps `app`, `version`, `commit`,
   `obs_base`; `active` keeps `profile`, `collection`; `compat` keeps `incompat`, `ro_compat`;
   each `app_keys` item keeps `root`, `file`, `section`, `key`, `value`, `restore`; `target` is
   exactly `{os, audio:{sample_rate, channels}}` (audio omitted if absent in the input). Dropped
   unknown names are listed in the output's `dropped_ext` item for the top level
   (`{"entry": null, "fields": [...]}`) as dotted names (`creator.x`, `app_keys.y`, `target.z`),
   sorted, together with unknown top-level fields. The input's existing `dropped_ext` items are
   kept, before the new ones.
7. `manifest --export` (claude-4): an export manifest's `target` may only have `os` and `audio`
   (`audio` only `sample_rate`, `channels`) → else `E_SCHEMA target`; an `app_keys` item with
   `restore:"never"` → `E_SCHEMA app_keys`.
8. Device names (codex-3): strip trailing spaces from the part before the first dot before
   comparing; so `CON .txt`, `nul  .json`, `aux .` → `E_PATH device-name` (a component that is
   only `CON ` still gives `trailing-dot-space`).
9. `revision FILE` (claude-6): one bounded read; a file over 4 MiB → `ERR E_LIMIT manifest`
   without reading it all; a non-regular file (directory, FIFO, device) → `ERR E_IO`; the hash
   and the parse use the same bytes. Same rule for `manifest FILE` and `hash FILE` keeps reading
   any size for regular files (hash has no limit) but `hash` of a non-regular file → `ERR E_IO`.
10. `child` (codex-5, claude-6): the output is validated as a local manifest before printing; if it
    fails (for example a limit), print that `ERR` and no JSON.
11. Error output (codex-4): on any `ERR`, stdout holds exactly the one `ERR` line and no JSON.
