No fix landed in the reviewed commit; all three Codex findings remain open.

FINDING codex-1 | Major | open | `Transform.cpp:272-276` allocates split14’s tuple, then `OutputValue` copies it at `:380-392`, violating the contract’s no-extra-allocation rule (`20-k1pr2-contract.md:95`).
FINDING codex-2 | Minor | open | `Transform.cpp:407-415` checks positions before unknown operations, returning `E_SCHEMA transforms` instead of `E_SCHEMA op` (`20-k1pr2-contract.md:78-80`).
FINDING codex-3 | Minor | open | `jc-check.cpp:639-657,991-994` accepts hexadecimal and leading-whitespace `--position` values, contrary to the finite-decimal requirement (`20-k1pr2-contract.md:142-143`).
VERDICT: request changes
