# jeiobs roadmap

Updated 2026-11-15. Milestones are outcome groups, not strict serial gates.
First coding priority: [VST3 upstream backport](tasks/001-vst3.md), an M3 slice
that may proceed before M0 is finished. This does not declare coexistence safe.

| Track | Outcome | Status |
| --- | --- | --- |
| Setup | Fork, stable pin, contribution and CI scaffolding | Scaffold authored; see HANDOFF for verification |
| M3-first | Evaluate/cherry-pick upstream VST3 with build dependencies | Ready for coding agent |
| M0 | Baseline builds and independent identity on all platforms | Audit started; implementation/test pending |
| M1 | Explicit read-only scene/profile import and plugin compatibility | Planned |
| M2 | Post-sum, pre-encoder master processing | Planned |
| M3 | Modern plugin hosting: VST3, CLAP, macOS AU | VST3 prioritized; remaining work planned |
| M4 | Per-mix chains, sends, sidechains and latency compensation | Planned |
| M5 | Professional I/O and release hardening | Planned |

## M0: coexistence

Independent app/install/config/log/cache/plugin/update identity on Windows,
macOS and Linux. Audit IPC, WebSocket ports, crash uploads, protocol handlers,
shortcuts, library resolution, virtual camera and uninstallers. Preserve libobs
symbols and source/module IDs. Do not replace OBS's virtual camera. Optional
system components require explicit installation consent. Test install, launch,
update and uninstall in both orders; hash OBS data before and after. Shared
hardware contention is distinct from filesystem coexistence.

## M1: explicit migration

Read-only import with preview, dry run and rollback; preserve unknown fields and
missing plugin/source settings. No silent secret import or script execution.
Trusted plugin discovery requires consent and fork-local installation. Test native
modules, scripts, WebSocket clients, VST2 presets and missing dependencies. State
architecture, ABI, signing and hard-coded path limitations; no universal promise.

## M2: master audio

Inspect libobs mixing/output/encoder contracts before choosing the hook. Identity
by default after summation and before encoder delivery for the selected mix.
Master fader (pre-insert initially), mute, ordered FX, per-effect/global bypass,
pre/post meters, clipping indicator and presets. Add true-peak/LUFS metering and
optional limiter; monitor the final stream without feedback. Preserve source
filters, timestamps, format and channel layout. Test silence, impulse, sine,
summed clipping, gain, order, bypass, channel mapping, timestamps, save/reload,
CPU load and recording/streaming regression.

## M3: plugin hosting

Retain VST2. Reuse upstream VST3 before writing a new host; distinguish upstream
source-filter support from future master-bus integration. Format-neutral hosting,
then CLAP and AUv2 (evaluate AUv3 separately). AAX/ARA2 excluded. Review exact SDK
licenses before import. Cover scanner isolation, editor/state/parameters, channel
negotiation, sample-rate changes, tails, latency and failure recovery. Runtime
worker isolation is separate work; a scanner subprocess does not isolate DSP.
No new allocation, blocking, I/O or UI calls in real-time audio paths.

## M4: routing and timing

Independent FX for six mixes, clear stream/record mapping, separate monitoring,
processed stream with dry recording, sends and sidechains, cycle rejection,
latency compensation including video sync. Advanced rates follow verified device
and encoder constraints. Test underruns, dropped workers and output isolation.

## M5: I/O and hardening

Evaluate Windows ASIO (license review), Core Audio, ALSA/JACK/PipeWire. Retain
capture compatibility. Expand platform CI, sanitizers, packaging smoke tests,
dependency provenance and compatibility matrix. Upstream review at least monthly
and promptly for security fixes; each integration gets a small reviewable PR.
No claimed support without execution evidence. Preserve GPL and upstream credit;
never use upstream update channels, signing credentials or OBS release branding.

## Workspace saving and offline configuration

Planned: [whole-workspace snapshots, portable export and offline editing](WORKSPACE_SAVING.md).
Includes version history/rollback and cross-platform master-FX suggestions with
preserved device bindings and missing-plugin placeholders. See
[task 002](tasks/002-workspace-saving.md) for the first bounded investigation.
VST3 remains the first coding task; these are proposed capabilities, not implemented.
