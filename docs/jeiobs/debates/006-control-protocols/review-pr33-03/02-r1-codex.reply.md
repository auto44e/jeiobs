Round-4 fixes verified; no new findings.

FINDING codex-1 | Major | accepted-fix | Strict parser unified across public paths (Mapping.cpp:257-287,1509-1599,1730-1736).
FINDING codex-2 | Minor | accepted-fix | Requirements enforce ASCII syntax (Mapping.cpp:338-349).
FINDING codex-3 | Major | accepted-fix | Address and target separators are escaped injectively (Mapping.cpp:1037-1072,1437-1501).
FINDING codex-4 | Major | accepted-fix | Invalid integer and IEEE32 raw values are rejected (Value.cpp:441-477).
FINDING codex-5 | Minor | accepted-fix | Hotkey expectation excludes effect while the fixture still supplies it (tests/cases/expected/compile/key-hotkey-with-effect.json:1; tests/cases/fixtures/compile/key-hotkey.json:9).
FINDING codex-6 | Minor | accepted-fix | Unused bytes local was removed (src/jc-check.cpp:162-167).
VERDICT: approve