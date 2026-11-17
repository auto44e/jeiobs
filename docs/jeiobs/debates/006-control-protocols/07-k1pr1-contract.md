# K1-PR1 contract: control-graph types, mapping schema, validator and compiler (both workers)

Design: `docs/jeiobs/tasks/006-control-protocols.md` ("K1 design", A1-A18). This PR is inert: no
OBS, no I/O, no protocols, no transform execution (PR2), no runtime (PR3). Library:
`shared/jeiobs-control/` (C++17, no Qt, no libobs; only nlohmann_json 3.11 as in
`shared/jeiobs-workspace`; copy its portability rules: GCC/Clang `-Wall -Wextra -Werror`, MSVC
`/W4 /WX`, no floating-point `from_chars` assumption, no `unordered_multimap` for large groups,
UTF-8 paths via `u8path`, explicit stacks for deep input).

## Decisions taken here for the K1 "Open before PR1" list
Value `Blob` <= 65,536 bytes; `Text` <= 4,096 bytes; `Tuple` <= 256 elements, nesting depth <= 4;
address <= 16 segments, a symbol/text segment <= 256 bytes; wildcard fan-out cap 64 (compile-time
product of range and set sizes); endpoint queue 4,096 events and per-cycle quota 256, privileged
lane 64 (PR3 constants, defined now in `Limits.hpp`); no per-route smoothing in K1.

## C++ API (unit-tested by the implementer in `jc-unit`)
- `Value`: exact tagged value: `Trigger | Bool | UInt{width_bits (1..128), raw (unsigned 128-bit)} |
  Int{width_bits, raw (signed 128)} | FloatBits{format: ieee32|ieee64, raw bits} | Text | Blob |
  Tuple{values}` with optional `unit` string and `Relative` metadata (`encoding`:
  twos_complement|sign_magnitude|binary_offset, `step` as a decimal string). `AsDouble()` is a
  derived view for transforms; `FromDoubleInCodeSpace(template, double)` returns a Value of the
  same tag/width (clamped, rounded to nearest, ties to even). Round-trip property: for every uint
  width 7, 14, 16, 32 and every code `c` in a dense sample, `FromDoubleInCodeSpace(v, v.AsDouble()) == v`
  (64/128-bit codes are exact only through raw; document that `AsDouble` is lossy above 2^53).
- `ControlAddress`: <= 16 segments, each `Symbol(string) | Int(uint64) | Text(string)`;
  `Pattern` segments add `Any`, `Range{lo,hi}`, `Set{ints}`; `Matches(pattern, address)`
  (equal length; per segment: Symbol equal bytes; Int equal; Text equal; Any matches any single
  segment; Range/Set match Int segments only).
- `WireEnvelope{protocol: string, bytes (<= 65,536), packet_count}` opaque; `Event{envelope?, address,
  value, principal}`; `Principal{trust: hw|virtual|loopback-net|remote-net|local-ui, endpoint_id, peer_id}`.
- `CapabilityClass`: mix, scene, source-property, filter, plugin-param, hotkey, output-control, profile,
  meta, egress-by-trust. `ClassOfTarget(target)` implements the table of A15 (below).
- All constructors validate limits and return a result with an error code; nothing throws out of the
  public API.

## Mapping file schema v1 (`mappings.json`, UTF-8 JSON, <= 4 MiB, depth <= 32, duplicate keys rejected,
## invalid UTF-8/NUL/BOM/non-finite rejected as in jeiobs-workspace manifests)
```
{"format":"jeiobs-control","major":1,"minor":0,
 "slots":[{"id":"main_surface","requires":["mcu.fader","feedback.motor"]}],
 "routes":[{"id":"r1","enabled":true,
   "match":{"slot":"main_surface"|null,"address":[SEG,...]},
   "guard":{"all":[PRED,...]},                       // optional
   "transforms":[{"op":"scale",...},...],            // optional, <= 16
   "target":{"kind":"mixer","id":"<uuid-or-name>","field":"volume"},
   "merge":{"policy":"touch-lease","priority":0}}]}  // merge optional
```
- Unknown fields are ignored at every level (and preserved by nothing in PR1). `major` must be 1 (other:
  `E_MAJOR`); `minor` integer >= 0. `slots` <= 256, `routes` <= 10,000, ids match `[a-z0-9_]{1,64}`.
