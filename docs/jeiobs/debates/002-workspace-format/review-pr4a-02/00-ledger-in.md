# Findings ledger

ID | severity | status | finder | source | lead's note
--- | --- | --- | --- | --- | ---
quick checks pending at start
local: test_config_lock 80/80 runs pass (Linux GCC Debug); no-op mutation fails 2 tests
claude-1, codex-4 | Major, Minor | fixed, awaiting finder | both | run 01 | lock-order wording rewritten
claude-3, codex-1 | Major | fixed, awaiting finder | both | run 01 | config_save_safe reads the file name under the lock; regression test
claude-2, codex-2, codex-6 | Minor | fixed, awaiting finder | both | run 01 | open_defaults and num_sections lock; config_close excluded
claude-4 | Minor | fixed, awaiting finder | claude | run 01 | %ignore in the SWIG interfaces
claude-5, codex-3, codex-7 | Minor | fixed, awaiting finder | both | run 01 | add_test NAME/COMMAND; atomic stop flag; unused event removed
claude-6, codex-5, codex-8 | Minor | fixed, awaiting finder | both | run 01 | stale citations replaced
