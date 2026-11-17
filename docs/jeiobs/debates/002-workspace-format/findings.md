# Findings ledger — PR #21 (S1-PR1)

Severity and status are each finder's latest, from review run 02 on `4f5738264` (run 01 is in
`review-01/`). Both peers approved run 02; no Blocker or Major is open. The open Minors are
carried into S1-PR2.

ID | severity | status | finder | source | lead's note
--- | --- | --- | --- | --- | ---
claude-1 | Minor | open → fixed (awaiting finder) | claude | review-02/03-r2-claude.reply.md (FINDING claude-1) | follow-up in S1-PR2
claude-2 | Major | accepted-fix | claude | review-02/03-r2-claude.reply.md (FINDING claude-2) | —
claude-3 | Major | accepted-fix | claude | review-02/03-r2-claude.reply.md (FINDING claude-3) | —
claude-4 | Minor | open → fixed (awaiting finder) | claude | review-02/03-r2-claude.reply.md (FINDING claude-4) | follow-up in S1-PR2
claude-5 | Minor | accepted-fix | claude | review-02/03-r2-claude.reply.md (FINDING claude-5) | —
claude-6 | Minor | open → fixed (awaiting finder) | claude | review-02/03-r2-claude.reply.md (FINDING claude-6) | follow-up in S1-PR2
claude-7 | Minor | accepted-fix | claude | review-02/03-r2-claude.reply.md (FINDING claude-7) | —
claude-8 | Minor | open → fixed (awaiting finder) | claude | review-02/03-r2-claude.reply.md (FINDING claude-8) | follow-up in S1-PR2
claude-9 | Minor | open → fixed (awaiting finder) | claude | review-02/03-r2-claude.reply.md (FINDING claude-9) | follow-up in S1-PR2
codex-1 | Major | accepted-fix | codex | review-02/04-r2-codex.reply.md (FINDING codex-1) | —
codex-2 | Major | accepted-fix | codex | review-02/04-r2-codex.reply.md (FINDING codex-2) | —
codex-3 | Major | accepted-fix | codex | review-02/04-r2-codex.reply.md (FINDING codex-3) | —
codex-4 | Minor | accepted-fix | codex | review-02/04-r2-codex.reply.md (FINDING codex-4) | —
codex-5 | Minor | accepted-fix | codex | review-02/04-r2-codex.reply.md (FINDING codex-5) | —
codex-6 | Minor | open | codex | review-02/04-r2-codex.reply.md (FINDING codex-6) | follow-up in S1-PR2

## PR #25 (recording the owner decisions), review `review-decisions`

PR #25 merged while this review was still running. Owner's approval, verbatim: "i think 25
should e merged, and you can continue self merge from here on for your next coupe of hours". The
review requested changes (rationale and flex missing, wording). The follow-up PR answers them.

ID | severity | status | finder | source | lead's note
--- | --- | --- | --- | --- | ---
claude-1 | Major | open → fixed (awaiting finder) | claude | review-decisions/03-r2-claude.reply.md (FINDING claude-1) | fixed in the follow-up PR (why/flex, wording, task 002 Next)
claude-2 | Minor | open → fixed (awaiting finder) | claude | review-decisions/03-r2-claude.reply.md (FINDING claude-2) | fixed in the follow-up PR (why/flex, wording, task 002 Next)
claude-3 | Minor | open → fixed (awaiting finder) | claude | review-decisions/03-r2-claude.reply.md (FINDING claude-3) | fixed in the follow-up PR (why/flex, wording, task 002 Next)
claude-4 | Minor | open → fixed (awaiting finder) | claude | review-decisions/03-r2-claude.reply.md (FINDING claude-4) | fixed in the follow-up PR (why/flex, wording, task 002 Next)
codex-1 | Major | open → fixed (awaiting finder) | codex | review-decisions/04-r2-codex.reply.md (FINDING codex-1) | fixed in the follow-up PR (why/flex, wording, task 002 Next)

PR #27 review runs: `review-decisions-2` (changes requested: purge, wording) and
`review-decisions-3` (both approve). Run 3's last open item:

ID | severity | status | finder | source | lead's note
--- | --- | --- | --- | --- | ---
claude-7 | Minor | open → fixed (awaiting finder) | claude | review-decisions-3/03-r2-claude.reply.md (FINDING claude-7) | wording now "the owner's answer"
claude-2 to claude-6, codex-1 | Major or Minor | accepted-fix | claude, codex | review-decisions-3 | all accepted by their finders in run 3

## PR #28 (S1-PR2), review `review-pr2-01` on `5bdc30e12`

The run failed closed in round 2 after Claude's replies (the lead created a branch in a shared
worktree during a turn, which the tamper check caught); Codex's round-2 reply is missing. The
findings below are from Claude's round-2 reply and Codex's round-1 reply. The lead's fixes are
`0a809589c` and `a2a90344a` with cases `7c2d9f50e`; libobs's parser served as the oracle for INI
sections (`26-libobs-ini-oracle.c`).

