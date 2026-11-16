# jeiobs implementation roadmap

Owner direction: 2026-11-15 (America/New_York). Baseline inspected:
`2dbad5000837e1f18f1dc92a3efc2538f2cb6355`. This roadmap commits scope for implementation; it does not
declare unbuilt features complete. Milestones group outcomes, not serial approval gates.

## Current evidence and priorities

The current handoff records VST3 merged, Linux enabled/disabled builds, runtime
checks and scanner robustness work. These are recorded historical results, not
tests rerun for this documentation change. macOS/Windows qualification remains
pending. See [HANDOFF](HANDOFF.md) and task 001 for exact evidence and limitations.
Continue VST3 fixes/platform completion while independent saving and timing work
proceeds. Snapshot saving is a full deliverable, not an investigation-only proposal.

| Track | Implementation slices | Completion outcome |
| --- | --- | --- |
| M0 Coexistence/platform foundation | Identity, storage, packaging, device registrations, platform CI | Independent install/update/uninstall; OBS untouched |
| M1 Persistence and compatibility | S1-S6 snapshots/offline editor; C1-C3 legacy saves | Complete save/restore/edit/share UI on Windows/macOS/Linux; OBS-compatible legacy documents |
| M2 Audio engine | A1 contracts, A2 master FX, A3 recovery | Bounded processing, explicit failure behavior, working master chain |
| M3 Plugin formats | P1 conformance inventory, P2 VST3, P3 CLAP, P4 AU, P5 validation | Full adopted host-spec coverage with feature-level evidence |
| M4 Routing/timing/monitoring | R1 graph/sidechains, R2 PDC, R3 listen bus, R4 media clocks | Aligned outputs and configurable independent monitoring |
| M5 Recording/I/O/release | T1-T3 timecode, native I/O, packaging qualification | Recoverable recordings with optional verified timecode and tested platform support |

## M0 — coexistence and native platforms

Keep independent executable/bundle/package IDs, installation/config/profile/log/
cache/plugin/update paths. Audit IPC, WebSocket defaults, virtual camera, crash
reporting, protocol handlers, shortcuts, uninstallers and DLL/library resolution.
Do not replace OBS system components; optional system-wide components need explicit
installation consent. Preserve upstream public symbols/module/source IDs.
Complete source audit, implement each platform adapter, then test install, launch,
save, update and uninstall in both orders with hashed OBS sentinel data.
Shared hardware contention must be reported, not mistaken for filesystem isolation.
Retain GPL notices, fork branding and fork-only publishing/signing infrastructure.

## M1 — full saving, snapshots and cross-platform offline editing

Implement all S1-S6 in [WORKSPACE_SAVING](WORKSPACE_SAVING.md), through production UI,
not only schema/design. [Task 002](tasks/002-workspace-saving.md) owns the full track.
Snapshots cover all collections/profiles, app settings/layouts, plugin state, master/
source FX, routing/device bindings and optional selected media; include history,
atomic writes, preview, restore points, rollback and portable change sets.

Offline editing must work on Windows, macOS and Linux in all six directed OS pairs,
plus same-OS round trips. Separate target-machine configuration from editor-machine
capabilities. Missing devices/plugins remain intact placeholders. Native plugins
are not required for pure offline editing. Mac-editing-Windows is an example, not
the only supported direction. Preserve existing privacy, secret-exclusion, inert
import and archive validation requirements. Future master/plugin structures can
be saved before their runtime execution exists, but must be labelled accordingly.

### C1-C3 — existing OBS-format saves (separate from snapshots)

C1 inventories every existing config/profile/scene/filter/state file and its actual
reader/writer behavior for the adopted OBS baseline; record a tested version matrix.
C2 implements a lossless preservation model for supported native documents and a
separate jeiobs extension store. Do not silently redefine OBS fields, replace source
IDs, inject mandatory jeiobs-only filters or discard unavailable sources.
Keep jeiobs-only master/listen/PDC/routing state in sidecars keyed to stable document
identity; any embedded extension requires demonstrated preservation by stock OBS.
Preserve original plugin payloads and backups; unknown data must survive jeiobs edits.

Saving an existing OBS-compatible document in jeiobs must leave the shared OBS
portion usable and semantically unchanged except for the user's intentional edits.
Never write into a live OBS installation automatically: edit fork-local copies or
explicit user-selected destinations. Warn before lossy representation; offer a
compatible copy and retain the full jeiobs state. OBS cannot execute jeiobs-only
features. If OBS strips extensions on re-save, reconcile the sidecar against document
revision/hash rather than blindly reapplying stale changes. Ask about conflicts.

