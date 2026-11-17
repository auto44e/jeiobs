A12, A15, and A16 resolve codex-2, codex-4, and codex-8. codex-7 remains open: A17 says `process_audio` writes balance, but source reads at `libobs/obs-source.c:4057-4059` and writes in the setter at `:5735-5737`.

I agree Claude’s `claude-14` remains Major: A13’s UDP prohibition conflicts with its Network MIDI 2.0 exception (`tasks/006-control-protocols.md:208-213`; `ROADMAP.md:245`). `claude-1` also remains open because A18 lacks per-decision attribution/flex details (`006-control-protocols.md:250-283`; `DECISIONS.md:47-54`).

FINDING codex-1 | Major | accepted-fix | A7 assigns balance to the atomic latest-value slot (tasks/006-control-protocols.md:160-167)
FINDING codex-2 | Major | accepted-fix | A12 makes bindings local-only and always omitted from exports (tasks/006-control-protocols.md:200-207)
FINDING codex-3 | Major | accepted-fix | A5 defines TargetState snapshots, polling, and feedback reads (tasks/006-control-protocols.md:145-150)
FINDING codex-4 | Major | accepted-fix | A15 defines target identities, paths, classes, and RequestProfileSwitch (tasks/006-control-protocols.md:220-238)
FINDING codex-5 | Major | accepted-fix | A1 adds principals, capability checks, defaults, and hard denies (tasks/006-control-protocols.md:117-126)
FINDING codex-6 | Major | accepted-fix | A6 defines endpoint queues, fairness, overflow, reducers, and privileged handling (tasks/006-control-protocols.md:151-159)
FINDING codex-7 | Minor | open | A17 misstates balance as written by process_audio; the source reads it at :4057-4059 and writes it in the setter at :5735-5737 (tasks/006-control-protocols.md:243-245; libobs/obs-source.c:4057-4059,5735-5737)
FINDING codex-8 | Minor | accepted-fix | A16 defines newest-per-writer absolute retention and separate relative-delta accumulation (tasks/006-control-protocols.md:240-242)
VERDICT: request changes