# PR #33 review run 01 — K1-PR1 (jeiobs-control types, schema, validator, compiler), head 1a8bd1eb4

Your working directory is a checkout of this commit. Change: `git diff a0373dec8 HEAD`. Read docs/jeiobs/tasks/006-control-protocols.md (K1 design A1-A18, Progress) and docs/jeiobs/debates/006-control-protocols/07-k1pr1-contract.md (the contract with addendum r1), shared/jeiobs-control/ (src, include, tests incl. tests/cases with 916 case lines).
Authorship: Codex worker wrote the library; Claude worker wrote the black-box cases from the contract without reading the library; the lead wrote the contract, decided eight readings (r1) and replaced a recursive cycle check (stack overflow under ASan on a 10,000-route chain) with an iterative one.
Lead's evidence (Linux, GCC, -Wall -Wextra -Werror): jc-unit and jc-cases pass, also under ASan+UBSan. No CI job yet for this library.
Review for: contract and design fidelity (A1-A18), exactness of values (128-bit, floats, round trips), the hard-deny list and privileged-wildcard rule (can a mapping reach a secret field or start an output through a wildcard), fan-out and cycle checks (soundness and complexity), strict JSON parsing and limits, portability (MSVC: 128-bit integers are not native; Apple libc++), test validity, doc accuracy. Owner (verbatim): "if you peers are more or less in agreement, no problem self merging".

Severity: Blocker = must not merge; Major = must be fixed before a self-merge; Minor = follow-up.
Other agent work runs on this host in separate worktrees; it does not touch this checkout.
