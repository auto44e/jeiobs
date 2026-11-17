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
