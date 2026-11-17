**Approve.** All my Majors are fixed. I checked the claude-11 fix by reading the code, not by running it. claude-7 and claude-8 stay open as Minor follow-ups, and I'm raising one new Minor (claude-14). I couldn't see the Windows/macOS CI results for this head. The run-02 Windows failure was claude-11, so that job should be green before the self-merge.

**Fixes I checked**
- **claude-11: accepted.**
  - `reserve` now runs only when `elements != size_t(-1) && elements <= 1024` (`Mapping.cpp:192-194`). nlohmann's text parser always passes `size_t(-1)`, so it never runs.
  - `start_array` ignores its count (`:223`).
  - Every other `reserve` uses the size of JSON that has already been parsed (`:454,480,503,589,606`; `jc-check.cpp:341`).
- **claude-12: accepted.**
  - `MakeInt` stores `SignExtended(raw)` (`Value.cpp:467`), after `FitsSigned` (`:317-334`).
  - int8 −128 now round-trips through `intRaw()` (`jc_unit.cpp:66-74`).
  - The jc-check decoders mask before negating (`jc-check.cpp:412-456`), so they still read the new form correctly.
- **claude-9: accepted.**
  - Edges are no longer stored. A cursor walks an index, keyed by first segment, that holds only enabled emit routes (`Mapping.cpp:1165-1255`), so memory is O(n).
  - Check 2 refuses wildcard emit routes, because emit is Meta (`:1633-1641,1388-1389`). So the `any` and integer-wildcard lists are always empty.
  - Time is still one `Matches` call per candidate. `perf-chain-10000` puts all 10,000 routes in the `s:c` bucket (`run-cases.cmake:143`), which is about 10^8 calls.
  - Optional improvement: emit-route patterns are always concrete, so keying the index by the whole `AddressKey` would return only the routes that match.
- **claude-13 (also codex-5): accepted.**
  - `key-hotkey-with-effect.json:1` no longer contains `effect`, and `cases.txt:913` accepts only that file.
  - Nits: the comment at `:904-905` still says the hotkey question is unsettled; `expected/compile/key-hotkey.json` is no longer referenced; and the file name says "with-effect".
- **codex-6:** the unused `std::string bytes;` is gone (`jc-check.cpp:162`). Codex has to close it.

**Still open (Minor)**
- **claude-7:** r4.5 promised a docs fix, but this head doesn't have it.
  - Still stale: `006:31` ("Nothing is built yet"), `006:318-323` ("Open before K1-PR1"), and `006:332-333`, which cites only r1.
  - `README.md:12` doesn't list `18`-`19` or `review-pr33-02/`, and `README.md:15` still says "not a gate review".
  - Still unrecorded: the `E_SCHEMA routes` reading for `enabled` (`Mapping.cpp:884-885`) and the caps at `Limits.hpp:30-31`.
- **claude-8:** unchanged, and r4 doesn't cover it. The edge rule `B.slot null or == A.slot` (`Mapping.cpp:1227-1230`) is sound only if an emitted event carries the emitting route's `match.slot`. The contract still doesn't say which slot it carries (`07:65,85-88`). This is a follow-up for PR3.

**New**
- **claude-14 (Minor): the duplicate-key check can be hash-flooded.**
  - It uses `std::unordered_set<std::string> keys;` (`Mapping.cpp:93`) and `if (!frame.keys.insert(value).second)` (`:205`).
  - MSVC hashes strings with unseeded FNV-1a and picks the bucket from the low bits. The low k bits of FNV-1a depend only on the low k bits of the state, so keys that share the low ~20 bits are cheap to generate. All of them land in one bucket.
  - A 4 MiB object of about 2.6×10^5 such keys costs about 3×10^10 key comparisons, which is minutes on Windows. That number is an estimate; I haven't run it.
  - It matters once PR4 or PR5 loads mapping files that a user imported.
  - Fix: use `frame.value.contains(value)` and drop `keys`. nlohmann's object is a `std::map`, so the lookup is O(log n), can't be flooded, and also saves the second copy of every key.

FINDING claude-1 | Major | accepted-fix | rounds then clamps (Value.cpp:602-681), FitsSigned requires sign extension (:317-334), Relative kept
FINDING claude-2 | Major | accepted-fix | one strict parser for ParseMapping/LoadMapping/CompileMapping(bytes) (Mapping.cpp:1583-1603,1712-1719); events same rules
FINDING claude-3 | Major | accepted-fix | hotkey always output-control (Mapping.cpp:1377-1378), wildcards refused
FINDING claude-4 | Major | accepted-fix | url/input/ffmpeg_options/playlist denied; PR4 must keep field a single key
FINDING claude-5 | Major | accepted-fix | nlohmann::json removes the quadratic insert; flooding of the dup-key set is claude-14
FINDING claude-6 | Major | accepted-fix | segment and key escaping injective (Mapping.cpp:1037-1061,1435-1447), merge groups by escaped key
FINDING claude-7 | Minor | open | r4.5 docs absent: 006:31,318-323,332-333, README:12,15 stale; E_SCHEMA routes (Mapping.cpp:884-885) and Limits.hpp:30-31 unrecorded
FINDING claude-8 | Minor | open | not in r4: contract (07:65,85-88) still doesn't fix the slot of an emitted event; edge rule Mapping.cpp:1227-1230 depends on it; PR3 follow-up
FINDING claude-9 | Minor | accepted-fix | lazy cursor over emit-only first-segment index (Mapping.cpp:1165-1255), no stored edges, O(n) memory
FINDING claude-10 | Minor | accepted-fix | wmain converts UTF-16 arguments as jw-check does (jc-check.cpp:633-655)
FINDING claude-11 | Major | accepted-fix | reserve only for elements != size_t(-1) && <= 1024 (Mapping.cpp:192-194); start_array ignores count (:223); CI on this head still to confirm
FINDING claude-12 | Minor | accepted-fix | MakeInt stores SignExtended raw (Value.cpp:467); MakeInt(w, v.intRaw()) round-trips int8 -128 (jc_unit.cpp:66-74)
FINDING claude-13 | Minor | accepted-fix | expected file without effect, cases.txt:913 accepts only it; nit: comment :904-905 stale, expected/compile/key-hotkey.json orphaned
FINDING claude-14 | Minor | open | dup-key "std::unordered_set<std::string> keys" (Mapping.cpp:93,205) hash-floodable on MSVC (unseeded FNV-1a, low-bit buckets); use frame.value.contains
VERDICT: approve
