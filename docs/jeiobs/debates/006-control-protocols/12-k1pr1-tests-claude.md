I committed the black-box cases as `40dba6c36`, "shared: black-box cases for jeiobs-control (K1-PR1)", on the detached HEAD. The commit holds only `shared/jeiobs-control/tests/cases/`: 680 files, which are `run-cases.cmake`, `cases.txt` with 916 cases, the fixtures, 13 expected files and `.gitattributes`. The working tree is clean.

**What I ran:** I tested the runner against a throwaway Python model of the contract I wrote in `/tmp/jeiobs-k1pr1-cases` (not committed).
- All 916 cases pass with my readings of the contract.
- All 916 also pass with every open point read the other way (`JC_REF_ALT=1`), so each "accept either" case really does accept both.
- Bugs I deliberately added to the model were caught: a fan-out product that wraps at 64 bits, Unicode lower-casing in the deny check (Kelvin sign), and grouping merges by the printed `target_key` string. A fake that always prints `OK` fails 684 cases.
- The model also caught four mistakes in my own fixtures before the commit.
- **Not run:** the real `jc-check` (the other worker is writing it), and Windows or macOS.

## Cases

| Section | Cases |
|---|---|
| usage (exit 2), files and paths (missing file, directory, `/dev/null`, non-ASCII path, path with a space) | 21 |
| valid mappings (full mapping, unknown fields everywhere, future minor, CRLF and tabs, raw UTF-8, escapes, number edges, id lengths) | 14 |
| JSON faults (duplicate keys incl. escaped, NUL raw and `\u0000`, `1e400` and others, 8 invalid UTF-8 forms, BOM, syntax errors) | 46 |
| limits (4 MiB, depth 32/33/100000, 256/257 slots, 10,000/10,001 routes, 16/17 transforms, 16/17 segments, 256/257-byte segments) and timing (10,000 × 64-wide sets, 10,000-route chain and cycle) | 31 |
| schema: top level, slots, routes, document order | 70 |
| schema: segments (every form valid and invalid, exactly-one-key) | 60 |
| schema: guard predicates, transforms (every op, every parameter wrong or missing), targets (every kind, enum and field), merge | 234 |
| E_DENIED: 12 fields × 2 kinds, case variants, 27 allowed look-alikes and other kinds | 62 |
| E_WILDCARD_PRIVILEGED: any/r/in × privileged and plain targets | 46 |
| E_FANOUT: 64 vs 65, products, 64-bit wrap traps | 21 |
| E_AMBIGUOUS_MERGE | 38 |
| E_CYCLE | 24 |
| compile output (exact bytes and key order, `target_key`, class of each kind, fanout) | 30 |
| match | 48 |
| value helpers | 171 |

## Readings I fixed (cases fail if the implementer reads these differently)
1. Scene-item `item_id` is a JSON integer.
2. The mixer `target_key` includes `field` (`mixer|field=volume|id=mic`).
3. Classes: a hotkey is classed by its effect (none→hotkey, scene→scene, mixer→mix, output/unknown→output-control). Egress prints `egress-by-trust`, graph is `mix`, transition and scene-item are `scene`, plugin-event is `plugin-param`.
4. Members the contract doesn't mark optional are required: `enabled`, `match.slot` (may be null) and `requires`. A guard must have `all`.
5. A 0-segment address is `E_SCHEMA address`. The contract sets no minimum, but your brief lists it as an error.
6. "Exactly one key" beats "unknown fields are ignored": `{"s":"a","x":1}` is an error.
7. Validation runs once in document order, so an `E_DUP` or `E_REF` in an earlier route wins over a schema error in a later one.
8. Depth: the top-level object is depth 1 (as in jeiobs-workspace). A tuple of scalars is depth 1, so 4 nested tuples are OK and 5 are not.
9. Value round trips are done in code space with clamping. The maximum u64, i64 and u128 values round-trip; 2^53+1 does not. 128-bit to double must round correctly; one case catches double rounding.

## Accepted either way
- `E_LIMIT` or `E_SCHEMA` for the count limits, 17-segment addresses and 257-byte segments.
- `E_SCHEMA` detail as the field or its container (e.g. `id` or `slots`, `op` or `transforms`).
- `E_MAJOR` or `E_SCHEMA major` for a missing or non-integer major.
- Other parse codes: NaN literal is `E_JSON` or `E_NONFINITE`; a lone surrogate escape is `E_JSON` or `E_UTF8`; `/dev/null` is `E_IO` or `E_JSON`; a bad event file is its own code or `E_SCHEMA event`.
- Whether hotkey `effect`, output `action` and egress `trust` belong in `target_key`: two expected files each.
- Codes with no detail in the contract match `ERR CODE( .+)?`.
- Round trips of NaN and infinity, and how NaN prints.

## Gaps in the contract for you to decide
1. `target_key` has no escaping. `source-property{id:"z",field:"x|id=y"}` and `{id:"y|id=z",field:"x"}` print the same key. My case treats them as different targets.
2. `emit` has no capability class (A15 has no row for it). If it is `meta`, every wildcard emit route is rejected; the wildcard cycle cases accept either error.
3. The cycle rule misses A (slot s1) → B (slot null) → A, which can loop at runtime. Not tested.
4. If `action` isn't an identity field, a start route and a stop route on the same output need merge policies. Not tested.
5. The order of the `slots` list in compile output isn't specified; my expected files use ids where document order equals sorted order.
6. Which error wins across different compile checks, multiple cycles or multiple merge groups isn't specified. Not tested.

## Not covered
- The blob limit (65,536 vs 65,537 bytes) needs a 128 KiB argument, more than a Windows command line or a single Linux argument can hold. It needs `jc-unit` or a file input.
- The `meta` class on its own: no PR1 target kind is surely `meta`.
- Malformed value JSON (bad hex and similar), whose error code isn't specified.
- Strict timing: there is only a per-case timeout, 120 s by default (`JC_CASE_TIMEOUT`).

**Process notes:**
- Besides the allowed commands I used read-only shell tools (`ls`, `cat`, `grep`, `sed -n`, `od`), plus `mkdir`/`cp`/`rm` in /tmp scratch directories and to copy files into the case directory.
- I read your implementer brief in /tmp but not `worker-impl.log`, and nothing else under `shared/jeiobs-control/`.
- The authoring script that generated the fixtures is in /tmp and not committed.
