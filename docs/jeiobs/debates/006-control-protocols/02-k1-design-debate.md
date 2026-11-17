# Peer debate — 2026-11-16T23:47:17Z
Topic: Design the JeiOBS control graph K1 per the brief. Round 1: propose and critique. Round 2: converge; list what you still disagree on.
Rounds: 2   Roles: claude=implementor codex=adversarial-reviewer
Context file: /tmp/jeiobs-k1/brief.md


[R1 CLAUDE] [claude did not complete within 600s or exited non-zero]
[R1 CODEX] Claude produced no reviewable content, so there is no specific Claude claim to agree with or refute. I agree only with the brief’s non-negotiables: one graph, lossless protocol payloads, bounded queues, no audio-thread I/O, unresolved device bindings, and opt-in network exposure (`ROADMAP.md:220-235`, `tasks/006-control-protocols.md:22-26`).

My concrete position:

1. Model

Use one immutable `ControlEvent`, but do not force every protocol into a scalar:

```cpp
struct ControlEvent {
    SourceId origin;
    uint64_t sequence;
    uint64_t monotonic_ns;
    WirePayload payload;       // MidiUmp | OscPacket | SurfaceEvent | WsRequest
    optional<ScalarView> view; // derived, never canonical
};
```

`MidiUmp` stores `std::array<uint32_t, 4>` plus word count 1–4, preserving 32/64/96/128-bit UMPs, group, channel, note, per-note index, raw values and message type. `OscPacket` preserves message/bundle structure, timetag, address, type tags and bounded typed arguments. Surface events preserve control ID, touch, button edge and relative/absolute semantics. Websocket requests preserve operation name and bounded JSON bytes/AST.

`ScalarView` carries exact integer width, signedness, units (`linear`, `dB`, `normalized`, etc.), resolution, and relative/absolute mode. Conversion is explicit in a transform; no implicit float normalization. Soft takeover state is keyed by `(route, source, target, bank)`.

2. Targets and OBS adapter

`TargetRef` should be:

```json
{"kind":"source-property","source_uuid":"...","property":"volume"}
```

Use source UUID, not name: OBS exposes UUID and name separately (`libobs/obs.h:1194-1210`). Scene items need `{scene_uuid, item_id}` because `obs_sceneitem_get_id` exists (`libobs/obs.h:1718,1814`). Filters use parent source UUID plus filter source UUID; never display-name matching. Properties use a typed allowlist obtained through `obs_properties_*`; do not expose unrestricted `obs_source_update`.

The adapter API should be only `resolve/read/apply/subscribe/enumerate`, with strong references held during application. Missing objects remain `unresolved` with last-seen UUID/name/type and an explicit reason. Rename does not break a UUID binding; deletion does.

The frontend adapter must marshal UI operations. `obs_frontend_set_current_scene` already invokes Qt with `WaitConnection()` (`frontend/OBSStudioAPI.cpp:57-75`), so protocol threads must not call it directly. Volume/mute are safe only through the control thread: OBS timestamps and queues volume actions under `audio_actions_mutex` (`libobs/obs-source.c:4552-4575`, `:5067-5087`). They are block/tick-accurate, not sample-accurate. `obs_output_start` can initialize a service and start an output (`libobs/obs-output.c:396-415`), so it requires a privileged explicit target, rate limit and confirmation policy.

3. Routing

Routes are many-to-many with ordered predicates, transforms, and feedback edges. Higher priority wins; equal-priority conflicting absolute writes are rejected at validation unless an explicit reducer is declared. Relative events preserve order. Absolute fader floods coalesce to latest; note/button/SysEx/action events never coalesce. Each event carries route hop count and visited route IDs; max 16 hops. Feedback additionally uses origin tokens and value-change suppression.

4. Threads

Each backend owns protocol I/O and bounded decode storage. A bounded MPSC queue feeds one graph-dispatch thread. Per-backend bounded output queues handle feedback. Coalesce absolute values, drop stale meter/display updates first, and reject new privileged actions on overflow. The graph thread alone calls OBS adapters. Audio/plugin application is block-accurate unless the plugin event contract explicitly supports sample offsets.

5. Library

```text
shared/jeiobs-control/
  include/jeiobs/control/{event,graph,target,transform,backend}.hpp
  src/
  tests/
```

No Qt, libobs, network loop, or protocol library. `ProtocolBackend` negotiates capabilities: payload types, precision, timestamps, feedback, discovery, maximum packet size, and transport security. K2/K3/K4 implement adapters against this interface, not model-specific branches.

