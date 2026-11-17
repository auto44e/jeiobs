# K1-PR2 contract: transform execution (both workers)

Design: `docs/jeiobs/tasks/006-control-protocols.md` (A10, A11; "Open before K1-PR1": no per-route smoothing).
PR1 (merged) validates `transforms` in `mappings.json` and stores them as `TransformSpec`
(`Mapping.hpp`; parsing in `src/Mapping.cpp` `ParseTransform`: scale `numbers=[a,b,c,d]`, curve `text`,
clamp `numbers=[min,max]`, deadzone `numbers=[width]`, quantize `integer=steps`, threshold
`numbers=[at]`, relative `text=encoding` and `numbers=[step]`; invert, toggle, momentary, join14,
split14 have no parameters). PR2 adds EXECUTION of those pipelines. Still inert: no OBS, no I/O, no
threads, no clock. The schema does not change (no new ops, no new fields).

## Model
A route's `transforms` is an ordered pipeline (0..16 stages). A pipeline is immutable. All mutable
state is in a caller-owned `TransformState` (one per route; PR3's runtime owns it):

```
struct TransformState { bool toggled = false; double position = 0.0; };   // fixed size, trivially copyable
```
`Apply(input, state)` runs the stages in order on one input `Value` and returns one of
- an output `Value` (state may have changed);
- NoOutput (nothing to deliver; state may have changed);
- an error `E_VALUE <detail>` (state UNCHANGED, including state touched by earlier stages of the
  same call: apply to a copy and commit only on success).

## Signal and stage types
Between stages the working signal is one of: `Number` (a double), `Bool`, `UInt` (the original
`Value`), `Tuple`.
- Entry conversion of the input Value: `UInt`, `Int`, finite `FloatBits` -> Number (the existing
  `Value::AsDouble()`); `Bool` -> Number 0 or 1; `Trigger` -> Number 1. A non-finite `FloatBits`
  input (NaN or infinity) -> `E_VALUE nonfinite`. `Text`, `Blob` -> `E_VALUE type`. A `Tuple` is
  accepted only when the first stage is `join14`; otherwise `E_VALUE type`.
- Numeric stages (scale, curve, invert, clamp, deadzone, quantize, threshold) take a Number (a
  Bool or UInt signal is converted as above) and produce a Number, except threshold which produces a
  Bool.
- Result of any numeric stage that is not finite (NaN or infinity, e.g. from huge scale factors) ->
  `E_VALUE nonfinite`. A result of negative zero becomes +0.0.
- Output Value: a Number -> `FloatBits` ieee64 (empty unit, no relative metadata); a Bool -> `Bool`.
  An EMPTY pipeline returns the input Value unchanged (bitwise `==`, unit and relative metadata
  kept) and never errors, whatever the kind.

## Stages (x is the incoming Number)
- `scale{in:[a,b], out:[c,d]}`: `y = c + (x - a) * (d - c) / (b - a)`, evaluated in that order in
  doubles. Not clamped (use `clamp`). `a != b` is guaranteed by validation.
- `curve{kind}` on a normalized signal: first clamp x to [0,1]; `linear`: y = x; `log`:
  y = log10(1 + 9x); `exp`: y = (10^x - 1) / 9 (the inverse of `log`; use `std::log10` and
  `std::pow(10.0, x)`). Endpoints are exact: 0 -> 0 and 1 -> 1 for all kinds.
- `invert`: `y = 1 - x` (normalized signals; no clamp).
- `clamp{min,max}`: `y = min(max(x, min), max)`.
- `deadzone{width}` on a bipolar normalized signal: if `|x| <= width` then 0, else
  `sign(x) * (|x| - width) / (1 - width)`. Not clamped.
- `quantize{steps}` on a normalized signal: clamp x to [0,1]; `n = steps - 1`;
  `y = nearbyint(x * n) / n` with ties to even (default rounding mode). Outputs are exactly the
  `steps` levels `k / n`.
- `threshold{at}`: Bool, `x >= at`.
- `momentary`: Bool, `x != 0` (a Trigger input is therefore true).
- `toggle` (stateful): a press is `x != 0`. A press flips `state.toggled` and outputs `Bool(state.toggled)`;
  a release (`x == 0`) outputs NoOutput. If `toggle` is not the last stage the later stages see the
  Bool; a NoOutput anywhere ends the call with NoOutput (later stages do not run, and state changes
  of earlier stages stay).
- `relative{encoding, step}` (stateful): the signal must be an input `UInt` Value of width w
  (1..64; otherwise `E_VALUE type`; it must be the first stage, otherwise validation of the
  pipeline fails at compile time, see below). Decode the raw code `r` to a signed count `d`:
  - `twos_complement`: `d = r` if `r < 2^(w-1)` else `r - 2^w`;
  - `sign_magnitude`: top bit is the sign, the rest the magnitude: `d = +m` or `-m` (negative zero is 0);
  - `binary_offset`: `d = r - 2^(w-1)` (7 bits: 64 is zero).
  Then `state.position = clamp(state.position + d * step, 0, 1)` and the output is the Number
  `state.position`. It is always emitted, even for `d == 0`. The initial position is 0.0 and the
  runtime may set it (PR3 seeds it from the target).
  (The value's own `Relative` metadata is ignored: the route's stage is authoritative.)
- `join14` (stateless): the signal must be an input `Tuple` of exactly 2 `UInt` Values (msb, lsb)
  with raw <= 127 each (any width >= 7), otherwise `E_VALUE type` (or `E_VALUE range`
  for raw > 127). Output `UInt` width 14, raw `msb * 128 + lsb`, empty unit. It must be the first stage.
- `split14` (stateless): the signal must be a `UInt` Value of width 14 (otherwise `E_VALUE type`).
  Output a `Tuple` of two `UInt` width 7: msb `raw >> 7`, lsb `raw & 127`. It must be the last stage.

## Pipeline compile checks (`CompileTransforms`)
Position rules, NEW in PR2 and reported as `E_SCHEMA transforms` (exactly that detail, no route id; the
first offending route in document order decides, and it is found by `ValidateMapping`, so `validate`,
`compile`, `match` and `transform` all report it): `join14` or `relative` at an index other than 0, or
`split14` at an index other than the last. PR1 outcomes are otherwise unchanged (unknown `op` stays
`E_SCHEMA op`, wrong params `E_SCHEMA transforms`, more than 16 stages as PR1 reports it). PR1 did
not check positions; this tightens a v1.0 format that no released build reads.

## C++ API (`Transform.hpp`, unit-tested by the implementer in `jc-unit`)
```
struct TransformState { bool toggled = false; double position = 0.0; };
class TransformPipeline {  // immutable after construction
public:
  static Result<TransformPipeline> Compile(const std::vector<TransformSpec> &stages);   // position rules
  enum class Outcome { Output, NoOutput };
  struct Applied { Outcome outcome = Outcome::NoOutput; std::optional<Value> value; };
  Result<Applied> Apply(const Value &input, TransformState &state) const;   // state untouched on error
  std::size_t size() const noexcept;
};
```
Nothing throws out of the public API; no allocation beyond the output Value; no recursion.

## `jc-check transform FILE ROUTE_ID INPUTS.json`
Loads and validates and compiles the mapping FILE (errors as for `compile`), finds the route by id
(unknown id: `ERR E_REF route`), reads INPUTS.json (strict JSON as `jc-check match`: an array of 1..1000
value JSON objects in the `jc-check value` format; anything else `ERR E_SCHEMA inputs`), applies them
in order to ONE fresh `TransformState` (`position` 0, `toggled` false), and prints ONE line of
compact JSON `{"outputs":[...]}` with one element per input:
- NoOutput -> `null`;
- an error -> `{"error":"E_VALUE <detail>"}` (the state is unchanged and the run continues);
- an output Value -> value JSON as in `jc-check value`, EXCEPT `float` values print
  `{"type":"float","format":"ieee32|ieee64","number":"<%.9g of the double>"}` (no `bits_hex`; -0 prints `0`),
  so results agree across libm implementations.
The route's own `enabled`/`guard`/match are ignored: only its `transforms` run. Exit 0 for all printed outputs
(also when an element is an error); exit 1 for `ERR` lines; 2 for usage.
An optional `--position P` before FILE sets the initial `state.position` (decimal number), and
`--toggled` sets `state.toggled` to true.

## Tests
- `jc-unit` unit tests for each stage, state commit/rollback, and PROPERTY TESTS with a fixed-seed
  xorshift64* generator (document the seed; 2000 cases each, deterministic): scale is monotone for
  a<b, c<d and maps `a` to `c` and `b` to `d`; `curve log` then `curve exp` returns x within 1e-12 on
  [0,1], and both are monotone with exact endpoints; `clamp` is idempotent and within [min,max];
  `invert` twice returns x within 1 ulp of 1; `quantize` outputs only the allowed levels, is
  idempotent, and is monotone; `join14(split14(c)) == c` for ALL 16384 codes; `toggle` after n presses
  has `toggled == (n % 2 == 1)` and a release never changes it; `relative`'s three encodings agree
  on the delta for corresponding codes over widths 7 and 14, position stays in [0,1], and a +d then
  -d pair returns the position (when no clamping happened); error calls never change the state;
  random NaN/infinity/huge inputs never produce non-finite outputs or throw.
- Black-box `cases.txt` lines for `jc-check transform` written by a Claude worker WITHOUT reading the
  library, from this contract.
- The existing 1128 case lines keep passing except where a case expects a mapping with a
  position-rule violation to compile; there should be none (tell the lead if there are).

## Portability (as PR1)
C++17, GCC/Clang `-Wall -Wextra -Werror`, MSVC `/W4 /WX`, no floating-point `from_chars`, `windows.h`
needs NOMINMAX, CLI arguments go through the existing UTF-8 `wmain` path on Windows. `%.9g` output is
formatted with `snprintf` in the "C" numeric conventions (no locale dependence: format via
`std::snprintf` only, and the library never calls `setlocale`).

## r1 (integration decisions after the first run; they win over the text above)
1. `scale` is the literal formula, evaluated left to right as `((x - a) * (d - c)) / (b - a) + c`, with no
   special case for `x == a` or `x == b`. A span that overflows (`d - c` or the product) gives `E_VALUE nonfinite`.
   The property "scale maps a to c" is exact; "maps b to d" holds within 4 epsilon of (|c| + |d|) (the rounding of the span; the bound is not meant for spans that underflow, and the unit test uses spans of at least 0.01).
2. Position rules run in `ValidateMapping` AFTER the whole file parsed: any parse or schema error anywhere in
   the file (an unknown `op` is `E_SCHEMA op`, a bad target `E_SCHEMA target`, in any route) is reported before
   a position error. Position errors are then reported for the first offending route in document order.
3. `jc-check transform`: an `--position` value that is not a finite decimal number in [0,1] is a usage error
   (usage line on stderr, exit 2); so is an unknown option or a missing argument.
4. A `float` inside a Tuple (reachable only through the empty pipeline, which returns its input unchanged) keeps its
   existing value JSON form with `bits_hex`; only top-level float elements of `outputs` print `number`.
5. `Compile(stages)` reports an unknown `op` (`E_SCHEMA op`) and bad parameters (`E_SCHEMA transforms`) before any
   position error, the same order as `ValidateMapping` (review run 01, codex-2 and claude-3).
6. `--position` accepts only a plain decimal number (digits, sign, point, exponent): no hexadecimal float, `inf`,
   `nan` or whitespace (review run 01, codex-3 and claude-4).
7. No allocation beyond the output Value: a join14 or split14 output is moved, not copied, into the result
   (review run 01, codex-1).
