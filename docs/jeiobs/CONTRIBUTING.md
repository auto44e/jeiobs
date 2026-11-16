# Agile contribution workflow

Owner direction, 2026-11-15: use the JeiOS approach with less strict gating.
Keep honest evidence, current handoffs, useful history and approachable onboarding.
Drop mandatory milestone sign-offs, fixed reviewer counts and verbose PR essays.
This document governs jeiobs contributions; upstream CONTRIBUTING.md is retained
for historical and upstream submission guidance. AI-assisted work is welcome here.

## Loop

1. Pick a bounded task from the backlog. First: upstream VST3 backport.
2. Create a branch from main, inspect the code, and open a small PR early.
3. Implement and test the changed behavior. Record genuine blockers plainly.
4. Update the handoff when status changes. The owner reviews and merges. The lead
   agent may merge its own PR only through the self-merge gate in
   [DECISIONS](DECISIONS.md), which requires both peer lineages.

Use Backlog → Ready → In progress → Review → Done; use Blocked with a reason and
next unblock action. Limit each contributor to one active implementation task.
A project board is helpful but never required to start a task. Issues and linked
PRs are the source of truth. Keep roadmap IDs in titles for easy filtering.

| Change risk | Expected evidence |
| --- | --- |
| Docs, templates, small maintenance | Diff/links or relevant script checks; no full platform matrix |
| UI, build, ordinary feature | Affected build plus focused behavior tests; note platform gaps |
| Real-time audio, persistence, installer, updater, plugin execution | Targeted regression/failure tests, compatibility impact and rollback; affected platform smoke tests before support/release claims |

Known unrelated failures may be tracked with a linked issue and maintainer
acceptance; do not hide failures or weaken security checks to get a green badge.
Use short design notes for ABI, persistent data format, process isolation or
major audio topology choices. Ordinary implementation choices belong in the PR.

Code may land as an opt-in experiment before every platform is qualified.
However, do not distribute an installer as coexistence-safe before the relevant
install/update/uninstall tests pass. Release publishing, signing and installers
are owner-only.
No branch-protection rules are imposed. The only required review is the self-merge
gate's two-lineage peer debate, and it applies to the lead agent's own PRs only.

Help is available from plain-language, fully guided contributions through expert
mode. Adjust coaching to the contributor; keep the same evidence requirements.
Peer agents may own separate bounded tasks; record actual authorship and review,
without presenting AI review as human review. No vote overrides observed failures.

Every PR should explain why the change helps, important tradeoffs, tests and gaps,
user-visible compatibility effects, and how to revert it. Keep routine PRs short.