6. Persistence

Add a workspace entry:

```json
"control": {
  "version": 1,
  "devices": [{
    "id":"midi-main",
    "protocol":"midi",
    "identity":{"ump_product_instance":"...","usb_serial":"...","port_name":"Launchpad"}
  }],
  "routes": [{
    "id":"r1",
    "source":{"device":"midi-main","group":0,"channel":0,"cc":7},
    "target":{"kind":"source-property","source_uuid":"...","property":"volume"},
    "transform":{"input":"uint7","output":"linear","soft_takeover":true},
    "priority":10
  }]
}
```

Secrets are references only; never store OSC/API tokens. Follow the workspace secret rules and fail closed (`WORKSPACE_FORMAT.md:337-378`). Device identity fallback order is UMP product-instance ID, USB serial, then port name. Missing devices retain mappings and become unresolved.

7. Security

Listeners default disabled and loopback-only. Require peer allowlists, per-peer/per-route rate limits, bounded nesting/blob sizes, and fuzzed parsers. Remote mappings default-deny stream keys, service URLs, credentials, arbitrary source settings, profile/collection changes, output destination changes, and output start/stop. Scene switching, mixer controls, hotkeys and explicitly allowlisted plugin parameters may be enabled per mapping. `obs-websocket` must not be treated as trusted merely because it is local.

8. PR slices

- K1-PR1: inert event/value/graph/transform model, validator, headless tests.
- K1-PR2: OBS target adapter, UUID resolution, queued frontend dispatch, fake-adapter tests.
- K1-PR3: feedback, learn, banks, takeover, conflict/loop/flood tests.
- K1-PR4: workspace persistence, capability negotiation, frontend binding, security gates.

K2 adds MIDI/UMP translation; K3 adds MCU/HUI surface semantics; K4 adds OSC packet/pattern semantics. None should require changing `ControlEvent`, `Route`, or `TargetRef`.

What Claude missed is therefore all eight requested decisions: no value representation, target identity policy, conflict semantics, thread/queue behavior, persistence schema, security boundary, adapter API, or PR decomposition.
[R2 CLAUDE] **Implementor reply to Codex (round 1).** Most of your skeleton holds up. Eight of its claims are wrong or can't be built as written, and one is a safety bug. I checked everything below against this worktree. The obs-websocket submodule (`e28f849`) isn't checked out here, so I couldn't verify any obs-websocket claims.

## Agree
- Bind sources by UUID, never by name. Scene items are `{scene_uuid, item_id}` (`obs.h:1728,1814`). Missing objects become unresolved with their last-seen data. Properties go through an allowlist, never raw `obs_source_update`.
- Many protocol threads feed one MPSC queue into one dispatcher. Absolute values coalesce; notes, buttons, SysEx and actions never do.
- Listeners are off by default and loopback-only. Your default-deny list stands. The shared library is inert, and K2–K4 must not change the core types.

## Dispute

1. **"Volume/mute are block/tick-accurate."** Wrong. `obs_source_set_volume` stamps `os_gettime_ns()` (`obs-source.c:4555`). `apply_audio_actions` then places the change at a frame offset inside the 1024-frame tick, while holding `audio_actions_mutex` on the audio thread (`:5291-5330`). So the change lands at a sample position, but that position is the call time: any queueing delay is heard directly as timing error. The action list also drains only for sources in the active audio tree (`obs-audio.c:599-624`) and is freed only when the source is destroyed (`obs-source.c:856`). A 1 kHz MIDI 2.0 or OSC fader on an inactive source therefore grows memory without limit. **Fix:** coalesce per target to at most one apply per audio tick, always, not just when a queue overflows.

2. **"The graph thread alone calls OBS adapters."** That stalls MIDI thru and faders behind the UI:
   - `obs_frontend_set_current_scene` blocks the caller (`BlockingQueuedConnection`, `qt-wrappers.hpp:82-85`, `OBSStudioAPI.cpp:67-75`).
   - `obs_source_update` runs the source's update callback synchronously for non-video sources (`obs-source.c:1103-1118`).

   Use three lanes:
   - **Fast:** the graph thread handles mute, volume, balance, plugin-parameter rings and MIDI out.
   - **UI:** posted with `QueuedConnection`, never waited on, coalesced per target. Hotkeys go here too, triggered the way the frontend's own router does it (`OBSBasic_Hotkeys.cpp:75-87`).
   - **Slow worker:** property updates and outputs.

   The UI reads graph state from snapshots and never blocks on the graph.

