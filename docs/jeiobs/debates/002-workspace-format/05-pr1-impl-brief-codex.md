# Worker brief (Codex, implementation): JeiOBS S1-PR1 workspace library

Work ONLY in your current directory, a git worktree of the JeiOBS repo (OBS Studio 33.0.1
fork) on a detached HEAD. Read first: `docs/jeiobs/WORKSPACE_FORMAT.md` (the format spec) and
`/tmp/jeiobs-s1/pr1/contract.md` (the exact PR1 contract: layout, CLI, error codes, schema).

Build `shared/jeiobs-workspace/` per the contract:
- `Limits` (constants), `PathRules` (validate a path → ok / unchecked / rule), `Hash` (streaming
  BLAKE2b-256 via OBS::blake2, lowercase hex), `Manifest` (bounded parse with nlohmann_json's SAX
  interface so limits, duplicates, NUL, non-finite and depth are enforced DURING the parse; UTF-8
  validated on raw bytes before or during parse; schema validation for local and export modes;
  `CarryOver(parent, draft)` child-writer rules; `ProjectForExport(manifest, bytes-hash,
  decisions, now)`), and the `jw-check` CLI exactly as the contract says.
- Unit tests in `tests/` with an in-tree `tests/jw_test.hpp` (no third-party framework),
  target `jw-unit`, including BLAKE2b-256 known answers: "" → 0e5751c026e543b2e8ab2eb06099daa1d1e5df47778f7787faab45cdf12fe3a8,
  "abc" → bddd813c634239723171ef3fee98579b94964e3bb1cb3e427262c8c068d52319.
- Register `jw-cases` exactly as the contract says (another worker writes
  `tests/cases/`; do NOT create or edit anything under `tests/cases/`).
- Do not touch files outside `shared/jeiobs-workspace/`. No new third-party dependency.
  Do not change `deps/`, `.github/`, the root CMakeLists, or docs.
- Style: OBS conventions (tabs, `.clang-format` at the repo root; clang-format 22 is at
  `/tmp/cf-venv/bin/clang-format`, run it on your files). Warnings: build clean with
  `-Wall -Wextra -Werror` on GCC (set it in the project for GCC/Clang; `/W4 /WX` for MSVC).
  Write portable code (Windows/macOS will build it in CI later): no POSIX-only calls in the
  library; open files in binary mode.
- Verify: `cmake -S shared/jeiobs-workspace -B /tmp/jeiobs-s1/pr1/build-impl -G Ninja &&
  cmake --build /tmp/jeiobs-s1/pr1/build-impl && ctest --test-dir /tmp/jeiobs-s1/pr1/build-impl
  --output-on-failure` must pass (jw-cases is absent until the other worker's files arrive).
- Commit on the detached HEAD (never create branches, never push):
  `git -c user.name=auto44e -c user.email=auto44e@gmail.com commit -m "shared: jeiobs-workspace
  library for S1-PR1 (limits, paths, hash, manifest, export projection)"`.

Final reply: what you built (files, public API), what you verified, every place where you
found the contract ambiguous and how you read it, and risks (untested platforms).
