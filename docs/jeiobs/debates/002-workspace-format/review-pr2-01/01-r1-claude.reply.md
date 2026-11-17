**Verdict: request changes.** I found 4 Majors and 4 Minors. I traced everything by reading the code and the OBS sources in this tree; I built and ran nothing.

**claude-1 (Major): OBS reads some `basic.ini` tokens that the redactor leaves in OUTFILE.** IniScan's line and section rules are narrower than OBS's parser:
- IniScan splits lines on LF only (`bytes.find('\n', offset)`, IniScan.cpp:129).
- It skips only space and tab before a key (:71).
- It accepts a header only if the whole trimmed line is `[...]` (`line[first] == '[' && line[trimmed_end - 1U] == ']'`, :144).

OBS treats CR, LF, CRLF and LFCR as line breaks (lexer.h:91-93, :101-103; lexer.c:261-263). It skips CR and LF before a key (config-file.c:174-177). It ends a header at the first `]` or line break and ignores the failure (:228), then reads the rest of that line as keys (:235). OBS reads each of these inputs as `[Twitch] Token=S`, and IniScan keeps `S`:
- `[Twitch] ;main\nToken=S\n`: no section is recognised, so the result is `exact`.
- `[Twitch\nToken=S\n`: same. The tests worker already flagged this (18-pr2-tests-claude.md:56-57) and it is unresolved.
- `[Twitch]\rToken=S\n`: the key becomes `[Twitch]\rToken`, so `exact`.
- `[Twitch]\n\rToken=S\n`: the key becomes `\rToken`, so `exact`.
- `[Twitch]]\nToken=S\n`: the section becomes `Twitch]`; it counts as unknown, but the token is kept.

Fix: copy the libobs parser rules, or fail closed (treat the file as invalid) on a CR not followed by LF and on any `[` line that is not `[name]` with optional trailing blanks. Add these five inputs as cases.

**claude-2 (Major): media-source `input` credentials are kept when OBS uses them.** Redactor.cpp:496 applies the URL rule to `input` only when `is_local_file` is the literal `false`.
- OBS reads the flag with `obs_data_get_bool` (obs-ffmpeg-source.c:396). That returns false for any non-boolean value: `item_valid(item, OBS_DATA_BOOLEAN) && ... ? *data : false` (obs-data.c:1712).
- The default `true` (obs-ffmpeg-source.c:119) is not applied over a value of the wrong type: `if (*item && (*item)->type != type) return;` (obs-data.c:923-924).
- So `0`, `null`, `"false"`, `1` and `"true"` all make OBS open `input` (:413-422).
- Cases 855-857 require these credentials to survive; for example ffmpeg-local-zero.json:20 has `rtmp://PLAINvalue-fl-1:PLAINvalue-fl-2@…?key=PLAINvalue-fl-3`.

Fix: always apply the URL rule to `input` (a local path has no `scheme://`, so it is untouched), and update the contract and those three cases.

**claude-3 (Major): memory and time are not bounded by the §12 limits.** §12:511-512 says limits are checked before the allocation they bound, and :523 says the values limit "bounds parser memory".
- **Memory:** every value, member and string keeps a full copy of its pointer (`value.pointer = pointer;` JsonScan.cpp:319; also :452, :491-492, :527, :590).
  - `{"<16 MiB key>":[0,…×10^6]}` is 18 MiB and inside every limit, but needs about 10^6 × 16 MiB, i.e. terabytes. A 4 KiB key in a ~2 MB file needs about 4 GB.
  - IniScan copies the section name into every key and pointer (IniScan.cpp:149, :165-166), and INI has no key-count limit. One 1 MiB `[…]` line plus about 2×10^7 `a=` lines (64 MiB total) needs about 4×10^13 bytes.
- **Time:**
  - Each lookup scans all values (`FindValues`/`FindFirstValue`, Redactor.cpp:91-110). That happens for every source (:482, :492-500), and for every VLC source with a heap `substr` per value (:504-507). Cost grows with sources × values.
  - Inside one string, each URL scans to the string's start or end (:267, :271-273), each query parameter scans to the end (:281), and each boundary mapping scans to the end of the document (:177-178).
  - A 16 MiB `server` value of `x://h/?a&a…` or `x://a://a://…` runs for hours.

