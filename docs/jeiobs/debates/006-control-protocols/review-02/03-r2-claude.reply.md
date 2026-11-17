Still request changes. The head is still cd991c9d4, so nothing changed since my round-1 reply. I hold six Majors open (claude-1, 4, 5, 12, 13, 14) and Codex holds two (codex-2, codex-4), so the gate's "no open blocker or major finding" (DECISIONS.md:187) is not met.

**On Codex's reply**
- **codex-2:** this is the same defect as claude-5. One sentence in A8 closes both: `ProjectForExport` always omits the bindings role and refuses an `include` decision for it, the way it drops `target.devices` (WORKSPACE_FORMAT:170-172). I'm dropping my `ro_compat` sub-point, because :525-531 already makes an older reader reject the unknown allowlist row.
- **codex-4:**
  - I agree that `RequestOutput(origin, output kind, start|stop)` (006:131) can't represent profile or collection switching. That is also claude-15's first item: A1 classes switching as `profile` (006:122), A2 as `output-control` (006:129).
  - Suggested split: identities for every A3 kind are settled before PR1, because they are schema. Read and write paths become an explicit open item for PR4.
- **codex-3:** the closure reason cites A3-A4 (private sources, hotkey fallback). The ledger's codex-3 is about the missing read/subscribe path, which A5 covers (006:145-150). Please restate the closure against A5 or reopen it. Nobody can check which wording is right, because the run-01 reply the ledger cites isn't committed (claude-12).
- **codex-1:** the atomic slot is fine, but the cited facts are wrong, so I'm raising new Minor claude-16:
  - A7 says "Balance is a plain field read on the audio thread" (006:163-164). `obs-source.c:4057-4059` is in `process_audio` (:4030), which `obs_source_output_audio` calls (:4085) on whichever thread pushes the audio.
  - A5 says "a bounded poller otherwise (properties, balance)" (006:147). But `audio_balance` is a declared signal (:96), emitted at :5735.
- **codex-5, codex-6:** I don't contest these closures. A1's defaults and grant storage (claude-13, claude-14) and the reducer semantics (claude-8) are separate issues. Codex has not stated a position on claude-12, 13 or 14; please do.

**My findings**
- **claude-1:** what's missing, concretely:
  - Who settled each item. 006:30-31 says only "the lead resolved the rest".
  - If-otherwise for two rejected alternatives at 006:112-113: "serials and addresses in portable files" and "validation that rejects every equal-priority writer pair".
  - If-otherwise for A8's reversal (02:153 vs 02:268-280) and for the PR1 scope (02:178 vs 02:282-284).
  - Flex or "firm" outside the five bullets. Static-only loops (006:199) has neither.
- **claude-4:** the only matching rule in task 006 is "wildcards" (006:70). Either carry over "equality, ranges and prefixes" (02:131) plus one line each on split, filter and remapped thru, or list routing under Open before K1-PR1.
- **claude-12:**
  - The shared context says "run 01 transcripts and ledger are under docs/jeiobs/debates/006-control-protocols/". That is false at this commit: the folder holds only `01`, `02` and `README.md`, with no `findings.md`.
  - Fix: commit `review-01/`, `review-02/` and `findings.md`, link them from 006:30, and have run 03 review that head. The exemption (DECISIONS.md:177-181) covers only one new run folder plus a modified `findings.md`.
- **claude-13:** "off by default" (006:22) doesn't answer this. Once a user enables a loopback TCP or HTTP listener for a local app, every web page and OBS browser-source widget gets the same `loopback-net` default, unless the browser's own local-network check blocks it. OSCQuery's WebSocket also needs an Origin check.
- **claude-6, 8, 10, 14, 15:** unchanged.

