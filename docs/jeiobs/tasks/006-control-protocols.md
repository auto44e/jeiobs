# Task 006 — control surfaces and open control protocols (M6)

Status: authorized scope; K1-K6 pending. Owner direction, 2026-11-16 (verbatim):
"make sure there's full MIDI1/2.0 flexibly routable along with mackie hui, osc, and
whatever open control standards out there commonly used".
Read [ROADMAP](../ROADMAP.md) M6, M1 (mappings are saved state), M3 (plugin parameters
and MIDI event inputs), M4 R1 (graph IDs) and M5 (timecode sources).

1. K1: design the control graph (sources, targets, transforms, feedback, learn, device
   identity, persistence, threading and flood policy) with a debate record; implement the
   model as a shared C++ library with headless tests, then the frontend binding.
2. K2: MIDI 1.0/2.0 core (UMP, translation, MIDI-CI, discovery/negotiation), native
   backends on all three OSes, virtual ports, RTP-MIDI and Network MIDI 2.0, clock/MTC/
   MMC/MSC; MIDI into plugin event inputs and out to ports.
3. K3: Mackie Control Universal and HUI surfaces, extenders and emulation modes.
4. K4: OSC 1.0/1.1 (UDP, TCP/SLIP, bundles, patterns), OSCQuery, documented namespace.
5. K5: Ableton Link, sACN/Art-Net, AES70/OCA, generic HID, obs-websocket as a graph
   source/target; each behind a capability boundary.
6. K6: capability matrix, cross-platform loopback/virtual-device tests, fuzzing,
   latency/jitter numbers and per-device hardware records.

Constraints: network listeners off by default and loopback-bound until the user opts in,
with peer allowlists and rate limits; control I/O never on the audio thread; no vendor code
for MCU/HUI; dependency choices (for example a MIDI 1/2 library, an OSC library, Link)
get a license review and an owner-merged dependency PR. Small PRs; record exact native CI
results and which behaviour is verified on real hardware.

## K1 design (draft v1 plus amendments, 2026-11-16)

From a Claude/Codex design debate ([debates/006-control-protocols](../debates/006-control-protocols/));
the lead resolved the rest. Nothing is built yet. Status: direction agreed; the amendments below
answer the first review round, and "Open before K1-PR1" lists what must still be settled. Checked against the code in this tree; the
obs-websocket submodule is not checked out here, so claims about it are unverified.

**Principle.** The graph owns deterministic intent; adapters own thread-safe application to OBS.
The graph never calls libobs directly.

1. **Values and events.** One event carries a `WireEnvelope` (protocol, raw UMP or bytes, packet
   count; opaque, used for thru and plugin events) and a `SemanticPayload` (a `ControlAddress` and
   a `ControlValue`). A `ControlAddress` is a path of typed segments, for example
   `[midi2,g0,ch0,pnc,note60,idx3]`, `[mcu,u0,strip3,fader]`, `[osc,"/xy",arg1]`. `ControlValue` is
   an exact tagged value: Trigger, Bool, UInt{width,raw128,unit}, Int, FloatBits{format,raw},
   Text, Blob (pooled), Tuple (bounded, ordered; OSC bundles keep nesting and time tags). Relative
   is metadata on a numeric value (encoding: two's complement, sign-magnitude, binary offset;
   step). A `double` view exists for transforms only; feedback maps back in the source's own code
   space (uint7/uint14/uint32 survive code → value → code). SysEx, MMC, MTC and MIDI-CI payloads
   are bounded, reassembled messages from a preallocated pool with a per-endpoint maximum
   (oversize is dropped and counted). A bundle is one dispatch unit, not a transaction: partial
   target failures are reported.
2. **Targets.** Bind by UUID, never by name: scene items `{scene_uuid, item_id}`, filters
   `{parent_uuid, filter_uuid}`, hotkeys `{registerer kind, registerer uuid, hotkey name}` (an
   `obs_hotkey_id` exists only at runtime), plugin parameters `{instance, parameter id or URI}`.
   The adapter checks the kind, refuses private sources, and routes property changes through an
   allowlist, never raw `obs_source_update`. A target missing at load is `unresolved` and keeps its
   last-seen data; a rename reconciliation proposes a candidate and never binds silently.