3. **Transport.** Never call `obs_output_start` directly. The public `obs_frontend_streaming_start/stop` call (`OBSStudioAPI.cpp:226-234`) also skips the `WarnBeforeStartingStream`/`WarnBeforeStoppingStream` checks. Those checks only run in `StreamActionTriggered` (`OBSBasic_Streaming.cpp:391-439`). We need an additive, fork-local `RequestStreaming(origin, start)` that applies the same checks. A network peer cannot answer a confirmation dialog.

4. **`std::array<uint32_t,4>` for MIDI.** That holds one UMP packet. SysEx7/8 messages are longer, and they carry MCU LCDs and scribble strips, MMC, MSC, MTC full-frame and MIDI-CI. The payload must be a bounded, reassembled message from a preallocated pool, with a per-endpoint maximum (oversize is dropped and counted). 14-bit CC pairs and RPN/NRPN are stateful multi-packet sequences. The K2 codec assembles them, and the raw packets are kept for thru.

5. **Routes that match on `WirePayload` (your `{"cc":7}`).** That makes the core understand MIDI. Instead, codecs emit:
   - a `ControlAddress`, a path of typed segments, for example `[midi2,g0,ch0,pnc,note60,idx3]`, `[mcu,u0,strip3,fader]` + `[…,fader_touch]`, or `[osc,"/xy",arg1]`;
   - a `ControlValue`: `{kind: Trigger|Bool|Uint|Int|Float|Relative|Text|Blob, bits, u/i/f/steps, pooled data, unit}`.

   The core only matches equality, ranges and prefixes. The wire payload travels opaquely for thru and plugin events. A multi-argument OSC message or a bundle becomes one *atomic batch*: dispatched in a single step, but not a transaction across OBS. Future OSC time tags go into a bounded scheduler with a horizon cap.

   Your `ScalarView` is labelled "derived" but is what routes operate on. Call it the canonical value and keep `bits`, so feedback can map back to the same code.

6. **Rejecting equal-priority writers at validation.** This breaks the main use case: an MCU fader, a tablet OSC fader and the OBS mixer all driving one volume. It also can't be decided statically once banks and layers exist. Arbitrate at runtime: a touch lease (MCU touch or UI slider press) beats soft takeover for non-holders, which beats last writer. Priority only breaks ties between simultaneous leases. Validation only warns about duplicate (address, layer) → same target.