C3 tests OBS → jeiobs save → stock OBS open/save → jeiobs reopen using real supported
OBS versions in disposable users. Verify scenes, sources, profiles, device bindings,
plugin state and unknown fields, including missing plugins and concurrent edits.
Document version-specific limits rather than promising all historical/future OBS.

## M2 — audio processing with graceful errors

A1 traces actual mix/encoder contracts and defines ownership, clock domain,
block format, bounded work, latency, tail, reset and error behavior.
A2 adds identity-by-default post-sum/pre-encoder master processing: fader (pre-insert
default), mute, ordered rack, per-effect/global bypass, pre/post meters, clipping,
presets, true-peak/LUFS metering and optional limiter. Keep source filters unchanged.
A3 implements fault containment and a visible, recoverable state machine: healthy,
degraded, bypassed/muted, recovering, restored. No new blocking locks, allocation,
filesystem/UI work on real-time paths. Scanner isolation is not DSP isolation;
plan runtime workers with bounded buffers and watchdogs.

For plugin crash/hang, NaN/Inf, overload, device loss, invalid channel mapping or
latency change: keep the timeline advancing, preserve known-good configuration,
rate-limit diagnostics, quarantine repeated faults and offer explicit recovery.
Never replay stale buffers or produce uncontrolled full-scale output. Use silence
for unavailable input; offer latency-aligned dry bypass only where safe and sensible.
If bypassing a safety limiter could create unsafe levels, default to controlled mute
or a tested safety path, not unbounded dry output. Use short ramps/crossfades where
appropriate, avoid restart loops, and do not block unaffected recording/stream paths.
Retain latency during bypass/recovery until a coordinated timing transition.

Acceptance: impulses/silence/sines, summed clipping, ordering, save/reload, timestamps,
CPU stress, injected process faults, malformed buffers, hot unplug/replug and sustained
stream/record tests. Measure recovery time, discontinuity and A/V offset; define
numerical budgets in A1 and fail tests when exceeded.

## M3 — full plugin-format implementation

Retain VST2 compatibility. Adopt VST3 and CLAP, and native AU on macOS; cover AUv2
and evaluate AUv3 as a separate adapter with explicit support scope. AAX/ARA2 remain
excluded. AU binaries need not execute on Windows/Linux; their document state must
still round-trip offline. Inspect exact official specs/SDK revisions and licenses
before implementation; no SDK provenance or conformance claim is implied here.

P1 creates a versioned host capability matrix with every mandatory host contract and
optional standardized interface/extension for each adopted spec version. Mark each
implemented/tested/pending/not applicable with reasons and a test or issue. The goal
is full host-side spec coverage, not a minimal audio-effect subset. Optional features
must be negotiated; preserve richer format-specific behavior through capability
interfaces rather than forcing everything into a lowest-common-denominator API.
New spec revisions require a reviewed delta; no blanket future-spec claim.

Cover lifecycle/threading, processing precision and block constraints, dynamic I/O,
multi-bus layouts, sidechains, parameter metadata and sample-accurate automation,
state/presets, editors/resizing, latency/tails, bypass, transport/time context,
offline rendering, events/MIDI/instruments and format-specific note/expression/
modulation features where the adopted format specifies them. The initial upstream
effects-only VST3 host is an intermediate milestone, not the final scope.
Standards-compliant unsupported requests need a clear negotiated failure, never
fabricated success. Vendor-private features cannot be guaranteed by spec compliance.

P2 completes VST3 and inherited robustness gaps; P3 adds CLAP; P4 native AU;
P5 runs available official validators, purpose-built boundary plugins and representative
real plugins across supported OS/architectures. Include version/state migration,
editor lifecycle, concurrent instances, missing SDKs and adversarial failure tests.
License/activation and cross-platform state portability remain plugin-dependent.

## M4 — sidechains, delay compensation and listen bus

R1 introduces an explicit graph with stable node/port IDs, main/aux buses, sends,
pre/post taps, independent six-mix FX and stream/record mapping. Sidechain any supported
source/bus to a compatible plugin input with gain/mute, channel mapping and meters.
Reject cycles by default; any future feedback mode must introduce explicit bounded
delay. Compensate sidechain and main input arrival at each processing node.

R2 implements plugin delay compensation across parallel paths, nested FX, sends,
sidechains, hardware/worker buffers and output branches. Align against a documented
master presentation timeline; budget device, resampling and plugin delays exactly
once. Separate intrinsic latency, reported plugin latency and user sync offsets.
Video must be buffered/scheduled as needed to remain aligned with delayed audio.
Do not silently shift only audio or reset timestamps when bypassing/recovering.
Latency changes use preallocated bounded capacity and coordinated transition at a
safe boundary. If required compensation exceeds limits, keep the last valid graph,
defer/reject the change or enter a documented degraded mode with a clear warning.

