# Task 006 — control surfaces and open control protocols (M6)

Status: authorized scope; K1-K6 pending. Owner direction, 2026-11-16 (verbatim):
"make sure there's full MIDI1/2.0 flexibly routable along with mackie hui, osc, and
whatever open control standards out there commonly used".
Read [ROADMAP](../ROADMAP.md) M6, M1 (mappings are saved state), M3 (plugin parameters
and MIDI event inputs), M4 R1 (graph IDs) and M5 (timecode sources).

1. K1: design the control graph (sources, targets, transforms, feedback, learn, device
   identity, persistence, threading and flood policy) with a debate record; implement the
   model as a shared C++ library with headless tests, then the frontend binding.
2. K2: MIDI 1.0/2.0 core (UMP, translation, MIDI-CI, discovery/negotiation), native
   backends on all three OSes, virtual ports, RTP-MIDI and Network MIDI 2.0, clock/MTC/
   MMC/MSC; MIDI into plugin event inputs and out to ports.
3. K3: Mackie Control Universal and HUI surfaces, extenders and emulation modes.
4. K4: OSC 1.0/1.1 (UDP, TCP/SLIP, bundles, patterns), OSCQuery, documented namespace.
5. K5: Ableton Link, sACN/Art-Net, AES70/OCA, generic HID, obs-websocket as a graph
   source/target; each behind a capability boundary.
6. K6: capability matrix, cross-platform loopback/virtual-device tests, fuzzing,
   latency/jitter numbers and per-device hardware records.

Constraints: network listeners off by default and loopback-bound until the user opts in,
with peer allowlists and rate limits; control I/O never on the audio thread; no vendor code
for MCU/HUI; dependency choices (for example a MIDI 1/2 library, an OSC library, Link)
get a license review and an owner-merged dependency PR. Small PRs; record exact native CI
results and which behaviour is verified on real hardware.
