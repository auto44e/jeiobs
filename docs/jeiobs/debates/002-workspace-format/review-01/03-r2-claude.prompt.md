You are one of two independent reviewers (Claude and Codex, different model lineages) in a
cross-talk review for the JeiOBS repository. Your working directory is a checkout of exactly the
commit under review. The lead agent wrote or integrated the change and must NOT decide on its own
whether your findings are resolved: under docs/jeiobs/DECISIONS.md a finding is closed only when the
reviewer who raised it accepts the fix or withdraws it, in writing, in a later round. Use one
severity scale: Blocker, Major, Minor. Only the finder may lower a finding's severity. You are
read-only: never modify, create or delete files, and never run commands that change state.
Verify claims in the files rather than trusting the context. Be terse; cite file:line.

End your reply with one line per finding YOU raised (every ledger row with you as finder, every
finding you raised in an earlier round, plus new ones), then the verdict. If you raised none, write
FINDING none. Use exactly this syntax:
FINDING <ID> | <Blocker|Major|Minor> | <open|accepted-fix|withdrawn> | <one-line reason>
VERDICT: approve
or
VERDICT: request changes
For a new finding, use ID <your-name>-<n> (for example claude-1) and quote the evidence.

REVIEWED COMMIT: 6c8269596926acf9e01bd398073a8aa23cbd0d86

TOPIC: Review run 01 of PR #21 (S1-PR1).

SHARED CONTEXT:
# PR #21 review run 01 — task 002 S1-PR1 (workspace format v1 + inert manifest library), head 6c8269596

Your working directory is a checkout of this commit. The change is `git diff 270f9d6c9 HEAD`
(main is 270f9d6c9). Read: docs/jeiobs/WORKSPACE_FORMAT.md (the spec), the PR1 contract at
docs/jeiobs/debates/002-workspace-format/04-pr1-contract.md (wins over the spec for PR1),
docs/jeiobs/s1-persistence-audit.md, shared/jeiobs-workspace/ (library, jw-check CLI, jw-unit,
tests/cases with 328 black-box cases), docs/jeiobs/tasks/002-workspace-saving.md (Progress).

Authorship: the Codex worker wrote the library; the Claude worker wrote the black-box cases from
the spec and contract without reading the library; the lead wrote the contract, fixed 14
contract mismatches the cases found (412d28adb), and the docs. The format doc was drafted by
Claude from the lead's resolutions of the design debate.

Lead's evidence (Linux, Ubuntu 24.04, GCC 13, CMake 3.28, -Wall -Wextra -Werror):
`cmake -S shared/jeiobs-workspace -B build && cmake --build build && ctest --test-dir build`:
jw-unit PASS, jw-cases 328/328 PASS. Windows/macOS not compiled (CI workflow is a separate,
owner-merged .github PR).

This is a new persistence format (owner rule: review rounds repeat until one finds no Blocker or
Major). Review for: spec correctness against the OBS code it cites (verify file:line), security of
the secret/placeholder/export rules, bounded parsing (can any input exceed a limit before it is
checked, crash, or hang?), path-rule completeness (Windows/macOS/Linux), compatibility rules,
whether the cases prove what they claim, C++ correctness and portability, and doc accuracy.

Severity: Blocker = must not merge; Major = must be fixed before a self-merge; Minor = follow-up.
Other agent work runs on this host in separate worktrees; it does not touch this checkout.

FINDINGS LEDGER (kept by the lead):
# Findings ledger — PR #21 (S1-PR1)

ID | severity | status | finder | source | lead's note
--- | --- | --- | --- | --- | ---

YOUR OWN PREVIOUS REPLY:
Requesting changes: 3 Major, 4 Minor. I reviewed by reading only, because no shell was available here, so I didn't build or run anything. The crash in claude-2 comes from reading the code.

## Major

