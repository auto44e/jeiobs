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

## Progress

- **S1-PR1** (2026-11-16): [WORKSPACE_FORMAT](../WORKSPACE_FORMAT.md) (format v1 spec),
  [s1-persistence-audit](../s1-persistence-audit.md), and `shared/jeiobs-workspace/` (C++17, no
  Qt, no libobs runtime): limits, path rules, BLAKE2b-256, bounded manifest parser, local and
  export schema validation, compatibility flags, child carry-over, export projection, and the
  `jw-check` CLI. Codex worker wrote the library and Claude worker the 328 black-box cases from
  the spec without reading the library; the lead fixed four contract mismatches those cases found.
  After review run 01: contract addendum r2 (allowlist-bound stored entries, consistent entry
  fields, bounded decisions and revision reads, `CON .txt`, derived fidelity, rebuilt export
  objects), fixed by the Codex worker with cases by the Claude worker (509 in total), and
  `jw-check` takes UTF-8 arguments on Windows.
  Linux (Ubuntu 24.04, GCC, CMake 3.28, `-Wall -Wextra -Werror`): `jw-unit` and `jw-cases`
  (509/509) pass. Windows and macOS not compiled: the 3-OS CI workflow is a separate,
  owner-merged PR. Correspondence: [debates/002-workspace-format](../debates/002-workspace-format/).
- Next: S1-PR2 (JSON scanner with byte positions, INI reader, redactor), then PR3 and PR4 (PR4
  adds `config_lock`, which the owner approved: WORKSPACE_FORMAT section 14). Also owed: an
  owner-merged PR that pins the Unicode Character Database and adds the generated collision
  tables (decision 3), and a purge for local history in or before S2 (decision 2).
