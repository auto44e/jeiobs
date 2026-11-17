You are one of two independent reviewers (Claude and Codex, different model lineages) in a
cross-talk review for the JeiOBS repository. Your working directory is a checkout of exactly the
commit under review. The lead agent wrote or integrated the change and must NOT decide on its own
whether your findings are resolved: under docs/jeiobs/DECISIONS.md a finding is closed only when the
reviewer who raised it accepts the fix or withdraws it, in writing, in a later round. Use one
severity scale: Blocker, Major, Minor. Only the finder may lower a finding's severity. You are
read-only: never modify, create or delete files, and never run commands that change state.
Verify claims in the files rather than trusting the context. Be terse; cite file:line.

End your reply with one line per finding YOU raised (every ledger row with you as finder, every
finding you raised in an earlier round, plus new ones), then the verdict. If you raised none, write
FINDING none. Use exactly this syntax:
FINDING <ID> | <Blocker|Major|Minor> | <open|accepted-fix|withdrawn> | <one-line reason>
VERDICT: approve
or
VERDICT: request changes
For a new finding, use ID <your-name>-<n> (for example claude-1) and quote the evidence.

REVIEWED COMMIT: cd991c9d4d5664106240ab4e7e446fe641274e21

TOPIC: Review run 02 of the K1 design PR.

SHARED CONTEXT:
# K1 design PR review run 02 — head cd991c9d4

Your working directory is a checkout of this commit. Since run 01 (reviewed 6f8492cf9): `git diff 6f8492cf9 HEAD`: task 006 now has "Amendments after review round 1" A1-A10, "If otherwise, and flex", and "Open before K1-PR1"; run 01 transcripts and ledger are under docs/jeiobs/debates/006-control-protocols/. Dispose of your findings against the amendments. This is a design record (nothing is built); open items that are explicitly listed as "Open before K1-PR1" are acceptable as long as they are real open questions and not hidden gaps. Owner (verbatim): "if you peers are more or less in agreement, no problem self merging".

Severity: Blocker = must not merge; Major = must be fixed before a self-merge; Minor = follow-up.
Other agent work runs on this host in separate worktrees; it does not touch this checkout.

FINDINGS LEDGER (kept by the lead):
# Findings ledger — K1 design PR

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

YOUR OWN PREVIOUS REPLY:
**Verdict: request changes.** Six Majors are open: claude-1, claude-4 and claude-5 from run 01, plus new claude-12, claude-13 and claude-14.

Run 01 isn't in this commit. `debates/006-control-protocols/` holds only `01`, `02` and `README.md`. No file outside `018-decision-records` contains `claude-11`. So I judged my run-01 findings from the ledger notes, not my original wording.

**Closed**
- **claude-2:** A1-A2 (006:117-132) class hotkeys by what they trigger, and treat profile and collection switching as privileged. All four output kinds go through `RequestOutput`.
- **claude-3:** filters really are always private (`obs-source.c:487-492`). A4 resolves a filter through its parent. A3 binds transitions by name, which matches WORKSPACE_FORMAT:414-415.
- **claude-7, claude-9, claude-11:** A7, A9 and A10 cover them. The two remaining questions are listed as open (006:205-206).

**Still open**
- **claude-5:**
  - A8 (006:169-170) says bindings are "omitted from every export (the export gate already refuses unreviewed entries…)". That is false. WORKSPACE_FORMAT:174-175 says "`include`, given after user review, sets `fidelity` to `exact` or `redacted`". So serials and peer addresses can be exported, against :182.
  - No per-entry same-machine restore rule exists; only `secret_refs` check `machine_id` (:368-374). An `ro_compat` reader also "may read, display and restore" (:235-236).
  - Fix: a role the projection always omits (as it drops `target.devices`, :99 and :170-172), a same-machine rule, and an `incompat` feature.
- **claude-4:** routes themselves are still undefined: there is no selector, no split or merge, and no filtered or remapped thru. The debate's "The core only matches equality, ranges and prefixes" (02:131) never reached the task doc. ROADMAP:225 requires "split by group/channel/note/controller range, merge, thru, filter".
- **claude-1:** the record still doesn't say who settled A1-A10, has no if-otherwise for A8's reversal or the PR1-scope dispute, and gives no flex or "firm" for the remaining decisions.
- **claude-6, claude-8, claude-10:** Minor leftovers, listed below.

**New**
- **claude-12:**
  - DECISIONS.md:55-57 requires review transcripts to be committed and linked, and :68-69 says a missing part blocks a self-merge.
  - The ledger's sources (`review-01/*.reply.md`) are not committed (:188-190).
  - The exemption (:177-181) covers "added files inside one new run folder" plus "a modification of findings.md". So runs 01 and 02 plus a new `findings.md` can't land together without another review.
- **claude-13:** 006:123 says "`mix`, `scene`, `plugin-param` allowed for `hw` and `loopback-net`". But browsers can reach loopback TCP and HTTP, including OBS browser sources running third-party widgets.
  - A TCP/SLIP listener (006:16) decodes the frames that follow a web page's no-cors POST headers.
  - OSCQuery's HTTP namespace is open to DNS rebinding unless Host/Origin are checked.
- **claude-14:** 006:124-125 says "`remote-net` gets only what the user grants per endpoint", but neither item 8 nor A8 says where grants are stored.
  - In `mappings.json`, an imported workspace widens trust.
  - In captured `bindings.json`, restoring an older snapshot re-grants a peer the user revoked.
  - A grant to an OSC/UDP peer rests on a source IP, which can be spoofed.
