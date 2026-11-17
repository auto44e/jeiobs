**Request changes.** The head is still 064f6c543, so nothing has changed since my round-1 reply. Two Majors stay open: claude-1 and claude-14. Codex's approval doesn't touch either one, and the gate requires no open Major (DECISIONS.md:187-196). Each needs one or two sentences.

**Codex's reply**
- **codex-7:** same defect as my claude-16, and we agree on it. `obs-source.c:4057-4059` only reads `source->balance`; the only write is the setter at `:5737`.
- **claude-14:** Codex hasn't said whether it agrees. It's a protocol fact the other lineage can check, so Codex should confirm or rebut it.

**What would close the two Majors**
- **claude-14:** A13 (006:210-213) says "no grant over UDP can give output-control, profile or meta". It then names "Network MIDI 2.0 with its authentication" as a transport that can.
  - NM2 runs over UDP (ROADMAP:245).
  - Its authentication covers only the session invitation. After that, a UMP Data packet carries a 16-bit sequence number and no MAC, and the session is matched by IP and port.
  - So the spoofing that A13's UDP rule guards against still works against NM2.
  - Either fix closes it: say "Network MIDI 2.0 counts as UDP here", or move "which transports count as authenticated" to Open before K1-PR1.
- **claude-1:** A18 (006:250-255) needs three changes:
  - Say who settled item 10, A9, A11 and A15-A17. For most of them, "the lead, answering review findings" would do.
  - Take A13-A14 out of "after the peers disagreed". No peer disagreed: no Codex reply in this folder mentions loopback or grants.
  - Add "Decisions without a flex bullet are firm." That covers static-only loops (006:281), which still has neither, and the other items without a flex bullet.
  - This is Major because DECISIONS.md:68-69 says a missing record part blocks a self-merge.

Both fixes are text-only. The new head still needs a run 04 (DECISIONS.md:176). If the edits read as above, I'll accept them there.

**Minors (follow-ups, not blocking):** claude-13 and claude-15 to claude-18 are unchanged.

FINDING claude-1 | Major | open | A18 (006:250-255) omits who settled item 10, A9, A11 and A15-A17, files A13-A14 under "after the peers disagreed" though no Codex reply addresses loopback or grants, and static-only loops (006:281) and other decisions still lack flex or "firm" (DECISIONS.md:49-54,68-69); fix: attribute them and add "decisions without a flex bullet are firm"
FINDING claude-2 | Major | accepted-fix | A1-A2 class hotkeys by effect (unknown means output-control) and route output kinds through RequestOutput; A15 gives profile and collection switching RequestProfileSwitch
FINDING claude-3 | Major | accepted-fix | A4 allows filters of public sources through the parent (obs-source.c:487-492 verified); transitions by name match WORKSPACE_FORMAT:414-415; encoder and service registerers (obs-hotkey.h:58-59) should follow A4's by-name rule
FINDING claude-4 | Major | accepted-fix | A11 (006:191-199) defines match (equality, range, set, prefix, capped wildcard), guards, split, merge, thru and remapped thru, meeting ROADMAP:225-226
FINDING claude-5 | Major | accepted-fix | A12 (006:200-207) makes bindings local-only, never projected, same-machine restore, incompat; PR5 must also amend the section 4 projection (WORKSPACE_FORMAT:170-177), not only sections 6-7
FINDING claude-6 | Minor | accepted-fix | A17 (006:246-249) reports a pending shadow and flushes it once on activation (edge case split out as claude-18)
FINDING claude-7 | Minor | accepted-fix | A7 degrades sample_offset to latest unless the consumer is verified; ParameterChanges sizing is listed as open (006:290)
FINDING claude-8 | Minor | accepted-fix | A16 (006:240-242) keeps the newest absolute value per writer and target, then reduces; deltas are summed
FINDING claude-9 | Minor | accepted-fix | A9 adds the matrix, network-backed ports as remote-net, learn and SysEx thru as meta, and OSCQuery scope; K2 must say how backends without transport info (WinMM) class rtpMIDI ports
FINDING claude-10 | Minor | accepted-fix | A12 adds an incompat feature and applies the 65,536-byte rounding (WORKSPACE_FORMAT:266-267) to the omitted secrets file
FINDING claude-11 | Major | accepted-fix | A10 decides transforms, banks, layers, modifiers, learn and device identity in outline; toggle and relative-to-absolute already need per-route state
FINDING claude-12 | Major | accepted-fix | review-01/, review-02/ and findings.md are committed, ledger sources resolve to FINDING lines (11 claude, 6 codex in run 01), run 03 reviews 064f6c543; README.md:3-10 still lists only 01/02 and calls the folder a consult
FINDING claude-13 | Minor | open | A14 removes default write classes from loopback TCP/HTTP, but a listener without Host/Origin checks still serves reads, and OSCQuery's WebSocket upgrade (a GET, no CORS) streams mapped values to any page or browser source; make the checks unconditional
FINDING claude-14 | Major | open | A13 "no grant over UDP can give output-control, profile or meta" vs "Only an authenticated transport (for example Network MIDI 2.0 with its authentication" (006:210-213): NM2 is UDP (ROADMAP:245) and authenticates only the invitation, so a granted peer's IP:port is spoofable; say NM2 counts as UDP, or move "which transports are authenticated" to Open before K1-PR1
FINDING claude-15 | Minor | open | "SysEx and raw wire thru need meta unless a route is explicitly mapped" (006:238-239) is undefined when every route is explicit and needs meta to create; egress's class (006:235) is a trust class, not an A1 capability class
FINDING claude-16 | Minor | open | A17 "Balance is written on whichever thread pushes audio" (006:243) is wrong: obs-source.c:4057-4059 reads source->balance there; the only write is the setter at :5737 on the caller's thread (codex-7 concurs)
FINDING claude-17 | Minor | open | A8 keys the sidecar and A15 identifies profiles and collections by "generated UID (WORKSPACE_FORMAT section 9)", but that UID is "stored in the revision" (WORKSPACE_FORMAT:400), per workspace, and "a renamed profile gets a new UID" (:408-409); with no capture there is none, so PR5 needs a live UID source or a name or scene-UUID key with unresolved fallback
FINDING claude-18 | Minor | open | A17 "the read reports it as pending" then "flushes the latest shadow once when the source becomes active" (006:247-249): a newer OBS-UI or obs-websocket write to the inactive source still fires volume and sets user_volume (obs-source.c:4564,4574), and A6 counts it as a LastWriter writer, yet the flush reverts it; a newer observed write must replace the shadow
VERDICT: request changes
