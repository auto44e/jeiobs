# Review run

- Started: 2026-11-17T00:33:11Z
- Reviewed commit: `cd991c9d4d5664106240ab4e7e446fe641274e21` (peers ran in a detached worktree at this commit)
- Author: lead; rounds: 2 (round 1 independent first pass, both at once, not isolated); cross-talk order: claude, codex; timeout per turn: 1500s
- Claude: claude 2.1.325.1736 (ASBX Claude Code, channel stable), model opus, effort max; tools Read, Glob, Grep; no MCP
- Codex: codex-cli 0.169.0 (engine started directly; the toolbox launcher runs only as Bedrock credential helper), effort xhigh; CODEX_HOME config without MCP/plugin/project sections, shell env "core", no project instructions (project_doc_max_bytes=0); --sandbox read-only, sandbox available
- Round 1 is an independent first pass, not isolated (see the script header)
- Script: `tools/jeiobs/peer-review/review.sh`, sha256 `63e7012b6f02f3151f72ee2ad2c1a6d849725472001d706e960af791c4eb99a4`
- Codex home: `~/.config/jeiobs-agent/codex-review`; config.toml sha256 `e3e76dcf155ea4674ca7ba34028a06eedba3e87ed3812d50da13ede1a7efbd44`; skills sha256 `253397a7192e7cf5c8088186e9a468f0696218ee6d62efe83d06c761e627bfeb`
- Context sha256 `ed88552510240e0a985f38f05fabab8014b236c67f92295178058d05b9f284a2`; ledger sha256 `c000a0710a908cc52e498138db62189a6e11cc081d43842331322cc7f5aa2cac`
- stderr logs: kept locally (not committed), `02-run-20261117T003310Z`
- Topic: Review run 02 of the K1 design PR.

## CI state at the reviewed commit

```text
quick checks pending at start
local: check_scaffold.py PASS
```
- Finished: 2026-11-17T00:53:48Z

## Files written by this run

The hash given for `00-meta.md` is of its header (the part before "Finished");
the runner prints the final hash of every file, this one included, when it exits.

- `00-context.md` sha256 `ed88552510240e0a985f38f05fabab8014b236c67f92295178058d05b9f284a2`
- `00-ledger-in.md` sha256 `c000a0710a908cc52e498138db62189a6e11cc081d43842331322cc7f5aa2cac`
- `00-meta.md` sha256 `597da44a3b62146104ea3b6a7113036feb3d693b04ce56c04876042a4684c049`
- `01-r1-claude.prompt.md` sha256 `ba6d91be4fd6a5abafdeaf245b20567b890323f797863c1151adc3cb7a7d93f4`
- `01-r1-claude.reply.md` sha256 `9646a827422fce1af8b4e470693d59229385f36077fd9eccdf80306e216b3914`
- `02-r1-codex.prompt.md` sha256 `ba6d91be4fd6a5abafdeaf245b20567b890323f797863c1151adc3cb7a7d93f4`
- `02-r1-codex.reply.md` sha256 `694801778343cb05fad9357d7c803fdb716456472b6f83cb8119262e85efdc64`
- `03-r2-claude.prompt.md` sha256 `b9c84292641f882e345f5f30fa41d7d8287d6d3f9ae5686a7511c212ef1a826c`
- `03-r2-claude.reply.md` sha256 `42267d5d2305896b31a51475107de64fbd88a3eb4ce98504df88f252e747d9ab`
- `04-r2-codex.prompt.md` sha256 `f24e909288b366546e1b01b128d22a055a44e213486ddae6667d162c65691172`
- `04-r2-codex.reply.md` sha256 `538d52cf8bedb0a025b9a4db38bf7d0f84f12911fff7d79fa25ee97ac868db98`

## Final dispositions