**claude-1: the spec has no allowlist gate on what restore/import may write.**
- The only stated write gate is PathRules (`WORKSPACE_FORMAT.md:557-559`: "files reach a root only through manifest paths that passed `PathRules`").
- PathRules (`:530-541`) accepts any relative path under any root. That includes OBS's per-user plugin directories under the config root (`PluginModuleLoader_Linux.cpp:71,198-214`, `PluginModuleLoader_MacOS.mm:55,139-143`) and the `obs-browser` profile.
- An entry's `role` is never tied to its `root`/`path`, so role-based restore rules such as forced websocket auth (§8) can be bypassed.
- The roots point to the same directory by default (`OBSApp.cpp:350-353`), but collision checks only compare entries within one root (`04-pr1-contract.md:69-70`; case `manifest-no-collision-other-root`).
- Fix in the spec: S2/S3 write only entries whose `(root, path, role)` matches the reader's §7 allowlist, re-apply the role rules to imported blobs, and check collisions on the resolved destination.

**claude-2: `ProjectForExport` crashes on manifests the validator accepts.**
- `unreviewed` is optional even when fidelity is `opaque-unreviewed` (`Manifest.cpp:537-545`).
- The include path still calls `entry.at("unreviewed").at("rules")` (`:1060`). That throws `out_of_range`, nothing catches it (`jw-check.cpp:229`), and the process ends in `std::terminate`.
- Omitted entries' `fidelity` is never validated (`:497-523`), yet they are still treated as opaque and need a decision (`:948-956`, `:1033`).
- `now` is stored unvalidated (`:968`). Invalid UTF-8 there makes the strict `dump()` throw (`jw-check.cpp:234`).
- Fix: require `unreviewed` for stored opaque entries, reject `fidelity` on omitted entries, validate `now`, and add cases for each.

**claude-3: the secret registry misses first-party URL fields, although `:279` says "Section 8 lists every secret key".**
- `ffmpeg_source` `/settings/input` is a network URL when `is_local_file` is false (`obs-ffmpeg-source.c:182,413-422`). Its `ffmpeg_options` (`:211,436`) can also hold credentials.
- `vlc_source` `/settings/playlist/*/value` accepts files and URLs (`vlc-video-source.c:637,1065`).
- Neither is in the `url_credential` row (`:344`) or in u1's list of typed URLs (`:388-389`).
- Every collection is opaque today, which hides this for now, but PR2 and PR3 will implement from this table.

## Minor

**claude-4: the export projection keeps fields it doesn't know.**
- It copies `creator`, `active` and `compat` whole (`Manifest.cpp:959-963`), the input `dropped_ext` items as they are (`:1006`), and non-object `app_keys` items (`:992-994`).
- `app_keys` items with a missing or unknown `restore` are exported (`:989`), so that check fails open.
- This contradicts §4 ("keeps only fields it knows"), and no case covers it (`08-pr1-tests-claude.md:51` lists it as not covered).
- `--export` validation accepts local-only `target` members and `restore: "never"` keys.
- The include decision's `fidelity` is chosen by the caller instead of derived from `redactions`: `expected/export-parse-invalid.json:19` is `redacted` with no redactions.

**claude-5: some valid OBS file names can't be represented, and the path rules have gaps.**
- `GetFileSafeName` (`OBSApp.cpp:1748-1774`, used at `OBSBasic_SceneCollections.cpp:251` and `OBSBasic_Profiles.cpp:151`) keeps names like `Aux` on Linux/macOS. PathRules rejects that path in every entry form, so the whole manifest fails.
- The same happens for components over 255 bytes (NTFS counts 255 UTF-16 units) and for subfolders deeper than 8 components.
- The spec doesn't say what capture does in these cases.
- Gaps in the rules themselves: NTFS 8.3 short names (`BASIC~1`) get past the ASCII collision key, the D145 key doesn't ignore the code points HFS+ ignores, and file-versus-directory conflicts (`a` and `a/b`) aren't rejected.

