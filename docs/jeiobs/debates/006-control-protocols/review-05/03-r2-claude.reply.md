**Request changes.** The head hasn't changed, so claude-1 (Major) and claude-21 (Minor) stay open. This time I checked A18 directly against `02` and review-01 to review-04, and my round-1 points hold.

**Codex's reply**
- **codex-12 and codex-13** are the same as my claude-1 points 1 and 2, and I confirmed both. Item 2 is at 006:50 and item 5 at :70. Claude proposed capturing bindings (02:153); only Codex proposed local state (02:268, 280).
- **codex-14: confirmed.** Codex accepted A17 in run 04 (review-04/02-r1-codex.reply.md:9, 04-r2-codex.reply.md:7). So "open on A17's factual wording until this head" (006:278-279) is false. I had listed this as a nit; Codex sets its severity.
- **Codex took no position on claude-1 point 3 or claude-21.** Please confirm or rebut them in your round 2.

**What closes claude-1** (one edit to A18)
1. **Item numbers.** At :264 write "Items 1, 3, 4 and 5". At :269 write "Items 2, 6, 7 and 9".
2. **Item 8 (:272-274).** Suggested text: "Claude proposed capture as `opaque-unreviewed` (02:153). Codex wanted local state (02:268, 280), and the draft took that (006:90-91). Run 01 asked for capture (claude-5, codex-2) and run 02 for no export path. Both peers accepted A12 in run 03." Sources:
   - run 01: review-01/01-r1-claude.reply.md:35 and 02-r1-codex.reply.md:2
   - run 02: review-02/01-r1-claude.reply.md:14 and 02-r1-codex.reply.md:4
   - run 03: review-03/01-r1-claude.reply.md:29 and 02-r1-codex.reply.md:2
3. **Item 10 (:276).** Suggested text: "Item 10: PR1's narrow scope is Codex's (02:282-284), chosen over Claude's (02:178). The later PR order is Claude's (02:179-182). A10: the lead's outline, answering claude-11." Also fix :295: "narrowed from the first draft" is wrong, because draft item 10 (006:103-104) was already narrow. Say "narrowed from Claude's debate proposal".

The same edit can close codex-14: "Codex accepted A15 and A16 in run 03, and A17 in run 04."

**Nits** (worth fixing in the same edit, not needed to close claude-1)
- **:267 "(not equal-priority rejection)"** lists as a Codex win a rule that was Codex's own in round 1 (02:46). Claude refuted it (02:135).
- **:266 "(not a `double` view)"** contradicts :45. The rejected option is "a `double` as the canonical value" (:111).
- **A8 appears in no bullet** (:270).
- **:280-283** should record the run 05 acceptances (claude-19, claude-20, codex-10, codex-11) once run 05 is committed. Avoid "this head" wording.

**claude-21 (Minor, open):** nothing changed at this head. A13 (006:211-212) still bars hardware and MCU/HUI surfaces from output-control and profile. That conflicts with A2 (006:132) and with ROADMAP:24, :224 and :250.

FINDING claude-1 | Major | open | A18 misnumbers items (006:264 "Items 1-4", :269 "Items 5-7 and 9"; targets are item 2 at :50, loops item 5 at :70); :272-273 says Claude wanted bindings outside, contra 02:153 (only Codex, 02:268,280); :276 gives A10's content for item 10, whose PR1 split is Codex's (02:282-284) over Claude's (02:178), and :295 "narrowed from the first draft" contradicts draft item 10 (006:103-104)
FINDING claude-2 | Major | accepted-fix | A1-A2 class hotkeys by effect (unknown means output-control) and route output kinds through RequestOutput; A15 gives profile and collection switching RequestProfileSwitch
FINDING claude-3 | Major | accepted-fix | A4 allows filters of public sources through the parent (obs-source.c:487-492 verified); transitions by name match WORKSPACE_FORMAT:414-415
FINDING claude-4 | Major | accepted-fix | A11 (006:191-199) defines match, guards, split, merge, thru and remapped thru, meeting ROADMAP:225-226
FINDING claude-5 | Major | accepted-fix | A12 (006:200-207) makes bindings local-only, never projected, same-machine restore, incompat; PR5 must also amend the section 4 projection (WORKSPACE_FORMAT:170-177)
FINDING claude-6 | Minor | accepted-fix | A17 reports a pending shadow and flushes it once on activation (edge case was claude-18)
FINDING claude-7 | Minor | accepted-fix | A7 degrades sample_offset to latest unless the consumer is verified; ParameterChanges sizing is listed as open (006:318-319)
FINDING claude-8 | Minor | accepted-fix | A16 (006:248-250) keeps the newest absolute value per writer and target, then reduces; deltas are summed
FINDING claude-9 | Minor | accepted-fix | A9 adds the matrix, network-backed ports as remote-net, learn and SysEx thru as meta, and OSCQuery scope; WinMM classing is open (006:314-316)
FINDING claude-10 | Minor | accepted-fix | A12 adds an incompat feature and applies the 65,536-byte rounding (WORKSPACE_FORMAT:266-267) to the omitted secrets file
FINDING claude-11 | Major | accepted-fix | A10 decides transforms, banks, layers, modifiers, learn and device identity in outline
FINDING claude-12 | Major | accepted-fix | review-01..04 and findings.md committed and ledger sources resolve; README.md:3-10 still lists only 01/02 and calls the folder a consult
FINDING claude-13 | Minor | accepted-fix | A14 (006:220-226) makes Host/Origin/no-CORS/close-on-malformed unconditional for TCP, HTTP and WebSocket, reads included; K4: pre-END SLIP bytes are a malformed frame
FINDING claude-14 | Major | accepted-fix | A13 (006:216-217) counts Network MIDI 2.0 as UDP with no privileged grant, consistent with A9 (006:175-176)
FINDING claude-15 | Minor | accepted-fix | A15 (006:245-247): creating or enabling SysEx/raw thru needs meta; egress class is the endpoint trust class mapped through A1's table
FINDING claude-16 | Minor | accepted-fix | A17 (006:251-255) matches obs-source.c: read at 4057-4059 (process_audio, called at 4085), signal 5735, write 5737, declared 96
FINDING claude-17 | Minor | accepted-fix | A8 (006:172-173) keys the sidecar by scene-UUID set with name fallback and unresolved; duplicates regenerate scene UUIDs (s1-persistence-audit.md:84)
FINDING claude-18 | Minor | accepted-fix | A17 (006:259-261): a newer observed OBS-UI or obs-websocket write wins and the shadow is dropped
FINDING claude-19 | Minor | accepted-fix | A13 (006:211-217) requires challenge-response with a per-session nonce, secret never sent, over TLS or A14-checked loopback TCP; K4 specifies, and no session is privileged until then
FINDING claude-20 | Minor | accepted-fix | A14 (006:221) requires Host to equal the bound address or a configured name, so LAN OSCQuery peers work and DNS rebinding still fails
FINDING claude-21 | Minor | open | A13 "Only an authenticated, confidential, connection-oriented transport can hold those classes" (006:211-212) is unscoped, so hw and MCU/HUI surfaces can never hold output-control or profile; transport targets (ROADMAP:224,250) and unknown-effect hotkeys (006:128) become unreachable, against ROADMAP:24 and A2 (006:132); scope it to network peers and state the hw path
VERDICT: request changes
