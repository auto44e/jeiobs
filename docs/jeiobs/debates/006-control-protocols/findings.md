# Findings ledger — K1 design PR

Review run 01 on `6f8492cf9` requested changes (design gaps). The lead answered with amendments
A1-A10 and an "Open before K1-PR1" list in task 006. Statuses are the finders' latest; "awaiting
finder" means a later run must confirm.

ID | severity | status | finder | source | note
--- | --- | --- | --- | --- | ---
claude-1 | Major | open → addressed in amendments A1-A10 (awaiting finder) | claude | review-01/03-r2-claude.reply.md (FINDING claude-1) | K1 record lacks If-otherwise costs, Flex and per-item attribution required by DECISIONS.md:47-54,68-69
claude-2 | Major | open → addressed in amendments A1-A10 (awaiting finder) | claude | review-01/03-r2-claude.reply.md (FINDING claude-2) | Frontend hotkeys and profile switching reach stream/record/output actions, bypassing RequestStreaming and local-admin rule
claude-3 | Major | open → addressed in amendments A1-A10 (awaiting finder) | claude | review-01/03-r2-claude.reply.md (FINDING claude-3) | refuses private sources blocks all filters (obs-source.c:487-492); transitions cannot bind by UUID; hotkey registerer UUIDs exist only for s
claude-4 | Major | open → addressed in amendments A1-A10 (awaiting finder) | claude | review-01/03-r2-claude.reply.md (FINDING claude-4) | No egress/port, plugin-event or R1/listen-bus target kinds; opaque thru cannot meet flexibly routable MIDI and VST3 needs translation
claude-5 | Major | open → addressed in amendments A1-A10 (awaiting finder) | claude | review-01/03-r2-claude.reply.md (FINDING claude-5) | bindings.json outside capture contradicts M1 snapshot scope; separate bindings export bypasses the WORKSPACE_FORMAT export gate
claude-6 | Minor | open → addressed in amendments A1-A10 (awaiting finder) | claude | review-01/03-r2-claude.reply.md (FINDING claude-6) | Action-list growth depends on audio_ts not advancing, not on inactivity; shadow state makes OBS values stale
claude-7 | Minor | open → addressed in amendments A1-A10 (awaiting finder) | claude | review-01/03-r2-claude.reply.md (FINDING claude-7) | Filters run on pushing threads (audio_mix sources aside) with raw timestamps and VST3 re-blocking; sample_offset and ParameterChanges preall
claude-8 | Minor | open → addressed in amendments A1-A10 (awaiting finder) | claude | review-01/03-r2-claude.reply.md (FINDING claude-8) | Shared queue lacks isolation; relative deltas and Sum/Min/Max coalescing undefined; external writers outside arbitration
claude-9 | Minor | open → addressed in amendments A1-A10 (awaiting finder) | claude | review-01/03-r2-claude.reply.md (FINDING claude-9) | No trust-by-target matrix, network-backed OS ports, dropped OSCQuery scope, learn and SysEx-thru rules, ambiguous section 8
claude-10 | Minor | open → addressed in amendments A1-A10 (awaiting finder) | claude | review-01/03-r2-claude.reply.md (FINDING claude-10) | Sidecar key and location (not rename-event driven), compat kind under section 12, secrets.json size, unresolved devices, OSC tag fidelity un
claude-11 | Major | open → addressed in amendments A1-A10 (awaiting finder) | claude | review-01/03-r2-claude.reply.md (FINDING claude-11) | Transforms, banks/layers/modifiers, learn and device-identity matching are K1 scope (006:9-10) but undecided, and they shape PR1 types
codex-1 | Major | open → addressed in amendments A1-A10 (awaiting finder) | codex | review-01/04-r2-codex.reply.md (FINDING codex-1) | Balance remains a plain cross-thread field, unlike the proposed queued/atomic boundary (docs/jeiobs/tasks/006-control-protocols.md:55-61; li
codex-2 | Major | open → addressed in amendments A1-A10 (awaiting finder) | codex | review-01/04-r2-codex.reply.md (FINDING codex-2) | Device bindings remain outside workspace capture/export, contradicting M1 snapshot scope (docs/jeiobs/tasks/006-control-protocols.md:87-93;
codex-3 | Major | open → addressed in amendments A1-A10 (awaiting finder) | codex | review-01/04-r2-codex.reply.md (FINDING codex-3) | No target read/subscribe path is specified for feedback, pickup, or arbitration (docs/jeiobs/tasks/006-control-protocols.md:37-61; docs/jeio
codex-4 | Major | open → addressed in amendments A1-A10 (awaiting finder) | codex | review-01/04-r2-codex.reply.md (FINDING codex-4) | Mixer/listen-bus, output, replay, and plugin-event targets remain unspecified; RequestStreaming omits recording/replay selection (docs/jeiob
codex-5 | Major | open → addressed in amendments A1-A10 (awaiting finder) | codex | review-01/04-r2-codex.reply.md (FINDING codex-5) | ControlCommand has no principal or authorization capability, while local-admin and remote-origin rules remain undefined (docs/jeiobs/tasks/0
codex-6 | Major | open → addressed in amendments A1-A10 (awaiting finder) | codex | review-01/04-r2-codex.reply.md (FINDING codex-6) | Queue policy lacks overflow, fairness, and rejection behavior for non-coalesced events, bundles, and privileged actions (docs/jeiobs/tasks/0

## Review run 02 (on `cd991c9d4`), answered by amendments A11-A18 in `472e6f02c`

ID | severity | status | finder | source | note
--- | --- | --- | --- | --- | ---
claude-1 | Major | open → fixed (awaiting finder) | claude | review-02/03-r2-claude.reply.md (FINDING claude-1) | A18 record and the added if-otherwise/flex
claude-2 | Major | accepted-fix | claude | review-02/03-r2-claude.reply.md (FINDING claude-2) | A1-A2 class hotkeys by effect (unknown means output-control), make profile and collection switching
claude-3 | Major | accepted-fix | claude | review-02/03-r2-claude.reply.md (FINDING claude-3) | A4 allows filters of public sources through the parent (obs-source.c:487-492 verified); transitions
claude-4 | Major | open → fixed (awaiting finder) | claude | review-02/03-r2-claude.reply.md (FINDING claude-4) | A11 defines routes
claude-5 | Major | open → fixed (awaiting finder) | claude | review-02/03-r2-claude.reply.md (FINDING claude-5) | A12: local-only role, never exported
claude-6 | Minor | open → fixed (awaiting finder) | claude | review-02/03-r2-claude.reply.md (FINDING claude-6) | A17 flush-on-activate
claude-7 | Minor | accepted-fix | claude | review-02/03-r2-claude.reply.md (FINDING claude-7) | A7 degrades sample_offset to latest unless the consumer is verified; ParameterChanges sizing is list
claude-8 | Minor | open → fixed (awaiting finder) | claude | review-02/03-r2-claude.reply.md (FINDING claude-8) | A16
claude-9 | Minor | accepted-fix | claude | review-02/03-r2-claude.reply.md (FINDING claude-9) | A9 adds the matrix, network-backed ports as remote-net, learn and SysEx thru as meta, and OSCQuery s
claude-10 | Minor | open → fixed (awaiting finder) | claude | review-02/03-r2-claude.reply.md (FINDING claude-10) | A12 rounding; incompat feature
claude-11 | Major | accepted-fix | claude | review-02/03-r2-claude.reply.md (FINDING claude-11) | A10 decides transforms, banks, layers, modifiers, learn and device identity in outline; toggle and r
claude-12 | Major | open → fixed (awaiting finder) | claude | review-02/03-r2-claude.reply.md (FINDING claude-12) | review-01 and review-02 transcripts committed; run 03 reviews this head
claude-13 | Major | open → fixed (awaiting finder) | claude | review-02/03-r2-claude.reply.md (FINDING claude-13) | A14
claude-14 | Major | open → fixed (awaiting finder) | claude | review-02/03-r2-claude.reply.md (FINDING claude-14) | A13
claude-15 | Minor | open → fixed (awaiting finder) | claude | review-02/03-r2-claude.reply.md (FINDING claude-15) | A15 unclassified deny; SysEx needs meta
claude-16 | Minor | open → fixed (awaiting finder) | claude | review-02/03-r2-claude.reply.md (FINDING claude-16) | A17
codex-1 | Major | accepted-fix | codex | review-02/04-r2-codex.reply.md (FINDING codex-1) | A7 assigns balance to the atomic latest-value slot (tasks/006-control-protocols.md:160-167)
codex-2 | Major | open → fixed (awaiting finder) | codex | review-02/04-r2-codex.reply.md (FINDING codex-2) | A12
codex-3 | Major | accepted-fix | codex | review-02/04-r2-codex.reply.md (FINDING codex-3) | A5 defines TargetState snapshots, polling, and feedback reads (tasks/006-control-protocols.md:145-15
codex-4 | Major | open → fixed (awaiting finder) | codex | review-02/04-r2-codex.reply.md (FINDING codex-4) | A15 table; RequestProfileSwitch
codex-5 | Major | accepted-fix | codex | review-02/04-r2-codex.reply.md (FINDING codex-5) | A1 adds principals, capability checks, defaults, and hard denies (tasks/006-control-protocols.md:117
codex-6 | Major | accepted-fix | codex | review-02/04-r2-codex.reply.md (FINDING codex-6) | A6 defines per-endpoint queues, fairness, overflow, reducers, and privileged handling (tasks/006-con
codex-7 | Minor | open → fixed (awaiting finder) | codex | review-02/04-r2-codex.reply.md (FINDING codex-7) | A17
codex-8 | Minor | open → fixed (awaiting finder) | codex | review-02/04-r2-codex.reply.md (FINDING codex-8) | A16