**claude-6: library I/O problems.**
- `revision` and `export` hash the file and parse it in two separate reads (`jw-check.cpp:150,159`; `:207,224`). §3 says to check the bytes against the name before parsing.
- `LoadedManifest` is declared but never used (`Manifest.hpp:52`).
- `LoadManifest` and `HashFile` open narrow `std::string` paths with `std::ifstream` (`Manifest.cpp:835`, `Hash.cpp:58`). On Windows that uses the ANSI code page, so a store under a non-ASCII `%APPDATA%` can't be opened. §15 only mentions `argv`.
- `CarryOver` doesn't re-validate its output. Parent plus draft can exceed 4 MiB or 250,000 values, producing a child the library itself would reject.

**claude-7: the export gate will refuse some legitimate exports and leaks one length.**
- §6 scans every staged byte for each live registry value (`:268-270`), including `username` (`:341`). Short, common usernames will match unrelated bytes and block exports, so the scan needs a minimum length or a kind exclusion.
- An `unparseable` omitted entry keeps its `size` in the export (`Manifest.cpp:1022-1028`). For a role that holds secrets, that size gives away the secret's length.

## Checked, no issue
- The in-tree citations I sampled are accurate: `obs-data.c:611,630,641-668`, `config-file.c:81,345-390,411-441,378-381,625-630`, `platform-nix.c:576-581`, `platform.c:261-284`, `OBSApp.cpp:418,548,561-613,1127-1130,1702-1717`, the blake2 and cmake lines, the OAuth, service and settings keys, and `vst3-scanner.cpp:646`.
- I couldn't check the obs-websocket citations: that submodule isn't checked out here.
- Parsing stays within its limits. The file size is checked before reading, values, depth and entries are counted during the parse, and duplicate keys and NUL are caught. The string limit is checked after the lexer has buffered the string, but that buffer can't exceed the 4 MiB manifest limit.

FINDING claude-1 | Major | open | import/restore write gate is PathRules only; must bind (root,path,role) to the §7 allowlist and check collisions on the resolved destination
FINDING claude-2 | Major | open | ProjectForExport throws on validator-accepted input (Manifest.cpp:1060; unreviewed optional at :537-545; `now` unvalidated)
FINDING claude-3 | Major | open | secret registry misses ffmpeg_source input/ffmpeg_options and vlc_source playlist URLs despite "lists every secret key"
FINDING claude-4 | Minor | open | export copies unknown content (creator/active/compat/dropped_ext/app_keys), --export accepts local-only fields, include fidelity not derived
FINDING claude-5 | Minor | open | live OBS names PathRules rejects have no capture outcome; 8.3, HFS+ and file-vs-directory gaps
FINDING claude-6 | Minor | open | hash and parse in separate reads, unused LoadedManifest, narrow-path ifstream on Windows, CarryOver output not re-validated
FINDING claude-7 | Minor | open | live-value scan false-positives on short usernames; unparseable omitted size reveals secret length
VERDICT: request changes

THE OTHER REVIEWER'S LATEST REPLY:
FINDING codex-1 | Major | open | “opaque-unreviewed” is accepted without “unreviewed” (Manifest.cpp:504-545), but export include dereferences it unconditionally (Manifest.cpp:1056-1060), causing termination.
FINDING codex-2 | Major | open | DECISIONS uses unrestricted `bytes.resize` and `Json::parse` (jw-check.cpp:56-74,212-223), bypassing parser limits and enabling memory exhaustion.
FINDING codex-3 | Major | open | `CON .txt` passes because the stem retains its space (PathRules.cpp:96-103,193-207), although the spec warns Windows may treat it as a device (WORKSPACE_FORMAT.md:624-625).
FINDING codex-4 | Minor | open | `PrintError` writes E_UNDECIDED to stdout (jw-check.cpp:32-38), contrary to the contract’s “nothing on stdout” requirement (04-pr1-contract.md:45-46).
VERDICT: request changes