3. **Applying commands.** The graph emits `ControlCommand {target, value, event_time_ns, mode}`
   with mode `latest`, `queued` or `sample_offset`. Adapter mechanisms: latest-value slots with at
   most one apply per audio tick for volume, mute and balance (`obs_source_set_volume` stamps call
   time, so queue delay is heard); bounded shadow state for inactive sources (OBS's per-source
   action list grows without bound there: never append to it for an inactive target); a
   single-writer DSP ring for plugin parameters; Qt queued calls, never waited on, for frontend
   operations (`obs_frontend_set_current_scene` blocks); a worker for properties and outputs.
   Streaming and recording start and stop go through a new additive, fork-local
   `RequestStreaming(origin, start)` that applies the warn-before checks the public frontend API
   skips; a network peer cannot answer a dialog, so remote origins are refused unless allowed.
4. **Arbitration.** Every target has an explicit merge policy: LastWriter, Priority, TouchLease,
   Sum, Min, Max or BooleanOr. Leases have owner, priority, expiry and release (a disconnected
   surface cannot hold a target). Soft takeover is a handoff policy, not a priority. Validation
   rejects an ambiguous reducer; several writers to one target are fine with a stated policy.
5. **Loops.** Static cycle rejection after wildcards and transforms are expanded, a bounded hop
   count with fixed-size route provenance, a per-endpoint echo window in code space, and a local,
   rate-limited oscillation breaker (so a peer cannot disable routes by injecting alternating
   values).
6. **Threads.** Backend I/O threads feed one bounded MPSC queue into one dispatcher. Absolute
   values coalesce per target; notes, buttons, SysEx and actions never coalesce. A dropped note-on
   drops its note-off; a note-off that cannot be queued raises an atomic all-notes-off flag. One
   injected monotonic clock (the same clock as `os_gettime_ns`); events carry receipt time and
   device or time-tag time with a late-event policy and a bounded scheduling horizon. Sample
   accuracy exists only where a ring is consumed in the audio callback (plugin parameters); the
   consumer differs on async sources (capture thread, under the filter mutex), which PR3
   verifies against the real code.
7. **Library.** `shared/jeiobs-control/` is inert (no Qt, no libobs runtime), like
   jeiobs-workspace. `ProtocolBackend` is a pure codec (headless fixtures) plus a platform
   transport, with capability negotiation (value widths, feedback, timestamps, limits, address
   namespaces), lifecycle, egress queues, backpressure and `tick(now)` (HUI keepalive, MCU
   handshake). The frontend side is `plugins/jeiobs-control`, modelled on obs-websocket;
   disabling it leaves OBS unchanged and is the rollback.
8. **Persistence** under `plugin_config/jeiobs-control/`: `mappings.json` (portable: logical device
   slots with capability requirements, routes, banks, transforms) is captured by workspaces;
   `bindings.json` (slot → product-instance ID, serial, port name, listener interface, peer
   addresses) is local state outside capture and export, with explicit import and export;
   `secrets.json` is never captured. Collection-scoped routes sit in a per-collection sidecar
   (ROADMAP C2) with scene UUIDs as authority. Adding these files to the workspace allowlist
   needs a `compat` feature (WORKSPACE_FORMAT section 5) and happens in PR5.
9. **Security.** Trust classes `hw`, `virtual`, `loopback-net`, `remote-net`. Hard deny, not
   configurable: stream key, service URL, output destination and any section 8 secret field, for
   reads (feedback, OSCQuery) as well as writes. Stream and record start/stop and output
   configuration are local-admin only by default. An OSC address pattern is capped in fan-out and
   never matches a privileged target. Learn mode cannot be armed remotely. An obs-websocket
   target needs a request-type allowlist and takes no network-origin source by default (it would
   launder unauthenticated input into an authenticated API). Per-peer and per-route rate limits,
   an audit log, fuzzed parsers.
10. **Slices.** K1-PR1: immutable types (address, value, event), schema version, validator and a
    minimal compiler. PR2: transforms with property tests. PR3: headless runtime (dispatcher,
    coalescing, merge policies, leases, takeover, echo and oscillation handling, learn state
    machine, injected clock; flood test of 10^6 events with bounded memory). PR4: the frontend
    module and adapter, with trust classes from the first PR that can reach OBS, unresolved state,
    `RequestStreaming`, a fake-backend loopback test and no network. PR5: persistence files,
    workspace allowlist rows and a minimal mapping and learn UI. K2 codecs start after PR1.

**Rejected:** calling libobs from the graph thread; a `double` as the canonical value; embedding
mappings in the manifest; serials and addresses in portable files; static-only loop rejection;
validation that rejects every equal-priority writer pair.

