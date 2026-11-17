# Review run

- Started: 2026-11-17T04:19:21Z
- Reviewed commit: `c3d8f64c2112a673104a6885eb0cb23bdbece588` (peers ran in a detached worktree at this commit)
- Author: lead; rounds: 2 (round 1 independent first pass, both at once, not isolated); cross-talk order: claude, codex; timeout per turn: 1500s
- Claude: claude 2.1.326.1734 (ASBX Claude Code, channel stable), model opus, effort max; tools Read, Glob, Grep; no MCP
- Codex: codex-cli 0.169.0 (engine started directly; the toolbox launcher runs only as Bedrock credential helper), effort xhigh; CODEX_HOME config without MCP/plugin/project sections, shell env "core", no project instructions (project_doc_max_bytes=0); --sandbox read-only, sandbox available
- Round 1 is an independent first pass, not isolated (see the script header)
- Script: `tools/jeiobs/peer-review/review.sh`, sha256 `63e7012b6f02f3151f72ee2ad2c1a6d849725472001d706e960af791c4eb99a4`
- Codex home: `~/.config/jeiobs-agent/codex-review`; config.toml sha256 `e3e76dcf155ea4674ca7ba34028a06eedba3e87ed3812d50da13ede1a7efbd44`; skills sha256 `253397a7192e7cf5c8088186e9a468f0696218ee6d62efe83d06c761e627bfeb`
- Context sha256 `79bdec9fa3de4fc4899f2635a3119b03816c11715d66dcc57180b6a3a01fa238`; ledger sha256 `c9e7d830155dcbfd7952ac763d51c754ed55a59267fb8157eeaacb6720b88199`
- stderr logs: kept locally (not committed), `06-run-20261117T041921Z`
- Topic: Review run 06 of the K1 design PR.

## CI state at the reviewed commit

```text
quick checks pending at start
local: check_scaffold.py PASS
```
- Finished: 2026-11-17T04:31:07Z

## Files written by this run

The hash given for `00-meta.md` is of its header (the part before "Finished");
the runner prints the final hash of every file, this one included, when it exits.

- `00-context.md` sha256 `79bdec9fa3de4fc4899f2635a3119b03816c11715d66dcc57180b6a3a01fa238`
- `00-ledger-in.md` sha256 `c9e7d830155dcbfd7952ac763d51c754ed55a59267fb8157eeaacb6720b88199`
- `00-meta.md` sha256 `94b7bd2506d45ea0c170fc9566a3a571ac4e5681338c6635d2e78f4a40e7ac28`
- `01-r1-claude.prompt.md` sha256 `cfd7cfde1631566c5ac50d98ae11c1580b8da05cace4145822e0267c91b99056`
- `01-r1-claude.reply.md` sha256 `4e9c527d0c7a82e3802ff1c57e0bc556517ae1fbfcf766ca8dc723bd9cf18ad1`
- `02-r1-codex.prompt.md` sha256 `cfd7cfde1631566c5ac50d98ae11c1580b8da05cace4145822e0267c91b99056`
- `02-r1-codex.reply.md` sha256 `10d1116dace968dec6708c7350b3b08712f46b8f1bfeb89e62e0a559756b807b`
- `03-r2-claude.prompt.md` sha256 `efe23aaa0672c2ec88717382ca04520ed10df6c8544190c0fe4e893c4fd65edf`
- `03-r2-claude.reply.md` sha256 `d68f8a9977645dfb17e8314c548d4f45e747b82ff3d36bd1d29f8c362c1100b3`
- `04-r2-codex.prompt.md` sha256 `61622fb7b76660ecc01384e9f41cf1101d29a2d2bddf51b80acc7656217aa68f`
- `04-r2-codex.reply.md` sha256 `ed095bd0e1254ae94ab485600cbe076af00bfb00e6196b4710d0101cb8457d70`

## Final dispositions

```text
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
FINDING claude-22 | Minor | open | A18 006:276 "review rounds 1 and 2 asked for capture" but run 02 asked for no export path (review-02/01-r1-claude.reply.md:14,41; 02-r1-codex.reply.md:4); :281 and :285 credit A16, A13, A14 to "the lead" though they are run-02 fixes claude-8, claude-14, claude-13 (review-02/01-r1-claude.reply.md:44,49-50); Claude's acceptances (review-03/03-r2-claude.reply.md:27,30) and run-05 codex-10/11 (review-05/04-r2-codex.reply.md:12-13) unrecorded
VERDICT: approve
FINDING codex-1 | Major | accepted-fix | Balance uses the latest-value slot (docs/jeiobs/tasks/006-control-protocols.md:163-167)
FINDING codex-2 | Major | accepted-fix | Bindings are local-only and omitted from exports (docs/jeiobs/tasks/006-control-protocols.md:200-207)
FINDING codex-3 | Major | accepted-fix | TargetState reads and feedback are defined (docs/jeiobs/tasks/006-control-protocols.md:145-150)
FINDING codex-4 | Major | accepted-fix | Target paths and profile switching are defined (docs/jeiobs/tasks/006-control-protocols.md:229-247)
FINDING codex-5 | Major | accepted-fix | Principals, capability checks, and hard denies are defined (docs/jeiobs/tasks/006-control-protocols.md:117-126)
FINDING codex-6 | Major | accepted-fix | Queue isolation and overflow handling are defined (docs/jeiobs/tasks/006-control-protocols.md:151-159)
FINDING codex-7 | Minor | accepted-fix | Balance reads and setter writes are distinguished (docs/jeiobs/tasks/006-control-protocols.md:253-257)
FINDING codex-8 | Minor | accepted-fix | Absolute values are retained per writer before reduction (docs/jeiobs/tasks/006-control-protocols.md:250-252)
FINDING codex-9 | Major | accepted-fix | A18 records Codex’s A12 acceptance (docs/jeiobs/tasks/006-control-protocols.md:274-277)
FINDING codex-10 | Minor | accepted-fix | Host matching supports bound addresses or configured names (docs/jeiobs/tasks/006-control-protocols.md:220-228)
FINDING codex-11 | Minor | accepted-fix | A13 defines nonce challenge-response and TLS/loopback requirements (docs/jeiobs/tasks/006-control-protocols.md:208-219)
FINDING codex-12 | Major | accepted-fix | A18 now assigns the correct item numbers (docs/jeiobs/tasks/006-control-protocols.md:264-272)
FINDING codex-13 | Major | accepted-fix | A18 correctly attributes the bindings positions (docs/jeiobs/tasks/006-control-protocols.md:274-277)
FINDING codex-14 | Major | accepted-fix | A18 records Codex’s prior A17 acceptance (docs/jeiobs/tasks/006-control-protocols.md:281-283)
VERDICT: approve
```
