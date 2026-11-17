# Review run

- Started: 2026-11-17T01:06:53Z
- Reviewed commit: `064f6c543ba833ceb012250626a5be43ba3e3c07` (peers ran in a detached worktree at this commit)
- Author: lead; rounds: 2 (round 1 independent first pass, both at once, not isolated); cross-talk order: claude, codex; timeout per turn: 1500s
- Claude: claude 2.1.326.1734 (ASBX Claude Code, channel stable), model opus, effort max; tools Read, Glob, Grep; no MCP
- Codex: codex-cli 0.169.0 (engine started directly; the toolbox launcher runs only as Bedrock credential helper), effort xhigh; CODEX_HOME config without MCP/plugin/project sections, shell env "core", no project instructions (project_doc_max_bytes=0); --sandbox read-only, sandbox available
- Round 1 is an independent first pass, not isolated (see the script header)
- Script: `tools/jeiobs/peer-review/review.sh`, sha256 `63e7012b6f02f3151f72ee2ad2c1a6d849725472001d706e960af791c4eb99a4`
- Codex home: `~/.config/jeiobs-agent/codex-review`; config.toml sha256 `e3e76dcf155ea4674ca7ba34028a06eedba3e87ed3812d50da13ede1a7efbd44`; skills sha256 `253397a7192e7cf5c8088186e9a468f0696218ee6d62efe83d06c761e627bfeb`
- Context sha256 `ed98a1591191339525d65500b6ff1a992e1c9f49b8e06a8c46619e81768f5805`; ledger sha256 `aca55f5069aead64458dce61cbd3e281f45afdc979fbba836e154f23173db815`
- stderr logs: kept locally (not committed), `03-run-20261117T010653Z`
- Topic: Review run 03 of the K1 design PR.

## CI state at the reviewed commit

```text
quick checks pending at start
local: check_scaffold.py PASS
```
- Finished: 2026-11-17T01:23:21Z

## Files written by this run

The hash given for `00-meta.md` is of its header (the part before "Finished");
the runner prints the final hash of every file, this one included, when it exits.

- `00-context.md` sha256 `ed98a1591191339525d65500b6ff1a992e1c9f49b8e06a8c46619e81768f5805`
- `00-ledger-in.md` sha256 `aca55f5069aead64458dce61cbd3e281f45afdc979fbba836e154f23173db815`
- `00-meta.md` sha256 `f4403806b0c3d7a07fc84ac5b5d49e16948f52b7683558aed93a769412036b0c`
- `01-r1-claude.prompt.md` sha256 `4cdc96f200b8dcce708176b53dd86770a03fea1a939c787ead77f71c7f8c0404`
- `01-r1-claude.reply.md` sha256 `995293ab9e7b854d03f75f81b7d8d8e338050f9906e25c3c6c163f84fe2030c6`
- `02-r1-codex.prompt.md` sha256 `4cdc96f200b8dcce708176b53dd86770a03fea1a939c787ead77f71c7f8c0404`
- `02-r1-codex.reply.md` sha256 `eff8c2d7003e91d17a751fcc5aadfa2aba52787240ba2250cfdba81a12ee96c6`
- `03-r2-claude.prompt.md` sha256 `8590239fab6461d69e25263f7d864523c8577911b7ab25acfbc61b0789ab7a07`
- `03-r2-claude.reply.md` sha256 `6f32da0708c10b675fa208e96d6281736a5bda5c63350088e58bfadffcd633e1`
- `04-r2-codex.prompt.md` sha256 `93348c7dbd7f48e7fa14e9a6b6ebb1f90fd57286170199a2af72f0568d64b891`
- `04-r2-codex.reply.md` sha256 `066a02f0f2b6f80ee4825163c1307c453a34f4776adfa724423e584ae4b2d3d4`

## Final dispositions

```text
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
FINDING codex-1 | Major | accepted-fix | A7 assigns balance to the atomic latest-value slot (tasks/006-control-protocols.md:160-167)
FINDING codex-2 | Major | accepted-fix | A12 makes bindings local-only and always omitted from exports (tasks/006-control-protocols.md:200-207)
FINDING codex-3 | Major | accepted-fix | A5 defines TargetState snapshots, polling, and feedback reads (tasks/006-control-protocols.md:145-150)
FINDING codex-4 | Major | accepted-fix | A15 defines target identities, paths, classes, and RequestProfileSwitch (tasks/006-control-protocols.md:220-238)
FINDING codex-5 | Major | accepted-fix | A1 adds principals, capability checks, defaults, and hard denies (tasks/006-control-protocols.md:117-126)
FINDING codex-6 | Major | accepted-fix | A6 defines endpoint queues, fairness, overflow, reducers, and privileged handling (tasks/006-control-protocols.md:151-159)
FINDING codex-7 | Minor | open | A17 misstates balance as written by process_audio; the source reads it at :4057-4059 and writes it in the setter at :5735-5737 (tasks/006-control-protocols.md:243-245; libobs/obs-source.c:4057-4059,5735-5737)
FINDING codex-8 | Minor | accepted-fix | A16 defines newest-per-writer absolute retention and separate relative-delta accumulation (tasks/006-control-protocols.md:240-242)
VERDICT: request changes
```