### Amendments after review round 1 (these override the numbered items above)

A1. **Authorization.** Every command carries a `Principal {origin class (hw, virtual,
    loopback-net, remote-net, local-ui), endpoint id, peer id}`. The adapter checks `(principal
    class, target capability class)` against a policy table before applying anything. Capability
    classes: `mix` (volume, mute, balance, monitoring), `scene`, `source-property`, `filter`,
    `plugin-param`, `hotkey`, `output-control` (start/stop stream, record, replay, virtual
    camera; output configuration), `profile` (profile and collection switching), `meta` (learn,
    mapping edits). Defaults: `mix`, `scene`, `plugin-param` allowed for `hw` and `loopback-net`;
    `output-control`, `profile`, `meta` local-admin (`local-ui`) only; `remote-net` gets only what
    the user grants per endpoint. Hard deny for everyone: stream key, service URL, output
    destination, any WORKSPACE_FORMAT section 8 secret field.
A2. **Everything that can start an output is `output-control`,** not only direct calls: hotkeys
    (a hotkey is classed by what it triggers; a hotkey of unknown effect is `output-control`),
    profile and scene-collection switching (they can change service and outputs), and the
    streaming, recording, replay-buffer and virtual-camera paths are all routed through the new
    fork-local `RequestOutput(origin, output kind, start|stop)`, which applies the warn-before
    checks. A network peer cannot answer a dialog, so those need an explicit grant.
