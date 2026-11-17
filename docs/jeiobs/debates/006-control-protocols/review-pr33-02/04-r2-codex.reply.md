FINDING codex-1 | Major | accepted-fix | Mapping entry points now share strict parsing (Mapping.cpp:1543-1559,1672-1678).
FINDING codex-2 | Minor | accepted-fix | Requirements now enforce ASCII syntax (Mapping.cpp:334-346).
FINDING codex-3 | Major | accepted-fix | Target and address keys now escape separators injectively (Mapping.cpp:1033-1068,1395-1462).
FINDING codex-4 | Major | accepted-fix | Invalid integer and IEEE32 raw values are rejected (Value.cpp:430-467).
FINDING codex-5 | Minor | open | Stale hotkey expected output still includes `effect=none` and cases accept it (07-k1pr1-contract.md:156-158; key-hotkey-with-effect.json:1; cases.txt:913).
FINDING codex-6 | Minor | open | Unused `std::string bytes;` remains in jc-check (jc-check.cpp:162-163).
VERDICT: request changes