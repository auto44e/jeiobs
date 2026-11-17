FINDING codex-1 | Major | accepted-fix | Strict parsing is unified across mapping and event entry points (Mapping.cpp:1509-1513,1592-1603,1712-1737).
FINDING codex-2 | Minor | accepted-fix | Requirements enforce ASCII syntax (Mapping.cpp:338-349).
FINDING codex-3 | Major | accepted-fix | Address and target keys escape all separators injectively (Mapping.cpp:1037-1072,1435-1502).
FINDING codex-4 | Major | accepted-fix | Invalid integer and IEEE32 raw values are rejected (Value.cpp:441-477).
FINDING codex-5 | Minor | accepted-fix | Hotkey expected output omits effect while the fixture supplies it (key-hotkey-with-effect.json:1; key-hotkey.json:9; cases.txt:913).
FINDING codex-6 | Minor | accepted-fix | Unused bytes local was removed (jc-check.cpp:162-167).
FINDING codex-7 | Minor | open | Duplicate detection uses attacker-controlled `"std::unordered_set<std::string> keys"` and can be hash-flooded within the 4 MiB input cap (Mapping.cpp:93,199-206; Limits.hpp:17).
VERDICT: approve