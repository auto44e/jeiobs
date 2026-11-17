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
FINDING codex-1 | Major | open | Public `"ParseMappingInternal(bytes, false)"` permits missing `enabled` and empty addresses, contrary to r1 requirements (Mapping.cpp:878-912,1455,1470-1473; 07-k1pr1-contract.md:115-118).
FINDING codex-2 | Minor | open | `"has_non_ascii"` makes `requires:["café"]` valid despite the required `[a-z0-9_.]{1,64}` regex (Mapping.cpp:332-349; 07-k1pr1-contract.md:115-116).
FINDING codex-3 | Major | open | Target-key escaping omits `\`, and slash-joined emit addresses collide (`[t:"a/s:b"]` equals `[t:"a",s:"b"]`), violating “distinct targets never share a key” (Mapping.cpp:1041-1063,1324-1333,1378-1380,1513-1516; 07-k1pr1-contract.md:119).
FINDING codex-4 | Major | open | `MakeInt` masks invalid high raw bits after validation—width-7 raw `0x80` becomes zero—and `MakeFloatBits` truncates 32-bit raw values, violating exact constructors (Value.cpp:234-243,286-310,424-446; 07-k1pr1-contract.md:20-34).
VERDICT: request changes
THE OTHER REVIEWER'S LATEST REPLY:
**Still request changes.** The head hasn't changed (1a8bd1eb4), so all ten of my findings still apply. I checked all four of Codex's findings against the code and they hold. I'm raising claude-6 to Major to match codex-3.

**Codex's findings**
- **codex-1: confirmed.** It is the same defect as claude-2. `ParseMapping` and `CompileMapping(bytes)` call `ParseMappingInternal(bytes, false)` (`Mapping.cpp:1473,1593`). That mode:
  - doesn't require `requires` (`:862-864`);
  - turns a missing `enabled` into true (`:887-888`);
  - accepts empty addresses (`:1455`).
- **codex-2: confirmed, and Minor is right.** It is part of claude-7. `IsRequirement` lets every byte ≥0x80 through (`Mapping.cpp:337-349`). The implementer kept this so that `valid-utf8-raw` passes (`10:43`).
- **codex-3: confirmed with its own example.**
  - `[t:"a/s:b"]` and `[t:"a"],[s:"b"]` both give `emit|address=t:a/s:b` (`Mapping.cpp:1054-1064`), and `\` is not escaped (`:1328`).
  - The merge check groups routes by this string (`:1535-1539`), so the merge check is only right if two different targets never get the same key.
  - The `@` case keeps the unescaped `\` as expected output (`cases.txt:795`, `expected/compile/key-unicode-and-escapes.json:12`).
  - So I'm raising claude-6 to Major.
- **codex-4: confirmed.**
  - `FitsSigned` only reads the bits inside the width, so it can never return false (`Value.cpp:286-310`). `MakeInt(7,{0,0x80})` passes and stores 0 (`:435`). This is the same root cause as claude-1.
  - `MakeFloatBits(Ieee32, 0x1'0000'0000)` quietly becomes +0.0f instead of an error (`:444-446`). I hadn't found that part; I agree with it.

**New evidence for my own findings (same IDs)**
- **claude-1:**
  - Any value that a transform turns into 127.5 or more (but under 128) is sent as uint7 **0**. A motor fader at the top of its travel would drop to the bottom.
  - `MakeInt` stores the bits masked to the width: for int8 −1, `intRaw()` is 0xff (`Value.cpp:435`). The contract says "raw (signed 128)" (`07:18`). The fix should pick one stored form and document it.
- **claude-3:**
  - `effect` is not part of the hotkey `target_key` (`Mapping.cpp:1355-1358`), but each route is classed by its own declared effect (`:1255-1265`). So one file can send two routes to the same hotkey with different classes: a concrete route with effect `output` (output-control) and a wildcard route with effect `scene` (scene).
  - `egress.trust` also comes from the file (`:800-811`).
  - PR4 must get both from the live binding, not from the mapping file.
- **claude-5:** switching to `nlohmann::json` changes no behaviour. Mapping.cpp only walks one-member objects (`value.begin()` at `:415,508,540`), so nothing depends on member order. The timing (minutes) is still my estimate, not a measurement.

**Waiting on Codex:** please confirm or dispute claude-3, claude-4 and claude-5 (Major), and claude-7 to claude-10 (Minor).

FINDING claude-1 | Major | open | FromDoubleInCodeSpace wraps instead of clamping (uint7 127.5→0, int8 127.5→−128, Value.cpp:576-601); FitsSigned never rejects (MakeInt(8,300)→44); relative_ dropped; no fractional-input tests
FINDING claude-2 | Major | open | ParseMapping/CompileMapping(bytes) use the lax pre-r1 schema (Mapping.cpp:862-888,1455,1473,1593); only LoadMapping is strict; jc-unit tests only the lax path (same defect as codex-1)
FINDING claude-3 | Major | open | hotkey class comes from the file's declared effect (Mapping.cpp:1255-1265), which is outside target_key (:1355-1358): OBSBasic.StartStreaming with effect "scene" passes a wildcard, against A2/A13
FINDING claude-4 | Major | open | hard deny omits section 8 source secrets (browser_source url, ffmpeg_source input/ffmpeg_options, vlc_source playlist value; WORKSPACE_FORMAT.md:350) that A1/item 9 require; cases.txt:605 pins url as allowed
FINDING claude-5 | Major | open | ordered_json member insert is a linear scan (Mapping.cpp:34,147): parsing a ~500k-key object that fits in 4 MiB takes Θ(n²) time; nlohmann::json is a drop-in fix
FINDING claude-6 | Major | open | raised from Minor, same defect as codex-3: emit target_key not injective (unescaped "/" in AddressKey, Mapping.cpp:1054-1064), "\" unescaped against r1.5 (:1328), and merge groups by this string (:1535-1539)
FINDING claude-7 | Minor | open | r1.3/r1.5/r1.8 and Progress disagree with code and cases (incl. codex-2's café); unrecorded caps, check order, A11 prefix/inert-denied deviations; stale 006:31, 006:318
FINDING claude-8 | Minor | open | cycle edge rule is sound only if an emitted event keeps the emitting route's slot; load-bearing emit=meta unrecorded (12:52-53)
FINDING claude-9 | Minor | open | merge check O(g²) with set inserts (Mapping.cpp:1541-1562); cycle graph keeps edges to non-emit routes, 25M edges from ~1.3 MB (:1121-1136)
FINDING claude-10 | Minor | open | jc-check takes ANSI argv on Windows (jc-check.cpp:411), unlike jw-check's wmain (jw-check.cpp:409-441); io-validate-utf8-path cannot pass there
VERDICT: request changes
