# Worker brief (Claude, black-box cases): JeiOBS K1-PR1

Your current directory is a JeiOBS worktree (detached HEAD, main). Create ONLY
`shared/jeiobs-control/tests/cases/` (the library itself is written by another worker in parallel:
do NOT create or read anything else under `shared/jeiobs-control/`). Work from
`docs/jeiobs/tasks/006-control-protocols.md` and `/tmp/jeiobs-k1pr1/contract.md` (the exact PR1
contract: schema, errors, compile checks, `jc-check` CLI). Model the runner on
`shared/jeiobs-workspace/tests/cases/` (read `run-cases.cmake`, how it takes `-DJC_CHECK=... -DCASES_DIR=...
-DWORK_DIR=...`, `cases.txt` line format, expected files, generated large inputs, `.gitattributes`): write
`run-cases.cmake`, `cases.txt`, `fixtures/`, `expected/`, a short README line at the top of cases.txt, and a
`.gitattributes` with `* -text -whitespace`. The runner is invoked as `cmake -DJC_CHECK=<path to jc-check>
-DCASES_DIR=<this dir> -DWORK_DIR=<scratch> -P run-cases.cmake`, prints `PASS|FAIL <case> (<detail>)`, and
fails if any case failed.
Cases (both sides of every rule; one fault per fixture): a valid full mapping; every schema error
(bad format/major/minor, id pattern, duplicate ids, unknown slot ref, each SEG form valid and invalid,
exactly-one-key rule, range lo>hi, set of 0/65 items/duplicates, address of 0/16/17 segments, every PRED
and every transform op valid and with each parameter wrong/missing, every target kind valid and with a
missing/extra-wrong field, bad enums); JSON-level faults (duplicate key, invalid UTF-8, NUL, BOM, 1e400,
depth 33, 4 MiB+1, 10,001 routes, 257 slots, 17th transform); each compile check with positive and
negative variants: E_DENIED (every denied field, case variants, a similar allowed field), E_WILDCARD_PRIVILEGED
(any/range/set x output-control, profile, hotkey effect output/unknown vs none), E_FANOUT (64 vs 65, ranges
and sets multiplied), E_AMBIGUOUS_MERGE (single route; two without merge; two with different policies; same
policy with equal/distinct priorities; the mixer `field` distinguishing keys; disabled routes excluded),
E_CYCLE (self loop, two-route cycle, three-route, near miss by one segment, slot mismatch, disabled route
excluded); compile JSON output (key order, target_key canonical form, class of every kind, fanout);
`match` (slot null vs equal vs different, wildcard, range, set, text vs symbol, length mismatch, disabled
route, document order); `value` helpers (uint round-trips for widths 7, 14, 16, 32, 64 and 128 at the
extremes, int, ieee32/ieee64 bit patterns incl. NaN payloads and denormals via bits_hex, blob/text/tuple limits
at the boundaries, `double` of 2^53 and 2^53+1, uint value that does not fit its bits).
Keep it portable (cmake -P only, Windows/macOS/Linux); generate big inputs in WORK_DIR. Allowed commands:
Read/Write/Edit/Glob/Grep, `cmake`, `python3` (authoring only), `git status/diff/add/-c … commit`. Validate your
runner with a throwaway fake `jc-check` in /tmp (not committed). Commit on the detached HEAD (no branch, no
push): `git -c user.name=auto44e -c user.email=auto44e@gmail.com commit -m "shared: black-box cases for jeiobs-control (K1-PR1)"`.
Final reply: case list, ambiguities and how you read them, what you could not cover.
