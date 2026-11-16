# PR #18 review run 08 — head 75c0ea435

Your working directory is a checkout of this commit. Since run 07 (reviewed d9f946609): `git diff d9f946609 HEAD`. The open findings from docs/jeiobs/debates/018-decision-records/07-run/03-r2-claude.reply.md and 04-r2-codex.reply.md are answered in findings.md (lead's note column).

Key point for N2/codex-6/claude-3: true confinement is not available on this host until the owner enables bubblewrap (verified: bwrap --ro-bind / / true -> 'setting up uid map: Permission denied'; kernel.apparmor_restrict_unprivileged_userns=1). The change makes the claims exact and enforces what can be enforced (engine binary, no MCP, core env, write-tamper check). If you hold that a self-merge is unsafe until bubblewrap works, keep the finding open as Major and say so explicitly: that question then goes to the owner (gate item 4).

Owner instruction (verbatim): "you all are clear to self merge #18 and onwards after fixing an error in one of the commits and after peer review".

Severity: Blocker = must not merge; Major = must be fixed before a self-merge; Minor = follow-up.
