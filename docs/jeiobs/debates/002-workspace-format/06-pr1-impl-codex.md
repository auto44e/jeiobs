Implemented and committed as `9f1133475`.

Built `shared/jeiobs-workspace/` with:

- `Limits`, `PathRules`, streaming BLAKE2b-256 `Hash`
- Bounded SAX manifest parsing and local/export schema validation
- Compatibility flags, collision checks, child carry-over, export projection
- `jw-check` CLI
- In-tree `jw_test.hpp` and `jw-unit`
- Optional `jw-cases` CTest registration

Verified:

- GCC build with `-Wall -Wextra -Werror`
- Required `cmake`, build, and `ctest` commands: passed `jw-unit`
- BLAKE2 known answers and file hashing
- Path validation smoke cases
- clang-format 22
- Clean staged scope; no files outside the requested directory changed

Ambiguities resolved:

- Defined value-oriented C++ APIs because signatures were unspecified.
- Used ASCII folding with `unchecked` for valid non-ASCII paths, per the PR1 contract over the broader Unicode spec.
- Interpreted unknown export fields as one grouped top-level `dropped_ext` record and one record per entry.
- Used `E_UTF8` for invalid manifest bytes; direct `path` checks return `E_PATH not-utf8`.
- Usage errors print to stderr with exit code 2.

Risks: `jw-cases` was unavailable in this worktree, and Windows/macOS builds, Unicode collision tables, and live OBS integration remain untested.
