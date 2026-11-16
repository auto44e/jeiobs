# AGain silence / ADelay delay investigation — Claude

_Raw peer correspondence, task 003 (2026-11-15/16). Codex tool-call output trimmed; prompt and final answer kept verbatim. AI-assisted, not human review._

## Prompt

````text
# JeiOBS task 003: VST3 scanner robustness — design brief

Repo: ~/oss/jeiobs (fork of OBS Studio 33.0.1; upstream VST3 host backported in
plugins/obs-vst3, by upstream authors). C++17, libobs util APIs. Platforms the
code must COMPILE on: Linux, Windows, macOS. Only Linux can be run/tested here.
Fork owner has approved deviating from upstream code for this.

## Current architecture (read the files; key points)
- Host plugin `obs-vst3.so` (plugins/obs-vst3/obs-vst3.cpp:169-353, plugin-main.cpp).
  On module load `retrieveVst3List(nullptr)` spawns the helper `obs-vst3-scanner
  <config>/vst3list.json` via libobs `os_process_pipe_create2(args, "r")`, then a
  std::thread `getScanResult` loops on blocking `os_process_pipe_read` (stdout)
  until it sees `VST3_SCAN_DONE\n` or `NO_VST3\n`, or the pipe closes. Then it
  loads the JSON list, reads all stderr, `os_process_pipe_destroy` (waitpid /
  WaitForSingleObject INFINITE), sets `vst3ScanDone`.
- `freeVst3List()` joins `vst3ScanThread`; called from `obs_module_unload` and
  from the "Rescan" button callback (only when scan is done).
- libobs pipe API (libobs/util/pipe.h) has NO kill, NO timeout, NO pid accessor.
- Scanner (plugins/obs-vst3/vst3-scan/vst3-scanner.cpp + VST3Scanner.cpp) runs in
  ONE process: for each bundle under /usr/lib/vst3, /usr/local/lib/vst3, ~/.vst3
  (platform equivalents) it either parses Contents/Resources/moduleinfo.json
  (no code loaded) or, if no moduleinfo or factory flag kClassesDiscardable,
  calls `VST3::Hosting::Module::create` (dlopen/LoadLibrary + GetPluginFactory)
  and enumerates classes (`addModuleClasses`). Cache path: `vst3CacheLoad` reuses
  the JSON but re-loads discardable modules and new bundles in-process too.
  Writes the JSON with obs_data_save_json_safe, prints VST3_SCAN_DONE.

## Observed failures (Linux, reproducible with tools/jeiobs/vst3-check/run.sh)
1. A bundle whose .so segfaults on load (constructor raise(SIGSEGV)) kills the
   scanner (exit 139) before the JSON is written → host logs "Pipe closed before
   result" → ZERO VST3 plugins available, though 2 good bundles exist.
2. A bundle whose .so hangs on load (constructor `for(;;) pause();`) → scanner
   never exits; no timeout anywhere.
3. With (2), OBS shutdown blocks forever in freeVst3List() join (it exited ~1 s
   after the scanner was killed manually).

## Proposed design (critique it; propose better if you see one)
A. Scanner becomes a coordinator that never loads plugin code itself. For each
   bundle that needs a code load, it spawns ITSELF in a child mode
   `obs-vst3-scanner --probe <bundle>` (via os_process_pipe), which loads that one
   module and prints its classes (e.g. one JSON object on stdout, then a marker).
   moduleinfo.json-only bundles stay parsed in the coordinator (no code load).
B. Per-probe timeout enforced INSIDE the child: a watchdog thread started before
   loading calls a hard exit after N s (POSIX `_exit`; Windows
   `TerminateProcess(GetCurrentProcess(),…)`, NOT ExitProcess because of the loader
   lock). The coordinator's blocking read then returns when the child dies.
   A crash/timeout/garbage output marks only that bundle as failed (logged with
   path and reason), the others are kept.
