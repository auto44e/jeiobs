**Request changes.** I found 5 Major and 5 Minor issues. Most of the design's code citations check out: the volume/mute timestamps and action list, `WaitConnection`, synchronous `obs_source_update`, the stream warn checks, UUID lookup reaching private sources, and the `guiToDsp` writers.

**claude-1 (Major): the decision record is missing required parts.**
- DECISIONS.md:47-54 requires each decision to state "If otherwise … the predicted cost" and "Flex … or 'firm'". DECISIONS.md:68-69 says "For a self-merge, it blocks the merge."
- In task 006, :110-112 lists the rejected options with no costs, and the K1 section has no Flex at all.
- ":30-31 the lead resolved the rest" doesn't say which items. Claude's R1 is empty (debates/006 README:4-7), and nobody replied to Codex's R2 disputes (02:189-284). So every one of those disputes was settled by the lead alone, and each should say so.
- The code facts the design relies on are cited only in the debate, never in the record. Task 003:121-169 shows the expected shape.

**claude-2 (Major): privileged actions can be reached through hotkey and profile targets.**
- Hotkeys are a target kind (:50). The frontend hotkeys call the actions directly:
  - `OBSBasic.StartStreaming`/`StopStreaming` call `basic.StartStreaming`/`StopStreaming` (OBSBasic_Hotkeys.cpp:142-149).
  - `ForceStopStreaming` (:152-160), recording (:163-170), replay buffer (:203-222), virtual camera (:237-246) and screenshot (:302) do the same.
- These skip `StreamActionTriggered`/`RecordActionTriggered` (OBSBasic_Streaming.cpp:391-439, OBSBasic_Recording.cpp:257-271). A mapped hotkey therefore bypasses both `RequestStreaming` (:62-64) and the "local-admin only by default" rule (:96-97). Script hotkeys have effects nobody can know in advance.
- `obs_frontend_set_current_profile` (OBSStudioAPI.cpp:193-209) swaps the service, key and outputs, which is output configuration by another name.
- The debate's agreed deny list included "profile/collection changes" (02:89, and 02:106: "Your default-deny list stands"), but §9 dropped it.
- `RequestStreaming(origin, start)` has no stream/record selector, although :62 says it covers both.

**claude-3 (Major): the identity rules contradict libobs.**
- :52 says the adapter "refuses private sources". But libobs makes every filter private: "Always mark filters as private so they aren't found by source enum/search functions" (obs-source.c:487-492). Read literally, the rule refuses every filter and plugin-parameter target (:49-51).
- Transitions are created private with a new UUID on every load (OBSBasic_Transitions.cpp:1466; s1-persistence-audit.md:82-83). WORKSPACE_FORMAT.md:414-415 therefore keys them by name, so "Bind by UUID, never by name" (:49) cannot address a transition.
- Only sources have UUIDs (obs.h:1204), so `registerer uuid` doesn't exist for frontend, output, encoder or service hotkeys.

**claude-4 (Major): "full MIDI1/2.0 flexibly routable" has no egress model.**
- The owner's words (:4-5) and ROADMAP.md:225-230 require "merge, thru, filter and transform" and "MIDI … into plugin event inputs and out to hardware, virtual and network ports".
- The design's targets are OBS objects only (:49-51), and thru and plugin events use an "opaque" WireEnvelope (:37-38). That leaves no target kind for an output endpoint, group or channel.
- There is no rule for re-encoding a transformed route: remapping, MIDI 1.0⇄UMP translation for the destination's protocol, and never interleaving SysEx on merge.
- VST3 cannot take opaque MIDI: notes must become `Vst::Event`s, and CCs must go to parameters through `IMidiMapping`.
- ROADMAP.md:220-224 also requires R1 node/port IDs, listen-bus controls, replay buffer and outputs, and the target list has none of them.
- K2–K4 must not change the core types (02:98, 02:106), so K1-PR1 needs these kinds now.

**claude-5 (Major): keeping `bindings.json` "outside capture and export" (:89-90) contradicts M1.**
- M1 says "Snapshots cover … routing/device bindings" (ROADMAP.md:42-43). WORKSPACE_FORMAT keeps the device inventory in local revisions and drops it only at export (WORKSPACE_FORMAT.md:99-101, :181-183).
- Under this design, a restore brings back the slots but not their bindings, and nothing says what happens to orphaned or mismatched bindings.
- The "explicit import and export" is a second export path outside the §6 gate. It would carry exactly what WORKSPACE_FORMAT.md:181-183 refuses to export.
- Fix: capture bindings in local revisions and omit them at export. Never let a restore re-open a listener interface or peer list without asking. Otherwise, record the deviation from M1 for the owner.

