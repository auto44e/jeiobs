# Review run

- Started: 2026-11-16T11:56:07Z
- Reviewed commit: `4f5738264c231c4faa1f1032b4042fd03dbd8d36` (peers ran in a detached worktree at this commit)
- Author: codex; rounds: 2 (round 1 independent first pass, both at once, not isolated); cross-talk order: claude, codex; timeout per turn: 1800s
- Claude: claude 2.1.325.1734 (ASBX Claude Code, channel stable), model opus, effort max; tools Read, Glob, Grep; no MCP
- Codex: codex-cli 0.169.0 (engine started directly; the toolbox launcher runs only as Bedrock credential helper), effort xhigh; CODEX_HOME config without MCP/plugin/project sections, shell env "core", no project instructions (project_doc_max_bytes=0); --sandbox read-only, sandbox UNAVAILABLE on this host (bubblewrap cannot create user namespaces); Codex commands ran unsandboxed, tripwires only
- Round 1 is an independent first pass, not isolated (see the script header)
- Script: `tools/jeiobs/peer-review/review.sh`, sha256 `63e7012b6f02f3151f72ee2ad2c1a6d849725472001d706e960af791c4eb99a4`
- Codex home: `~/.config/jeiobs-agent/codex-review-c`; config.toml sha256 `e3e76dcf155ea4674ca7ba34028a06eedba3e87ed3812d50da13ede1a7efbd44`; skills sha256 `253397a7192e7cf5c8088186e9a468f0696218ee6d62efe83d06c761e627bfeb`
- Context sha256 `86b19a70a1e6049423491d8583cafcd9ddf98039c5057de4027cbac1e964949b`; ledger sha256 `3d8397962078279f1ceee8a580d0f5a4638a5e5895655395d66e17771f06e919`
- stderr logs: kept locally (not committed), `02-run-20261116T115607Z`
- Topic: Review run 02 of PR #21 (S1-PR1).

## CI state at the reviewed commit

```text
quick checks for 4f5738264: pushed at review start
local: jw-unit PASS, jw-cases 509/509 (Linux)
```
- Finished: 2026-11-16T12:14:13Z

## Files written by this run

The hash given for `00-meta.md` is of its header (the part before "Finished");
the runner prints the final hash of every file, this one included, when it exits.

- `00-context.md` sha256 `86b19a70a1e6049423491d8583cafcd9ddf98039c5057de4027cbac1e964949b`
- `00-ledger-in.md` sha256 `3d8397962078279f1ceee8a580d0f5a4638a5e5895655395d66e17771f06e919`
- `00-meta.md` sha256 `ec8ccfb89c3322a839db16ceb7bb431168cb1d9613d8d22fb9443a363401cde7`
- `01-r1-claude.prompt.md` sha256 `32b55a858350e8537e5569c90d9cc7898ad437c41541195e413e6437f86f5262`
- `01-r1-claude.reply.md` sha256 `b5179d1fc0cc9cd4d4bea9123aae7766181d1d43e5c1b4ee88fb0b07274bb4cb`
- `02-r1-codex.prompt.md` sha256 `32b55a858350e8537e5569c90d9cc7898ad437c41541195e413e6437f86f5262`
- `02-r1-codex.reply.md` sha256 `0725373805c915301c47b8c33b2549217e2a047475c44615285a44b4760d05c4`
- `03-r2-claude.prompt.md` sha256 `e55d7ed1d6473d5990c1a4451a459d1cd2a7e0339f61e95806775c4e1fbeba6e`
- `03-r2-claude.reply.md` sha256 `137e9398f47672dae4184d3fd957358ccd4bc7fcee587e60932c1f47fd34812e`
- `04-r2-codex.prompt.md` sha256 `0b72bd6159ce41dddf9564ce11cbbe18a02f18ef41a0d6f3fd3aec7d120ad121`
- `04-r2-codex.reply.md` sha256 `105083ca9afcbe4270606fac38666a7f0227162f821994968cbb7a58573e1d02`

## Final dispositions

```text
FINDING claude-1 | Minor | open | §8:362-363 keeps auth on only for S2 though S3 import writes roots (:528-529), so the exporter's known placeholder could go live; §5:239 lacks "a new §7 row needs an incompat feature"
FINDING claude-2 | Major | accepted-fix | unreviewed/omitted-field/now checks and catch-alls (Manifest.cpp:645-675,1053-1080,1169,1355)
FINDING claude-3 | Major | accepted-fix | ffmpeg_source input/ffmpeg_options and vlc playlist value added (WORKSPACE_FORMAT.md:346,390-392)
FINDING claude-4 | Minor | open | app_keys with missing/unknown restore exported and pass --export (Manifest.cpp:750-752,1278-1280); input dropped_ext copied unchecked (:1292); known values never type-checked, so nested data under creator/active/target.audio/app_keys keys survives rebuild and --export (:783-799,1243-1287)
FINDING claude-5 | Minor | accepted-fix | recorded as a PR3 open item (WORKSPACE_FORMAT.md:641-647)
FINDING claude-6 | Minor | open | export parses (jw-check.cpp:194) and hashes (:209) in two reads; other parts fixed
FINDING claude-7 | Minor | accepted-fix | 8-byte scan minimum and rounded unparseable sizes (WORKSPACE_FORMAT.md:264-272)
FINDING claude-8 | Minor | open | spec not updated: §12:547-549 device rule lacks the trailing-space strip; §15:632-636 out-of-date wmain and CON notes; §7:287 secret column
FINDING claude-9 | Minor | open | §8 url_credential misses user.ini [BasicWindow] ExtraBrowserDocks dock URLs (OBSBasic_Browser.cpp:73-90)
VERDICT: approve
FINDING codex-1 | Major | accepted-fix | Opaque metadata is validated before export dereference.
FINDING codex-2 | Major | accepted-fix | Decisions use bounded reads and SAX parsing.
FINDING codex-3 | Major | accepted-fix | Device-name checks trim spaces before extensions.
FINDING codex-4 | Minor | accepted-fix | Errors emit exactly one stdout ERR line.
FINDING codex-5 | Minor | accepted-fix | Child output is revalidated before printing.
FINDING codex-6 | Minor | open | `audio:{}` or partial/incorrectly typed audio passes export validation despite the exact target shape requirement (Manifest.cpp:739-747,1263-1272; 09-pr1-contract-r2.md:45-46).
VERDICT: approve
```
