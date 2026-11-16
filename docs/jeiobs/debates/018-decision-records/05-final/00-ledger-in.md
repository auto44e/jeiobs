# Findings ledger — PR #18

Kept by the lead. Status "fixed" means the lead believes it is fixed; the finding
stays open until its finder accepts the fix or withdraws it in a later round
(DECISIONS gate item 4). Statuses below that say "withdrawn" or "accepted" quote
the finder's round-2 replies in 04-r2-crosstalk.md.

ID | severity | status | finder | finding (verbatim or close quote) | lead's fix
--- | --- | --- | --- | --- | ---
c1 | Blocker | withdrawn (codex r2) | codex | "A missing part … not a reason to block a merge" permits self-merge without the record | missing parts block a self-merge (DECISIONS)
c2 | Major | withdrawn (codex r2) | codex | gate permits one parallel round for docs; directive requires debate | two cross-talk rounds for every self-merge
c3 | Major | withdrawn as text; see N2 (codex r2) | codex | lead may reject every finding unilaterally | finder must withdraw; ledger; explicit FINDING lines
c4 | Major | withdrawn (codex r2) | codex | release/signing/installer PRs not excluded from self-merge | owner-only classes
c5 | Minor (lowered by codex) | fixed in 7e0d8f40c+1 | codex | CONTRIBUTING "maintainer"; residual CONTRIBUTING:29, ROADMAP:227 | both now say owner / point to DECISIONS
c6 | — | withdrawn (codex r2) | codex | transcripts keep `~/` and `/tmp` paths | policy states `~` and `/tmp` are allowed
c7 | Minor, owner decision (codex r2) | open for owner | codex | personal email `auto44e@gmail.com` in transcripts | it is the public commit identity; kept, owner may ask to remove
c8 | Minor | withdrawn (codex r2) | codex | D2 wording ("not stdout markers") | D2 rewritten
k1 | High→closed | withdrawn (claude r2) | claude | lead can reject any finding and merge; lead decides "inconclusive" | as c3
k2 | High→closed | withdrawn (claude r2) | claude | tests only need to be reported | tests must pass or owner accepts
k3 | High→closed | withdrawn (claude r2) | claude | lead picks own tier; policy/.github self-mergeable; FLEX self-amending | owner-only classes; FLEX owner-only
k4 | Medium | fixed; see N1 | claude | merged SHA must equal reviewed SHA | `--match-head-commit`; rewrite rule
k5 | Medium | fixed | claude | directive paraphrased; terms undefined; self-merges indistinguishable | verbatim quotes; terms; commit-message marker
k6 | Low | withdrawn (claude r2) | claude | four contradictions; missing verbatim prompts | fixed / disclosed
k7 | Medium | fixed | claude, codex | D2/D4/D5/D6 accuracy | records corrected
M1 | Major | fixed | claude (codex agreed) | dependency, pin, hash, submodule and `deps/` changes self-mergeable | owner-only class, file paths named
M2 | Major | fixed | claude (codex agreed) | finding closure not tracked verbatim per round; two severity scales | this ledger; one scale; explicit FINDING lines; review.sh relays each peer's own reply
M3 | Major | fixed | claude (codex agreed) | #18's own transcripts not committed | this folder
N1 | Major | fixed | claude (codex agreed) | head moved to 7e0d8f40c without review; second purpose | owner's verbatim instruction recorded; final round reviews the full new head
N2 | Major | fixed | claude (codex agreed) | debate script outside the repo, contradicts gate, Claude not sandboxed | `tools/jeiobs/peer-review/review.sh` (owner-only path): read-only peers, gate-consistent preamble, saves prompts, fails closed
N3 | Major | fixed | claude (codex agreed) | force-push/branch deletion not owner-only; approval not recorded verbatim | owner-only admin class; verbatim quotes in DECISIONS and the history note; AGENTS force-push exception
N4 | Major | fixed | claude (codex agreed) | save-format breaks, migrations, ABI/module-ID breaks not owner-decided | "owner-decided, then self-mergeable"
N5 | Medium | fixed | claude (codex agreed) | no owner-hold mechanism | `owner-hold` label or unanswered owner comment blocks
N6 | Major | fixed | codex | script persists replies, not prompts | review.sh saves every prompt
N7 | Major | fixed | codex | script fails open on peer failure | review.sh exits 3 and writes FAILED
k8 | Medium | fixed | claude | "no shared state" false; lead lineage undefined | terms corrected; lead-written code reviewed by both
k9 | Minor | fixed | claude | transcripts cite pre-rewrite SHAs without a pointer | README in each debate folder
k10 | Minor | deferred (separate PR) | claude | `linux-deps.sh` installs `aqtinstall` unpinned | dependency change, owner-only class: needs its own PR
CI | Error | fixed | owner | quick checks failed on main after the force push ("bad object") | workflow falls back to HEAD when the base commit no longer exists
