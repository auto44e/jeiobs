**Request changes.** Two Majors are still open: claude-1 and claude-14. Each needs about one sentence to fix. A11, A12, A16 and A17 fix claude-4, 5, 6, 8 and 10, and committing the transcripts fixes claude-12. I couldn't check the transcript sha256 values against `00-meta.md` because I have no shell.

**Still Major**
- **claude-14:** A13 now keeps grants machine-local, which fixes the storage part. But A13 contradicts itself at 006:211-213. It says "no grant over UDP can give output-control, profile or meta", then names "Network MIDI 2.0 with its authentication" as a transport that can.
  - NM2 runs over UDP (ROADMAP:245, "Network MIDI 2.0 (UDP)").
  - Its authentication covers only the session invitation. UMP Data commands carry a sequence number and no MAC, so a granted peer's IP and port can be spoofed exactly like plain UDP.
  - Fix: say that NM2 counts as UDP.
- **claude-1:** A18 (006:250-255) still has gaps.
  - It doesn't say who settled item 10, A9, A11 or A15-A17.
  - It files A13-A14 under "after the peers disagreed". They were the lead's answers to claude-13 and claude-14, and Codex never took a position on those (review-02/02 and 04 contain only codex-* lines).
  - Static-only loops (006:281) still has neither flex nor "firm"; I raised this last run. Several other decisions have no flex bullet either.
  - Fix: extend A18 (or the PR description) and add "decisions without a flex bullet are firm".

**Lowered or kept at Minor**
- **claude-13, lowered from Major:** A14 removes the default write classes. Reads remain exposed on a listener that doesn't check Host or Origin. OSCQuery's WebSocket upgrade isn't subject to CORS, so any web page or browser source can subscribe (LISTEN) to mapped values. Make those checks unconditional.
- **claude-15:** It's unclear what "unless a route is explicitly mapped" (006:238-239) excludes. Every route is an explicit mapping, and creating one already needs `meta`. Also, the egress row's class (006:235) is a trust class, not an A1 capability class.
- **claude-16:** A17 says balance "is written on whichever thread pushes audio". At `obs-source.c:4058-4059` that thread only *reads* `source->balance`. The only write is the setter at `:5737`, on the caller's thread.

**New Minors**
- **claude-17:** the generated profile and collection UID isn't available at runtime (details in the FINDING line).
- **claude-18:** the flush when a source becomes active can undo a newer write made through OBS or obs-websocket (details in the FINDING line).

**Codex's findings:** codex-2 (A12), codex-4 (A15, with the claude-17 caveat) and codex-8 (A16) look fixed to me. codex-7 is fixed except for the read/write wording in claude-16.

FINDING claude-1 | Major | open | A18 (006:250-255) omits who settled item 10, A9, A11 and A15-A17, and files A13-A14 under "after the peers disagreed" though they answer claude-13/14 with no split; static-only loops (006:281) and other decisions still lack flex or "firm" (DECISIONS.md:49-54,68-69)
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
FINDING claude-12 | Major | accepted-fix | review-01/, review-02/ and findings.md are committed, ledger sources resolve to FINDING lines, run 03 reviews 064f6c543; README.md:3-10 still lists only 01/02 and calls the folder a consult
FINDING claude-13 | Minor | open | Lowered: A14 removes default write classes from loopback TCP/HTTP, but a listener without Host/Origin checks still serves reads, and OSCQuery's WebSocket upgrade (no CORS) streams mapped values to any page or browser source; make the checks unconditional
FINDING claude-14 | Major | open | A13 "no grant over UDP can give output-control, profile or meta" vs "Only an authenticated transport (for example Network MIDI 2.0 with its authentication" (006:211-213): NM2 is UDP (ROADMAP:245) and authenticates only the invitation, so a granted peer's IP:port is spoofable; NM2 must count as UDP
FINDING claude-15 | Minor | open | A15 adds classes and deny-if-unclassified, but "SysEx and raw wire thru need meta unless a route is explicitly mapped" (006:238-239) is undefined when every route is explicit and needs meta to create; egress's class (006:235) is a trust class, not an A1 capability class
FINDING claude-16 | Minor | open | A17 "Balance is written on whichever thread pushes audio" (006:243) is wrong: obs-source.c:4058-4059 reads source->balance there; the only write is the setter at :5737 on the caller's thread
FINDING claude-17 | Minor | open | A8 keys the sidecar and A15 identifies profiles and collections by "generated UID (WORKSPACE_FORMAT section 9)", but that UID is "stored in the revision" (WORKSPACE_FORMAT:400), per workspace, and "a renamed profile gets a new UID" (:408-409); with no capture there is none, so PR5 needs a live UID source or a name or scene-UUID key with unresolved fallback
FINDING claude-18 | Minor | open | A17 "the read reports it as pending" then "flushes the latest shadow once when the source becomes active" (006:247-249): a newer OBS-UI or obs-websocket write to the inactive source still fires volume and sets user_volume (obs-source.c:4564,4574), and A6 counts it as a LastWriter writer, yet the flush reverts it; a newer observed write must replace the shadow
VERDICT: request changes
