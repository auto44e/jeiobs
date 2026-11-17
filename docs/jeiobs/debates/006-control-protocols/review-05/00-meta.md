# Review run

- Started: 2026-11-17T02:48:10Z
- Reviewed commit: `4f8417ff74471d604ca8d4ba987f549e6e443266` (peers ran in a detached worktree at this commit)
- Author: lead; rounds: 2 (round 1 independent first pass, both at once, not isolated); cross-talk order: claude, codex; timeout per turn: 1500s
- Claude: claude 2.1.326.1734 (ASBX Claude Code, channel stable), model opus, effort max; tools Read, Glob, Grep; no MCP
- Codex: codex-cli 0.169.0 (engine started directly; the toolbox launcher runs only as Bedrock credential helper), effort xhigh; CODEX_HOME config without MCP/plugin/project sections, shell env "core", no project instructions (project_doc_max_bytes=0); --sandbox read-only, sandbox available
- Round 1 is an independent first pass, not isolated (see the script header)
- Script: `tools/jeiobs/peer-review/review.sh`, sha256 `63e7012b6f02f3151f72ee2ad2c1a6d849725472001d706e960af791c4eb99a4`
- Codex home: `~/.config/jeiobs-agent/codex-review`; config.toml sha256 `e3e76dcf155ea4674ca7ba34028a06eedba3e87ed3812d50da13ede1a7efbd44`; skills sha256 `253397a7192e7cf5c8088186e9a468f0696218ee6d62efe83d06c761e627bfeb`
- Context sha256 `f5ba2d8dbf3f65c9f5efd390479e0e639e71fdfe9f9b8527bab43a364e723c70`; ledger sha256 `acb5feb4b1c91bddd9a8748ed6de385d9fd2795652409df29de8a36bc886ba8e`
- stderr logs: kept locally (not committed), `05-run-20261117T024809Z`
- Topic: Review run 05 of the K1 design PR.

## CI state at the reviewed commit

```text
quick checks pending at start
local: check_scaffold.py PASS
```
- Finished: 2026-11-17T03:04:10Z

## Files written by this run

The hash given for `00-meta.md` is of its header (the part before "Finished");
the runner prints the final hash of every file, this one included, when it exits.

- `00-context.md` sha256 `f5ba2d8dbf3f65c9f5efd390479e0e639e71fdfe9f9b8527bab43a364e723c70`
- `00-ledger-in.md` sha256 `acb5feb4b1c91bddd9a8748ed6de385d9fd2795652409df29de8a36bc886ba8e`
- `00-meta.md` sha256 `f53de1c10f0d959d4da34ebf392565c167d9c4bd8f5f2c133085efea5e5fb9c5`
- `01-r1-claude.prompt.md` sha256 `76e9864817064b8543992548dd3cf59fac5242aff56c74ba5b99d6769737db13`
- `01-r1-claude.reply.md` sha256 `f9aebf6e5a72e21e39f57f2e91d3f675602370327df6fd7de934c5f11a1b1b06`
- `02-r1-codex.prompt.md` sha256 `76e9864817064b8543992548dd3cf59fac5242aff56c74ba5b99d6769737db13`
- `02-r1-codex.reply.md` sha256 `1ddecdbf2a54accf9cec20af1e3173431c9b6be4225efe00da19ed00b0ba39dc`
- `03-r2-claude.prompt.md` sha256 `b6ddd5f4eb5395f7556a970096529dfd1a983accef3f73cd14e147c0f7d92f48`
- `03-r2-claude.reply.md` sha256 `c1a9dd804d7fd3924e6a14c5931b07a61e40869c7369fb179c867480cb6bf131`
- `04-r2-codex.prompt.md` sha256 `46367cb8d70c26f958849996bd1b85f3311310ef69722495e116d877fe8a0640`
- `04-r2-codex.reply.md` sha256 `5a1f4cdad67b2b2b75a4b1bb63b7139e609c9a3d6cf92b80d0ace95dd12fa622`

## Final dispositions

```text
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
FINDING codex-1 | Major | accepted-fix | A7 assigns balance to the latest-value slot (006:251-255)
FINDING codex-2 | Major | accepted-fix | A12 makes bindings local-only and export-omitted (006:200-207)
FINDING codex-3 | Major | accepted-fix | A5 defines target-state reads and feedback (006:145-150)
FINDING codex-4 | Major | accepted-fix | A15 defines target paths and profile switching (006:227-247)
FINDING codex-5 | Major | accepted-fix | A1 defines principals, capability checks, and hard denies (006:117-126)
FINDING codex-6 | Major | accepted-fix | A6 defines queue isolation and overflow handling (006:151-159)
FINDING codex-7 | Minor | accepted-fix | A17 distinguishes balance reads from setter writes (006:251-255)
FINDING codex-8 | Minor | accepted-fix | A16 defines per-writer absolute retention (006:248-250)
FINDING codex-9 | Major | accepted-fix | A18 records Codex’s A12 acceptance (006:272-283)
FINDING codex-10 | Minor | accepted-fix | A14 matches Host to the bound address or configured name (006:218-226)
FINDING codex-11 | Minor | accepted-fix | A13 adds nonce challenge-response and TLS/loopback requirements (006:208-217)
FINDING codex-12 | Major | open | A18 misnumbers targets and loops: targets are item 2 and loops item 5 (006:38-73,264-271)
FINDING codex-13 | Major | open | A18 falsely says both peers wanted bindings outside capture; only Codex did (006:272-275; 02-k1-design-debate.md:153,266-280)
FINDING codex-14 | Major | open | A18 says A17 was open until this head despite Codex accepting it in review-04 (006:276-279; review-04/04-r2-codex.reply.md:7)
VERDICT: request changes
```
