Findings:

FINDING codex-1 | Major | open | `Transform.cpp:272-276` allocates `split14`’s tuple, then `OutputValue` deep-copies it at `:380-392`; this violates the contract’s “no allocation beyond the output Value” (`20-k1pr2-contract.md:95`).
FINDING codex-2 | Minor | open | `Transform.cpp:407-415` checks position before unknown operations, so direct `Compile` can return `E_SCHEMA transforms` instead of the contract’s “unknown op stays E_SCHEMA op” (`20-k1pr2-contract.md:78`).
FINDING codex-3 | Minor | open | `jc-check.cpp:639-657` uses `strtod`, which accepts hexadecimal floats such as `0x1p-1`; this is accepted by `:991-994` despite r1 requiring a “finite decimal number” (`20-k1pr2-contract.md:142`).

VERDICT: request changes
