**Request changes.** The head is still 6f8492cf9, so claude-1 to claude-10 stay open. I checked all six of Codex's findings against the code and agree with each; codex-2 is the same issue as claude-5. I'm adding one new Major, claude-11.

**Codex's findings**
- **codex-1: agree.**
  - `process_audio` reads `source->balance` twice on the thread that pushes the audio (obs-source.c:4057-4059). The setter is a plain store after the signal (:5737), with no action list and no timestamp.
  - So a balance change lands on the next buffer the source pushes, and isn't ordered with volume or mute.
  - OBS's own mixer already writes it from the UI thread (OBSAdvAudioCtrl.cpp:474,478). The fix is either to state this in the record or to choose a fork-local atomic, which is a libobs change.
  - I'd rate it Minor, but that's Codex's call.
- **codex-2: agree.** It's the same issue as claude-5, and one fix closes both.
- **codex-3: agree.** More evidence:
  - Pickup and soft takeover (ROADMAP.md:227) need the target's current value, and so does arbitration against the OBS mixer (006:65-67).
  - The change signals fire on whichever thread wrote the value: `volume` (obs-source.c:4564-4566), `mute` (:5082), `audio_balance` (:5735).
  - Meter callbacks run synchronously on the thread that pushes the audio (obs-source.c:1696 → obs-audio-controls.c:492-518 → :200-205). Sometimes that is an OS audio callback.
  - So the subscribe path needs a non-blocking handoff, and the record must name it. claude-8's point about writers outside the graph needs the same path.
- **codex-4: agree.** This overlaps claude-4, and claude-2's point that `RequestStreaming` has no stream/record selector. Two more gaps:
  - §3 applies volume, mute and balance and calls `obs_frontend_set_current_scene` (:56-61), but §2 gives sources and scenes no identity.
  - Codex's `source-property` kind (02:35) and mixer monitoring (01:17) were dropped.
- **codex-5: agree.**
  - "local-admin" (:97) is not one of the four trust classes (:94).
  - `ControlCommand` (:55) carries no origin; only `RequestStreaming` does (:63).
  - "armed remotely" (:98) is undefined.
  - Per claude-2, the hotkey path reaches the same actions with no origin at all.
- **codex-6: agree.**
  - Codex's R1 rules "drop stale meter/display updates first, and reject new privileged actions on overflow" (02:50) never reached the record.
  - The shared queue holds events before routing, so "coalesce per target" (:73-74) can only happen after dispatch. A fader flood can still fill the shared queue ahead of buttons and note-offs.

**Refinements to my own findings**
- **claude-7:** "none run in the mix callback" has one exception. A source that sets `audio_mix` pushes its audio, and runs its filters, from the audio thread (obs-source.c:5412-5439, 5508-5509). No in-tree source sets `audio_mix`; `audio_line` sets only the flag (obs.c:1219-1224). The finding stands.
- **claude-10:** WORKSPACE_FORMAT.md:416-417 rejects "a sidecar updated on rename events". So the sidecar key must not use the rename-event move proposed at 02:158.

**New, claude-11 (Major): four items in K1's own scope are not designed.**
- The task makes these part of the K1 design: "transforms, feedback, learn, device identity" (006:9-10). The brief asks for "split/merge, thru, filters, banks/layers/modifiers, MIDI learn" (01:20-21).
- The record mentions "banks" once, only as file contents (:88). It never mentions layers, modifiers, curves, toggle/momentary or pickup.
- Learn appears only as "cannot be armed remotely" (:98) and "learn state machine" (:104). Transforms appear only as "A `double` view exists for transforms only" (:44) and "PR2: transforms" (:103).
- Device matching lists fields (:89) but not the order ROADMAP.md:233-235 sets. There is no rule for two identical devices without serials.
- The debate agreed K2–K4 must not change the core types (02:98, 02:106). Bank, layer and modifier context, and transform state, therefore shape PR1's route keys and schema; Codex keyed takeover state by `(route, source, target, bank)` (02:28).
- Fix: decide each item, or mark it undecided and name the PR that decides it and the constraint it puts on PR1.

FINDING claude-1 | Major | open | K1 record lacks If-otherwise costs, Flex and per-item attribution required by DECISIONS.md:47-54,68-69
FINDING claude-2 | Major | open | Frontend hotkeys and profile switching reach stream/record/output actions, bypassing RequestStreaming and local-admin rule
FINDING claude-3 | Major | open | refuses private sources blocks all filters (obs-source.c:487-492); transitions cannot bind by UUID; hotkey registerer UUIDs exist only for sources
FINDING claude-4 | Major | open | No egress/port, plugin-event or R1/listen-bus target kinds; opaque thru cannot meet flexibly routable MIDI and VST3 needs translation
FINDING claude-5 | Major | open | bindings.json outside capture contradicts M1 snapshot scope; separate bindings export bypasses the WORKSPACE_FORMAT export gate
FINDING claude-6 | Minor | open | Action-list growth depends on audio_ts not advancing, not on inactivity; shadow state makes OBS values stale
FINDING claude-7 | Minor | open | Filters run on pushing threads (audio_mix sources aside) with raw timestamps and VST3 re-blocking; sample_offset and ParameterChanges preallocation unspecified
FINDING claude-8 | Minor | open | Shared queue lacks isolation; relative deltas and Sum/Min/Max coalescing undefined; external writers outside arbitration
FINDING claude-9 | Minor | open | No trust-by-target matrix, network-backed OS ports, dropped OSCQuery scope, learn and SysEx-thru rules, ambiguous section 8
FINDING claude-10 | Minor | open | Sidecar key and location (not rename-event driven), compat kind under section 12, secrets.json size, unresolved devices, OSC tag fidelity unspecified
FINDING claude-11 | Major | open | Transforms, banks/layers/modifiers, learn and device-identity matching are K1 scope (006:9-10) but undecided, and they shape PR1 types
VERDICT: request changes
