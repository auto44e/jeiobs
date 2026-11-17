**Verdict: request changes.** The commit is unchanged, so nothing has been fixed yet. I re-checked my nine findings and all still hold. I'm narrowing claude-6 and correcting claude-9, and adding two new Minors (claude-10, claude-11).

**Codex's findings**
- **codex-1: I disagree and can't reproduce it.**
  - The lone `.bak` entry is built at `CapturePlanner.cpp:986-993` and never sets `identity_uid`.
  - `:1116` only writes `collection` when `identity_uid` is not empty, and `:1086-1092` lists only collections that have a main file.
  - So the filesystem path left in `bak_fs_path` (`:523`, skipped at `:899-900`) never reaches the output. The expected file `bak-alone.json` shows `Lone.json.bak` with no `collection`.
  - Reusing `bak_fs_path` to hold the UID (`:907,914,918,962,1090`) is fragile and worth renaming, but it is not a bug.
- **codex-2: I agree.** It is the same defect as claude-4. One more silent drop: an entry whose `symlink_status` fails is skipped (`Fs.cpp:249-255`).
- **codex-3: I agree, at Major.**
  - The planner keeps every scene UUID of 256 bytes or less with no count cap (`CapturePlanner.cpp:582-592`), but the validator rejects more than 100,000 (`Manifest.cpp:858`).
  - 100,001 scene items fit easily within 64 MiB and 1,000,000 values (`Limits.hpp:24,26`). The capture then can't be written as a valid manifest.
  - I'm moving the scene-UUID part of claude-6 to codex-3.
- **codex-4: I disagree.**
  - A symlink whose name fails PathRules can't get its own entry. The validator rejects that path (`Manifest.cpp:786-790`), and rule 4 forbids recording the name.
  - The planner instead counts it in the folder's `names` entry and adds 0 bytes for it (`:224-229` adds size only for files and directories). It never follows it.
  - The precedence in r1.5 can only apply to representable names. That is a wording fix in the contract, not a code change.

**My findings, re-checked**
- **claude-1:** Confirmed against `parent-identity.json:43` and `identity-parent.json`.
  - The "Copy" case passes only because `current_set_count` also counts Main (`:889-897`). If Main's scenes are edited and Copy is left as is, Copy sorts first and takes Main's UID.
  - The spec says "in this order" (`WORKSPACE_FORMAT.md:417`), so it also puts same-path matching first.
- **claude-3:** The frontend follows links for profile folders (`OBSBasic_Profiles.cpp:351-352`, `entry.is_directory()`). So for OBS a linked app dir is a working config, which the planner reports as nothing.
- **claude-6, narrowed to the remaining parts:**
  - the `ReviewRules` scans (`ReviewRules.cpp:23-31,64-71`), and a unit test (`jw_unit.cpp:579-586`) that can't tell the table apart from one that accepts every type;
  - `collections_` and `profiles_` sit outside the entry limit until `:1053`, every `.json` is read first, and the lookup at `:514` is O(n²);
  - `:805` builds a string per (entry, profile) pair.
- **claude-9:** Drop the §9 ordering point, because `:417` covers it.
  - New gap: r1.2 says a lone `.bak` gets "NO collection/profile UID".
  - But the planner and all three expected files give a lone `service.json.bak` the profile UID (`bak-alone.json` e6).
  - The contract should say "collection" only.

**New**
- **claude-10 (Minor):** `cases.txt:1332-1334` still accepts `bak-alone-alt2` and `bak-alone-alt3`.
  - Both give the lone `.bak` a collection UID and a `collections[]` item, which r1.2 forbids. So jw-cases would not catch the regression codex-1 describes.
  - Keep only `bak-alone.json`.
- **claude-11 (Minor):** The planner doesn't check the 64 MiB blob limit or the 256 MiB revision limit that the validator enforces (`Manifest.cpp:830-835`).
  - A collection over 64 MiB fails `ReadBounded` with `E_LIMIT`. It then gets a misleading `collection-unparseable` warning (`CapturePlanner.cpp:543-547`) and is still planned as stored.
  - The contract has no rule for this. It needs one (omit with a reason, or `E_LIMIT`) before PR4.

FINDING claude-1 | Major | open | rule-2 claim by an earlier-sorted collection steals a later collection's rule-1 path UID; binding depends on name order (CapturePlanner.cpp:898-919)
FINDING claude-2 | Major | open | `themes` skip in ProcessDirectory (CapturePlanner.cpp:383-385) silently drops a profile folder named themes
FINDING claude-3 | Major | open | symlinked or junctioned app dir or owned subdir gives an empty or partial plan with no entry or warning (Fs.cpp:47-59,233-237; CapturePlanner.cpp:408-413)
FINDING claude-4 | Major | open | listing errors are ignored: EACCES, mid-listing errors and Windows unpaired-surrogate names silently truncate the plan (CapturePlanner.cpp:401,413; Fs.cpp:240,249-256,267)
FINDING claude-5 | Major | open | planned `count` field unknown to the v1 schema and spec; export and CarryOver treat it as a dropped extension (Manifest.cpp:1121-1129,1253)
FINDING claude-6 | Minor | open | quadratic ReviewRules scans with a non-discriminating unit test; collections_/profiles_ outside the entry limit, O(n²) find_if (:514), per-pair strings (:805); scene-UUID part moved to codex-3
FINDING claude-7 | Minor | open | collapse accounting: duplicate container paths (false E_COLLISION), 101-sample truncation, recursive counting
FINDING claude-8 | Minor | open | directories without basic.ini become profiles with UIDs, unlike OBS
FINDING claude-9 | Minor | open | WORKSPACE_FORMAT.md §4/§7/§13/§15 stale for PR3 fields and slice; r1.2 wording wrongly covers profile `.bak` UIDs
FINDING claude-10 | Minor | open | cases.txt:1332-1334 still accepts bak-alone-alt2/alt3, which r1.2 forbids, so a lone-.bak UID regression passes jw-cases
FINDING claude-11 | Minor | open | planner plans stored files over the 64 MiB blob / 256 MiB revision limits (Manifest.cpp:830-835); oversize collection gets a misleading collection-unparseable warning (CapturePlanner.cpp:543-547)
VERDICT: request changes
