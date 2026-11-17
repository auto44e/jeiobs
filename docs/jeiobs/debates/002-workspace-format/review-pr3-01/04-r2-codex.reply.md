FINDING codex-1 | Major | withdrawn | Lone `.bak` branch emits no UID (`CapturePlanner.cpp:986-993,1116-1118`).
FINDING codex-2 | Major | open | RealFs listing errors are discarded, allowing incomplete successful plans (`CapturePlanner.cpp:370-414`; `Fs.cpp:239-263`).
FINDING codex-3 | Major | open | Scene UUID extraction is quadratic and uncapped beyond the 100,000-item schema limit (`CapturePlanner.cpp:550-592`; `Manifest.cpp:856-867`).
FINDING codex-4 | Minor | withdrawn | Invalid-name symlinks require aggregate representation; this is a contract ambiguity, not a code defect (`CapturePlanner.cpp:375-381`).
VERDICT: request changes