Fix: store pointers as parent index plus token, build one pointer→value index, make URL scanning a single pass, and add a total budget. Add these inputs to the generated cases with a time bound.

**claude-4 (Major): every real `modules.json` is marked `parse:"invalid"`.** Redactor.cpp:666 requires a top-level object for all JSON roles. OBS writes and reads an array (`nlohmann::json data = nlohmann::json::array();`, PluginManager.cpp:194, read at :140). §7:293 says this role is `exact`. As written, every user gets an `opaque-unreviewed` entry that blocks export until decided (§6:270). The tests worker flagged it (18-pr2-tests-claude.md:36), and case 884 accepts either result. Fix: accept a top-level array for `app.modules` and make case 884 expect one result.

**claude-5 (Minor): a valid file can return `ERR E_JSON`.** For `"https://h/?token=https://u:p@x/"` the URL rule produces two overlapping ranges, [17,31) and [25,28). The overlap check (`unique.back().report.end > edit.report.start`, Redactor.cpp:601-602) then fails the whole call with E_JSON (:687-690). Nothing leaks, but the contract says the file should be stored. Fix: merge overlapping URL ranges.

**claude-6 (Minor): the collection rule only covers where OBS writes sources, not everywhere it loads them.**
- OBS picks the source type from `versioned_id`, and uses `id` only when that is empty (obs.c:2264, 2281-2282, 2293).
- OBS also loads items in `groups` as sources (OBSBasic_SceneCollections.cpp:1223, 1270-1274), and the six audio-device objects through `obs_load_source` (:1014, :1259-1264).
- The redactor looks only at `/sources/<i>/id` (Redactor.cpp:474, 481-482).

This needs a crafted file, and collections are always export-gated. Fix: match `id` or `versioned_id`, and cover `groups` and the audio-device keys.

**claude-7 (Minor): a JSON file with a BOM is omitted although OBS loads it.** OBS strips a leading BOM (`os_fread_utf8`, platform.c:187-192). §12:541 rejects a BOM only in a manifest, but the contract (lines 9, 57) and JsonScan.cpp:623-627 reject it in documents. A `service.json`, collection or websocket `config.json` with a BOM is therefore omitted. Fix: skip one leading BOM in documents (offsets unchanged), or record why not.

**claude-8 (Minor): docs and contract disagree in four places.**
- WORKSPACE_FORMAT.md:98 says that from PR2 the validator checks `target.audio` against the blob; PR2 does not.
- §7:288 and §8:389 say unknown `user.ini` sections (and unknown keys) make the file `opaque-unreviewed`. The contract (lines 93-94) and Redactor.cpp:569-578 check only ExtraBrowserDocks, so plugin data in `user.ini` would be `exact` and exported without a decision. Either implement it or record it as PR3 work in §13.
- §7:295-296 lists the decklink and AJA roles, but `IsRedactRole` rejects them (Redactor.cpp:35-46).
- The contract (lines 66-69) says a non-string registry value adds 1 to the unreviewed count; for collections the code discards it (`ignored_unreviewed`, Redactor.cpp:479, 519). Case 866 accepts both; it should expect one.

FINDING claude-1 | Major | open | IniScan line/header rules differ from config-file.c; OBS-live [Twitch]/[YouTube]/[Restream] tokens stay in OUTFILE (5 repros)
FINDING claude-2 | Major | open | input redacted only for literal false; OBS plays input for any non-boolean is_local_file; cases 855-857 keep the credentials
FINDING claude-3 | Major | open | per-value pointer/section copies and repeated full scans: in-limit inputs need terabytes of memory or hours of CPU
FINDING claude-4 | Major | open | app.modules requires an object but OBS writes an array; every real modules.json becomes parse invalid / opaque-unreviewed
FINDING claude-5 | Minor | open | overlapping URL ranges make ApplyEdits fail, so a valid file returns ERR E_JSON
FINDING claude-6 | Minor | open | collection rule ignores versioned_id, groups and the audio-device keys that OBS loads as sources
FINDING claude-7 | Minor | open | BOM rejected in JSON documents although OBS strips it; §12 rejects BOM only in manifests
FINDING claude-8 | Minor | open | doc/contract drift: §4:98 audio check, user.ini u1 rules, decklink/AJA roles, collection non-string count
VERDICT: request changes
