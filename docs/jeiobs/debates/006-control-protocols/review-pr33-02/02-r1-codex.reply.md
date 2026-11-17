The original four findings are fixed. Two issues remain.

FINDING codex-1 | Major | accepted-fix | Strict parsing is unified across mapping entry points (Mapping.cpp:1469-1548,1672-1678).
FINDING codex-2 | Minor | accepted-fix | Requirements now enforce ASCII syntax (Mapping.cpp:336-349).
FINDING codex-3 | Major | accepted-fix | Target and address keys now escape separators injectively (Mapping.cpp:1033-1068,1395-1462).
FINDING codex-4 | Major | accepted-fix | Out-of-range integer and IEEE32 raw values are rejected (Value.cpp:430-467).
FINDING codex-5 | Major | open | R3 forbids hotkey `effect` in target keys, but the stale expected file includes `effect=none` and cases still accept it (07-k1pr1-contract.md:156-157; tests/cases/expected/compile/key-hotkey-with-effect.json:1; tests/cases/cases.txt:913).
FINDING codex-6 | Blocker | open | Unused `"std::string bytes;"` conflicts with `-Wall -Wextra -Werror` (src/jc-check.cpp:162; CMakeLists.txt:35-36).
VERDICT: request changes