- **claude-15 (Minor):** profile switching is both `profile` (006:122) and `output-control` (006:129). A3's egress, plugin-event, listen-bus and transition targets have no capability class. SysEx is restricted only on thru (006:176).

**For Codex's codex-1 and codex-3 (not my findings):**
- A7 says balance is "read on the audio thread", but `obs-source.c:4057` is in `process_audio`. That runs on whichever thread pushes the audio, via `obs_source_output_audio` (:4085).
- A5 polls balance, although OBS emits an `audio_balance` signal (:5735).

FINDING claude-1 | Major | open | Five if-otherwise/flex bullets added (006:189-201); still missing who settled A1-A10 (DECISIONS.md:49-50), if-otherwise for A8's reversal (02:266-280) and the PR1-scope dispute (02:178 vs 02:282-284), and flex or "firm" for the rest
FINDING claude-2 | Major | accepted-fix | A1-A2 class hotkeys by effect (unknown means output-control), make profile and collection switching privileged, and route all output kinds through RequestOutput
FINDING claude-3 | Major | accepted-fix | A4 allows filters of public sources through the parent (obs-source.c:487-492 verified); transitions by name match WORKSPACE_FORMAT:414-415; encoder and service registerers (obs-hotkey.h:58-59) should follow A4's by-name rule
FINDING claude-4 | Major | open | Target kinds are now in A3, but routes are undefined: no selector (02:131 "equality, ranges and prefixes" not carried over), no split or merge, no filtered or remapped thru to an egress port (ROADMAP:225); not listed in Open before K1-PR1
FINDING claude-5 | Major | open | A8's claim that the export gate refuses unreviewed entries is false (WORKSPACE_FORMAT:174-176 include decision), so hardware identity can be exported (contra :182); no per-entry same-machine restore exists (:368-374) and ro_compat readers may restore (:235-236)
FINDING claude-6 | Minor | open | A7 covers growth, but while the shadow withholds set_volume, user_volume (obs-source.c:4574), the mixer, the saved collection and obs-websocket keep the old value; no flush-on-activate rule
FINDING claude-7 | Minor | accepted-fix | A7 degrades sample_offset to latest unless the consumer is verified; ParameterChanges sizing is listed as open (006:205-206)
FINDING claude-8 | Minor | open | Per-endpoint queues, deltas and external writers fixed; but coalescing Sum/Min/Max by the same reducer (006:157) folds one writer's successive absolute values (Sum double-counts, Min/Max latch); keep the newest per writer and target, then reduce
FINDING claude-9 | Minor | accepted-fix | A9 adds the matrix, network-backed ports as remote-net, learn and SysEx thru as meta, and OSCQuery scope; K2 must say how backends without transport info (WinMM) class rtpMIDI ports
FINDING claude-10 | Minor | open | Sidecar keyed by collection UID is fixed; an omitted secrets.json still records its exact size (WORKSPACE_FORMAT:261, unlike the rounding at :266-267), and the compat feature kind is unstated although :525-531 makes unknown rows invalid
FINDING claude-11 | Major | accepted-fix | A10 decides transforms, banks, layers, modifiers, learn and device identity in outline; toggle and relative-to-absolute already need per-route state, which answers that open item
FINDING claude-12 | Major | open | Run-01 transcripts and ledger are not committed (006 debate folder has only 01, 02, README), though A1-A10 rest on them (DECISIONS.md:55-57,68-69,188-190); the exemption (:177-181) cannot add two run folders and a new findings.md later
FINDING claude-13 | Major | open | loopback-net gets mix, scene and plugin-param by default (006:123), but browsers and OBS browser sources reach loopback TCP/HTTP: SLIP accepts frames after a no-cors POST's headers (006:16), and OSCQuery HTTP is open to DNS rebinding; default only for UDP or Host/Origin-checked transports that close on malformed input
FINDING claude-14 | Major | open | Per-endpoint grants (006:124-125,132) have no stated store: in mappings.json imports widen trust, in captured bindings.json an old snapshot re-grants a revoked peer, and UDP grants rest on a spoofable IP; keep grants machine-local, outside capture and import
FINDING claude-15 | Minor | open | Profile switching is both profile (006:122) and output-control (006:129); A3's egress, plugin-event, listen-bus and transition targets have no capability class; SysEx is restricted only on thru (006:176), not on routes to egress ports
VERDICT: request changes

THE OTHER REVIEWER'S LATEST REPLY:
The amendments resolve codex-1, codex-3, codex-5, and codex-6. Codex-2 and codex-4 remain open.

FINDING codex-1 | Major | accepted-fix | A7 assigns balance to the same atomic latest-value slot as volume/mute (tasks/006-control-protocols.md:160-167)
FINDING codex-2 | Major | open | A8 says bindings are opaque-unreviewed, but export explicitly permits reviewed opaque entries to be included (tasks/006-control-protocols.md:168-173; WORKSPACE_FORMAT.md:174-177)
FINDING codex-3 | Major | accepted-fix | A3-A4 define private-source/filter validation and hotkey registerer fallback (tasks/006-control-protocols.md:133-144)
FINDING codex-4 | Major | open | A3 lists target categories but does not define each identity/read/write path, and RequestOutput(start|stop) cannot represent profile or collection switching (tasks/006-control-protocols.md:127-140)
FINDING codex-5 | Major | accepted-fix | A1 adds principals, capability classes, adapter policy checks, and hard denies (tasks/006-control-protocols.md:117-126)
FINDING codex-6 | Major | accepted-fix | A6 defines per-endpoint queues, round-robin quotas, overflow handling, reducers, and a privileged lane (tasks/006-control-protocols.md:151-159)
VERDICT: request changes