ID | severity | status | finder | source | lead's note
--- | --- | --- | --- | --- | ---
claude-1 | Major | open → fixed (awaiting finder) | claude | review-pr2-01/03-r2-claude.reply.md (FINDING claude-1) | fixed per contract r3 / r3.1 (19-pr2-contract-r3.md)
claude-2 | Major | open → fixed (awaiting finder) | claude | review-pr2-01/03-r2-claude.reply.md (FINDING claude-2) | fixed per contract r3 / r3.1 (19-pr2-contract-r3.md)
claude-3 | Major | open → fixed (awaiting finder) | claude | review-pr2-01/03-r2-claude.reply.md (FINDING claude-3) | fixed per contract r3 / r3.1 (19-pr2-contract-r3.md)
claude-4 | Major | open → fixed (awaiting finder) | claude | review-pr2-01/03-r2-claude.reply.md (FINDING claude-4) | fixed per contract r3 / r3.1 (19-pr2-contract-r3.md)
claude-5 | Minor | open → fixed (awaiting finder) | claude | review-pr2-01/03-r2-claude.reply.md (FINDING claude-5) | fixed per contract r3 / r3.1 (19-pr2-contract-r3.md)
claude-6 | Minor | open → fixed (awaiting finder) | claude | review-pr2-01/03-r2-claude.reply.md (FINDING claude-6) | fixed per contract r3 / r3.1 (19-pr2-contract-r3.md)
claude-7 | Minor | open → fixed (awaiting finder) | claude | review-pr2-01/03-r2-claude.reply.md (FINDING claude-7) | fixed per contract r3 / r3.1 (19-pr2-contract-r3.md)
claude-8 | Minor | open → fixed (awaiting finder) | claude | review-pr2-01/03-r2-claude.reply.md (FINDING claude-8) | fixed per contract r3 / r3.1 (19-pr2-contract-r3.md)
claude-9 | Major | open → fixed (awaiting finder) | claude | review-pr2-01/03-r2-claude.reply.md (FINDING claude-9) | fixed per contract r3 / r3.1 (19-pr2-contract-r3.md)
claude-10 | Minor | open → fixed (awaiting finder) | claude | review-pr2-01/03-r2-claude.reply.md (FINDING claude-10) | fixed per contract r3 / r3.1 (19-pr2-contract-r3.md)
claude-11 | Minor | open → fixed (awaiting finder) | claude | review-pr2-01/03-r2-claude.reply.md (FINDING claude-11) | fixed per contract r3 / r3.1 (19-pr2-contract-r3.md)
codex-1 | Major | open → fixed (awaiting finder) | codex | review-pr2-01/02-r1-codex.reply.md (FINDING codex-1) | fixed per contract r3 / r3.1 (19-pr2-contract-r3.md)
codex-2 | Major | open → fixed (awaiting finder) | codex | review-pr2-01/02-r1-codex.reply.md (FINDING codex-2) | fixed per contract r3 / r3.1 (19-pr2-contract-r3.md)
codex-3 | Minor | open → fixed (awaiting finder) | codex | review-pr2-01/02-r1-codex.reply.md (FINDING codex-3) | fixed per contract r3 / r3.1 (19-pr2-contract-r3.md)

## PR #28 (S1-PR2), review `review-pr2-02` on `e9de5fb28`

Fixes: `13a23baba` (library), cases `581f3a812`, spec edits (INI rules, URL rule, INI limits, `input`).

