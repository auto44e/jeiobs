**Request changes.** claude-1 is still open at Major; my other findings are fixed.

**claude-1 (Major, still open).** A18 (006:258-269) now attributes most items and adds "firm" (:269). Five problems remain:
- **Item 10 has no attribution.** :259 covers items 1-7 and 9, and :268 covers item 8. The debate split on PR1 scope (02-k1-design-debate.md:178 vs :282-284).
- **The A12 line is wrong.** :264-267 says "only Claude's review tested" A12 and "Codex has not accepted A12-A14". Codex raised codex-2 on bindings in runs 01 and 02 (review-01/02-r1-codex.reply.md:2, review-02/04-r2-codex.reply.md:2). In run 03 it accepted "A12 makes bindings local-only and always omitted from exports" (review-03/04-r2-codex.reply.md:6). Codex also addressed A13 there (:3).
- **The debate line on bindings is wrong.** :265 says "the debate had Claude and Codex wanting bindings outside capture". Claude actually proposed bindings "captured `opaque-unreviewed`" (02:153). Only Codex wanted local state (02:268, 280).
- **The round 2 line leaves Codex out.** :262-263 says "Claude asked, Codex agreed where noted". Codex also asked for A15-A17 (codex-4, 8, 7: review-02/04-r2-codex.reply.md:4,7,8), and it accepted A15 and A16 in run 03 (review-03/04-r2-codex.reply.md:8,12). Nothing in the doc is marked "noted".
- **A8 is in no bullet.** :260-261 skips it, and its new sidecar key (:172-173) is the lead's answer to claude-17.

These are text fixes. If those lines are corrected, I'll accept in run 05.

**Accepted**
- **claude-13:** A14 now applies the checks to every TCP, HTTP and WebSocket listener, reads included, and OSCQuery is GET-only (006:220-222). One note for K4: bytes before the first SLIP END must count as a malformed frame. Otherwise the HTTP headers of a browser's no-cors POST are discarded as noise and the frames in its body get parsed.
- **claude-14:** Network MIDI 2.0 now counts as UDP (006:213-214), which matches A9 (:175-176).
- **claude-15:** Creating or enabling a SysEx or raw thru route needs meta (006:241-242), and the egress class is explained (:242-243). The PR4 matrix still has to list the egress rows.
- **claude-16 / codex-7:** I checked A17 against the code. Balance is read in process_audio (obs-source.c:4057-4059), which runs at :4085. The signal fires at :5735 and the setter writes at :5737; the signal is declared at :96.
- **claude-17:** Duplicated collections don't collide on the scene-UUID key, because duplicating regenerates `sources` UUIDs (s1-persistence-audit.md:84).
- **claude-18:** Fixed (006:255-257).

**New Minor**
- **claude-19:** A13 allows only one transport to hold privileged classes, "a token-authenticated OSC/TCP session" (006:212-213). It doesn't say how the token is exchanged. OSC has no authentication and plain TCP is unencrypted, so a token sent as-is gives output-control to anyone who can capture traffic on the network.

FINDING claude-1 | Major | open | A18 (006:258-269): item 10 unattributed; "only Claude's review tested… Codex has not accepted A12" contradicts review-03/04-r2-codex.reply.md:6 (codex-2 accepted A12); "debate had Claude and Codex wanting bindings outside capture" contradicts 02:153; Codex also asked for A15-A17 (review-02/04-r2-codex.reply.md:4,7,8); A8 in no bullet
FINDING claude-2 | Major | accepted-fix | A1-A2 class hotkeys by effect (unknown means output-control) and route output kinds through RequestOutput; A15 gives profile and collection switching RequestProfileSwitch
FINDING claude-3 | Major | accepted-fix | A4 allows filters of public sources through the parent (obs-source.c:487-492 verified); transitions by name match WORKSPACE_FORMAT:414-415
FINDING claude-4 | Major | accepted-fix | A11 (006:191-199) defines match, guards, split, merge, thru and remapped thru, meeting ROADMAP:225-226
FINDING claude-5 | Major | accepted-fix | A12 (006:200-207) makes bindings local-only, never projected, same-machine restore, incompat; PR5 must also amend the section 4 projection (WORKSPACE_FORMAT:170-177)
FINDING claude-6 | Minor | accepted-fix | A17 reports a pending shadow and flushes it once on activation (edge case was claude-18)
FINDING claude-7 | Minor | accepted-fix | A7 degrades sample_offset to latest unless the consumer is verified; ParameterChanges sizing is listed as open (006:304)
FINDING claude-8 | Minor | accepted-fix | A16 (006:244-246) keeps the newest absolute value per writer and target, then reduces; deltas are summed
FINDING claude-9 | Minor | accepted-fix | A9 adds the matrix, network-backed ports as remote-net, learn and SysEx thru as meta, and OSCQuery scope; WinMM classing is open (006:299-301)
FINDING claude-10 | Minor | accepted-fix | A12 adds an incompat feature and applies the 65,536-byte rounding (WORKSPACE_FORMAT:266-267) to the omitted secrets file
FINDING claude-11 | Major | accepted-fix | A10 decides transforms, banks, layers, modifiers, learn and device identity in outline
FINDING claude-12 | Major | accepted-fix | review-01..03 and findings.md committed and ledger sources resolve; README.md:3-10 still lists only 01/02 and calls the folder a consult
FINDING claude-13 | Minor | accepted-fix | A14 (006:220-222) makes Host/Origin/no-CORS/close-on-malformed unconditional for TCP, HTTP and WebSocket, reads included; K4: pre-END SLIP bytes are a malformed frame
FINDING claude-14 | Major | accepted-fix | A13 (006:213-214) counts Network MIDI 2.0 as UDP with no privileged grant, consistent with A9 (006:175-176)
FINDING claude-15 | Minor | accepted-fix | A15 (006:241-243): creating or enabling SysEx/raw thru needs meta; egress class is the endpoint trust class mapped through A1's table
FINDING claude-16 | Minor | accepted-fix | A17 (006:247-251) matches obs-source.c: read at 4057-4059 (process_audio, called at 4085), signal 5735, write 5737, declared 96
FINDING claude-17 | Minor | accepted-fix | A8 (006:172-173) keys the sidecar by scene-UUID set with name fallback and unresolved; duplicates regenerate scene UUIDs (s1-persistence-audit.md:84)
FINDING claude-18 | Minor | accepted-fix | A17 (006:255-257): a newer observed OBS-UI or obs-websocket write wins and the shadow is dropped
FINDING claude-19 | Minor | open | A13 "a token-authenticated OSC/TCP session whose secret is in secrets.json" (006:212-213) leaves the handshake undefined; over plain TCP a token sent as-is gives output-control to anyone capturing traffic; K4 or Open: challenge-response, plus TLS or a stated on-path risk
VERDICT: request changes