**claude-6 (Minor): "inactive" is the wrong test for when the action list grows.**
- Every `OBS_SOURCE_AUDIO` source joins `first_audio_source` (obs-source.c:267-279) and is rendered every tick whether active or not (obs-audio.c:608-618; `push_audio_tree` has no activity check, :30-43).
- The list stops draining only when rendering returns before `apply_audio_volume` (obs-source.c:5391-5396, 5449-5452, 5494-5515).
- So an active source with no audio still grows its list. Holding shadow state for an inactive source that has live audio leaves the OBS mixer, `obs_source_get_volume` and saved collections stale.
- Use "`audio_ts` not advancing" instead (`obs_source_get_audio_timestamp`, :5531). Balance never uses the action list at all (:5725-5739).

**claude-7 (Minor): the sample-accuracy claims don't match the code.**
- Audio filters run only in `obs_source_output_audio`, on whatever thread pushes the audio (obs-source.c:3904-3921, 4087-4088). None run in the mix callback (:5375-5410), so the description at :78 is wrong.
- Filters see the raw source timestamp (:4050-4052); timestamps count as direct only within `MAX_TS_VAR` of now (:1619-1625).
- obs-vst3 re-blocks audio into its own segments (obs-vst3.cpp:1086-1096), so `event_ns − block_ts` needs a reconstructed segment time and a fallback to offset 0.
- `inputParameterChanges` is default-constructed (VST3Plugin.cpp:254). It needs preallocated queues, a per-parameter point cap and ascending offsets when merged with the existing `guiToDsp` writers (VST3ComponentHolder.cpp:44, 92). The SDK isn't in this tree, so its allocation behaviour is unverified.

**claude-8 (Minor): flood and arbitration gaps.**
- A single shared MPSC queue (:73) lets one flooding peer cause dropped hardware events. It needs per-endpoint or per-class quotas, and a flood test that checks isolation, not just bounded memory.
- Relative deltas have no flood rule.
- Coalescing "per target" (:73-74) breaks Sum/Min/Max (:65-66); coalesce per writer instead.
- Writers outside the graph (the OBS mixer, obs-websocket, scripts, plugin editors) take no part in leases or takeover.

**claude-9 (Minor): the security policy is incomplete.**
- There is no default matrix of trust class × target class (:94-97).
- An OS MIDI port can be backed by the network (CoreMIDI network sessions, rtpMIDI, ALSA `aseqnet`), so an unknown transport must get the lowest trust class.
- The debate's "OSCQuery lists only mapped targets" (02:172) was dropped, and MIDI-CI Property Exchange is another read path.
- Learn mode should only bind sources whose class is allowed to reach the target.
- The property allowlist doesn't exclude path or URL properties.
- SysEx thru from non-local peers to `hw` ports should be denied by default.
- "section 8" at :95 is ambiguous; it should say WORKSPACE_FORMAT §8.

**claude-10 (Minor): persistence and value details.**
- The per-collection sidecar (:91-92) has no stated key or location. It must not go in `basic/scenes/`, because OBS lists every `*.json` there as a collection (OBSBasic_SceneCollections.cpp:440-445).
- §12 makes a stored entry with an unknown role invalid (WORKSPACE_FORMAT.md:525-531), so the new rows need an `incompat` feature or a §12 amendment.
- `secrets.json` needs its own omitted row; otherwise its exact size is exported (WORKSPACE_FORMAT.md:259-263).
- The visible "unresolved" state for missing or ambiguous devices (ROADMAP.md:233-235) isn't restated.
- `ControlValue` has no OSC Nil and doesn't say how the OSC type tags `s`/`S`, `c`, `r`, `m` and `t` survive (brief 01:13-14).

FINDING claude-1 | Major | open | K1 record lacks If-otherwise costs, Flex and per-item attribution required by DECISIONS.md:47-54,68-69
FINDING claude-2 | Major | open | Frontend hotkeys and profile switching reach stream/record/output actions, bypassing RequestStreaming and local-admin rule
FINDING claude-3 | Major | open | refuses private sources blocks all filters (obs-source.c:487-492); transitions cannot bind by UUID; hotkey registerer UUIDs exist only for sources
FINDING claude-4 | Major | open | No egress/port, plugin-event or R1/listen-bus target kinds; opaque thru cannot meet flexibly routable MIDI and VST3 needs translation
FINDING claude-5 | Major | open | bindings.json outside capture contradicts M1 snapshot scope; separate bindings export bypasses the WORKSPACE_FORMAT export gate
FINDING claude-6 | Minor | open | Action-list growth depends on audio_ts not advancing, not on inactivity; shadow state makes OBS values stale
FINDING claude-7 | Minor | open | Filters run on pushing threads with raw timestamps and VST3 re-blocking; sample_offset and ParameterChanges preallocation unspecified
FINDING claude-8 | Minor | open | Shared queue lacks isolation; relative deltas and Sum/Min/Max coalescing undefined; external writers outside arbitration
FINDING claude-9 | Minor | open | No trust-by-target matrix, network-backed OS ports, dropped OSCQuery scope, learn and SysEx-thru rules, ambiguous section 8
FINDING claude-10 | Minor | open | Sidecar key and location, compat kind under section 12, secrets.json size, unresolved devices, OSC tag fidelity unspecified
VERDICT: request changes
