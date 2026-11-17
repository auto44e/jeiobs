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

## K1 design (decided 2026-11-16)

From a Claude/Codex design debate ([debates/006-control-protocols](../debates/006-control-protocols/));
the lead resolved the rest. Nothing is built yet. Checked against the code in this tree; the
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
