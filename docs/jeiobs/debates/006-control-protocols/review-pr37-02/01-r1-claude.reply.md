I approve. The fixes for claude-2/codex-1, claude-3 and claude-4 hold up in the code, so I accept them. claude-1 and claude-5 stay open as Minors: one new test can never fail, and the new docs have three wrong statements. I only had read and search tools, so I built and ran nothing.

**Fixes I verified**
- **claude-2 / codex-1 (the split14 output was copied):**
  - `OutputValue` now moves the result out of `owned` (`Transform.cpp:392-393`).
  - `Value` declares no copy, move or destructor of its own (`Value.hpp:79-132`), and `Result` is a plain struct holding a `std::optional` (`Error.hpp:51-56`). So `std::move(*owned)` really moves, and so do the later steps (`:466-467`).
  - Nothing reads `owned` after the move (`:459-467`).
  - A split14 call now allocates once: `MakeTuple`'s vector, which is the output itself (`Value.cpp:518-550`).
  - The copy fallback at `:395` can't be reached: only join14 and split14 produce a UInt or Tuple signal, and both write `owned`.
- **claude-3:** `Compile` now checks op and parameters, stage by stage, before positions (`Transform.cpp:410-420`). That is the same order as `ParseTransform` (`Mapping.cpp:612-684`). The unit test at `jc_unit.cpp:409-413` would have failed on the old order. The position rule still exists twice, but both copies are identical and each is tested, which I accept.
- **claude-4:** `IsPlainDecimal` runs before `strtod` (`jc-check.cpp:1009-1011`), which rejects hex floats, `inf`, `nan` and whitespace. `value fromdouble` behaves as before. Contract r1.4 matches `TransformValueJson` (`:818-822`).

**Still open**
- **claude-1:**
  - **The new "errors keep state" property can't fail** (`jc_unit.cpp:536-543`). A Text input is rejected at entry (`Transform.cpp:437-440`), before `working_state` exists (`:441`). The test would pass even if `Apply` wrote straight into `state` with no rollback.
    - Fix: add the failing scale from `:551-552` after `relative`, so every call errors after the position moved, then assert the state is unchanged.
  - **"an exact level k / n" (`:504`) is checked with 1e-9 slack.** `JC_CHECK_EQ(y, level / n)` is exact and would pass.
  - The run-01 rounds already pass (deadzone/threshold, −0, quantize over 2..64 steps, position bounds).
  - I drop two run-01 points that only appeared in my reply body: the identity-scale NaN property and the fixed grids.
- **claude-5:**
  - **r1.1 says "the test uses inputs with |c|, |d| >= 1e-3"** (`20-k1pr2-contract.md:138`). That is false: `jc_unit.cpp:589-590` draws c from [-10, 10) with no lower bound on |c| or |d|. What keeps the bound valid is that both spans are at least 0.01.
  - **`cases.txt:1327-1329` reads as if the blind scale cases assumed endpoint special handling.** They assumed the literal formula (`24-k1pr2-tests-claude.md:96-97`); the implementation was the side that changed (task 006:345-346).
  - **The two cases you wrote yourself** (`cases.txt:1512-1513`, hex float and nan) sit under the header "written from the contract without reading the library" (`:1307`). The list of your changes at `:1327-1330` doesn't mention them.

**Small things (no finding)**
- The `IsPlainDecimal` comment says "one optional sign", but the function only filters characters, and an exponent can carry its own sign. It is `strtod` that enforces the format.
- The ledger header still says 1292 case lines; the tree has 1294.
- No case covers r1.4 (a float inside a tuple keeps `bits_hex`); adding one is optional.

FINDING claude-1 | Minor | open | new error-state property can't fail: Text is rejected in InitialSignal (Transform.cpp:437-440) before working_state exists, so jc_unit.cpp:536-543 passes without rollback; "exact level k / n" (:504) checked only within 1e-9
FINDING claude-2 | Minor | accepted-fix | OutputValue moves the join14/split14 result out of `owned` (Transform.cpp:392-393); Value has implicit moves, `owned` not read afterwards, split14 makes one allocation
FINDING claude-3 | Minor | accepted-fix | Compile checks op/params per stage before positions (Transform.cpp:410-420), same order as ParseTransform; unit test jc_unit.cpp:409-413
FINDING claude-4 | Minor | accepted-fix | IsPlainDecimal before strtod (jc-check.cpp:660-675, 1009-1011), cases :1512-1513; r1.4 recorded and matches TransformValueJson
FINDING claude-5 | Minor | open | r1.1 "test uses inputs with \|c\|, \|d\| >= 1e-3" is false (jc_unit.cpp:589-590; only spans >= 0.01); cases.txt:1327-1329 says the blind scale cases assumed endpoint handling (they assumed the literal formula); lead-written cases :1512-1513 not marked
VERDICT: approve
