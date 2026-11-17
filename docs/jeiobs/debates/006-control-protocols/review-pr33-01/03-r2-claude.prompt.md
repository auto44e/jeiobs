You are one of two independent reviewers (Claude and Codex, different model lineages) in a
cross-talk review for the JeiOBS repository. Your working directory is a checkout of exactly the
commit under review. The lead agent wrote or integrated the change and must NOT decide on its own
whether your findings are resolved: under docs/jeiobs/DECISIONS.md a finding is closed only when the
reviewer who raised it accepts the fix or withdraws it, in writing, in a later round. Use one
severity scale: Blocker, Major, Minor. Only the finder may lower a finding's severity. You are
read-only: never modify, create or delete files, and never run commands that change state.
Verify claims in the files rather than trusting the context. Be terse; cite file:line.

End your reply with one line per finding YOU raised (every ledger row with you as finder, every
finding you raised in an earlier round, plus new ones), then the verdict. If you raised none, write
FINDING none. Use exactly this syntax:
FINDING <ID> | <Blocker|Major|Minor> | <open|accepted-fix|withdrawn> | <one-line reason>
VERDICT: approve
or
VERDICT: request changes
For a new finding, use ID <your-name>-<n> (for example claude-1) and quote the evidence.

REVIEWED COMMIT: 1a8bd1eb4cbb54afb16eeb18d6b14a34ab82d9ad

TOPIC: Review run 01 of pr33.

SHARED CONTEXT:
# PR #33 review run 01 — K1-PR1 (jeiobs-control types, schema, validator, compiler), head 1a8bd1eb4

Your working directory is a checkout of this commit. Change: `git diff a0373dec8 HEAD`. Read docs/jeiobs/tasks/006-control-protocols.md (K1 design A1-A18, Progress) and docs/jeiobs/debates/006-control-protocols/07-k1pr1-contract.md (the contract with addendum r1), shared/jeiobs-control/ (src, include, tests incl. tests/cases with 916 case lines).
Authorship: Codex worker wrote the library; Claude worker wrote the black-box cases from the contract without reading the library; the lead wrote the contract, decided eight readings (r1) and replaced a recursive cycle check (stack overflow under ASan on a 10,000-route chain) with an iterative one.
Lead's evidence (Linux, GCC, -Wall -Wextra -Werror): jc-unit and jc-cases pass, also under ASan+UBSan. No CI job yet for this library.
Review for: contract and design fidelity (A1-A18), exactness of values (128-bit, floats, round trips), the hard-deny list and privileged-wildcard rule (can a mapping reach a secret field or start an output through a wildcard), fan-out and cycle checks (soundness and complexity), strict JSON parsing and limits, portability (MSVC: 128-bit integers are not native; Apple libc++), test validity, doc accuracy. Owner (verbatim): "if you peers are more or less in agreement, no problem self merging".

Severity: Blocker = must not merge; Major = must be fixed before a self-merge; Minor = follow-up.
Other agent work runs on this host in separate worktrees; it does not touch this checkout.

FINDINGS LEDGER (kept by the lead):
# Findings ledger

ID | severity | status | finder | source | lead's note
--- | --- | --- | --- | --- | ---

YOUR OWN PREVIOUS REPLY:
**Request changes: 5 Major and 5 Minor findings.** Several parts hold up: the portable 128-bit storage, rounding a 128-bit integer to a double, the fan-out product, the iterative Tarjan cycle search, and the byte-level JSON checks. The problems are value clamping, a second laxer schema, two ways a mapping can reach what A1 and A2 forbid, and a quadratic parser.

### Major

**claude-1: `FromDoubleInCodeSpace` wraps around instead of clamping just below the top code.**
- **Unsigned** (`Value.cpp:576-585`): it clamps against 2^bits (`if (clamped >= maximum_double)`), then masks the rounded value.
  - uint7 127.5 or 127.6 rounds to 128, and the mask gives **0**.
  - uint14 16383.5 and uint32 4294967295.5 do the same. Every width up to 52 is affected.
