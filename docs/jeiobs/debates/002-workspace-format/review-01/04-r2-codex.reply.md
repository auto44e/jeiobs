FINDING codex-1 | Major | open | Export include dereferences optional `unreviewed` data accepted by validation (Manifest.cpp:504-545,1056-1061).
FINDING codex-2 | Major | open | Decisions input uses unrestricted `resize`/`Json::parse`, bypassing limits (jw-check.cpp:56-74,212-223).
FINDING codex-3 | Major | open | `CON .txt` passes device-name validation despite Windows trimming spaces (PathRules.cpp:96-103,193-207; WORKSPACE_FORMAT.md:624-625).
FINDING codex-4 | Minor | open | `PrintError` writes export errors to stdout despite the contract requiring nothing there (jw-check.cpp:32-38; 04-pr1-contract.md:45-46).
FINDING codex-5 | Minor | open | `CarryOver` copies parent extensions without rechecking output limits, so valid inputs can produce an unreadable child (Manifest.cpp:871-875,893-900;821-824).
VERDICT: request changes