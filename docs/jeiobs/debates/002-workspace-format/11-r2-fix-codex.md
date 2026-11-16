Implemented and committed as `19ed051c2` (detached HEAD). Worktree is clean.

Addendum coverage:

1. Stored-entry allowlist: Manifest.cpp:493, enforced at Manifest.cpp:610.

2. Entry-field consistency and exception safety: Manifest.cpp:620, Manifest.cpp:1163, CLI catch at jw-check.cpp:232.

3. Strict calendar-valid UTC `--now`: Manifest.cpp:1050.

4. Bounded shared decisions reader/parser: Manifest.cpp:319, jw-check.cpp:199.

5. Derived export fidelity: Manifest.cpp:1333.

6. Rebuilt known export objects and dotted dropped fields: Manifest.cpp:930.

7. Export target/app-key restrictions: Manifest.cpp:727.

8. Windows device-name trimming: PathRules.cpp:96.

9. Bounded single-read revisions; regular-file checks for manifest/hash: jw-check.cpp:126, Hash.cpp:57.

10. Child output revalidation: jw-check.cpp:182.

11. `ERR` output remains a single stdout line: jw-check.cpp:33.

Also removed unused `LoadedManifest`; added shared `ReadFile`/`ParseJson` APIs at Manifest.hpp:53.

Verification:

- Required configure/build/`ctest -R jw-unit`: passed.
- Added unit coverage in jw_unit.cpp:66.
- Manual CLI checks passed for `CON .txt`, invalid dates, directory decisions, duplicate decisions, and non-regular files.
- Existing black-box suite: 311/328 passed. The 17 failures are stale pre-r2 cases: old child allowlist paths, old required decision fidelity, old nested-export expectations, and parse-invalid decision semantics. `tests/cases/` was not modified.

Ambiguity decisions: retained the original contract’s rejection of decisions for non-opaque entries; interpreted “valid calendar time” as year ≥1 and seconds 00–59; nested unknown fields are emitted as dotted names and existing `dropped_ext` entries remain first.
