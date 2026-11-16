# jeiobs contributor and agent instructions

Read README.md, docs/jeiobs/HANDOFF.md, docs/jeiobs/ROADMAP.md and
 docs/jeiobs/CONTRIBUTING.md first, then upstream CODESTYLE.md and COPYING.
This is an OBS fork, not the JeiOS kernel. Preserve upstream history, notices,
GPL-2.0-or-later licensing, module/source identifiers and public ABI where possible.

## Working agreement

- Ship small, reviewable PRs with one purpose. Branch from main; no force pushes.
- The first coding task is docs/jeiobs/tasks/001-vst3.md. Roadmap numbers express
  priorities and dependencies, not a compulsory serial approval chain.
  Independent experiments may proceed in parallel; unfinished work stays opt-in.
- Inspect real code and pinned dependencies before proposing integration APIs.
- Follow upstream language/style conventions. Apply NASA/JPL-inspired habits
  where useful: bounded work, explicit ownership, checked errors and assertions.
  Do not impose kernel rules or claim whole-codebase compliance.
- New real-time audio code must avoid allocation, blocking locks, filesystem and
  UI work. Add meaningful signal, timing and failure tests for audio changes.
- Never modify an existing OBS installation, profiles or plugin directories.
  Until coexistence is tested, use disposable VMs/accounts, not live OBS data.
- AI-assisted contributions are welcome here. Record tool assistance honestly;
  agent review is not independent human qualification. Upstream's submission
  policy remains applicable to submissions to obsproject, not this fork.
- Match tests to changed behavior. Docs-only work does not require full builds.
  Mark tests as passed, failed, skipped or blocked; never turn a plan into evidence.
- Update HANDOFF when implementation, blockers or next tasks change; preserve
  dated observations in docs/jeiobs/history. Routine typo fixes need no ceremony.
- Ask the owner for product scope, incompatible licensing or destructive decisions;
  ordinary refactors and changes to previously tested code need no extra approval.
- No secrets or personal logs. SDK additions need recorded provenance/license review.
  AAX and ARA2 remain out of scope. Release publishing, signing, installers and
  updaters are owner-only. Merging: the lead agent may merge its own PR only through
  the self-merge gate in docs/jeiobs/DECISIONS.md (peer debate, owner-only classes
  excluded); every other PR is merged by the owner.

## PR evidence

State problem, final change, key design choices, tests actually run, limitations,
compatibility impact and rollback. Record decisions with why / if otherwise / flex
and commit peer transcripts under docs/jeiobs/debates/ (docs/jeiobs/DECISIONS.md).
Keep detail proportional to risk. See docs/jeiobs/CONTRIBUTING.md for risk tiers.

## Expanded implementation direction — 2026-11-15

Follow the expanded ROADMAP tracks for complete snapshot/offline delivery, OBS-compatible
legacy saves, graceful audio recovery, PDC/A/V timing, sidechains, listen bus, media
conversion and recording timecode. Task 002 requires S1-S6, not investigation alone.
Implement shared logic and native Windows/macOS/Linux adapters autonomously in small
PRs. Run available native CI; do not leave avoidable platform stubs for the owner.
Record blocked native/hardware tests without calling cross-compilation runtime proof.
The original VST3-first instruction is historical sequencing: continue its remaining
validation while independent authorized tracks proceed. Full plugin host-spec coverage
requires a versioned capability matrix; do not label partial hosting fully conformant.
