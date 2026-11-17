# Worker brief (Claude, black-box cases): JeiOBS K1-PR2 (transform execution)

Your current directory is a JeiOBS worktree (detached HEAD, main with K1-PR1 merged). You write black-box
test cases for `jc-check transform` from `/tmp/jeiobs-k1pr2/contract.md` (the exact PR2 contract) WITHOUT
reading the new implementation (it is written in parallel by another worker; do not read
`shared/jeiobs-control/src/` or `include/` at all). You MAY read `shared/jeiobs-control/tests/cases/` (the
runner `run-cases.cmake`, the `cases.txt` header and an example of how mapping fixtures and value JSON are written).
Write ONLY under `/tmp/jeiobs-k1pr2/out/`:
- `cases.txt`: new case lines, ready to append to `shared/jeiobs-control/tests/cases/cases.txt` (same line format; names
  prefixed `transform-`), with an expected `EXPECTED` check for each case;
- `fixtures/`: new mapping fixtures and `INPUTS.json` files, named as the case lines reference them (relative to
  the cases directory, e.g. `fixtures/transform/NAME.json`);
- `notes.md`: one line per case: the contract rule and how the expected value was computed (use python3 for the
  arithmetic; round `%.9g` as the contract says).
Cover every stage with typical and boundary inputs and both sides of every rule: scale (a>b, c>d, unclamped
extrapolation, identity), curve linear/log/exp including clamping of out-of-range inputs and exact endpoints,
invert, clamp (min==max), deadzone (at the edge, negative inputs), quantize (steps 2, 3 with tie-to-even
examples, 128), threshold (at the edge), momentary (Trigger, Bool, 0, UInt codes), toggle (press, release, several
presses, `--toggled`, a following stage seeing the Bool, NoOutput ending the pipeline), relative in all three
encodings at widths 7 and 14 with positive, negative, zero and extreme codes, step sizes, clamping at 0 and 1,
`--position`, state carried across several inputs, join14 (valid, raw 128 range error, wrong shape, widths),
split14 (valid, wrong width), input conversions (UInt, Int, ieee32/64 floats incl. NaN and infinity as
`bits_hex`, Bool, Trigger, Text, Blob, Tuple where not allowed), the empty pipeline (identity, bitwise equal
incl. unit/relative metadata, any kind), non-finite results from huge scale factors, error elements not
changing state (an error in the middle of a state-carrying run), multi-stage pipelines, the position-rule
errors for `validate`/`compile`/`transform` (relative or join14 not first, split14 not last, both sides),
and usage/ERR forms (unknown route `ERR E_REF route`, bad INPUTS shape `ERR E_SCHEMA inputs`, 0 and 1001
inputs, `--position` and `--toggled`). Do not invent behavior the contract does not state; if a case would
depend on an unstated reading, leave it out and say so in notes.md. Keep it portable (the runner is `cmake -P`).
Allowed commands: Read/Write/Edit/Glob/Grep, `python3` (authoring only), `ls`. No git commits.
Final reply: the list of case groups, ambiguities, what you could not cover.