ID | severity | status | finder | source | lead's note
--- | --- | --- | --- | --- | ---
claude-1 | Major | accepted-fix | claude | review-pr2-02/03-r2-claude.reply.md (FINDING claude-1) | fixed per contract r3.2 (29-pr2-fix3-codex.md); spec updated
claude-2 | Major | accepted-fix | claude | review-pr2-02/03-r2-claude.reply.md (FINDING claude-2) | fixed per contract r3.2 (29-pr2-fix3-codex.md); spec updated
claude-3 | Major | open → fixed (awaiting finder) | claude | review-pr2-02/03-r2-claude.reply.md (FINDING claude-3) | fixed per contract r3.2 (29-pr2-fix3-codex.md); spec updated
claude-4 | Major | accepted-fix | claude | review-pr2-02/03-r2-claude.reply.md (FINDING claude-4) | fixed per contract r3.2 (29-pr2-fix3-codex.md); spec updated
claude-5 | Minor | accepted-fix | claude | review-pr2-02/03-r2-claude.reply.md (FINDING claude-5) | fixed per contract r3.2 (29-pr2-fix3-codex.md); spec updated
claude-6 | Minor | open → fixed (awaiting finder) | claude | review-pr2-02/03-r2-claude.reply.md (FINDING claude-6) | fixed per contract r3.2 (29-pr2-fix3-codex.md); spec updated
claude-7 | Minor | accepted-fix | claude | review-pr2-02/03-r2-claude.reply.md (FINDING claude-7) | fixed per contract r3.2 (29-pr2-fix3-codex.md); spec updated
claude-8 | Minor | accepted-fix | claude | review-pr2-02/03-r2-claude.reply.md (FINDING claude-8) | fixed per contract r3.2 (29-pr2-fix3-codex.md); spec updated
claude-9 | Major | accepted-fix | claude | review-pr2-02/03-r2-claude.reply.md (FINDING claude-9) | fixed per contract r3.2 (29-pr2-fix3-codex.md); spec updated
claude-10 | Minor | accepted-fix | claude | review-pr2-02/03-r2-claude.reply.md (FINDING claude-10) | fixed per contract r3.2 (29-pr2-fix3-codex.md); spec updated
claude-11 | Minor | accepted-fix | claude | review-pr2-02/03-r2-claude.reply.md (FINDING claude-11) | fixed per contract r3.2 (29-pr2-fix3-codex.md); spec updated
claude-12 | Minor | open → fixed (awaiting finder) | claude | review-pr2-02/03-r2-claude.reply.md (FINDING claude-12) | fixed per contract r3.2 (29-pr2-fix3-codex.md); spec updated
claude-13 | Major | open → fixed (awaiting finder) | claude | review-pr2-02/03-r2-claude.reply.md (FINDING claude-13) | fixed per contract r3.2 (29-pr2-fix3-codex.md); spec updated
claude-14 | Minor | open → fixed (awaiting finder) | claude | review-pr2-02/03-r2-claude.reply.md (FINDING claude-14) | fixed per contract r3.2 (29-pr2-fix3-codex.md); spec updated
claude-15 | Minor | open → fixed (awaiting finder) | claude | review-pr2-02/03-r2-claude.reply.md (FINDING claude-15) | fixed per contract r3.2 (29-pr2-fix3-codex.md); spec updated
codex-1 | Major | accepted-fix | codex | review-pr2-02/04-r2-codex.reply.md (FINDING codex-1) | fixed per contract r3.2 (29-pr2-fix3-codex.md); spec updated
codex-2 | Major | accepted-fix | codex | review-pr2-02/04-r2-codex.reply.md (FINDING codex-2) | fixed per contract r3.2 (29-pr2-fix3-codex.md); spec updated
codex-3 | Minor | accepted-fix | codex | review-pr2-02/04-r2-codex.reply.md (FINDING codex-3) | fixed per contract r3.2 (29-pr2-fix3-codex.md); spec updated
codex-4 | Major | open → fixed (awaiting finder) | codex | review-pr2-02/04-r2-codex.reply.md (FINDING codex-4) | fixed per contract r3.2 (29-pr2-fix3-codex.md); spec updated
codex-5 | Minor | open → fixed (awaiting finder) | codex | review-pr2-02/04-r2-codex.reply.md (FINDING codex-5) | fixed per contract r3.2 (29-pr2-fix3-codex.md); spec updated

## PR #28 (S1-PR2), review `review-pr2-03` on `19ae5d70d`

Codex accepted all its findings. Claude's open items: claude-13 (raised against a head before the
macOS fixes) and claude-16 (Minor).