- **Signed** (`:588-601`): int8 127.5 gives magnitude 128, raw 0x80, so **−128**. Widths up to 53 are affected.
- **`MakeInt` accepts it.** `FitsSigned` (`:286-310`) only looks at the bits inside the width, so it never fails: `MakeInt(8,{0,300})` returns 44.
- The contract says "clamped, rounded to nearest, ties to even" (`07:21-22`).
- **Relative metadata is lost.** The function rebuilds the value through `Make*` (`:570-614`) and drops `relative_`. `operator==` compares `relative_` (`:676`), so the round trip fails for any value that carries it.
- **Why no test caught it:** no test passes a non-integral double.
  - `jc_unit.cpp:37` only tries codes 0..1023.
  - The CLI only exposes `roundtrip`.
  - jc-unit also skips the limits that exist only in the C++ API (blob, `WireEnvelope`, `Principal`, `WithRelative`), although `12:59` left the blob limit to it.
- **Fix:** round first, then clamp to 2^bits−1 (unsigned) or 2^(bits−1)−1 (signed). Make `FitsSigned` require a proper sign extension, keep `relative_`, and add tests with fractional inputs.

**claude-2: the public byte API still uses the pre-r1 schema.**
- `ParseMapping(bytes)` and `CompileMapping(bytes)` (`Mapping.hpp:186,190`) call `ParseMappingInternal(bytes, false)` (`Mapping.cpp:1473,1593`).
- In that mode:
  - `requires` is optional (`:862-864`).
  - A missing `enabled` becomes true (`:887-888`).
  - Empty `match` and `emit` addresses are accepted (`:1455`).
  - All three go against r1.3 and r1.4.
- Only `LoadMapping(path)` is strict (`:1486`). jc-cases run through the CLI, which calls it, so they never reach the lax mode.
- jc-unit tests only the lax mode, and with inputs r1 forbids (`jc_unit.cpp:22-31,85,107-108`: no `enabled`, `"address":[]`). The implementer's report calls this "Legacy compatibility for `jc-unit`" (`10:34`).
- Any later caller that already holds the bytes (workspace restore, import) gets the lax validator.
- `ValidateMapping(const Mapping&)` also skips requirement strings and empty addresses (`:1066-1115`).
- **Fix:** keep one strict parser and update the jc-unit inputs.

**claude-3: the mapping file itself decides a hotkey's class through its `effect` field.**
- `ClassOfTarget` takes the class from the declared effect (`Mapping.cpp:1255-1265`; contract `07:62,79-80`).
- Example: take the `wild-ok-any-hotkey-scene` fixture and set `name` to `OBSBasic.StartStreaming` (registered at `frontend/widgets/OBSBasic_Hotkeys.cpp:143`).
  - With a wildcard address it compiles, with class `scene`.
  - A1 grants `scene` to hw and loopback-net by default (`006:123-124`).
- So a wildcard route can start streaming. That breaks A2 ("classed by what it triggers", `006:127-128`) and A13 ("an imported mapping cannot widen trust", `006:209-210`).
- `cases.txt:649-651,657-659,665-667` pin this as allowed, and nothing says PR4 derives or checks the effect.
- **Fix:** let a declared effect only raise the class, never lower it. Until the adapter derives the effect from registerer and name, treat every hotkey as output-control for `E_WILDCARD_PRIVILEGED` and the policy table, and write that down as a PR4 requirement.

**claude-4: the hard-deny list misses the section 8 secrets that live in source settings, and the cases require `url` to be allowed.**
- A1 (`006:125-126`) and item 9 (`006:96-98`, reads as well as writes) deny "any WORKSPACE_FORMAT section 8 secret field".
- Section 8 (`WORKSPACE_FORMAT.md:350`) lists:
  - `browser_source` `url`;
  - `ffmpeg_source` `input` and `ffmpeg_options`;
  - `vlc_source` `playlist/*/value` (URL userinfo, SRT `passphrase` and `streamid`);
  - the query key `access_token`.