Support configurable latency/quality ceilings and a low-latency monitoring mode;
show its A/V/PDC tradeoffs. An interactive monitoring choice must not silently
change recording/stream sync. Test impulses and flash/click fixtures at joins,
dynamic latency changes, underruns and long-run clock drift; report measured error.

R3 adds an optional listen bus inspired by a small control-room monitor section:
select final program or a source/mix, PFL/AFL or solo audition, independent output
device, level/mute/dim, mono/channel checks, meters and monitor-only inserts.
Default to program monitoring; disabling the feature preserves ordinary monitoring.
Listen/solo/dim and monitor plugins must not affect stream/record audio. No automatic
loop back into capture; detect obvious self-routing. Save monitor settings separately
from program intent, with device mapping validated at restore. Test that monitor-only
controls leave program output unchanged, including device loss and exclusive access.

R4 handles mixed sample rates, PCM depths and media frame rates automatically.
Use one explicit master clock and timestamped buffers; high-quality resampling and
asynchronous drift correction for independent device clocks. Negotiate plugin/device
formats at adapters, not by reinterpreting samples. Choose internal precision after
auditing current libobs/host contracts; dither only at appropriate final integer
quantization, not every hop. Track clipping/headroom and conversion latency.
Preserve source timestamps, handle rational rates and variable-frame-rate media,
schedule video repeat/drop/interpolation according to the selected output policy,
and avoid cumulative rounding drift. Do not change playback speed or pitch silently.
Expose advanced clock source, output rate/depth/frame rate, SRC quality, buffering,
sync offsets and cadence choices with clear reset-to-default controls.
Defaults favor correctness/quality within a bounded real-time budget; unsupported
device/encoder settings get an explanation and explicit alternative.
Test mixed 44.1/48/96 kHz, integer/float, fractional/integer/VFR video, timestamp
discontinuities, device clock drift and long recordings.

## M5 — timecode, I/O and qualification

T1 defines a recording timecode model distinct from media timestamps. T2 implements
per-output enable/disable and customization for embedded timecode where the muxer/
container supports it, with optional visible burn-in for users who mean rendered
timecode. Both may be selected independently; warn that burn-in changes pixels.
Expose start value, elapsed/free-run/time-of-day/external source when available,
rational rate and drop/non-drop rules where valid. Default new sessions to elapsed
00:00:00:00, rate matched to output and automatic valid numbering policy; existing
profiles retain their setting and timecode is opt-in. Time-of-day requires explicit
choice because it discloses capture time. External-source loss has a visible,
configurable holdover/reacquisition policy without timestamp jumps.

Scope includes every recording path: main, per-track/isolated output where supported,
replay buffer, segmented recording, pause/resume and remux. Specify whether each
operation continues or starts a new timecode sequence; use actual capture timeline
for replay clips. Do not pretend unsupported containers carry native timecode:
show the capability and offer a supported container, burn-in or documented sidecar.
Remux must preserve or explicitly report loss. Burn-in must be output-specific and
must not contaminate a clean stream output. Storage timecode does not replace PDC
or fix an out-of-sync recording.

T3 verifies frame-accurate readback in independent readers/NLEs, fractional-rate
boundaries, pause/replay/segments, long-run drift, remux, missing external clocks
and recovery. Include configuration snapshot/OBS-compatible-save interactions.
Native I/O: evaluate ASIO licensing, Core Audio and ALSA/JACK/PipeWire while retaining
upstream capture. Expand sanitizers, dependency manifests, reproducible packaging
and platform smoke tests. Review upstream monthly and promptly for security fixes;
no release support claim without execution evidence.

## Autonomous cross-platform delivery

Agents should implement shared C/C++ models and algorithms with narrow native
adapters, synthetic fixtures and headless tests first. Proactively implement all
applicable Windows/macOS/Linux paths in the same feature sequence; do not knowingly
leave Windows/macOS stubs for the owner to finish. Use available native CI runners
to compile and exercise platform code; cross-compilation alone is not runtime proof.
OS-specific features use explicit capability boundaries rather than scattered
preprocessor assumptions. Cover macOS arm64/x86_64 and Windows/Linux supported
architectures with a declared matrix; unsupported targets stay labelled.
Agents can proceed autonomously with ordinary design/code/test work and small PRs.
Record runner/hardware blockers and exact reproduction steps; reserve scarce human
hardware time for native UI, drivers, licensed plugins and actual device validation.
Keep evidence honest and current. No extra owner approval for routine implementation;
maintainer merge/release authority remains unchanged.