ID | severity | status | finder | source | lead's note
--- | --- | --- | --- | --- | ---
claude-1 | Major | accepted-fix | claude | review-pr2-03/03-r2-claude.reply.md (FINDING claude-1) | —
claude-2 | Major | accepted-fix | claude | review-pr2-03/03-r2-claude.reply.md (FINDING claude-2) | —
claude-3 | Major | accepted-fix | claude | review-pr2-03/03-r2-claude.reply.md (FINDING claude-3) | —
claude-4 | Major | accepted-fix | claude | review-pr2-03/03-r2-claude.reply.md (FINDING claude-4) | —
claude-5 | Minor | accepted-fix | claude | review-pr2-03/03-r2-claude.reply.md (FINDING claude-5) | —
claude-6 | Minor | accepted-fix | claude | review-pr2-03/03-r2-claude.reply.md (FINDING claude-6) | —
claude-7 | Minor | accepted-fix | claude | review-pr2-03/03-r2-claude.reply.md (FINDING claude-7) | —
claude-8 | Minor | accepted-fix | claude | review-pr2-03/03-r2-claude.reply.md (FINDING claude-8) | —
claude-9 | Major | accepted-fix | claude | review-pr2-03/03-r2-claude.reply.md (FINDING claude-9) | —
claude-10 | Minor | accepted-fix | claude | review-pr2-03/03-r2-claude.reply.md (FINDING claude-10) | —
claude-11 | Minor | accepted-fix | claude | review-pr2-03/03-r2-claude.reply.md (FINDING claude-11) | —
claude-12 | Minor | accepted-fix | claude | review-pr2-03/03-r2-claude.reply.md (FINDING claude-12) | —
claude-13 | Major | open → fixed (awaiting finder) | claude | review-pr2-03/03-r2-claude.reply.md (FINDING claude-13) | macOS fallback landed (a504fa9e0, bff778d98) and the libc++ multimap fix (f7aa68ebe); macos-15 CI passes at f7aa68ebe
claude-14 | Minor | accepted-fix | claude | review-pr2-03/03-r2-claude.reply.md (FINDING claude-14) | —
claude-15 | Minor | accepted-fix | claude | review-pr2-03/03-r2-claude.reply.md (FINDING claude-15) | —
claude-16 | Minor | open → fixed (awaiting finder) | claude | review-pr2-03/03-r2-claude.reply.md (FINDING claude-16) | long section names trigger the fallback; spec documents /~section-<n>, header rule, report budget (64 bytes per item, platform-independent)
codex-1 | Major | accepted-fix | codex | review-pr2-03/04-r2-codex.reply.md (FINDING codex-1) | —
codex-2 | Major | accepted-fix | codex | review-pr2-03/04-r2-codex.reply.md (FINDING codex-2) | —
codex-3 | Minor | accepted-fix | codex | review-pr2-03/04-r2-codex.reply.md (FINDING codex-3) | —
codex-4 | Major | accepted-fix | codex | review-pr2-03/04-r2-codex.reply.md (FINDING codex-4) | —
codex-5 | Minor | accepted-fix | codex | review-pr2-03/04-r2-codex.reply.md (FINDING codex-5) | —

## PR #28 (S1-PR2), review `review-pr2-04` on `d610e03a1`: both peers approve

Open Minors go to S1-PR3: exponent clamping and a number-length limit, locale and underflow
tests, an isolated test for the long-section-name trigger, and two stale lines in section 15.

ID | severity | status | finder | source | lead's note
--- | --- | --- | --- | --- | ---
claude-1 | Major | accepted-fix | claude | review-pr2-04/03-r2-claude.reply.md (FINDING claude-1) | -
claude-2 | Major | accepted-fix | claude | review-pr2-04/03-r2-claude.reply.md (FINDING claude-2) | -
claude-3 | Major | accepted-fix | claude | review-pr2-04/03-r2-claude.reply.md (FINDING claude-3) | -
claude-4 | Major | accepted-fix | claude | review-pr2-04/03-r2-claude.reply.md (FINDING claude-4) | -
claude-5 | Minor | accepted-fix | claude | review-pr2-04/03-r2-claude.reply.md (FINDING claude-5) | -
claude-6 | Minor | accepted-fix | claude | review-pr2-04/03-r2-claude.reply.md (FINDING claude-6) | -
claude-7 | Minor | accepted-fix | claude | review-pr2-04/03-r2-claude.reply.md (FINDING claude-7) | -
claude-8 | Minor | accepted-fix | claude | review-pr2-04/03-r2-claude.reply.md (FINDING claude-8) | -
claude-9 | Major | accepted-fix | claude | review-pr2-04/03-r2-claude.reply.md (FINDING claude-9) | -
claude-10 | Minor | accepted-fix | claude | review-pr2-04/03-r2-claude.reply.md (FINDING claude-10) | -
claude-11 | Minor | accepted-fix | claude | review-pr2-04/03-r2-claude.reply.md (FINDING claude-11) | -
claude-12 | Minor | accepted-fix | claude | review-pr2-04/03-r2-claude.reply.md (FINDING claude-12) | -
claude-13 | Major | accepted-fix | claude | review-pr2-04/03-r2-claude.reply.md (FINDING claude-13) | -
claude-14 | Minor | accepted-fix | claude | review-pr2-04/03-r2-claude.reply.md (FINDING claude-14) | -
claude-15 | Minor | accepted-fix | claude | review-pr2-04/03-r2-claude.reply.md (FINDING claude-15) | -
claude-16 | Minor | accepted-fix | claude | review-pr2-04/03-r2-claude.reply.md (FINDING claude-16) | -
claude-17 | Minor | open (Minor follow-up in S1-PR3) | claude | review-pr2-04/03-r2-claude.reply.md (FINDING claude-17) | -
claude-18 | Minor | open (Minor follow-up in S1-PR3) | claude | review-pr2-04/03-r2-claude.reply.md (FINDING claude-18) | -
codex-1 | Major | accepted-fix | codex | review-pr2-04/04-r2-codex.reply.md (FINDING codex-1) | -
codex-2 | Major | accepted-fix | codex | review-pr2-04/04-r2-codex.reply.md (FINDING codex-2) | -
codex-3 | Minor | accepted-fix | codex | review-pr2-04/04-r2-codex.reply.md (FINDING codex-3) | -
codex-4 | Major | accepted-fix | codex | review-pr2-04/04-r2-codex.reply.md (FINDING codex-4) | -
codex-5 | Minor | accepted-fix | codex | review-pr2-04/04-r2-codex.reply.md (FINDING codex-5) | -

