# Unicode tables for the workspace collision key

`shared/jeiobs-workspace/src/UnicodeTables.inc` holds the Unicode data behind the path collision key
(`docs/jeiobs/WORKSPACE_FORMAT.md` section 12): canonical decompositions, canonical combining classes and
full case foldings. It is generated, never edited.

Pinned input: Unicode Character Database 18.0.0, with the sha256 of each file in `fetch-ucd.sh`.

```
tools/jeiobs/unicode/fetch-ucd.sh /tmp/ucd
tools/jeiobs/unicode/gen_tables.py /tmp/ucd shared/jeiobs-workspace/src/UnicodeTables.inc
cmake -S shared/jeiobs-workspace -B build -DJW_UCD_DIR=/tmp/ucd
cmake --build build && ctest --test-dir build -R jw-unicode-conformance
```

To move to a new Unicode version: change `UCD_VERSION` and the hashes in `fetch-ucd.sh`, regenerate,
run the conformance test and the case suite, and commit the script change and the generated file
together. The output is deterministic, so a clean regeneration must leave `git diff` empty.

The Unicode data is under the Unicode License v3 (`shared/jeiobs-workspace/third_party/unicode/LICENSE`).
