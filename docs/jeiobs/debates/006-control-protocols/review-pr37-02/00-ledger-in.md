# Findings ledger

ID | severity | status | finder | source | lead's note
--- | --- | --- | --- | --- | ---
quick checks pending at start
local: jc-unit, jc-cases (1292 lines) PASS on Linux, also under ASan+UBSan
codex-1 | Major | fixed, awaiting finder | codex | run 01 | OutputValue moves the join14/split14 result out of `owned`
claude-2 | Minor | fixed, awaiting finder | claude | run 01 | same as codex-1
claude-3, codex-2 | Minor | fixed, awaiting finder | both | run 01 | Compile order: op and params before positions (contract r1.5)
claude-4, codex-3 | Minor | fixed, awaiting finder | both | run 01 | --position plain decimal only (contract r1.6); nested floats keep bits_hex (r1.4)
claude-1 | Minor | fixed, awaiting finder | claude | run 01 | TestTransformExtraCoverage added
claude-5 | Minor | fixed, awaiting finder | claude | run 01 | comments and docs corrected