## PR #32 (S1-PR3), review `review-pr3-01` on `21489bd03`

Fixed in the next head per contract r2 (items 1-10) and r3: two-pass identity, `themes` only at the
config root, symlinked roots, listing errors, the `count` field, linear scene-UUID extraction,
collapse accounting, profiles need `basic.ini`, too-large files.

ID | severity | status | finder | source | lead's note
--- | --- | --- | --- | --- | ---
claude-1 | Major | open → fixed (awaiting finder) | claude | review-pr3-01/03-r2-claude.reply.md (FINDING claude-1) | contract r2/r3
claude-2 | Major | open → fixed (awaiting finder) | claude | review-pr3-01/03-r2-claude.reply.md (FINDING claude-2) | contract r2/r3
claude-3 | Major | open → fixed (awaiting finder) | claude | review-pr3-01/03-r2-claude.reply.md (FINDING claude-3) | contract r2/r3
claude-4 | Major | open → fixed (awaiting finder) | claude | review-pr3-01/03-r2-claude.reply.md (FINDING claude-4) | contract r2/r3
claude-5 | Major | open → fixed (awaiting finder) | claude | review-pr3-01/03-r2-claude.reply.md (FINDING claude-5) | contract r2/r3
claude-6 | Minor | open → fixed (awaiting finder) | claude | review-pr3-01/03-r2-claude.reply.md (FINDING claude-6) | contract r2/r3
claude-7 | Minor | open → fixed (awaiting finder) | claude | review-pr3-01/03-r2-claude.reply.md (FINDING claude-7) | contract r2/r3
claude-8 | Minor | open → fixed (awaiting finder) | claude | review-pr3-01/03-r2-claude.reply.md (FINDING claude-8) | contract r2/r3
claude-9 | Minor | open → fixed (awaiting finder) | claude | review-pr3-01/03-r2-claude.reply.md (FINDING claude-9) | contract r2/r3
claude-10 | Minor | open → fixed (awaiting finder) | claude | review-pr3-01/03-r2-claude.reply.md (FINDING claude-10) | contract r2/r3
claude-11 | Minor | open → fixed (awaiting finder) | claude | review-pr3-01/03-r2-claude.reply.md (FINDING claude-11) | contract r2/r3
codex-1 | Major | withdrawn | codex | review-pr3-01/04-r2-codex.reply.md (FINDING codex-1) | contract r2/r3
codex-2 | Major | open → fixed (awaiting finder) | codex | review-pr3-01/04-r2-codex.reply.md (FINDING codex-2) | contract r2/r3
codex-3 | Major | open → fixed (awaiting finder) | codex | review-pr3-01/04-r2-codex.reply.md (FINDING codex-3) | contract r2/r3
codex-4 | Minor | withdrawn | codex | review-pr3-01/04-r2-codex.reply.md (FINDING codex-4) | contract r2/r3

## PR #32 (S1-PR3), review `review-pr3-02` on `5827f23dc`

Fixed per contract r4: `windows.h` with NOMINMAX (a Major the first Windows CI run also hit), listing
errors on every path, no silent drops, owned subpaths only, linear merges and the 256 MiB budget in plan
order. Spec sections 4, 7, 8, 13 updated in this head.

