**Approve.** claude-1 and claude-21 are fixed. I'm adding one new Minor, claude-22, for some attribution slips left in A18. It's a follow-up and doesn't block a self-merge.

**claude-1: accepted.** I checked A18 against `02` and review-01 to review-05.
- **Item numbers:** :266 ("Items 1, 3, 4 and 5") and :271 ("Items 2, 6, 7 and 9") now match the list at :38-73.
- **Item 8:** :274-275 now matches the debate. Claude proposed capture as `opaque-unreviewed` (02:153) and Codex wanted local state (02:268, 280). The draft took Codex's position (:90-91).
- **Item 10:** :278-279 matches 02:178 vs 02:282-284, and the flex line at :299-300 is fixed.
- **Codex's acceptances all check out:**
  - A12, A15 and A16: review-03/04-r2-codex.reply.md:1, 6, 8, 12.
  - A17: review-04/02-r1-codex.reply.md:9 and 04-r2-codex.reply.md:7.
  - Codex confirmed claude-14 at review-03/04-r2-codex.reply.md:3.
- codex-12, codex-13 and codex-14 look fixed to me. Closing them is Codex's call.

**claude-21: accepted.** A13 (:212-214) now limits the transport rule to network principals. `hw` and `virtual` endpoints follow A1's table, and a USB MCU or HUI surface can be granted output-control locally, with the grant stored in grants.json (:208). This fits A2 (:132), ROADMAP:24 and ROADMAP:269. Network-backed OS ports stay `remote-net` (:175-176), and WinMM classing is still listed as open (:318-320).
- One note for PR4's matrix: a `virtual` port is often a network bridge, so granting it output-control should come with a warning.

**claude-22 (new, Minor)**
1. **Item 8, run 02 (:276).** A18 says "review rounds 1 and 2 asked for capture". A8 already captured bindings, so run 02 asked for no export path instead:
   - claude-5's fix: "a role the projection always omits, a same-machine rule, and an incompat feature" (review-02/01-r1-claude.reply.md:14, 41).
   - codex-2: review-02/02-r1-codex.reply.md:4.
   - That fix is A12, which :276-277 credits to "the lead". I asked for this wording last run (review-05/03-r2-claude.reply.md:10).
2. **A13, A14 and A16.** :285 says A13 and A14 were "proposed by the lead", and :281 credits A16 to "the lead". The substance of all three comes from fixes proposed in run 02 (review-02/01-r1-claude.reply.md:44, 49-50):
   - A13: claude-14, "keep grants machine-local, outside capture and import".
   - A14: claude-13, "default only for UDP or Host/Origin-checked transports".
   - A16: claude-8, "keep the newest per writer and target, then reduce". codex-8 raised the same gap.
3. **Only Codex's acceptances are recorded.** Claude accepted A12 and A16 in run 03 (review-03/03-r2-claude.reply.md:27, 30) and A17 in run 04.

Suggested text:
- :276: "run 01 asked for capture, run 02 for no export path (claude-5, codex-2); both peers accepted A12 in run 03"
- :281: "A16: claude-8's run-02 fix (codex-8 raised the same gap); both accepted in run 03"
- :285: "the lead's text of the run-02 fixes in claude-14 and claude-13, refined in run 04 (claude-19, claude-20, codex-10, codex-11)"

**Nits** (not needed to close anything):
- A8 is still in no bullet (:272).
- The later PR order is Claude's (02:179-182), and A18 doesn't say so.
- A10 answered claude-11.
- debates/006 README.md:10 still calls the folder a "design consult".