- `SEG` is an object with EXACTLY ONE key: `s` (symbol string), `i` (integer 0..2^53), `t` (text),
  `any` (`true`), `r` ([lo,hi] integers, lo<=hi), `in` (array of 1..64 distinct integers). Otherwise
  `E_SCHEMA address`.
- `PRED`: `{"value":{"min":n,"max":n}}` (n finite numbers, min<=max) | `{"modifier":"name"}` |
  `{"layer":"name"}` | `{"bank":int>=0}`; unknown predicate -> `E_SCHEMA guard`.
- Transform `op` and params (validated only): `scale{in:[a,b],out:[c,d]}` (a!=b), `curve{kind:linear|log|exp}`,
  `invert{}`, `clamp{min,max}` (min<=max), `deadzone{width in [0,1)}`, `quantize{steps>=2 integer}`,
  `toggle{}`, `momentary{}`, `threshold{at}`, `relative{encoding, step>0}`, `join14{}`, `split14{}`;
  unknown op -> `E_SCHEMA op`; wrong/missing param -> `E_SCHEMA transforms`.
- `target.kind` and its fields (anything else -> `E_SCHEMA target`): `source-property{id,field}`,
  `filter-property{id,field}`, `scene{id}`, `scene-item{scene_id,item_id}`, `mixer{id,field in
  [volume,mute,balance,monitoring,mixers,sync_offset]}`, `transition{name}`, `hotkey{registerer_kind,
  registerer_id,name,effect in [none,scene,mixer,output,unknown]}`, `output{output in
  [stream,record,replay,virtualcam],action in [start,stop,toggle]}`, `profile{name}`,
  `plugin-param{instance,param}`, `plugin-event{instance}`, `graph{node,port}`, `egress{endpoint,
  trust in [hw,virtual,loopback-net,remote-net]}`, `emit{address:[SEG without any/r/in]}` (re-injects an
  event; used for compile-time cycle checks).
- `merge.policy` in last-writer|priority|touch-lease|sum|min|max|boolean-or; `priority` integer
  (default 0).

## Validation errors (first error wins; deterministic order = document order)
`E_IO`, `E_LIMIT <what>`, `E_JSON`, `E_DUPKEY`, `E_UTF8`, `E_NUL`, `E_NONFINITE`, `E_BOM`, `E_MAJOR`,
`E_SCHEMA <field>`, `E_DUP <id>`, `E_REF <slot id>` (match.slot not in slots).

## Compile checks (after validation), error code + first offending id
1. `E_DENIED <route id>`: hard deny, not configurable: a `source-property` or `filter-property` target whose
   `field` (compared ASCII case-insensitively) is one of: key, stream_key, bearer_token, username, password,
   server, token, refresh_token, auth, passphrase, secret, streamid. Nothing else is denied in PR1.
2. `E_WILDCARD_PRIVILEGED <route id>`: a route whose address has an `any`/`r`/`in` segment and whose
   target class is output-control, profile or meta (class by A15; `hotkey` with effect `output` or
   `unknown` is output-control; `output` and `profile` kinds are output-control and profile).
3. `E_FANOUT <route id>`: product of range sizes (hi-lo+1) and set sizes over the pattern > 64.
4. `E_AMBIGUOUS_MERGE <first route id of the group>`: enabled routes sharing a target key (kind and all
   identity fields; for `mixer` also `field`) must either be a single route, or ALL declare a merge
   policy, the same one, and for `priority` pairwise distinct priorities.