ID | severity | status | finder | source | lead's note
--- | --- | --- | --- | --- | ---
claude-1 | Major | accepted-fix | claude | review-pr3-02/03-r2-claude.reply.md (FINDING claude-1) | contract r4 and the Windows fix
claude-2 | Major | accepted-fix | claude | review-pr3-02/03-r2-claude.reply.md (FINDING claude-2) | contract r4 and the Windows fix
claude-3 | Major | accepted-fix | claude | review-pr3-02/03-r2-claude.reply.md (FINDING claude-3) | contract r4 and the Windows fix
claude-4 | Major | accepted-fix | claude | review-pr3-02/03-r2-claude.reply.md (FINDING claude-4) | contract r4 and the Windows fix
claude-5 | Major | accepted-fix | claude | review-pr3-02/03-r2-claude.reply.md (FINDING claude-5) | contract r4 and the Windows fix
claude-6 | Minor | accepted-fix | claude | review-pr3-02/03-r2-claude.reply.md (FINDING claude-6) | contract r4 and the Windows fix
claude-7 | Minor | open → fixed (awaiting finder) | claude | review-pr3-02/03-r2-claude.reply.md (FINDING claude-7) | contract r4 and the Windows fix
claude-8 | Minor | accepted-fix | claude | review-pr3-02/03-r2-claude.reply.md (FINDING claude-8) | contract r4 and the Windows fix
claude-9 | Minor | open → fixed (awaiting finder) | claude | review-pr3-02/03-r2-claude.reply.md (FINDING claude-9) | contract r4 and the Windows fix
claude-10 | Minor | accepted-fix | claude | review-pr3-02/03-r2-claude.reply.md (FINDING claude-10) | contract r4 and the Windows fix
claude-11 | Minor | accepted-fix | claude | review-pr3-02/03-r2-claude.reply.md (FINDING claude-11) | contract r4 and the Windows fix
claude-12 | Major | open → fixed (awaiting finder) | claude | review-pr3-02/03-r2-claude.reply.md (FINDING claude-12) | contract r4 and the Windows fix
claude-13 | Minor | open → fixed (awaiting finder) | claude | review-pr3-02/03-r2-claude.reply.md (FINDING claude-13) | contract r4 and the Windows fix
claude-14 | Minor | open → fixed (awaiting finder) | claude | review-pr3-02/03-r2-claude.reply.md (FINDING claude-14) | contract r4 and the Windows fix
claude-15 | Minor | open → fixed (awaiting finder) | claude | review-pr3-02/03-r2-claude.reply.md (FINDING claude-15) | contract r4 and the Windows fix
claude-16 | Minor | open → fixed (awaiting finder) | claude | review-pr3-02/03-r2-claude.reply.md (FINDING claude-16) | contract r4 and the Windows fix
codex-1 | Major | withdrawn | codex | review-pr3-02/04-r2-codex.reply.md (FINDING codex-1) | contract r4 and the Windows fix
codex-2 | Major | open → fixed (awaiting finder) | codex | review-pr3-02/04-r2-codex.reply.md (FINDING codex-2) | contract r4 and the Windows fix
codex-3 | Major | accepted-fix | codex | review-pr3-02/04-r2-codex.reply.md (FINDING codex-3) | contract r4 and the Windows fix
codex-4 | Minor | withdrawn | codex | review-pr3-02/04-r2-codex.reply.md (FINDING codex-4) | contract r4 and the Windows fix

## Review run 03 (pr32)

Both peers: request changes (Claude: Major claude-17, fixed in 0190edd89; Minors claude-13 and claude-14 carried to S1-PR4).

ID | severity | status | finder | source | lead's note
--- | --- | --- | --- | --- | ---
claude-1 | Major | accepted-fix | claude | review-pr32-03/03-r2-claude.reply.md (FINDING claude-1) | -
claude-2 | Major | accepted-fix | claude | review-pr32-03/03-r2-claude.reply.md (FINDING claude-2) | -
claude-3 | Major | accepted-fix | claude | review-pr32-03/03-r2-claude.reply.md (FINDING claude-3) | -
claude-4 | Major | accepted-fix | claude | review-pr32-03/03-r2-claude.reply.md (FINDING claude-4) | -
claude-5 | Major | accepted-fix | claude | review-pr32-03/03-r2-claude.reply.md (FINDING claude-5) | -
claude-6 | Minor | accepted-fix | claude | review-pr32-03/03-r2-claude.reply.md (FINDING claude-6) | -
claude-7 | Minor | accepted-fix | claude | review-pr32-03/03-r2-claude.reply.md (FINDING claude-7) | -
claude-8 | Minor | accepted-fix | claude | review-pr32-03/03-r2-claude.reply.md (FINDING claude-8) | -
claude-9 | Minor | accepted-fix | claude | review-pr32-03/03-r2-claude.reply.md (FINDING claude-9) | -
claude-10 | Minor | accepted-fix | claude | review-pr32-03/03-r2-claude.reply.md (FINDING claude-10) | -
claude-11 | Minor | accepted-fix | claude | review-pr32-03/03-r2-claude.reply.md (FINDING claude-11) | -
claude-12 | Major | accepted-fix | claude | review-pr32-03/03-r2-claude.reply.md (FINDING claude-12) | -
claude-13 | Minor | open → fixed (awaiting finder) | claude | review-pr32-03/03-r2-claude.reply.md (FINDING claude-13) | -
claude-14 | Minor | open → fixed (awaiting finder) | claude | review-pr32-03/03-r2-claude.reply.md (FINDING claude-14) | -
claude-15 | Minor | accepted-fix | claude | review-pr32-03/03-r2-claude.reply.md (FINDING claude-15) | -
claude-16 | Minor | accepted-fix | claude | review-pr32-03/03-r2-claude.reply.md (FINDING claude-16) | -
claude-17 | Major | open → fixed (awaiting finder) | claude | review-pr32-03/03-r2-claude.reply.md (FINDING claude-17) | -
codex-1 | Major | withdrawn | codex | review-pr32-03/04-r2-codex.reply.md (FINDING codex-1) | -
codex-2 | Major | accepted-fix | codex | review-pr32-03/04-r2-codex.reply.md (FINDING codex-2) | -
codex-3 | Major | accepted-fix | codex | review-pr32-03/04-r2-codex.reply.md (FINDING codex-3) | -
codex-4 | Minor | withdrawn | codex | review-pr32-03/04-r2-codex.reply.md (FINDING codex-4) | -
codex-5 | Major | withdrawn | codex | review-pr32-03/04-r2-codex.reply.md (FINDING codex-5) | -

