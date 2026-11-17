# K1 design brief: the JeiOBS control graph (task 006, M6)

Repo ~/oss/jeiobs (OBS Studio 33.0.1 fork; C/C++, Qt6 frontend, libobs, obs-websocket plugin). Read:
docs/jeiobs/ROADMAP.md (M6 section, M1, M3, M4 R1), docs/jeiobs/tasks/006-control-protocols.md,
docs/jeiobs/WORKSPACE_FORMAT.md (how mappings would be saved; secrets/redaction rules),
docs/jeiobs/DECISIONS.md. Owner direction (verbatim): "make sure there's full MIDI1/2.0 flexibly
routable along with mackie hui, osc, and whatever open control standards out there commonly used".

Decide, concretely (be specific: types, file layout, JSON examples, thread model; cite file:line
of OBS code you rely on; max ~1000 words):
1. The model: sources, targets, routes, transforms, feedback channels. How is a MIDI 2.0 UMP
   (32/64/96/128-bit, groups, per-note controllers, 32-bit values) represented next to an OSC
   message (typed args, address patterns, bundles/time tags), a MCU/HUI surface event and an
   obs-websocket request, in ONE value type without lossy normalization? Value precision, units,
   relative vs absolute encoders, soft takeover.
2. Addressing targets in OBS: scenes, sources, filters, properties, hotkeys, mixer (volume, mute,
   monitoring), outputs, plugin parameters (M3), listen bus (M4 R3). Stable IDs: what OBS gives
   (source/scene UUIDs, names), what breaks on rename, how to bind a mapping to a thing that is
   missing at load (unresolved state). Use libobs APIs only through a narrow adapter (name them).
3. Routing semantics: many-to-many, split/merge, thru, filters, banks/layers/modifiers, MIDI
   learn, feedback loops (echo suppression, loop detection), priority and conflict rules.
4. Threading and real time: where each protocol I/O runs, queues (bounded, lock-free where
   needed), flood policy (coalesce/drop), how a control changes an audio-thread object safely (OBS
   audio callbacks, obs_source_* thread rules), sample-accurate vs block-accurate.
5. Library layout: inert shared C++ library `shared/jeiobs-control/` (no Qt, no libobs runtime,
   like jeiobs-workspace) vs frontend binding; what is unit-testable headlessly; protocol
   plug-ins (`ProtocolBackend` interface): capabilities negotiation.
6. Persistence: mapping files in the workspace (M1): schema fragment, secrets (OSC/API tokens),
   device identity (UMP endpoint product instance ID, USB serial, port name), missing devices.
7. Security: network listeners default off/loopback, peer allowlists, rate limits, bounded
   parsers; what a malicious OSC/MIDI peer can reach (can it start a stream? change an output
   URL? read a stream key?). Which targets are NOT remotely controllable by default.
8. First implementation slices (PR-sized) K1-PR1..PR4, and how K2 (MIDI 1.0/2.0 core), K3
   (Mackie/HUI), K4 (OSC) plug in without rewriting K1.
Do not edit files.
