I confirm Claude’s disputed findings:

- claude-3: confirmed; declared hotkey `effect` controls classification, so a wildcard can be labeled `scene` while targeting an output hotkey (`Mapping.cpp:742-760,1255-1265,1520-1525`; `docs/jeiobs/tasks/006-control-protocols.md:127-132`).
- claude-4: confirmed; section-8 fields such as `url`, `input`, `ffmpeg_options`, and playlist `value` are accepted, while the deny list covers only the narrower names (`Mapping.cpp:709-713,1513-1517`; `docs/jeiobs/WORKSPACE_FORMAT.md:350`).
- claude-5: confirmed; `ordered_json` inserts through a vector-backed object, making large unknown-key objects quadratic (`Mapping.cpp:34,129-148`; `Limits.hpp:17,31`).
- claude-7: confirmed as contract/progress drift; r1 requires strict fields/regex/escaping, but public parsing remains lax and requirements accept non-ASCII (`Mapping.cpp:332-350,1470-1486`; contract:115-119).
- claude-8: confirmed; cycle analysis assumes the emitted event retains the source slot, but `emit` carries only an address (`Mapping.cpp:812-818,1128-1132`; contract:85-90).
- claude-9: confirmed; merge validation is group-size quadratic and cycle construction stores edges to non-emit routes (`Mapping.cpp:1121-1135,1541-1561`; `Limits.hpp:33`).
- claude-10: confirmed; `jc-check` uses ANSI `main` argv on Windows while the sibling CLI converts UTF-16 to UTF-8 (`jc-check.cpp:411-424`; `jw-check.cpp:416-445`).

FINDING codex-1 | Major | open | Public ParseMapping/CompileMapping(bytes) use lax parsing: missing enabled/requires and empty addresses are accepted (`Mapping.cpp:878-912,1455,1470-1473`; contract:115-118).
FINDING codex-2 | Minor | open | IsRequirement accepts non-ASCII bytes, so `requires:["café"]` violates the required ASCII regex (`Mapping.cpp:332-350`; contract:115-116).
FINDING codex-3 | Major | open | Emit target keys are non-injective: `/` is unescaped and `\` is omitted, so distinct addresses collide (`Mapping.cpp:1041-1063,1324-1333,1378-1380`; contract:119).
FINDING codex-4 | Major | open | MakeInt masks invalid high bits after ineffective validation, and MakeFloatBits truncates oversized IEEE32 raw values (`Value.cpp:286-310,424-446`; contract:17-24).
VERDICT: request changes