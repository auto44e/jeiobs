**Request changes.** The head is unchanged since my round-1 reply, so claude-1 stays open at Major. Codex's codex-9 is the same defect, and I agree with it. One edit to A18 closes both.

**Codex's reply**
- **codex-7 accepted:** agreed. A17 matches the code: balance is read at `obs-source.c:4057-4059` (in `process_audio`, called at `:4085`), the signal fires at `:5735`, the setter writes at `:5737`, and the signal is declared at `:96`.
- **codex-9:** confirmed. A18 contradicts the lead's own ledger in the same commit. `findings.md:79` records codex-2 as accepted-fix, and Codex's acceptance is at `review-03/02-r1-codex.reply.md:2` and `review-03/04-r2-codex.reply.md:1,6`.

**claude-1: everything A18 must correct (006:258-269)**
1. **:259 says the debate agreed items 1-7 and 9 and the principle.** That's true only for items 2, 6, 7 and 9.
   - The debate had two rounds (02:3), and Codex's round-2 disputes 1-4 (02:191-264) were never answered.
   - The lead adopted those disputes as the principle and items 3, 1, 4 and 5. Claude had proposed libobs calls from the graph thread (02:117).
   - I raised this in run 01 (`review-01/01-r1-claude.reply.md:6`) and missed it in round 1 of this run.
2. **Item 10 has no attribution.** The lead took Codex's position (02:282-284) over Claude's (02:178).
3. **A12 (:264-267) is attributed to the lead alone, "tested" only by Claude.**
   - Codex asked for A12 (codex-2, `review-02/04-r2-codex.reply.md:2`) and accepted it.
   - Codex also addressed A13 (`review-03/04-r2-codex.reply.md:3`).
   - Only A14 has no Codex reply.
4. **:265 says both peers wanted bindings outside capture.** Claude proposed capturing them as `opaque-unreviewed` (02:153). Only Codex wanted local state (02:268, 280).
5. **:262 says "Claude asked" for review round 2.** Codex asked for A12 and A15-A17 too (`review-02/04-r2-codex.reply.md:2,4,7,8`) and has now accepted all four. "Where noted" refers to nothing.
6. **A8 appears in no bullet.** :268 calls item 8 settled by the debate except for bindings. Codex also disputed the sidecar reconciliation (02:280), and A8's new key is the lead's answer to claude-10 and claude-17.

**New Minor: claude-20.** A14 says the listener "enforces a loopback `Host`" (006:218) and makes this "unconditional for every TCP, HTTP and WebSocket listener" (:220). A9 expects remote peers to read OSCQuery (006:177), and OSCQuery is K4 scope. A listener bound to a LAN address would reject every one of those peers. The fix is to say the Host header must name the bound address or a configured name.

FINDING claude-1 | Major | open | A18 (006:258-269): :259 says the debate agreed items 1,3,4,5 and the principle, but Codex's round-2 disputes 1-4 (02:191-264) went unanswered and the lead adopted them; item 10 unattributed (02:178 vs 02:282-284); "Codex has not accepted A12-A14" contradicts review-03/04-r2-codex.reply.md:6 and findings.md:79, and Codex addressed A13 (:3); :265 contradicts 02:153; Codex also asked for A12, A15-A17 (review-02/04-r2-codex.reply.md:2,4,7,8); A8 is in no bullet
FINDING claude-2 | Major | accepted-fix | A1-A2 class hotkeys by effect (unknown means output-control) and route output kinds through RequestOutput; A15 gives profile and collection switching RequestProfileSwitch
FINDING claude-3 | Major | accepted-fix | A4 allows filters of public sources through the parent (obs-source.c:487-492 verified); transitions by name match WORKSPACE_FORMAT:414-415
FINDING claude-4 | Major | accepted-fix | A11 (006:191-199) defines match, guards, split, merge, thru and remapped thru, meeting ROADMAP:225-226
FINDING claude-5 | Major | accepted-fix | A12 (006:200-207) makes bindings local-only, never projected, same-machine restore, incompat; PR5 must also amend the section 4 projection (WORKSPACE_FORMAT:170-177)
FINDING claude-6 | Minor | accepted-fix | A17 reports a pending shadow and flushes it once on activation (edge case was claude-18)
FINDING claude-7 | Minor | accepted-fix | A7 degrades sample_offset to latest unless the consumer is verified; ParameterChanges sizing is listed as open (006:303-304)
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
FINDING claude-20 | Minor | open | A14 "enforces a loopback `Host`" made "unconditional for every TCP, HTTP and WebSocket listener" (006:218-220) rejects every peer of a LAN-bound OSCQuery listener, which A9 (006:177) and K4 expect; require Host to match the bound address or a configured name
VERDICT: request changes