5. `E_CYCLE <route id>`: build the graph route A -> route B when A's target is `emit` with address E and
   B's pattern can match E (B pattern segments match E's concrete segments per `Matches`, equal length,
   B.slot null or equal to A.slot); any cycle (including A -> A) is `E_CYCLE` naming the lowest-ordered
   route id on the cycle in document order.
Disabled routes (`enabled:false`) take part in validation and E_DENIED/E_WILDCARD_PRIVILEGED/E_FANOUT but
NOT in the merge (4) or cycle (5) checks.

## CLI `jc-check` (exit 0 = OK, 1 = ERR, 2 = usage; one result line on stdout, JSON for compile/match)
- `jc-check validate FILE` -> `OK` or `ERR <CODE> [detail]`.
- `jc-check compile FILE` -> validates, runs compile checks; on success prints compact JSON (keys sorted):
  `{"routes":[{"id","enabled","class","target_key","merge":{"policy","priority"},"fanout"}], "slots":[ids]}`;
  `target_key` is the canonical string `kind|field1=value1|field2=value2` with identity fields in
  alphabetical order; `class` is the capability class name; `merge` is the declared one or
  `{"policy":"last-writer","priority":0}` for a lone route without one; `fanout` is the product (1 if
  none). Routes appear in document order.
- `jc-check match FILE EVENT.json` -> compiles, then prints `{"matches":["r1","r3"]}` (enabled routes
  whose slot equals the event slot or is null, and whose pattern matches the event address, document
  order). EVENT.json: `{"slot":"main_surface"|null,"address":[SEG...(concrete only: s,i,t)]}`; a bad event
  -> `ERR E_SCHEMA event`.
- `jc-check value OP VALUE_JSON [ARG]`: value helper for black-box checks of exactness:
  `jc-check value roundtrip '{"type":"uint","bits":14,"value":"8191"}'` -> `OK`/`ERR E_VALUE`;
  value JSON: `{"type":"trigger"}`, `{"type":"bool","value":true}`, `{"type":"uint","bits":N,"value":"<decimal>"}`,
  `{"type":"int","bits":N,"value":"<decimal>"}`, `{"type":"float","format":"ieee32|ieee64","bits_hex":"..."}`,
  `{"type":"text","value":"..."}`, `{"type":"blob","hex":"..."}`, `{"type":"tuple","values":[...]}`; `jc-check value
  double VALUE_JSON` -> the derived double with 17 significant digits (`%.17g`); `... value limits VALUE_JSON`
  -> `OK` or `ERR E_LIMIT <what>` (checks blob/text/tuple limits and uint/int `value` fits `bits`).

## r1 (integration decisions; win over the text above)
1. `scene-item.item_id` is an integer 0..2^53 (OBS scene item ids are int64); `scene_id` a string.
2. `minor`: any integer 0..2^53.
3. Required fields: `slots[].requires` (array of 0..64 strings, each `[a-z0-9_.]{1,64}`); `routes[].enabled`
   (boolean). Missing -> `E_SCHEMA <field>`.
4. An address (route `match.address`, `emit.address`, event `address`) has 1..16 segments; 0 -> `E_SCHEMA address`
   (event: `E_SCHEMA event`).
5. `target_key` escapes `\`, `|` and `=` in values with a backslash, so distinct targets never share a key.
6. EVENT.json is parsed with the same strict rules as mapping files (duplicate keys `E_DUPKEY`, BOM `E_BOM`,
   invalid UTF-8, NUL, non-finite, depth, size).
7. `AsDouble` of a 64- or 128-bit integer is correctly rounded (round to nearest, ties to even) from the
   exact integer, not by converting two halves.
8. `value limits`: an int/uint `value` outside `bits` (including i128 min - 1 and a uint7 of 128 inside a
   tuple) is `E_LIMIT value`, wherever it is nested.
