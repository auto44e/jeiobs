# Whole-workspace snapshots and offline configuration

Status: committed full implementation scope; runtime implementation pending. Owner request: 2026-11-15.
Planning baseline: 5e17d4f99b2c15756b2ff5bbeb2e4454b80feb8a (main).
This extends M1 persistence and connects to M2 master processing and M3 hosting.
Continue remaining VST3 validation alongside this independent implementation track.

## User outcomes

Save a complete recoverable workspace instead of exporting scenes individually.
Open a Windows workspace on a Mac without the original RØDECaster, virtual audio
devices or capture hardware; suggest VST3/CLAP effects for the master bus, save
the edits, and apply them back on Windows without losing the original setup.
Matching plugin names alone must not imply cross-platform state compatibility.

## Saving and restoring

Provide Save Snapshot, Restore Snapshot and Export Portable Workspace.
A snapshot captures a consistent revision of all scene collections, profiles,
application settings/layouts, source/filter and plugin settings, serialized plugin
state, master/FX chains, routing and device bindings. Record selected profile and
collection as well as saved alternatives. Capture a coherent revision, not files
copied while independent writers are changing them. Never serialize on audio threads.

Keep named versions with timestamps, parent revision and content hashes. Write
atomically through a validated staging area; failed writes preserve the previous
snapshot. Restore offers a preview, missing-dependency report, pre-apply restore
point and rollback. Do not silently overwrite a newer working revision.
Save/restore is scoped to jeiobs, never the user's OBS installation.

Media is referenced by default. Optional portable export can include explicitly
selected assets with a size preview; omit large media by default. Record missing
or excluded assets and support relinking without rewriting the originating machine's
paths. Do not bundle plugin executables, installers, license activations or credentials.
Exclude stream keys, tokens, passwords and authenticated URLs from exports and
snapshots. Keep machine-local credential references for reauthorization on restore.

Unknown third-party state can contain secrets or private paths: treat opaque blobs
as unreviewed, not automatically sanitized. Portable export must disclose this,
allow omission, and block sensitive/unknown payloads by default until reviewed.
Preserve omitted-state placeholders and report resulting restore limitations.

## Offline editor

Open an inert document model: no capture devices, streaming, recording, browser
sources, network sources, scripts, plugin scans or plugin execution on open.
Unavailable hardware, source types and plugins remain editable placeholders.
Preserve unknown fields and opaque state without substitution, deletion or automatic
normalization. A read-only fallback is required for unsupported future schemas.

Keep intended target OS, architecture, sample rate, channel layout and available
plugin inventory separate from editor-machine capabilities. Preserve target device
IDs and routing; local audition mappings are temporary overlays, not automatic
Windows-to-Mac rewrites. Remapping on the destination requires user selection.

Allow adding/removing/reordering/bypassing master effects and editing supported
parameters offline. Identify a plugin by format and its stable format-specific ID,
plus vendor/version metadata; binary paths are local resolution hints only.
Store plugin-native serialized state and stable parameter IDs where supported.
Do not convert VST3 to CLAP/AU state or assume equal IDs across formats. Preserve
the original state before any version migration; flag incompatible revisions,
architectures, channel layouts and unsupported state with actionable diagnostics.
When no parameter metadata exists, preserve the blob and offer placement/bypass
editing rather than inventing parameter controls.

Optional local plugin-editor/audition mode is a separate explicit action, using
the existing host's safety model and isolated scanning. It must disclose that it
executes native code and may need licensing/network access. Pure offline editing
must remain usable without loading any plugin.

## Sharing edits and applying them

Export either a full portable workspace or a proposed change set, with a base
workspace ID/revision/hash and stable object IDs. Show the recipient a readable
diff, including effect order, parameters and any target-routing changes.
Detect concurrent edits and require explicit resolution; never last-writer-wins
over an unrelated live configuration. Preserve untouched hardware/profile settings.
Validate dependencies, create a restore point, stage and commit all affected
configuration together. A failed apply must leave the old revision usable.
Do not apply topology/state changes while streaming/recording; require a safe
stop boundary in the first implementation. Rollback must survive process restart.

## Implementation architecture (interfaces to finalize)

A versioned Portable Workspace Format and Offline Configuration Engine are required deliverables.
Choose the container/extension and finalize the schema after inspecting actual
persistence paths; no format or API is frozen by this planning PR.
Suggested contents: manifest, profiles/collections/settings, logical routing,
plugin inventory/state, optional assets and integrity metadata.
Separate portable intent, machine bindings, credentials references and audition
overlays. Integrity hashes detect corruption; they are not signatures or proof of trust.

Require bounded archive/file counts, total expanded sizes and state sizes, safe
relative paths, duplicate-entry rejection and no traversal, symlinks or automatic
fetch/execution on import. Define schema upgrades and forward-compatibility rules.
A plugin scanner process is not isolation for runtime DSP.

## Bounded implementation tasks

| Task | Deliverable | Completion evidence |
| --- | --- | --- |
| S1 | Inventory actual persistence owners and propose versioned manifest/document model | Synthetic fixtures cover each saved domain; specify consistency/secret boundaries |
| S2 | Local snapshot writer, history and transactional restore | Interrupted write/restore, disk-full, corruption and rollback tests; original OBS untouched |
| S3 | Portable export/import, selected assets and dependency report | Missing assets, secret omission, relinking and malicious archive tests |
| S4 | Inert offline editor and placeholder round-trip | All directed Windows/macOS/Linux pairs round-trip without original devices/plugins; no hardware/network/plugin activation |
| S5 | Plugin identity/state and master-chain editing | VST3/CLAP state/version/channel cases, absent plugins, unchanged opaque blobs; integrate only available hosts |
| S6 | Reviewable change sets and conflict-aware apply | Divergent recipient revisions, partial failure and restart-safe rollback; untouched routing preserved |

S1-S4 can progress independently of runtime master-bus support using synthetic
documents. S5's actual master-bus execution depends on M2; VST3/CLAP runtime
coverage depends on their respective hosts. Saving a proposed chain is not proof
that it executes. Use disposable accounts/VMs until M0 coexistence is qualified.
Keep PRs small and use risk-based checks, not mandatory full-milestone sign-offs.

## End-to-end acceptance

1. Build a synthetic Windows workspace with RØDECaster-like bindings, additional
   virtual/physical devices, multiple profiles/collections, sources and FX.
2. Save, export and open on Mac in pure offline mode with unavailable hardware.
   Verify all original bindings, unknown source fields and unedited state survive.
3. Add a proposed VST3/CLAP master chain. Save and return a readable change set.
4. With compatible test plugins installed on Windows, preview and apply after
   validation; verify order/state, unchanged device routing and successful rollback.
5. Repeat with missing plugins, mismatched versions/format/architecture, a concurrent
   recipient edit, missing media, future schema, corrupted archive and interrupted
   restore. Report gaps; do not silently discard state or claim universal portability.

This PR contains documentation only. No snapshot, offline editor, schema, plugin
compatibility or restore safety has been implemented or runtime-tested.

## Full delivery requirement

Implement S1-S6 end to end, including user-facing save/restore/export/offline-edit/apply
flows, native platform adapters, automated tests and supported-platform qualification.
S1 is the starting slice, not the completion criterion. Pure offline editing must
work in every direction between Windows, macOS and Linux and on the same OS.
Use the cross-platform delivery and legacy-config compatibility requirements in
[ROADMAP.md](ROADMAP.md). Native plugin execution remains platform-dependent.
This documentation update does not itself implement these features.
