**Approve.** The head is the same one I reviewed in round 1, so my dispositions stand. Codex's reply raises nothing new and disputes none of my findings. I agree with all six of Codex's dispositions.

**Codex's reply**
- **codex-5:** confirmed. The fixture still supplies the effect (`fixtures/compile/key-hotkey.json:9`), the expected file leaves it out (`expected/compile/key-hotkey-with-effect.json:1`), and `cases.txt:913` accepts only that file.
- **codex-6:** confirmed. `jc-check.cpp:162` is now `auto event_bytes = ReadFile(event_path);`, with no `bytes` local.
- Codex didn't address claude-14. It is Minor and doesn't block the merge.

**Ledger corrections for the lead**
- **claude-7 and claude-8 are listed as "fixed per contract r4", but they aren't.**
  - r4 (`07:160-168`) has no item for claude-8.
  - The docs fix promised in r4.5 isn't in this head: `006:31` still says "Nothing is built yet", `README.md:12` stops at `13`–`17`/`review-pr33-01/`, and `README.md:15` still says "not a gate review".
- **claude-14 is missing from the ledger.** I raised it in round 1; please add it.
- **The CI evidence for claude-11 isn't in this tree.** No file under `.github/` mentions `jeiobs-control`, which matches `006:335-336`. The Windows run that confirmed claude-11 must have run outside this tree. Before the self-merge, cite that run and a green Windows run on this head in the merge record.

**claude-14, re-checked**
- `Mapping.cpp:93` is the only unordered container in the library.
- `Json` is `nlohmann::json` (`Mapping.cpp:35`), whose default object type is `std::map`. So `frame.value.contains(value)` in `key()` costs O(log n) per key, even on hostile input.
  - The check still catches every duplicate: each value is attached under its key (`:148`) before the parser delivers the next key.
- **The limits don't stop the flood.**
  - 4 MiB (`Limits.hpp:17`) at about 16 bytes per member allows about 2.6×10^5 keys, which is under `kJsonValues` (10^6, `:31`).
  - `ParseEvent` uses the same parser and the same 4 MiB cap (`Mapping.cpp:1732-1735`), so event files are exposed too.

FINDING claude-1 | Major | accepted-fix | rounds then clamps (Value.cpp:602-681), FitsSigned requires sign extension (:317-334), Relative kept
FINDING claude-2 | Major | accepted-fix | one strict parser for ParseMapping/LoadMapping/CompileMapping(bytes)/ParseEvent (Mapping.cpp:257-287,1583-1603,1712-1719,1730-1735)
FINDING claude-3 | Major | accepted-fix | hotkey always output-control (Mapping.cpp:1377-1378), wildcards refused (:1633-1641)
FINDING claude-4 | Major | accepted-fix | url/input/ffmpeg_options/playlist denied (jc_unit.cpp:164-171); PR4 must keep field a single key
FINDING claude-5 | Major | accepted-fix | nlohmann::json removes the quadratic insert; flooding of the dup-key set is claude-14
FINDING claude-6 | Major | accepted-fix | segment and key escaping injective (Mapping.cpp:1037-1073,1435-1447), merge groups by escaped key
FINDING claude-7 | Minor | open | r4.5 docs absent: 006:31,318-323,332-333, README:12,15 stale; E_SCHEMA routes for enabled (Mapping.cpp:884-885) vs r1.3 and caps Limits.hpp:30-31 unrecorded
FINDING claude-8 | Minor | open | not in r4 (07:160-168): contract (07:65,85-88) doesn't fix an emitted event's slot; edge rule Mapping.cpp:1227-1230 relies on it; emit=Meta (:1388-1389) unrecorded; PR3 follow-up
FINDING claude-9 | Minor | accepted-fix | lazy cursor over emit-only first-segment index (Mapping.cpp:1165-1255), no stored edges, O(n) memory
FINDING claude-10 | Minor | accepted-fix | wmain converts UTF-16 arguments as jw-check does (jc-check.cpp:633-655)
FINDING claude-11 | Major | accepted-fix | reserve only for elements != size_t(-1) && <= 1024 (Mapping.cpp:192-194); start_array ignores count (:223); Windows run on this head must be cited, none in .github
FINDING claude-12 | Minor | accepted-fix | MakeInt stores SignExtended raw (Value.cpp:467); MakeInt(w, v.intRaw()) round-trips int8 -128 (jc_unit.cpp:66-74)
FINDING claude-13 | Minor | accepted-fix | expected file without effect, cases.txt:913 accepts only it; nits: comment :904-905 stale, expected/compile/key-hotkey.json orphaned
FINDING claude-14 | Minor | open | dup-key "std::unordered_set<std::string> keys" (Mapping.cpp:93,205) hash-floodable on MSVC (unseeded FNV-1a, low-bit buckets); use frame.value.contains (std::map)
VERDICT: approve
