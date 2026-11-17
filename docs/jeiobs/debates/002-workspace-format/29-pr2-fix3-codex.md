Implemented and committed as `13a23baba`.

- Item 1 — `Redactor.cpp:38-52, 585-586, 860-872`: bounded report/pointer construction, long-section sentinel pointers, and URL escape scans limited to each value.
- Item 2 — `Redactor.cpp:1071-1079, 1104-1105`; `jw_fuzz_smoke.cpp:53-94`: fail-closed apply failures and exact output/report validation.
- Item 3 — `IniScan.cpp:157-258`; `Redactor.cpp:917-922`: whole-file malformed-header fallback, including sectionless and pre-header tokens.
- Item 4 — `Redactor.cpp:757-815`; `jw_unit.cpp:474-481`: scans `groups` and six audio-device objects.

OBS evidence: audio devices are loaded through `obs_load_source` at `frontend/widgets/OBSBasic_SceneCollections.cpp:1007-1024`, invoked at lines 1259-1264; `groups` is appended into `sources` at lines 1270-1274.

Results:

- Normal CTest: 3/3 passed.
- ASan/UBSan CTest: 3/3 passed.
- clang-format and `git diff --check`: passed.
- `/tmp/gt/gt` oracle checks: passed.
- `tests/cases/` unchanged.

Ambiguity resolved: long sections already count as unknown sections, so they are not double-counted while still using bounded sentinel pointers.
