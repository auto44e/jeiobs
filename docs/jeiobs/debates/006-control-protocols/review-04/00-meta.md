# Review run

- Started: 2026-11-17T01:49:29Z
- Reviewed commit: `6c8044531e99c4632edecd2eec9d87ce9e81285e` (peers ran in a detached worktree at this commit)
- Author: lead; rounds: 2 (round 1 independent first pass, both at once, not isolated); cross-talk order: claude, codex; timeout per turn: 1500s
- Claude: claude 2.1.326.1734 (ASBX Claude Code, channel stable), model opus, effort max; tools Read, Glob, Grep; no MCP
- Codex: codex-cli 0.169.0 (engine started directly; the toolbox launcher runs only as Bedrock credential helper), effort xhigh; CODEX_HOME config without MCP/plugin/project sections, shell env "core", no project instructions (project_doc_max_bytes=0); --sandbox read-only, sandbox available
- Round 1 is an independent first pass, not isolated (see the script header)
- Script: `tools/jeiobs/peer-review/review.sh`, sha256 `63e7012b6f02f3151f72ee2ad2c1a6d849725472001d706e960af791c4eb99a4`
- Codex home: `~/.config/jeiobs-agent/codex-review`; config.toml sha256 `e3e76dcf155ea4674ca7ba34028a06eedba3e87ed3812d50da13ede1a7efbd44`; skills sha256 `253397a7192e7cf5c8088186e9a468f0696218ee6d62efe83d06c761e627bfeb`
- Context sha256 `0f0dc9e2b01ced01c70f4ae934fb3abbe6b27f346fe0b9a8d57f528dcd1f8b2f`; ledger sha256 `83cbf43207b30acf4c4e7481899c6c484cf98c2fa938d14e2de6592ee2afe481`
- stderr logs: kept locally (not committed), `04-run-20261117T014928Z`
- Topic: Review run 04 of the K1 design PR.

## CI state at the reviewed commit

```text
quick checks pending at start
local: check_scaffold.py PASS
```
- Finished: 2026-11-17T02:01:43Z

## Files written by this run

The hash given for `00-meta.md` is of its header (the part before "Finished");
the runner prints the final hash of every file, this one included, when it exits.

- `00-context.md` sha256 `0f0dc9e2b01ced01c70f4ae934fb3abbe6b27f346fe0b9a8d57f528dcd1f8b2f`
- `00-ledger-in.md` sha256 `83cbf43207b30acf4c4e7481899c6c484cf98c2fa938d14e2de6592ee2afe481`
- `00-meta.md` sha256 `21e2f4ebdaa88996a6be54faced92c5b8484d38d98746859d71b2d31e6d254ad`
- `01-r1-claude.prompt.md` sha256 `482ccbd288066637d04b7575eaaa89e99f3f76b4ecfcf15f67ab71dfa950663b`
- `01-r1-claude.reply.md` sha256 `acd37e8926108ac82f3047a9c092bae52769c476f148cddc8d6e7b6a483cfc4d`
- `02-r1-codex.prompt.md` sha256 `482ccbd288066637d04b7575eaaa89e99f3f76b4ecfcf15f67ab71dfa950663b`
- `02-r1-codex.reply.md` sha256 `ac3bd6bcb0fac932481cac06e24e76b42abf73013f3d3b17e9eb44b517ce46e7`
- `03-r2-claude.prompt.md` sha256 `bb3a26cee384bc13b05fff7e20c0fcc7ce75069c71462fe5cd923710e0815f0e`
- `03-r2-claude.reply.md` sha256 `30f209a6adcbb93d5c7106642136ec15ca3be024ebfd7dfc1cf95c4e38a334aa`
- `04-r2-codex.prompt.md` sha256 `3b0c14ae1d4faef0d99684d83cce4a43026db552716ae7c20d5ca9d479a91213`
- `04-r2-codex.reply.md` sha256 `0f89a8bf3f5778b3ea33442ad92281594fb47b6c7bd12e5f88acf8be4be81d3f`

## Final dispositions

```text
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
FINDING codex-1 | Major | accepted-fix | A7 assigns balance to the atomic latest-value slot (006:160-167)
FINDING codex-2 | Major | accepted-fix | A12 makes bindings local-only and excluded from exports (006:200-207)
FINDING codex-3 | Major | accepted-fix | A5 defines TargetState snapshots, polling, and feedback reads (006:145-150)
FINDING codex-4 | Major | accepted-fix | A15 defines target identities, paths, classes, and profile switching (006:223-243)
FINDING codex-5 | Major | accepted-fix | A1 defines principals, capability checks, defaults, and hard denies (006:117-126)
FINDING codex-6 | Major | accepted-fix | A6 defines queue isolation, fairness, overflow, reducers, and privileged handling (006:151-159)
FINDING codex-7 | Minor | accepted-fix | A17 now correctly distinguishes balance reads and setter writes (006:247-251)
FINDING codex-8 | Minor | accepted-fix | A16 defines per-writer absolute retention and relative-delta accumulation (006:244-246)
FINDING codex-9 | Major | open | A18 still says “Codex has not accepted A12-A14,” contradicting Codex’s acceptance of A12 (006:267; review-03/04-r2-codex.reply.md:1,6)
FINDING codex-10 | Minor | open | A14 requires “a loopback Host” “unconditional for every ... listener,” but A9 exposes OSCQuery to remote peers; LAN Host matching is undefined (006:174-177,218-222)
FINDING codex-11 | Minor | open | A13 specifies a “token-authenticated OSC/TCP session” but no handshake, replay, or confidentiality requirement (006:208-214; ROADMAP:266-270)
VERDICT: request changes
```