```text
FINDING claude-1 | Major | open | Five if-otherwise/flex bullets exist (006:189-201); still missing who settled each item and A1-A10 (DECISIONS.md:49-50 vs 006:30-31), if-otherwise for 006:112-113 "serials and addresses in portable files" and "rejects every equal-priority writer pair", for A8's reversal (02:153 vs 02:268-280) and PR1 scope (02:178 vs 02:282-284), and flex or "firm" for the rest
FINDING claude-2 | Major | accepted-fix | A1-A2 class hotkeys by effect (unknown means output-control), make profile and collection switching privileged, and route all output kinds through RequestOutput
FINDING claude-3 | Major | accepted-fix | A4 allows filters of public sources through the parent (obs-source.c:487-492 verified); transitions by name match WORKSPACE_FORMAT:414-415; encoder and service registerers (obs-hotkey.h:58-59) should follow A4's by-name rule
FINDING claude-4 | Major | open | Routes still undefined: the only matching rule is "wildcards" (006:70); 02:131 "equality, ranges and prefixes" not carried over; no split, filter or remapped thru (ROADMAP:225); neither decided nor in Open before K1-PR1 (006:203-206)
FINDING claude-5 | Major | open | A8's "the export gate already refuses unreviewed entries" is false: include after review sets fidelity exact/redacted (WORKSPACE_FORMAT:174-175), so serials and peer addresses can be exported (contra :182); fix: the projection always omits the bindings role; ro_compat sub-point dropped (:525-531)
FINDING claude-6 | Minor | open | A7 covers growth, but while the shadow withholds set_volume, user_volume (obs-source.c:4574), the mixer, the saved collection and obs-websocket keep the old value; no flush-on-activate rule
FINDING claude-7 | Minor | accepted-fix | A7 degrades sample_offset to latest unless the consumer is verified; ParameterChanges sizing is listed as open (006:205-206)
FINDING claude-8 | Minor | open | Coalescing Sum/Min/Max "by the same reducer" (006:157) folds one writer's successive absolute values (Sum double-counts, Min/Max latch); keep the newest per writer and target, then reduce
FINDING claude-9 | Minor | accepted-fix | A9 adds the matrix, network-backed ports as remote-net, learn and SysEx thru as meta, and OSCQuery scope; K2 must say how backends without transport info (WinMM) class rtpMIDI ports
FINDING claude-10 | Minor | open | An omitted secrets.json in a listed module folder records its exact size (WORKSPACE_FORMAT:261,298; rounding only at :266-267); the compat feature kind is unstated although :525-531 makes unknown rows invalid
FINDING claude-11 | Major | accepted-fix | A10 decides transforms, banks, layers, modifiers, learn and device identity in outline; toggle and relative-to-absolute already need per-route state, which answers that open item
FINDING claude-12 | Major | open | debates/006-control-protocols has only 01, 02, README (no review-01/, review-02/, findings.md), so ledger sources and A1-A10's correspondence are uncommitted (DECISIONS.md:55-57,187-190); the exemption (:177-181) can't add them, so that head needs run 03
FINDING claude-13 | Major | open | loopback-net gets mix, scene and plugin-param by default (006:123); once a loopback TCP/HTTP listener is enabled (006:22), web pages and OBS browser sources reach it: SLIP resyncs after a no-cors POST's headers (006:16), OSCQuery HTTP is open to DNS rebinding; default only for UDP or Host/Origin-checked transports that close on malformed input
FINDING claude-14 | Major | open | Per-endpoint grants (006:124-125,132) have no stated store: in mappings.json imports widen trust, in captured bindings.json an old snapshot re-grants a revoked peer, and UDP grants rest on a spoofable IP; keep grants machine-local, outside capture and import
FINDING claude-15 | Minor | open | Profile switching is both profile (006:122) and output-control (006:129), and RequestOutput can't carry it (as codex-4 says); A3's egress, plugin-event, listen-bus and transition targets have no class and no deny-if-unclassified rule; SysEx is restricted only on thru (006:176)
FINDING claude-16 | Minor | open | A7 "Balance is a plain field read on the audio thread" (006:163-164) is wrong: obs-source.c:4057-4059 is in process_audio, called from obs_source_output_audio (:4085) on the pushing thread; A5 polls balance (006:147) though audio_balance is a signal (:96, :5735)
VERDICT: request changes
FINDING codex-1 | Major | accepted-fix | A7 assigns balance to the atomic latest-value slot (tasks/006-control-protocols.md:160-167)
FINDING codex-2 | Major | open | A8 conflicts with export rules permitting reviewed opaque entries (tasks/006-control-protocols.md:168-173; WORKSPACE_FORMAT.md:168-177)
FINDING codex-3 | Major | accepted-fix | A5 defines TargetState snapshots, polling, and feedback reads (tasks/006-control-protocols.md:145-150)
FINDING codex-4 | Major | open | A3 lists target kinds without concrete identity/read/write paths, and RequestOutput cannot represent profile or collection switching (tasks/006-control-protocols.md:127-140)
FINDING codex-5 | Major | accepted-fix | A1 adds principals, capability checks, defaults, and hard denies (tasks/006-control-protocols.md:117-126)
FINDING codex-6 | Major | accepted-fix | A6 defines per-endpoint queues, fairness, overflow, reducers, and privileged handling (tasks/006-control-protocols.md:151-159)
FINDING codex-7 | Minor | open | A7 says “Balance is a plain field read on the audio thread,” but callers include capture threads and `audio_balance` is an existing signal (tasks/006-control-protocols.md:160-167; libobs/obs-source.c:96,4085,5735)
FINDING codex-8 | Minor | open | A6 says “Sum, Min and Max reducers coalesce by the same reducer” without defining absolute-vs-delta or per-writer semantics, so successive absolute writes can double-count (tasks/006-control-protocols.md:151-159)
VERDICT: request changes
```