## Review run 04 (PR #32): both peers approve

Open Minors go to S1-PR4: a symlinked `basic.ini` (claude-13), `lstat` errors other than "missing" on app
dirs (claude-14), and the documented exception that an unreadable folder with an unrepresentable name
is reported only through a `listing-error` warning on its container (codex-6).

ID | severity | status | finder | source | lead's note
--- | --- | --- | --- | --- | ---
claude-1 | Major | accepted-fix | claude | review-pr32-04/03-r2-claude.reply.md (FINDING claude-1) | -
claude-2 | Major | accepted-fix | claude | review-pr32-04/03-r2-claude.reply.md (FINDING claude-2) | -
claude-3 | Major | accepted-fix | claude | review-pr32-04/03-r2-claude.reply.md (FINDING claude-3) | -
claude-4 | Major | accepted-fix | claude | review-pr32-04/03-r2-claude.reply.md (FINDING claude-4) | -
claude-5 | Major | accepted-fix | claude | review-pr32-04/03-r2-claude.reply.md (FINDING claude-5) | -
claude-6 | Minor | accepted-fix | claude | review-pr32-04/03-r2-claude.reply.md (FINDING claude-6) | -
claude-7 | Minor | accepted-fix | claude | review-pr32-04/03-r2-claude.reply.md (FINDING claude-7) | -
claude-8 | Minor | accepted-fix | claude | review-pr32-04/03-r2-claude.reply.md (FINDING claude-8) | -
claude-9 | Minor | accepted-fix | claude | review-pr32-04/03-r2-claude.reply.md (FINDING claude-9) | -
claude-10 | Minor | accepted-fix | claude | review-pr32-04/03-r2-claude.reply.md (FINDING claude-10) | -
claude-11 | Minor | accepted-fix | claude | review-pr32-04/03-r2-claude.reply.md (FINDING claude-11) | -
claude-12 | Major | accepted-fix | claude | review-pr32-04/03-r2-claude.reply.md (FINDING claude-12) | -
claude-13 | Minor | open (Minor, S1-PR4) | claude | review-pr32-04/03-r2-claude.reply.md (FINDING claude-13) | -
claude-14 | Minor | open (Minor, S1-PR4) | claude | review-pr32-04/03-r2-claude.reply.md (FINDING claude-14) | -
claude-15 | Minor | accepted-fix | claude | review-pr32-04/03-r2-claude.reply.md (FINDING claude-15) | -
claude-16 | Minor | accepted-fix | claude | review-pr32-04/03-r2-claude.reply.md (FINDING claude-16) | -
claude-17 | Major | accepted-fix | claude | review-pr32-04/03-r2-claude.reply.md (FINDING claude-17) | -
codex-1 | Major | withdrawn | codex | review-pr32-04/04-r2-codex.reply.md (FINDING codex-1) | -
codex-2 | Major | accepted-fix | codex | review-pr32-04/04-r2-codex.reply.md (FINDING codex-2) | -
codex-3 | Major | accepted-fix | codex | review-pr32-04/04-r2-codex.reply.md (FINDING codex-3) | -
codex-4 | Minor | withdrawn | codex | review-pr32-04/04-r2-codex.reply.md (FINDING codex-4) | -
codex-5 | Major | withdrawn | codex | review-pr32-04/04-r2-codex.reply.md (FINDING codex-5) | -
codex-6 | Minor | open (Minor, S1-PR4) | codex | review-pr32-04/04-r2-codex.reply.md (FINDING codex-6) | -

## PR #38 (S1-PR4a, `config_lock`), review `review-pr4a-01` on `c23b46031`

Run 01: both peers request changes. Claude found one Major (the lock-order sentence) and raised its
`config_save_safe` Minor to Major once Codex found the same root cause (codex-1, a pre-existing race
in libobs: `config_save_safe` read `config->file` before taking the lock). The lead fixed all of
them; run 02 on the fixed head checks the fixes.