A3. **Target kinds** (K1 must define all of them, each with identity, read path, write path):
    source and filter properties; scene and scene-item state (visibility, transform); mixer
    (volume, mute, balance, monitoring type, sync offset, track mask); transitions (by name and
    kind, because transitions have no stable UUID; a rename is delete plus add, as in
    WORKSPACE_FORMAT section 9); hotkeys; outputs (stream, record, replay buffer, virtual
    camera); plugin parameters and plugin events (notes and MIDI into a plugin's event input);
    M4 graph and listen-bus controls (sends, PFL/AFL, dim, monitor level, as R1 and R3 define
    them); and egress targets (a MIDI/OSC/surface output port) so a route can end at hardware.
A4. **Private sources and filters.** The adapter refuses private sources as targets, but a filter
    of a public source is a legal target (resolve through the filter's parent and verify it).
    Hotkey registerers: sources and filters have UUIDs; frontend and output registerers are bound
    by `{registerer kind, registerer name, hotkey name}` with an unresolved fallback.
A5. **Reading targets (feedback, pickup, arbitration).** Each adapter exposes a read path: a
    `TargetState` snapshot kept current by libobs signals where they exist (volume, mute, item
    visible, scene change, output state) and a bounded poller otherwise (properties, balance).
    Reads never block the dispatcher. Soft takeover compares against the snapshot; feedback to
    surfaces is generated from snapshots. The snapshot can lag the real value: feedback is
    labelled with its age.
A6. **Queue policy.** One bounded queue per endpoint (isolation), drained by the dispatcher
    round-robin with a per-endpoint quota. Overflow: coalescible absolute values keep the newest;
    non-coalescible events (notes, buttons, SysEx, actions, bundles) are rejected whole and
    counted, with the stuck-note rule; an endpoint that overflows repeatedly is throttled and
    reported. Privileged (`output-control`, `profile`, `meta`) commands use a small separate
    lane so a flood cannot delay or starve them. Relative values accumulate (sum of deltas)
    during coalescing; Sum, Min and Max reducers coalesce by the same reducer; writers outside the
    graph (the OBS UI, obs-websocket) are observed through the read path and count as a writer
    with LastWriter policy.
A7. **Audio-thread facts to verify, not assume** (PR3 gate): the per-source audio action list
    keeps growing while `audio_ts` does not advance (inactive or stalled sources), so the adapter
    applies volume and mute through a latest-value slot and owns its shadow state; the shadow can
    be stale, so reads come from the OBS value when the source is active. Balance is a plain
    field read on the audio thread (`obs-source.c:4057-4059`, `:5725-5738`), so it gets the same
    atomic slot. Filters run on whichever thread pushes audio, with raw timestamps and VST3
    re-blocking, so `sample_offset` is only meaningful for a filter whose real-time consumer was
    verified; otherwise the mode degrades to `latest` and says so.
A8. **Persistence correction.** `bindings.json` is captured by workspaces as `opaque-unreviewed`,
    restored only on the same `machine_id`, and omitted from every export (the export gate
    already refuses unreviewed entries; there is no separate bindings export path). This keeps
    M1's promise that a snapshot restores a machine's setup while hardware identity never leaves
    the machine. `secrets.json` stays out of capture. The sidecar is keyed by the collection's
    scene-UUID set with a name fallback and an `unresolved` state on mismatch, not by the OBS file name. The generated UID of WORKSPACE_FORMAT section 9 exists only inside a captured revision, so it cannot key live configuration; PR5 decides whether a live identity source is added.
A9. **Trust and scope details.** A trust-by-target matrix is part of PR4 (table of class × target
    × default). OS-level MIDI ports that are network-backed (RTP-MIDI sessions, Network MIDI 2.0)
    are `remote-net`, not `hw`. Learn mode and SysEx thru are `meta` and local-admin only.
    OSCQuery lists only mapped targets the peer class may read.
A10. **K1 scope that shapes the types** (decided in outline here; PR1 fixes the exact types):
    *Transforms* are pure, bounded functions in a pipeline: scale/offset, curve (linear, log,
    custom points), invert, clamp, deadzone, quantize, 14-bit and 32-bit join and split, relative
    to absolute, toggle and momentary, threshold, and a small expression-free set (no scripting
    in K1). *Banks, layers and modifiers* are explicit state machines in the graph
    (`BankState`, `LayerState`, `ModifierSet`); a route's address may be qualified by them.
    *Learn* is a session state machine: it captures the next event from a chosen endpoint and
    proposes an address and range; the user confirms. *Device identity*: a logical slot with
    capability requirements, bound locally by product-instance ID, USB serial or port name, in
    that order of preference.

### Amendments after review round 2 (override A1-A10 and the numbered items where they differ)

A11. **Routes.** A route is `{match, guard, transform pipeline, target, merge policy}`. `match` is
     evaluated on a `ControlAddress`: per-segment equality, numeric range, set membership, prefix,
     and a single-segment wildcard (never matching a privileged target; fan-out per message is
     capped). Split: one source address feeds several routes, each independent. Merge: several
     routes to one target, governed by that target's merge policy. Thru: a route whose target is an
     egress endpoint with the identity transform and the wire envelope forwarded unchanged unless a
     channel/group remap transform is present. Filter: a guard predicate on the route (value range,
     address predicate, device state, an active bank, layer or modifier). Remapped thru is thru
     plus remap. Routes with no match, or whose target is unresolved or denied, are inert and listed.
A12. **Bindings and exports (replaces A8's export claim).** `bindings.json` is captured by
     workspaces so history restores a machine's setup, but its role is `local-only`: restored only
     on the same `machine_id` and always omitted from exports, reviewed or not (reason
     `local-only`). The projection never emits it, so no reviewed-include path can export serials or
     peer addresses. PR5 makes the WORKSPACE_FORMAT change: a `local-only` role class in sections 6
     and 7, an `incompat` feature so an older reader refuses to restore a revision with these roles,
     and rounding of an omitted secrets file's recorded size (section 6's 65,536-byte rule applies to
     it).
A13. **Grants.** Peer grants and trust overrides are machine-local state in `grants.json`, outside
     capture, export and import, so an old snapshot cannot re-grant a revoked peer and an imported
     mapping cannot widen trust. UDP source addresses are spoofable: a UDP peer allowlist is
     advisory, and no grant over UDP can give `output-control`, `profile` or `meta`. Only an
     authenticated, confidential, connection-oriented transport can hold those classes for a
     NETWORK peer (this rule is about network principals only; `hw` and `virtual` endpoints follow
     A1's policy table, so a USB MCU or HUI surface can be granted `output-control` locally): an OSC/TCP
     session that proves a secret from `secrets.json` by challenge-response with a per-session nonce
     (the secret is never sent), and that runs over TLS, or over loopback TCP that passes A14's
     checks. K4 specifies the handshake, replay protection and the TLS choice; until it does, no
     session can hold a privileged class. Network MIDI 2.0 is UDP and authenticates
     only the invitation, so it counts as UDP here: no privileged grant.
A14. **Loopback is not trusted by default.** A listener on `127.0.0.1` is reachable by web pages
     and browser sources (DNS rebinding, `no-cors` POSTs). Default grants for `loopback-net`
     apply to UDP only. TCP and HTTP listeners (OSC over TCP/SLIP, OSCQuery) default to read-only
     until the listener enforces a `Host` that equals the address it is bound to (or a configured name),
     rejects any `Origin`, sends no CORS headers,
     and closes the connection on the first malformed frame; they never get write classes by
     default. Those checks are unconditional for every TCP, HTTP and WebSocket listener, reads
     included (a mapped value streamed over an OSCQuery WebSocket to a web page or a browser
     source is a leak). OSCQuery is GET-only.
A15. **Targets, concretely** (every kind has all of: identity, read path, write path, class;
     unclassified means deny):

     | Target | Identity | Read | Write | Class |
     | --- | --- | --- | --- | --- |
     | source property | source UUID + property | settings snapshot on `update` signal | queued `obs_source_update` via allowlist | source-property |
     | filter property | filter UUID (+ parent) | same | same | filter |
     | scene, scene item | scene UUID, item id | item signals | queued frontend call | scene |
     | mixer (volume, mute, balance, monitoring, mixers, sync) | source UUID | signals (`volume`, `mute`, `audio_balance`, ...) | latest-value slot for volume, mute and balance; queued for the rest | mix |
     | transition | name + kind | frontend current-transition event | queued frontend call | scene |
     | hotkey | registerer + name | none | queued, through the frontend hotkey router | by effect; unknown is output-control |
     | output (stream, record, replay, virtual camera) | output kind | output signals | `RequestOutput(origin, kind, start or stop)` | output-control |
     | profile, collection | name + scene-UUID set (collections) | frontend events | `RequestProfileSwitch(origin, target)` with the same confirmation rules | profile |
     | plugin parameter, plugin event | instance + parameter id or URI | M3 state snapshot | single-writer ring | plugin-param |
     | M4 graph and listen bus | node/port id | graph snapshot | the R1/R3 API | mix |
     | egress endpoint | endpoint id | none | endpoint egress queue | the endpoint's trust class |

     `RequestOutput` covers only output kinds. Profile and collection switching use their own
     request because it changes service and outputs. SysEx and raw wire thru are `meta` operations: creating or enabling such a
     route needs `meta`; a mapped route then runs without it. The egress row's class is the
     endpoint's trust class, which A1's table maps to the capability classes it may carry.
A16. **Coalescing.** Keep the newest absolute value per writer and per target, then apply the
     target's reducer (Sum, Min, Max, ...). Relative values accumulate as the sum of deltas. A
     writer's successive absolute values are never summed.
A17. **A7 corrected.** The balance field is READ on whichever thread pushes audio (`process_audio`,
     called from `obs_source_output_audio`, `obs-source.c:4057-4059,4085`) and WRITTEN by the setter
     on the caller's thread (`:5735-5737`), so it is an unsynchronized cross-thread field and gets the
     same latest-value slot as volume. `audio_balance` is an existing signal (`:96,5735`): reads are
     signal driven.
     Volume and mute reads come from the signal when the source is active. While an inactive
     source holds a shadow value the read reports it as `pending`; `user_volume`, the mixer, the
     saved collection and obs-websocket keep the old value until the adapter flushes the latest
     shadow once when the source becomes active, unless a newer write by another writer (the OBS UI or
     obs-websocket, observed through the signals) arrived meanwhile; a newer observed write wins and
     the shadow is dropped.
A18. **Record, who settled what** (sources: the debate transcript `02`, the review transcripts
     `review-01` to `review-05`, and the ledger; item numbers are those of "K1 design" above).
     - Items 1 (values), 3 (applying commands), 4 (arbitration) and 5 (loops): Claude proposed in
       the debate's round 2 (Codex's round-1 skeleton came first; Claude's round-1 answer timed out),
       Codex disputed them in its round-2 reply, and the lead adopted Codex's positions where they
       differed: exact tagged values, adapters own application with the graph emitting commands,
       explicit merge policies, and layered loop protection. Claude's address-path idea was kept.
     - Items 2 (targets by UUID), 6 (threads and queues), 7 (library layout) and 9 (security
       basics): agreed by both in the debate; the review rounds then added A1-A7 and A9, each accepted
       by the finder that raised it (the ledger lists the acceptances).
     - Item 8 (persistence): Claude's round-2 proposal captured `bindings.json` as
       `opaque-unreviewed`; Codex wanted bindings outside capture and export; the first draft took
       Codex's position; review round 1 asked for capture and round 2 asked that bindings have no export path; the lead chose capture as
       `local-only` with no export path (A12), and Codex accepted A12 (`review-03/04-r2-codex.reply.md`).
     - Item 10 (slices): Claude proposed a large PR1 (all types, validator, compiler, transforms);
       Codex disputed its size; the lead took Codex's narrower PR1. A10: the lead's outline of
       transform, bank, layer, modifier, learn and device-identity scope; PR1 fixes the exact types.
     - A11 (routes): the lead's definition, in answer to Claude's finding. A15: the lead, accepted by Codex in review-03; A16: the lead, in answer to
       Claude's `claude-8` and Codex's `codex-8`, accepted in review-03. A17: the lead, corrected after both peers' findings and
       accepted by Codex in review-04.
     - A13 (grants machine-local, UDP and Network MIDI 2.0 never privileged) and A14 (listener
       checks): written by the lead as answers to Claude's run-02 findings `claude-13` and `claude-14` and then tested by its later reviews; Codex confirmed `claude-14` as
       Major and raised `codex-10` and `codex-11`, but has not stated acceptance of A13 or A14 as a
       whole.
     **Every decision above that has no flex bullet is firm.**

**Additional if-otherwise and flex.**
- Serials and peer addresses in portable files would let a mapping restore across machines;
  rejected because they identify hardware and people (WORKSPACE_FORMAT section 4). Reopen only if
  the owner wants cross-machine restores of device bindings, with an explicit user prompt.
- Rejecting every equal-priority writer pair would be simpler to validate; rejected because an MCU
  fader, a tablet fader and the OBS mixer driving one volume is the main use. Reopen if arbitration
  bugs show up in real use.
- Capture of `bindings.json` was reversed from the first draft: leaving it out of snapshots breaks
  M1's "restores a machine's setup". Reopen if `local-only` restores prove fragile.
- PR1 scope (types, validator, minimal compiler only) follows Codex's narrower proposal over
  Claude's larger PR1, so the foundation stays reviewable; reopen if PR2 keeps changing PR1's types.
- Loopback TCP/HTTP read-only by default costs convenience for local web controllers; reopen with a
  Host/Origin-checked transport and measured misuse resistance.

**If otherwise, and flex.**
- Direct libobs calls from the graph thread would have been simpler and faster to write; they were
  rejected because several calls block (`obs_frontend_set_current_scene`) and one stalled call would
  stall every surface. Reopen if measurements show the adapter lanes cost more latency than a
  surface user can notice (target: median under 5 ms, p99 under 20 ms from event to apply).
- A `double` canonical value would be simpler; rejected because it loses MIDI 2.0 32-bit codes and
  breaks feedback round trips. Reopen only with a proof that no protocol needs more than 53 bits.
- Mappings in the manifest would avoid new files; rejected because mappings are live configuration
  that must exist without any snapshot. Reopen if workspaces gain a first-class "configuration
  slice" concept.
- Static-only loop rejection is cheaper; rejected because external echo is a runtime problem.
- Reopen the whole principal and capability model if the owner wants a different default for
  remote peers.

**Open before K1-PR1** (not decided yet, must be before types are fixed): which backends can tell
a network-backed OS MIDI port from a hardware one (WinMM cannot; K2 decides the class for such
ports); the exact `ControlValue`
tuple nesting limits; the OSC address-pattern fan-out cap number; the default endpoint quota and
queue sizes; whether transforms need per-route state (for example smoothing); how `ParameterChanges`
preallocation for VST3 is sized (A7); the policy table defaults beyond the ones in A1.

## Progress

- **K1-PR1** (2026-11-17): `shared/jeiobs-control/` (C++17, no Qt, no libobs): exact tagged values,
  addresses and patterns, wire envelope, events and principals, capability classes (A15), the
  `mappings.json` v1 schema and validator, and the compile checks (hard deny, wildcard on privileged
  targets, fan-out cap 64, ambiguous merge, cycles), with the `jc-check` CLI. It settles the "Open
  before K1-PR1" limits (contract in `debates/006-control-protocols/07-k1pr1-contract.md`). Codex
  wrote the library and Claude the black-box cases blind; the cases decided eight readings
  (contract r1), and AddressSanitizer found a recursive cycle check that overflowed the stack on a
  10,000-route chain (now iterative). Linux (GCC): `jc-unit` and `jc-cases` (1128 case lines) pass,
  also under ASan and UBSan. Not yet in CI: the library needs its own workflow job (owner-merged
  `.github` change).
- Next: K1-PR2 (transform execution with property tests), then PR3 (headless runtime).
