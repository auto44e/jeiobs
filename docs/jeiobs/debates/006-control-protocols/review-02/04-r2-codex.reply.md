FINDING codex-1 | Major | accepted-fix | A7 assigns balance to the atomic latest-value slot (tasks/006-control-protocols.md:160-167)
FINDING codex-2 | Major | open | A8 conflicts with export rules permitting reviewed opaque entries (tasks/006-control-protocols.md:168-173; WORKSPACE_FORMAT.md:168-177)
FINDING codex-3 | Major | accepted-fix | A5 defines TargetState snapshots, polling, and feedback reads (tasks/006-control-protocols.md:145-150)
FINDING codex-4 | Major | open | A3 lists target kinds without concrete identity/read/write paths, and RequestOutput cannot represent profile or collection switching (tasks/006-control-protocols.md:127-140)
FINDING codex-5 | Major | accepted-fix | A1 adds principals, capability checks, defaults, and hard denies (tasks/006-control-protocols.md:117-126)
FINDING codex-6 | Major | accepted-fix | A6 defines per-endpoint queues, fairness, overflow, reducers, and privileged handling (tasks/006-control-protocols.md:151-159)
FINDING codex-7 | Minor | open | A7 says “Balance is a plain field read on the audio thread,” but callers include capture threads and `audio_balance` is an existing signal (tasks/006-control-protocols.md:160-167; libobs/obs-source.c:96,4085,5735)
FINDING codex-8 | Minor | open | A6 says “Sum, Min and Max reducers coalesce by the same reducer” without defining absolute-vs-delta or per-writer semantics, so successive absolute writes can double-count (tasks/006-control-protocols.md:151-159)
VERDICT: request changes