- None of these is in the contract's list (`07:75-77`) or in `IsSecretField` (`Mapping.cpp:981-982`). `cases.txt:605` requires a `source-property` with field `url` to compile.
- **Fix:** either deny these names and flip that case, or record that the PR4 adapter denies them per source type for reads and writes, and that the compile-time list is only part of A1's deny.

**claude-5: parsing a JSON object takes time quadratic in its number of members.**
- `using Json = nlohmann::ordered_json;` (`Mapping.cpp:34`). `BoundedSax` adds each member with `frame.value[key]` (`:147`), which is a linear search in nlohmann's `ordered_map`.
- Unknown members are ignored (`07:48`). So a single object with about 500,000 keys (`{"aaa":0,...}`) fits in 4 MiB and stays under `kJsonValues` (`Limits.hpp:31`).
- That is roughly 10^11 key comparisons. I estimate minutes; I could not run it.
- `jeiobs-workspace` uses `nlohmann::json` (`Manifest.hpp:29`), and nothing here needs member order: duplicate keys are tracked in their own set (`:92,199`).
- **Fix:** switch to `nlohmann::json` and add a test with a very wide object.

### Minor

- **claude-6: `target_key` can give two different targets the same key.**
  - `AddressKey` joins segments with an unescaped `/` (`Mapping.cpp:1054-1064`). Emit addresses `[{"t":"a/i:1"}]` and `[{"t":"a"},{"i":1}]` both become `emit|address=t:a/i:1`. That produces a false `E_AMBIGUOUS_MERGE`, against r1.5 (`07:119`).
  - `\` is not escaped either (`:1328`), although r1.5 says it is. The expected output agrees with the code, not the contract (`expected/compile/key-unicode-and-escapes.json:12`); Codex reported this at `10:43`.
- **claude-7: the documents disagree with what was built.**
  - r1.3 allows only `[a-z0-9_.]` in requirements, but `IsRequirement` accepts any byte ≥0x80 (`:337-349`), and a fixture uses `"café"` (`fixtures/valid/utf8-raw.json:6`).
  - r1.3 promises `E_SCHEMA <field>`, but a missing `enabled` gives `E_SCHEMA routes` (`:889-890`).
  - The Progress note (`006:332-333`) says the cases decided the r1 readings, yet r1.3 and r1.5 contradict the cases.
  - `value limits` reports a top-level `"-1"` as `E_LIMIT` but the same value nested as `E_VALUE` (`jc-check.cpp:362-375`), against r1.8.
  - The contract doesn't mention the 1,000,000-value and 64 KiB-string caps (`Limits.hpp:30-31`), or that compile errors are ordered by check number first.
  - A11 has a prefix match (`006:192`) and says denied routes are "inert and listed" (`006:199`). The contract has no prefix, and `E_DENIED` fails the whole file. Neither change is recorded.
  - `006:31` ("Nothing is built yet") and the "Open before K1-PR1" list (`006:318-323`) are out of date.
- **claude-8: whether the cycle check is sound depends on emit semantics nobody has decided.**
  - The edge rule (`07:85-87`, `Mapping.cpp:1128-1131`) is only sound if a re-injected event carries the slot of the route that emitted it.
  - If PR3 keeps the triggering event's slot instead, route A (slot s1, `[a]` emits `[b]`) and route B (slot null, `[b]` emits `[a]`) loop at runtime, and the check sees no B→A edge.
  - Classing emit as `meta` (`:1275-1276`) is also unrecorded. Only that class stops a wildcard route from emitting into a concrete output route. A1 restricts `meta` to local-ui, so someone is likely to change it.
  - The case author flagged both points (`12:52-53`).
- **claude-9: the merge and cycle checks are quadratic.**
  - Every route in a merge group re-checks the whole group (`:1541-1562`). 10,000 routes on one target with distinct priorities is a valid file and costs 10^8 set inserts.
  - The cycle graph also stores edges to non-emit routes, which cannot be on a cycle (`:1121-1136`). 5,000 emit routes plus 5,000 routes matching their address (about 1.3 MB of file) store 25 million edges, at least 200 MB. Codex flagged this at `10:23`.
- **claude-10: `jc-check` reads its arguments in the ANSI code page on Windows.**
  - It uses `main(int, char**)` (`jc-check.cpp:411`) and passes `argv` to `u8path` (`:50`).
  - So `io-validate-utf8-path` (`cases.txt:52`) can't pass on Windows.
  - `jw-check.cpp:409-441` uses `wmain` for this, and the contract says to copy its portability rules (`07:6-8`).

FINDING claude-1 | Major | open | FromDoubleInCodeSpace wraps instead of clamping (uint7 127.5→0, int8 127.5→−128, Value.cpp:576-601); FitsSigned never rejects; relative_ dropped; no fractional-input tests
FINDING claude-2 | Major | open | ParseMapping/CompileMapping(bytes) use a lax pre-r1 schema (Mapping.cpp:862-888,1455,1473); only LoadMapping is strict; jc-unit tests only the lax path
FINDING claude-3 | Major | open | hotkey class comes from the file's declared effect (Mapping.cpp:1255-1265): OBSBasic.StartStreaming with effect "scene" takes wildcards and a default-granted class, against A2/A13
FINDING claude-4 | Major | open | hard deny omits section 8 source secrets (browser url, ffmpeg input/ffmpeg_options, vlc playlist, access_token) required by A1/item 9; cases.txt:605 pins url allowed
FINDING claude-5 | Major | open | ordered_json member insert is a linear scan (Mapping.cpp:34,147): Θ(n²) object parse, ~500k-key unknown member fits in 4 MiB
FINDING claude-6 | Minor | open | emit target_key not injective (unescaped "/" in AddressKey, Mapping.cpp:1054-1064); "\" not escaped against r1.5 (:1328)
FINDING claude-7 | Minor | open | r1.3/r1.5/r1.8 and Progress disagree with code and cases; unrecorded caps, check order, A11 prefix/inert-denied deviations; stale 006:31, 006:318
FINDING claude-8 | Minor | open | cycle edge rule sound only if emit keeps the emitting route's slot; load-bearing emit=meta unrecorded (12:52-53)
FINDING claude-9 | Minor | open | merge check O(g²) with set inserts (Mapping.cpp:1541-1562); cycle graph keeps edges to non-emit routes, 25M edges from ~1.3 MB (:1121-1136)
FINDING claude-10 | Minor | open | jc-check uses ANSI argv on Windows (jc-check.cpp:411), unlike jw-check's wmain; io-validate-utf8-path cannot pass there
VERDICT: request changes

THE OTHER REVIEWER'S LATEST REPLY:
FINDING codex-1 | Major | open | Public `"ParseMappingInternal(bytes, false)"` permits missing `enabled` and empty addresses, contrary to r1 requirements (Mapping.cpp:878-912,1455,1470-1473; 07-k1pr1-contract.md:115-118).
FINDING codex-2 | Minor | open | `"has_non_ascii"` makes `requires:["café"]` valid despite the required `[a-z0-9_.]{1,64}` regex (Mapping.cpp:332-349; 07-k1pr1-contract.md:115-116).
FINDING codex-3 | Major | open | Target-key escaping omits `\`, and slash-joined emit addresses collide (`[t:"a/s:b"]` equals `[t:"a",s:"b"]`), violating “distinct targets never share a key” (Mapping.cpp:1041-1063,1324-1333,1378-1380,1513-1516; 07-k1pr1-contract.md:119).
FINDING codex-4 | Major | open | `MakeInt` masks invalid high raw bits after validation—width-7 raw `0x80` becomes zero—and `MakeFloatBits` truncates 32-bit raw values, violating exact constructors (Value.cpp:234-243,286-310,424-446; 07-k1pr1-contract.md:20-34).
VERDICT: request changes