Codex findings 1–11 are fixed and accepted. A18 still contains three Major attribution errors.

FINDING codex-1 | Major | accepted-fix | Balance uses the latest-value slot (tasks/006-control-protocols.md:251-255)
FINDING codex-2 | Major | accepted-fix | A12 makes bindings local-only and export-omitted (tasks/006-control-protocols.md:200-205)
FINDING codex-3 | Major | accepted-fix | A5 defines TargetState reads and feedback (tasks/006-control-protocols.md:145-150)
FINDING codex-4 | Major | accepted-fix | A15 defines target paths and profile switching (tasks/006-control-protocols.md:227-247)
FINDING codex-5 | Major | accepted-fix | A1 defines principals and capability checks (tasks/006-control-protocols.md:117-126)
FINDING codex-6 | Major | accepted-fix | A6 defines queue isolation and overflow handling (tasks/006-control-protocols.md:151-159)
FINDING codex-7 | Minor | accepted-fix | A17 distinguishes balance reads from setter writes (tasks/006-control-protocols.md:251-255)
FINDING codex-8 | Minor | accepted-fix | A16 defines per-writer absolute retention (tasks/006-control-protocols.md:248-250)
FINDING codex-9 | Major | accepted-fix | A18 now records “Codex accepted A12” (tasks/006-control-protocols.md:272-275)
FINDING codex-10 | Minor | accepted-fix | A14 matches Host to the bound address or configured name (tasks/006-control-protocols.md:218-226)
FINDING codex-11 | Minor | accepted-fix | A13 adds nonce challenge-response, TLS/loopback, and replay requirements (tasks/006-control-protocols.md:208-217)
FINDING codex-12 | Major | open | A18 mislabels the numbered items: it quotes “Items 1-4” for loops and “Items 5-7 and 9” for targets, but targets are item 2 and loops item 5 (tasks/006-control-protocols.md:38-73,264-271)
FINDING codex-13 | Major | open | A18 says “the debate had Claude and Codex want bindings outside” but Claude proposed bindings “captured opaque-unreviewed”; only Codex proposed local state (02-k1-design-debate.md:144-154,266-280; tasks/006-control-protocols.md:272-275)
FINDING codex-14 | Major | open | A18 says A17 was “open ... until this head,” but Codex marked it “accepted-fix” in review-04/04-r2-codex.reply.md:7
VERDICT: request changes