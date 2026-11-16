# Task 002 — implement complete workspace saving and offline editing

Status: authorized implementation scope; S1-S6 pending.
Read [WORKSPACE_SAVING](../WORKSPACE_SAVING.md), [ROADMAP](../ROADMAP.md) and AGENTS.md.
Deliver working snapshot save/history/restore, portable export/import, cross-platform
offline editing, plugin-state preservation and conflict-aware change-set apply.
Investigation alone does not complete this task.

1. S1: audit actual persistence writers and define versioned schema, secret boundaries,
   coherent capture and inert document model; add synthetic round-trip fixtures.
2. S2: implement atomic snapshots/history and transactional restore/rollback.
3. S3: implement portable packaging, selected assets, validation and relinking.
4. S4: implement offline UI without source/device/plugin activation.
5. S5: implement plugin identity/state and master-chain editing; preserve unavailable
   capabilities until runtime support exists, without false execution claims.
6. S6: implement reviewable diffs, conflict resolution, restore points and atomic apply.

Use separate reviewable PRs and continue through all six slices. Shared code and
native adapters must cover Windows/macOS/Linux; test all six directed OS pairs plus
same-OS save/edit/restore. Native plugin execution is a separate capability.
Test disk-full, corruption, malicious input, interrupted writes/restores, unknown
fields, missing assets/devices/plugins, concurrent edits and restart-safe rollback.
Integrate legacy OBS-compatible-save tests separately from snapshot serialization.
No live OBS writes or required owner hardware for synthetic development.
Record exact native CI results and remaining hardware/UI/licensed-plugin tests.
