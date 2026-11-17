Implemented and committed as `7830ea01c`.

Files/API:

- Added `JsonScan`, `IniScan`, and `Redactor` public headers/implementations.
- Added `jw-check redact`.
- Added Part B manifest validation and single-read export hashing.
- Added `jw-fuzz-smoke` and optional Clang `jw-fuzz`.
- Added unit coverage; `tests/cases/` untouched.

Verification:

- `jw-unit`: passed
- `jw-fuzz-smoke`: passed
- `jw-cases`: passed
- Worktree clean, detached HEAD.
- Optional libFuzzer build not run because Clang is unavailable.

Ambiguities:

- “Known values” for `creator`/`active` was interpreted as defined fields only; unknown extension fields remain compatible.
- Existing export compatibility retains `E_SCHEMA target` for forbidden `target.audio.explicit/src`; other audio-shape errors use `target.audio`.
- Collection unreviewed counts follow the explicit `max(1, unknown sources)` formula.

Risks: Windows/macOS builds and runtime CSPRNG/platform behavior were not exercised locally.
