**Request changes.** claude-19 and claude-20 are fixed. claude-1 stays open at Major. The A18 rewrite gets the Codex attributions right, but it numbers two bullets wrongly and still misstates items 8 and 10. I'm adding one new Minor, claude-21. I have no shell, so I checked the changes against my run-04 quotes rather than `git diff`.

**claude-1: what's still wrong in A18** (each fix is one line)
1. **Item numbers are wrong.** :264 says "Items 1-4 (values, applying commands, arbitration, loops)" and :269 says "Items 5-7 and 9 (targets by UUID, …)". Item 2 is Targets (006:50) and item 5 is Loops (006:70). The labels are right, so the numbers should be "1, 3, 4 and 5" and "2, 6, 7 and 9".
2. **Item 8 (raised in run 04, not fixed).** :272-274 says the debate had "Claude and Codex want bindings outside the portable workspace", and that the reviews "asked for capture instead". The transcript says otherwise:
   - Claude proposed capturing bindings as `opaque-unreviewed` (02:153).
   - Codex disputed exactly that: "local state, not merely an opaque-unreviewed workspace file" (02:268, 280). Draft item 8 (006:90-91) took Codex's position.
   - Suggested text: "Run 01 (claude-5, codex-2) asked for capture; run 02 asked for no export path; both peers accepted A12 in run 03."
3. **Item 10 (raised in run 04, not fixed).** :276 gives item 10 A10's content, but item 10 is the PR slicing:
   - PR1's narrow scope is Codex's (02:282-284), chosen over Claude's (02:178).
   - The later PR order is Claude's (02:179-182).
   - :295 says PR1 was "narrowed from the first draft". The committed draft's item 10 (006:103-104) is already narrow. The wide PR1 in the transcripts is Claude's debate proposal.

Nits (not needed to close claude-1):
- A8 still appears in no bullet (:270 lists "A1-A7 and A9").
- :267 lists "(not equal-priority rejection)" among Codex's wins over Claude. That was Codex's own round-1 rule (02:46), which Claude refuted (02:135). Also, "(not a `double` view)" (:266) clashes with :44, which keeps a double view for transforms.
- :278-279 "open on A17's factual wording until this head" is inaccurate. Codex accepted A17 at 6c8044531 (review-04/02-r1-codex.reply.md:9, 04-r2-codex.reply.md:7), and "this head" will mean nothing after merge.

If points 1-3 are fixed as above, I'll accept claude-1 in run 06.

**Accepted**
- **claude-19:** A13 (006:211-217) now requires challenge-response with a per-session nonce, never sends the secret, and runs over TLS or over loopback TCP that passes A14. K4 specifies the details, and until then no session holds a privileged class.
- **claude-20:** A14 (006:221) requires the Host header to equal the bound address or a configured name. LAN OSCQuery peers can connect, and DNS rebinding still fails.
- codex-9, codex-10 and codex-11 look fixed to me (006:274-275, :221, :211-217). Closing them is Codex's call.

**New Minor, claude-21.** A13 (006:211-212) says "Only an authenticated, confidential, connection-oriented transport can hold those classes", with no scope.
- Read literally, a hardware MIDI or MCU/HUI surface can never hold output-control or profile, even when the local user grants it.
- That makes transport targets (ROADMAP:224, :250) and unknown-effect hotkeys (006:128) unreachable from any controller, against ROADMAP:24.
- A2 (006:132) suggests only network peers need a grant.
- Fix: limit the sentence to network peers, and say how the local user enables output-control for a hardware surface.

FINDING claude-1 | Major | open | A18 006:264 "Items 1-4 (values, applying commands, arbitration, loops)" and :269 "Items 5-7 and 9 (targets by UUID...)" but item 2 is Targets (006:50), item 5 Loops (006:70); :272-274 still says Claude wanted bindings outside and later "asked for capture instead", contra 02:153 (only Codex 02:268,280); :276 gives item 10 A10's content though PR1 scope was Codex's (02:282-284) over Claude's (02:178)
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
FINDING claude-21 | Minor | open | A13 "Only an authenticated, confidential, connection-oriented transport can hold those classes" (006:211-212) is unscoped: hw and MCU/HUI surfaces could never hold output-control or profile, so transport targets (ROADMAP:224,250) and unknown-effect hotkeys (006:128) are unreachable, while A2 (006:132) implies only network peers need a grant; scope to network peers and state the hw path
VERDICT: request changes
