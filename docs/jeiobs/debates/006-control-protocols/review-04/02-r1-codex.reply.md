A17 is fixed. A18 falsely says Codex did not accept A12, while the committed reply explicitly accepts it (`tasks/...:264-267`; `review-03/04-r2-codex.reply.md:1,6-8`). This misrecords correspondence required by `DECISIONS.md:47-57`.

FINDING codex-1 | Major | accepted-fix | A7 assigns balance to the atomic latest-value slot
FINDING codex-2 | Major | accepted-fix | A12 makes bindings local-only and excluded from exports
FINDING codex-3 | Major | accepted-fix | A5 defines TargetState snapshots, polling, and feedback reads
FINDING codex-4 | Major | accepted-fix | A15 defines target identities, paths, classes, and profile switching
FINDING codex-5 | Major | accepted-fix | A1 adds principals, capability checks, defaults, and hard denies
FINDING codex-6 | Major | accepted-fix | A6 defines queue isolation, fairness, overflow, reducers, and privileged handling
FINDING codex-7 | Minor | accepted-fix | A17 correctly distinguishes balance reads from setter writes
FINDING codex-8 | Minor | accepted-fix | A16 defines per-writer absolute retention and relative-delta accumulation
FINDING codex-9 | Major | open | A18 falsely says “Codex has not accepted A12-A14”; Codex explicitly accepted A12
VERDICT: request changes