ID | severity | status | finder | source | lead's note
--- | --- | --- | --- | --- | ---
claude-1 | Major | fixed (awaiting finder) | claude | review-pr4a-01/03-r2-claude.reply.md (FINDING claude-1) | header and sphinx now say: one fixed order relative to this lock, hold short, do not wait for another user of the config
claude-2 | Minor | fixed (awaiting finder) | claude | review-pr4a-01/03-r2-claude.reply.md (FINDING claude-2) | "any config_* function except config_close"; `config_open_defaults` and `config_num_sections` now lock
claude-3 | Major | fixed (awaiting finder) | claude | review-pr4a-01/03-r2-claude.reply.md (FINDING claude-3) | `config_save_safe` reads the file name under the lock; a 3-thread concurrent test fails before the fix (3 of 20 runs) and passes after (0 of 60)
claude-4 | Minor | fixed (awaiting finder) | claude | review-pr4a-01/03-r2-claude.reply.md (FINDING claude-4) | `%ignore config_lock/config_unlock` in obslua.i and obspython.i
claude-5 | Minor | fixed (awaiting finder) | claude | review-pr4a-01/03-r2-claude.reply.md (FINDING claude-5) | `add_test(NAME ... COMMAND target)`; unused event removed; the test is Linux-only evidence and says so
claude-6 | Minor | fixed (awaiting finder) | claude | review-pr4a-01/03-r2-claude.reply.md (FINDING claude-6) | stale statement and line citations replaced by function names
codex-1 | Major | fixed (awaiting finder) | codex | review-pr4a-01/04-r2-codex.reply.md (FINDING codex-1) | as claude-3
codex-2 | Minor | fixed (awaiting finder) | codex | review-pr4a-01/04-r2-codex.reply.md (FINDING codex-2) | as claude-2
codex-3 | Minor | fixed (awaiting finder) | codex | review-pr4a-01/04-r2-codex.reply.md (FINDING codex-3) | `os_atomic_load_bool` and `os_atomic_store_bool` for the stop flag
codex-4 | Minor | fixed (awaiting finder) | codex | review-pr4a-01/04-r2-codex.reply.md (FINDING codex-4) | as claude-1
codex-5 | Minor | fixed (awaiting finder) | codex | review-pr4a-01/04-r2-codex.reply.md (FINDING codex-5) | as claude-6
codex-6 | Minor | fixed (awaiting finder) | codex | review-pr4a-01/04-r2-codex.reply.md (FINDING codex-6) | as claude-2
codex-7 | Minor | fixed (awaiting finder) | codex | review-pr4a-01/04-r2-codex.reply.md (FINDING codex-7) | as claude-5
codex-8 | Minor | fixed (awaiting finder) | codex | review-pr4a-01/04-r2-codex.reply.md (FINDING codex-8) | as claude-6

### PR #38 run 02 (`review-pr4a-02`) on `e3ecb4179`: both peers approve

Every run 01 finding is `accepted-fix` by its finder (Claude and Codex both check each against the code). Three new
Minors, all fixed in the PR after the run: `config_save` tested `config->file` before taking the lock (claude-8,
codex-9), stale `config-file.c:` line citations in section 10 and PR2 (claude-7, codex-10), and the
concurrent test asserted `>= 0` where it can assert the final counter 399 (claude-9, codex-11).
## PR #36 (Unicode collision tables), review `review-pr36-01` on `804e22766`: both peers approve

Run 01 (lead authored; Claude first): both approve with Minors only. The lead fixed all of them in the
PR (each is "fixed, awaiting finder": the two peers do not re-review Minors). Evidence: Linux
(GCC, also ASan and UBSan), Ubuntu and Windows CI, macOS on `macos-15-intel` (probe branch, the arm64
pool had no runner).

ID | severity | status | finder | source | lead's note
--- | --- | --- | --- | --- | ---
claude-1 | Minor | fixed (awaiting finder) | claude | review-pr36-01/03-r2-claude.reply.md (FINDING claude-1) | CI regenerates the tables and `cmp`s them; conformance now checks every combining class against UnicodeData.txt
claude-2 | Minor | fixed (awaiting finder) | claude | review-pr36-01/03-r2-claude.reply.md (FINDING claude-2) | section 12 states the stability policy and that recovery quarantines, never deletes
claude-3 | Minor | fixed (awaiting finder) | claude | review-pr36-01/03-r2-claude.reply.md (FINDING claude-3) | "one root of a manifest"; section 15 names normalization-only clashes
claude-4 | Minor | fixed (awaiting finder) | claude | review-pr36-01/03-r2-claude.reply.md (FINDING claude-4) | NOTICE.md and task 002 record the license-text obligation for PR4
codex-1 | Minor | withdrawn | codex | review-pr36-01/04-r2-codex.reply.md (FINDING codex-1) | -
codex-2 | Minor | fixed (awaiting finder) | codex | review-pr36-01/04-r2-codex.reply.md (FINDING codex-2) | pairwise class-reordering test (1.94 million checks); a changed class value fails it