FINDING claude-1 | Major | open | Five if-otherwise/flex bullets exist (006:189-201); still missing who settled each item and A1-A10 (DECISIONS.md:49-50 vs 006:30-31), if-otherwise for 006:112-113 "serials and addresses in portable files" and "rejects every equal-priority writer pair", for A8's reversal (02:153 vs 02:268-280) and PR1 scope (02:178 vs 02:282-284), and flex or "firm" for the rest
FINDING claude-2 | Major | accepted-fix | A1-A2 class hotkeys by effect (unknown means output-control), make profile and collection switching privileged, and route all output kinds through RequestOutput
FINDING claude-3 | Major | accepted-fix | A4 allows filters of public sources through the parent (obs-source.c:487-492 verified); transitions by name match WORKSPACE_FORMAT:414-415; encoder and service registerers (obs-hotkey.h:58-59) should follow A4's by-name rule
FINDING claude-4 | Major | open | Routes still undefined: the only matching rule is "wildcards" (006:70); 02:131 "equality, ranges and prefixes" not carried over; no split, filter or remapped thru (ROADMAP:225); neither decided nor in Open before K1-PR1 (006:203-206)
FINDING claude-5 | Major | open | A8's "the export gate already refuses unreviewed entries" is false: include after review sets fidelity exact/redacted (WORKSPACE_FORMAT:174-175), so serials and peer addresses can be exported (contra :182); fix: the projection always omits the bindings role; ro_compat sub-point dropped (:525-531)
FINDING claude-6 | Minor | open | A7 covers growth, but while the shadow withholds set_volume, user_volume (obs-source.c:4574), the mixer, the saved collection and obs-websocket keep the old value; no flush-on-activate rule
FINDING claude-7 | Minor | accepted-fix | A7 degrades sample_offset to latest unless the consumer is verified; ParameterChanges sizing is listed as open (006:205-206)
FINDING claude-8 | Minor | open | Coalescing Sum/Min/Max "by the same reducer" (006:157) folds one writer's successive absolute values (Sum double-counts, Min/Max latch); keep the newest per writer and target, then reduce
FINDING claude-9 | Minor | accepted-fix | A9 adds the matrix, network-backed ports as remote-net, learn and SysEx thru as meta, and OSCQuery scope; K2 must say how backends without transport info (WinMM) class rtpMIDI ports
FINDING claude-10 | Minor | open | An omitted secrets.json in a listed module folder records its exact size (WORKSPACE_FORMAT:261,298; rounding only at :266-267); the compat feature kind is unstated although :525-531 makes unknown rows invalid
FINDING claude-11 | Major | accepted-fix | A10 decides transforms, banks, layers, modifiers, learn and device identity in outline; toggle and relative-to-absolute already need per-route state, which answers that open item
FINDING claude-12 | Major | open | debates/006-control-protocols has only 01, 02, README (no review-01/, review-02/, findings.md), so ledger sources and A1-A10's correspondence are uncommitted (DECISIONS.md:55-57,187-190); the exemption (:177-181) can't add them, so that head needs run 03
FINDING claude-13 | Major | open | loopback-net gets mix, scene and plugin-param by default (006:123); once a loopback TCP/HTTP listener is enabled (006:22), web pages and OBS browser sources reach it: SLIP resyncs after a no-cors POST's headers (006:16), OSCQuery HTTP is open to DNS rebinding; default only for UDP or Host/Origin-checked transports that close on malformed input
FINDING claude-14 | Major | open | Per-endpoint grants (006:124-125,132) have no stated store: in mappings.json imports widen trust, in captured bindings.json an old snapshot re-grants a revoked peer, and UDP grants rest on a spoofable IP; keep grants machine-local, outside capture and import
FINDING claude-15 | Minor | open | Profile switching is both profile (006:122) and output-control (006:129), and RequestOutput can't carry it (as codex-4 says); A3's egress, plugin-event, listen-bus and transition targets have no class and no deny-if-unclassified rule; SysEx is restricted only on thru (006:176)
FINDING claude-16 | Minor | open | A7 "Balance is a plain field read on the audio thread" (006:163-164) is wrong: obs-source.c:4057-4059 is in process_audio, called from obs_source_output_audio (:4085) on the pushing thread; A5 polls balance (006:147) though audio_balance is a signal (:96, :5735)
VERDICT: request changes
