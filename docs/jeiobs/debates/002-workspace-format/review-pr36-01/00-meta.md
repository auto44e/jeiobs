# Review run

- Started: 2026-11-17T15:54:18Z
- Reviewed commit: `804e22766a42192f2d5e8a29185504080fc8e6ae` (peers ran in a detached worktree at this commit)
- Author: lead; rounds: 2 (round 1 independent first pass, both at once, not isolated); cross-talk order: claude, codex; timeout per turn: 1800s
- Claude: claude 2.1.326.1734 (ASBX Claude Code, channel stable), model opus, effort max; tools Read, Glob, Grep; no MCP
- Codex: codex-cli 0.169.0 (engine started directly; the toolbox launcher runs only as Bedrock credential helper), effort xhigh; CODEX_HOME config without MCP/plugin/project sections, shell env "core", no project instructions (project_doc_max_bytes=0); --sandbox read-only, sandbox available
- Round 1 is an independent first pass, not isolated (see the script header)
- Script: `tools/jeiobs/peer-review/review.sh`, sha256 `63e7012b6f02f3151f72ee2ad2c1a6d849725472001d706e960af791c4eb99a4`
- Codex home: `~/.config/jeiobs-agent/codex-review`; config.toml sha256 `e3e76dcf155ea4674ca7ba34028a06eedba3e87ed3812d50da13ede1a7efbd44`; skills sha256 `253397a7192e7cf5c8088186e9a468f0696218ee6d62efe83d06c761e627bfeb`
- Context sha256 `8034569acaf077d2d4864b9671fc017a87ec45d753dd4ddfe4fef4f3d308d099`; ledger sha256 `7479c8afd3c35d17a64b2756c36394d8bc8df1bac1ae96883ce8b23e04913525`
- stderr logs: kept locally (not committed), `01-run-20261117T155418Z`
- Topic: Review run 01 of pr36.

## CI state at the reviewed commit

```text
CI: not yet reported at review start (run on PR #36)
```
- Finished: 2026-11-17T16:16:29Z

## Files written by this run

The hash given for `00-meta.md` is of its header (the part before "Finished");
the runner prints the final hash of every file, this one included, when it exits.

- `00-context.md` sha256 `8034569acaf077d2d4864b9671fc017a87ec45d753dd4ddfe4fef4f3d308d099`
- `00-ledger-in.md` sha256 `7479c8afd3c35d17a64b2756c36394d8bc8df1bac1ae96883ce8b23e04913525`
- `00-meta.md` sha256 `5da5fa3792569737f6dd8083a472d63ea6401fd2e20f1c042724595ec4e2b52b`
- `01-r1-claude.prompt.md` sha256 `bfb9d928416dac2a556b5db8ae7da0f7ce65527bcca4b4737e81382d56f592b4`
- `01-r1-claude.reply.md` sha256 `f9ac06ffd11a5fa35e42955a8f455933f7c08037f4fb2bdc2dd72c227ed03f1d`
- `02-r1-codex.prompt.md` sha256 `bfb9d928416dac2a556b5db8ae7da0f7ce65527bcca4b4737e81382d56f592b4`
- `02-r1-codex.reply.md` sha256 `fa77f0acaf241312eb860db9f73a699ddfa390e309c9caa03d45e147e6ad6154`
- `03-r2-claude.prompt.md` sha256 `ab875e86eb9e3a2ca4465399ce63f2246627a6e83827fddd94d910d47ee0a1d6`
- `03-r2-claude.reply.md` sha256 `c31e3b829be2c74f034dd59b0c09cea7c9cf2ab147fc1d2c69664c37c5c09fad`
- `04-r2-codex.prompt.md` sha256 `3659d2313f784b9ccd825c3f3508a2a5750052eee5273a8630f53939fe30287f`
- `04-r2-codex.reply.md` sha256 `32ce070bec87759fecac752bbe49a8de28eb14b11834481aba1b950ec4fbf034`

## Final dispositions

```text
FINDING claude-1 | Minor | open | CI does not regenerate UnicodeTables.inc and cmp it (README.md:18 unenforced; jeiobs-workspace.yaml:37-56); ccc rows only partly covered by jw-unicode-conformance (same gap as codex-2)
FINDING claude-2 | Minor | open | Version-dependent E_COLLISION (WORKSPACE_FORMAT:581-583) plus section 11 recovery (:511-521) can quarantine a valid stored revision after a Unicode pin bump; reader rule not stated
FINDING claude-3 | Minor | open | WORKSPACE_FORMAT:575 says "one manifest" but the rule is per root; section 15 (:714-718) omits normalization-only clashes and the case-collision code covering them
FINDING claude-4 | Minor | open | Unicode License v3 needs the notice with copies or in associated documentation (LICENSE:19-22); nothing records this for binaries once PR4 links the library (NOTICE.md:33-38)
VERDICT: approve
FINDING codex-1 | Minor | withdrawn | The task says the old `OK unchecked` output and flag “are gone,” matching `docs/jeiobs/tasks/002-workspace-saving.md:63-65` and `shared/jeiobs-workspace/src/jw-check.cpp:123-125`.
FINDING codex-2 | Minor | open | Unlisted-codepoint checks use one scalar at a time, so they cannot detect incorrect combining-class ordering (`shared/jeiobs-workspace/tests/jw_unicode_conformance.cpp:152-179`).
VERDICT: approve
```