FINDING claude-1 | Major | accepted-fix | A18 numbers items per :38-73 (:266 "1, 3, 4 and 5", :271 "2, 6, 7 and 9"); item 8 matches 02:153 vs 02:268,280; item 10 matches 02:178 vs 02:282-284; flex :299-300 fixed; residue tracked as claude-22
FINDING claude-2 | Major | accepted-fix | A1-A2 class hotkeys by effect (unknown means output-control) and route output kinds through RequestOutput; A15 gives profile and collection switching RequestProfileSwitch
FINDING claude-3 | Major | accepted-fix | A4 allows filters of public sources through the parent (obs-source.c:487-492 verified); transitions by name match WORKSPACE_FORMAT:414-415
FINDING claude-4 | Major | accepted-fix | A11 (006:191-199) defines match, guards, split, merge, thru and remapped thru, meeting ROADMAP:225-226
FINDING claude-5 | Major | accepted-fix | A12 (006:200-207) makes bindings local-only, never projected, same-machine restore, incompat; PR5 must also amend the section 4 projection (WORKSPACE_FORMAT:170-177)
FINDING claude-6 | Minor | accepted-fix | A17 reports a pending shadow and flushes it once on activation (edge case was claude-18)
FINDING claude-7 | Minor | accepted-fix | A7 degrades sample_offset to latest unless the consumer is verified; ParameterChanges sizing is listed as open (006:322-323)
FINDING claude-8 | Minor | accepted-fix | A16 (006:250-252) keeps the newest absolute value per writer and target, then reduces; deltas are summed
FINDING claude-9 | Minor | accepted-fix | A9 adds the matrix, network-backed ports as remote-net, learn and SysEx thru as meta, and OSCQuery scope; WinMM classing is open (006:318-320)
FINDING claude-10 | Minor | accepted-fix | A12 adds an incompat feature and applies the 65,536-byte rounding (WORKSPACE_FORMAT:266-267) to the omitted secrets file
FINDING claude-11 | Major | accepted-fix | A10 decides transforms, banks, layers, modifiers, learn and device identity in outline
FINDING claude-12 | Major | accepted-fix | review-01..05 and findings.md committed and ledger sources resolve; README.md:3-10 still lists only 01/02 and calls the folder a consult
FINDING claude-13 | Minor | accepted-fix | A14 (006:220-228) makes Host/Origin/no-CORS/close-on-malformed unconditional for TCP, HTTP and WebSocket, reads included; K4: pre-END SLIP bytes are a malformed frame
FINDING claude-14 | Major | accepted-fix | A13 (006:218-219) counts Network MIDI 2.0 as UDP with no privileged grant, consistent with A9 (006:175-176)
FINDING claude-15 | Minor | accepted-fix | A15 (006:247-249): creating or enabling SysEx/raw thru needs meta; egress class is the endpoint trust class mapped through A1's table
FINDING claude-16 | Minor | accepted-fix | A17 (006:253-257) matches obs-source.c: read at 4057-4059 (process_audio, called at 4085), signal 5735, write 5737, declared 96
FINDING claude-17 | Minor | accepted-fix | A8 (006:172-173) keys the sidecar by scene-UUID set with name fallback and unresolved; duplicates regenerate scene UUIDs (s1-persistence-audit.md:84)
FINDING claude-18 | Minor | accepted-fix | A17 (006:261-263): a newer observed OBS-UI or obs-websocket write wins and the shadow is dropped
FINDING claude-19 | Minor | accepted-fix | A13 (006:215-218) requires challenge-response with a per-session nonce, secret never sent, over TLS or A14-checked loopback TCP; K4 specifies, and no session is privileged until then
FINDING claude-20 | Minor | accepted-fix | A14 (006:223) requires Host to equal the bound address or a configured name, so LAN OSCQuery peers work and DNS rebinding still fails
FINDING claude-21 | Minor | accepted-fix | A13 (006:212-214) limits the transport rule to network principals; hw and virtual follow A1 with a local grant (grants.json :208), so USB MCU/HUI can reach output-control (ROADMAP:24; A2 006:132)
FINDING claude-22 | Minor | open | A18 006:276 "review rounds 1 and 2 asked for capture" but run 02 asked for no export path (review-02/01-r1-claude.reply.md:14,41; 02-r1-codex.reply.md:4); :285 "proposed by the lead" (A13, A14) and :281 "A16: the lead" are run-02 fixes claude-14, claude-13, claude-8 (review-02/01-r1-claude.reply.md:44,49-50); Claude's acceptances unrecorded (review-03/03-r2-claude.reply.md:27,30)
VERDICT: approve