7. **Hop count plus visited route IDs.** These can't see external loops: feedback that a device or another app echoes back arrives as a fresh event at hop 0. A visited set is also unbounded per-event state. Use:
   - static cycle rejection when the graph is compiled (matching R1's "reject cycles by default");
   - an echo window per (endpoint, address), compared in the source's own code space (uint14 back to uint14);
   - an oscillation breaker that disables the route and shows it.

   Origin tokens only work for synchronous signals. `volume` fires on the calling thread before the action is queued (`obs-source.c:4564-4566`), so a thread-local "applying route R" tag works there. Deferred updates and UI-lane actions need the code-space comparison.

8. **Persistence.** Two conflicts with `WORKSPACE_FORMAT.md`:
   - **`"control"` inside the manifest.** The manifest indexes captured files (§4), and mappings are live configuration that must exist without any snapshot.
   - **Serials and product-instance IDs in the portable mapping.** §4 rejects exporting the device inventory "because names, endpoint IDs and serials identify hardware and people". The UMP Product Instance ID is effectively a serial number.

   Proposal, under `plugin_config/jeiobs-control/`:

   | File | Contents | Capture and export |
   | --- | --- | --- |
   | `mappings.json` | portable: logical device slots by model identity, routes, banks, transforms | captured normally |
   | `bindings.json` | slot → product-instance ID, USB serial, MCU serial or port name; listener interface; peer IPs | captured `opaque-unreviewed`, so export fails closed and defaults to omit |
   | `secrets.json` | Network MIDI 2.0 shared secrets, websocket client passwords | never captured (`secret-store`, like `obs-browser/`) |

   A missing binding leaves the slot unresolved with its routes intact.

   Routes that target collection objects go in a per-collection sidecar, `collections/<stem>.json`, as C2 requires. Each records its scene-UUID set so a rename can be reconciled by unique match, as in §9. jeiobs moves the sidecar on `OBS_FRONTEND_EVENT_SCENE_COLLECTION_RENAMED` (`obs-frontend-api.h:62`). Don't store mappings via `obs_frontend_add_save_callback`: C2 bars embedding without demonstrated stock-OBS preservation.

## Missed

- **Resolving by UUID needs checks.** `obs_get_source_by_uuid` searches every source, including private sources and filters (`obs-source.c:280-289`, `obs.c:2053-2056`). The adapter must verify the kind and the filter's parent (`obs_filter_get_parent`) and refuse private sources.
- **Hotkeys.** `obs_hotkey_id` exists only at runtime. Bind `{registerer kind, registerer UUID, hotkey name}` instead.
- **M3 plugin parameters.** `guiToDsp` is the SDK's single-writer ring. It already has writer call sites (`VST3ComponentHolder.cpp:44,92`), always with offset 0. The graph needs its own `ctrlToDsp` single-writer ring, merged at `VST3Plugin.cpp:401`, with `sampleOffset = (event_ns − block.timestamp)·rate`, clamped. That is the only place sample-accurate timing is real.
  - On async sources the filter runs on the capture thread, under `filter_mutex` (`obs-source.c:4085-4086`). So the real-time consumer differs per source.
  - Backends must stamp events with the same monotonic clock as `os_gettime_ns`, injected into the library.
- **Stuck notes.** "Never coalesce" isn't an overflow policy. If a note-on is dropped, drop its note-off. If a note-off can't be queued, raise an atomic all-notes-off flag that the next block honours.
- **Backend timers.** HUI needs a ping/keepalive and MCU needs its handshake and device query. `ProtocolBackend` needs `tick(now)` and per-backend state machines. Split each backend into a pure codec (headless fixtures) and a platform transport.
- **Security:**
  - An incoming OSC address is itself a pattern: one `/*` packet fans out to every target. Cap matches per message, and never let a wildcard match a privileged target.
  - OSC or MIDI → obs-websocket launders unauthenticated input into an authenticated API (for example, rewriting the stream service URL or key). The websocket target needs a request-type allowlist and accepts no network-origin sources by default. Prefer obs-websocket's vendor request/event API for inbound and feedback.
  - Feedback and OSCQuery are read paths. The deny list covers reads: §8 secret fields and URLs never go to scribble strips or peers. OSCQuery lists only mapped targets.
  - Any local process can create a virtual MIDI port. Use trust classes `hw | virtual | loopback-net | remote-net`. Learn mode can't be armed remotely.
  - Stream key, service URL and output destination are a **hard** deny, not configurable.

## Revised slices

- **PR1:** all types, including banks, layers, leases and feedback (otherwise PR3 rewrites PR1). Also the validator, graph compile with cycle check, and the transform engine. Property tests: every uint7/uint14/uint32 code survives code → double → code.
- **PR2:** headless runtime: dispatcher, lanes, coalescing, leases and takeover, echo and oscillation handling, learn state machine, injected clock. Flood test: 10⁶ events, bounded memory, p99 latency.
- **PR3:** frontend module `plugins/jeiobs-control`, modelled on obs-websocket, so disabling it is the rollback. Libobs adapter, trust and capability classes (they must exist from the first PR that can reach OBS, not PR4), unresolved state, a fake-backend loopback test, no network.
- **PR4:** the three files, jeiobs-workspace allowlist roles, minimal mapping and learn UI.
- **K2:** codecs start after PR1, in parallel.
[R2 CODEX] **Adversarial review of Claude’s proposal**

I agree with the UUID-first binding, unresolved targets, property allowlists, separated codec/transport design, bounded parsing, loopback/off-by-default listeners, and refusal to call `obs_output_start` directly. The cited `OBSStudioAPI.cpp:226-234` versus `OBSBasic_Streaming.cpp:391-439` distinction is a real safety issue.

I also agree that `std::array<uint32_t,4>` is insufficient for SysEx, MMC, MTC, and HUI display traffic. Raw UMP preservation must exist independently of semantic routing.

### Disputes

1. **The “fast lane” must not call libobs directly.**

Claude correctly identifies that frontend calls can block and that `obs_source_update` executes callbacks synchronously (`obs-source.c:1103-1118`). However, moving volume, mute, or plugin writes into a graph-thread “fast lane” does not make those calls safe or real-time.

The graph should emit typed adapter commands:

```cpp
ControlCommand {
  TargetId target;
  Value value;
  uint64_t event_time_ns;
  ApplyMode mode; // latest, queued, sample_offset
}
```

The adapter owns the execution mechanism:

- atomics/latest-value slots for volume, mute, balance;
- bounded DSP rings for plugin parameters;
- Qt queued calls for frontend-only operations;
- a worker for properties and output configuration.

`obs_source_set_volume` stamping `os_gettime_ns()` (`obs-source.c:4555`) proves public volume changes are arrival-time/block-approximate, not sample-accurate. Also, coalescing once per tick does not fix inactive-source accumulation. Inactive targets need bounded shadow state, applied when they become active; never append indefinitely to OBS’s per-source action list.

2. **The proposed `ControlValue` is still underspecified and risks lossy normalization.**

A `double`-derived scalar cannot be the canonical representation. Use exact tagged values:

```cpp
Value =
  Trigger
| Bool
| UInt { width_bits, raw_u128, unit }
| Int  { width_bits, raw_i128, unit }
| FloatBits { format, raw_bits } // IEEE32/64 or future format
| Text
| Blob { pool_handle, length }
| Tuple { bounded_values[] };
```

`Relative` should be metadata on a numeric value, including encoding (`twos_complement`, `sign_magnitude`, `binary_offset`) and step semantics—not a vague scalar kind.

The event also needs both:

```cpp
WireEnvelope { protocol, raw_ump_or_bytes, packet_count }
SemanticPayload { address, value }
```

OSC multi-argument messages and bundles should preserve heterogeneous tuples, nesting, timetags, and original ordering. A bundle can be one dispatch unit, but must report partial target failures; “atomic” must not imply transactional OBS changes.

3. **Runtime arbitration needs explicit merge policy, not only leases.**

A touch lease is useful for MCU motor/fader controls, but “touch beats soft takeover, which beats last writer” is not a complete many-to-many rule. Soft takeover is a handoff policy, not an arbitration priority.

Each target needs an explicit policy:

```cpp
MergePolicy = LastWriter | Priority | TouchLease | Sum | Min | Max | BooleanOr
```

Leases require owner, priority, expiry, and release behavior. Otherwise a disconnected surface can retain control forever. Validation should reject ambiguous reducers, not merely warn about duplicate routes.

4. **Static cycle rejection is insufficient.**

Static rejection works only after wildcard/prefix routes and transforms are expanded into a finite dependency graph. Dynamic address generation, external device echoes, and another application’s feedback loop remain runtime problems.

Use all three:

- compile-time cycle rejection where resolvable;
- bounded hop count and fixed-size route provenance;
- per-endpoint echo suppression using protocol-native code-space values and timestamps.

A route-disabling oscillation breaker is necessary, but must be rate-limited and locally scoped; otherwise a malicious peer can disable legitimate routes by injecting alternating values.

5. **The persistence proposal is still too portable by default.**

A `bindings.json` containing product-instance IDs, serials, port names, and peer IPs should be local state, not merely an `opaque-unreviewed` workspace file. “Export fails closed” is weaker than not placing hardware identity in the portable workspace at all.

Portable mappings should contain logical slots and capability requirements:

```json
{
  "slot": "main_surface",
  "requires": ["mcu.fader", "feedback.motor"],
  "routes": []
}
```

Per-user bindings and secrets belong outside capture/export, with explicit import/export. Scene UUIDs should be authoritative; collection fingerprints and “unique set” reconciliation may silently bind a mapping to the wrong collection. Automatic reconciliation should produce an unresolved candidate, never silently select one.

6. **The proposed PR1 is too large.**

“All types, banks, layers, leases, feedback, validator, compiler, and transforms” is not a small reviewable PR. PR1 should establish immutable event/value/address types, bounded ownership rules, schema versioning, and a minimal compiler. Arbitration, feedback, and learn should follow with property tests. Otherwise every later protocol PR depends on an unstable, oversized foundation.

### Missed requirements

- `ProtocolBackend` needs lifecycle, egress queues, shutdown, backpressure, and capability negotiation—not just `tick(now)`.
- Capability negotiation must describe value widths, feedback support, timestamps, transport limits, and address namespaces.
- MIDI and OSC clocks need explicit receipt time versus device/timetag time, with late-event policy and bounded scheduling horizon.
- Security needs per-peer and per-route rate limits, auditability, and target capability classes. Stream keys, service URLs, and secret fields should be hard-deny; stream start/configuration should be local-admin-only by default.
- Plugin targets require instance identity plus parameter identity/URI, not only source identity. The claimed `ctrlToDsp` integration remains unverified until the submodule and plugin code are inspected.
- Tests need parser fuzzing, malformed OSC/UMP cases, reconnect/device-loss tests, stale-lease tests, route-loop tests, and proof that disabled control plugins leave OBS behavior unchanged.

The core direction is sound, but the design currently overclaims real-time safety and portability. The critical correction is: **the graph owns deterministic intent; adapters own thread-safe OBS application.**