C. Overall scan deadline in the coordinator, and a bounded host shutdown: since
   libobs can't kill the scanner, options are (i) add `os_process_pipe_kill()`
   (or pid accessor) to libobs (POSIX kill(SIGKILL), Win TerminateProcess) and
   kill on unload; (ii) a cancel-file the host creates on unload that the
   coordinator polls between probes; (iii) accept the bounded wait of one probe
   timeout. Pick one and justify.
D. Keep the JSON cache format compatible (version 1) unless there's a reason;
   optionally remember failed bundles so a crashing bundle isn't re-probed every
   launch (with invalidation by mtime) — say whether that's worth it now.

## Constraints
- Small, reviewable diff; follow the existing code style (tabs, clang-format
  OBS style, 120 cols). No new third-party deps. Must still build with
  ENABLE_VST3=OFF (untouched) and compile on Windows/macOS (no Linux-only APIs
  without #ifdef).
- Don't change DSP/audio paths. Don't load plugin code in the OBS process during
  scanning.
- Evidence will be the Linux runtime check: with crash.vst3 + hang.vst3 + 2 good
  bundles present, the 3 good classes are listed, the scan finishes within the
  timeout, and OBS exits cleanly within a few seconds.

## Also open (separate, investigate only)
- `again-sample-accurate` (Steinberg SDK 3.8.0 sample) outputs silence through
  the host filter; ADelay works. Suspect the processor's gain parameter starts at 0
  unless the host applies controller defaults/state to the processor. See
  plugins/obs-vst3/VST3Plugin.cpp and the SDK sample at
  ~/oss/jeiobs-deps/vst3-full/vst3sdk/public.sdk/samples/vst/again_sampleaccurate.
- ADelay measured 1.078 s delay vs 1.000 s configured (host processes fixed 10 ms
  blocks, obs-vst3.h:34).

Your job: INVESTIGATE ONLY the 'Also open' items. (1) Why does again-sample-accurate output silence through this host while adelay works? Read plugins/obs-vst3/VST3Plugin.cpp (init, setupProcessing, setActive, parameter/state transfer between controller and processor, IParameterChanges usage, process()) and the SDK sample sources under ~/oss/jeiobs-deps/vst3-full/vst3sdk/public.sdk/samples/vst/again_sampleaccurate (how its processor initialises gain, whether it relies on the host to send parameter values / setComponentState). Identify the root cause with file:line evidence and the minimal host-side fix consistent with the VST3 spec. Say whether this is a host bug or sample behaviour. (2) Briefly assess the 1.078 s vs 1.000 s ADelay delay: could fixed 10 ms blocking (obs-vst3.h:34) plus host buffering explain ~78 ms? Max 700 words. Do not edit files.````

## Answer

## (1) Why AGain Sample Accurate is silent: the sample causes it; the host also has a smaller sync gap

**Root cause:** the AGain processor's Gain starts at 0.0, and nothing in this host ever changes it.

- `parameters` is a `std::array<SampleAccurate::Parameter,2>` (agsa_processor.cpp:57). The constructor only calls `setParamID` (:62-67), and the class defaults to `initValue = 0.` (sampleaccurate.h:43, 144-148). Gain is a plain multiplier (agsa_processor.cpp:192-202), so output = input × 0.
- The controller advertises a default of 1.0 (agsa_controller.cpp:46), so a UI would show 100 %.
- The gain can only change through `inputParameterChanges` (:211-226, :239) or `setState` (:89-120, applied at :231-237).
- The host sends neither to a fresh instance:
  - `VST3Plugin::init` (VST3Plugin.cpp:223-378) transfers no state or parameters.
  - In SDK 3.8.0, `PlugProvider::connectComponents` only connects the two parts (plugprovider.cpp:225-257).
  - `guiToDsp` is only fed by `performEdit` and `restartComponent` (VST3ComponentHolder.cpp:41-48, 85-97). AGain has no editor, so nobody can move the gain.
  - `loadStates` only runs when saved state exists (obs-vst3.cpp:766-779). Saved state comes from the processor, so it is also 0.
- ADelay works because its processor sets its own default: `mDelay {1.}` (adelayprocessor.h:48). The clearest comparison: "Advanced Tutorial", in the **same bundle**, sets `gainParameter {ParameterID::Gain, 1.}` (tutorial.cpp:69).

**Runtime check:** I ran the existing probe on the existing build with throwaway HOMEs in `/tmp/jeiobs-agsa2.IF8E`, with PulseAudio unreachable. AGain had peak 0.000 and no onset. Advanced Tutorial had peak 0.500 (unity gain) and onset 0.746 s. ADelay had peak 0.500.

**Verdict: the sample causes the silence.** In VST3 the processor owns the state, and the host syncs the controller *from* it (`getState` → `setComponentState`). No rule says the host must push controller defaults into the processor. A host that follows the spec also outputs silence; it just shows Gain at 0 %.

**Host bug (real, but secondary):** the host skips that sync after creating the plugin. So the controller says 100 % while the processor runs at 0, and `restartComponent(kParamValuesChanged)` pushes those unsynced controller values into the processor.

**Minimal fix that follows the spec** (it won't make AGain audible): add this after VST3Plugin.cpp:291, before `setActive`:
```cpp
Steinberg::MemoryStream s;
if (vstPlug->getState(&s) == kResultOk) {
	s.seek(0, IBStream::kIBSeekSet, nullptr);
	editController->setComponentState(&s);
}
```
Don't push controller defaults into the processor; that would override defaults the processor owns.

Side note: VST3Plugin.cpp:230 `sampleRate = sampleRate;` assigns the parameter to itself, so the member stays 0. It's harmless today because nothing reads the member.

## (2) ADelay at 1.078 s: the 10 ms blocks don't explain ~78 ms

- **ADelay itself is exact:** 48000 samples (adelayprocessor.cpp:162), and it reports 0 latency.
- **The 10 ms block stage adds no delay to the audio.** It is an exact first-in, first-out buffer with no leading zeros (obs-vst3.cpp:1002-1033). It has two side effects:
  - It holds back the first packet (`return nullptr`, :1014-1016), so audio reaches OBS about one packet (~21 ms) late.
  - It stamps its output 10 ms *early* (:809, :1032). If OBS honoured that, the wet audio would arrive earlier, not later.
- **Likely cause, plausible but not proven: how libobs places a source's audio.**
  - libobs fixes the source's position from its first packet. After that it appends each packet whose timestamp is within 70 ms of the expected time (obs-source.c:1489, 1636-1639, 1654-1662).
  - When a source's buffer runs short, libobs moves `audio_ts` forward one window but keeps the data (obs-audio.c:289-298), so everything after it plays later for good.
  - Audio that arrives late, as it does through this filter, can pick up several ~21 ms slips. libobs never corrects an offset under 70 ms.
- **The measurement is also noisy.** Dry and wet run in separate OBS processes, and each onset includes that run's media start-up time.
  - Your runs: 68 and 89 ms extra; your bypassed-filter control showed only 1.8 ms.
  - My clean runs: 100 and 67 ms extra, with dry onsets of 0.544 vs 0.709 s.
  - The extra delay is always positive, but the method can't pin it down to better than tens of ms.
- **To settle it:** measure dry and wet in the same run (two sources playing the same file, panned left and right), or compare the filter's input and output timestamps with `obs_source_add_audio_capture_callback`.

## Problem with the check script

The probe listens to the whole mix, which includes the profile's default Desktop Audio capture. My first rerun (`/tmp/jeiobs-agsa.HlpK`) picked up other audio playing on this machine: a 0.91 peak at 3.2 s, and an onset at 0.09 s, before the tone starts. run.sh should set `PULSE_SERVER` to an invalid path, or tap the source instead of the mix.

I edited no repo files. All six OBS runs used throwaway HOMEs under /tmp.
