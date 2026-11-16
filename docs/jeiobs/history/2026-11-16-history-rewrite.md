# History rewrite — 2026-11-16

Owner instruction (2026-11-16, verbatim, given in the chat after the lead
reported the problem):

> it might be destructive but do clean these up and do neccesary repo repairs if
> needed

Nine fork commits on public `main` recorded the
maintainer's corporate email and full name as **committer** (author was the fork
identity). They came from `git cherry-pick` and `git rebase` runs that did not set
the committer identity (VST3 backport cherry-picks, task 003 test cherry-picks, the
help-links rebase).

## What was done

1. Full mirror backup of `auto44e/jeiobs` (all heads, tags and pull refs) taken
   first and kept outside the repository.
2. Rewrote only the affected commits and their descendants: **37 commits**, from
   the boundary `5824b0430` (merge of #7) and `61f3ad953`. Committer name and email
   became `auto44e <auto44e@gmail.com>`. Trees, authors, author and committer
   dates and messages are unchanged, except that hash references inside commit
   messages were updated to the new SHAs. Upstream OBS history, all tags and all
   earlier fork commits keep their hashes and signatures.
   - A first attempt rewrote the whole repository (filter-repo drops GPG
     signatures, so 12,915 commits changed, including upstream history and tags).
     It was caught before pushing and discarded.
3. Force-pushed `main` and `jeiobs/decision-records` (PR #18) with leases pinned
   to the checked SHAs. Deleted eight merged branches that still held the old
   commits: `building-fixes`, `expanded-implementation-roadmap`, `help-links`,
   `identity-ui` (its only unique commit was the #11 merge, re-landed by #13),
   `local-logs`, `vst3-runtime-check`, `vst3-scanner-robustness`, `vst3-upstream`.
4. Verified from a fresh clone that no branch carries the address, that `main`'s
   tree is byte-identical to before (`b76d7cdb1`), and that the commit count is
   unchanged (15,864).
5. Docs that cite SHAs of rewritten commits were updated. Debate transcripts were
   left verbatim; use the table below to translate.

## Not fixed by the rewrite

GitHub keeps `refs/pull/<n>/head` for every PR. Those refs still point at the old
commits, so old SHAs remain viewable on GitHub until GitHub Support removes them.
That request has to come from the owner's account.

## Prevention

- The maintainer's local clone sets a repo-local identity, and a local `pre-push`
  hook refuses commits whose author or committer is not the fork identity.
- Agent rule (repo memory): cherry-pick, rebase and merge must set the committer
  identity explicitly, and `git log origin/main..HEAD --format='%ae %ce'` is
  checked before every push.

## Old → new SHAs (rewritten commits)

| Old | New | Subject |
| --- | --- | --- |
| `0171f16ef` | `b76d7cdb1` | Merge pull request #17 from auto44e/jeiobs/expanded-implementation-roadmap |
| `03a4f8f6e` | `49ff8beda` | CI: Update build setup scripts for VST3 |
| `0e63cf295` | `b3befabd8` | obs-vst3: log which bundles the scanner skipped |
| `12a8c00c5` | `ca85a570f` | obs-vst3: final review fixes for the scanner probes |
| `12f50216e` | `c85d62a32` | build-aux: Disable obs-vst3 for flatpak |
| `1cbbb82a4` | `979130776` | Merge pull request #12 from auto44e/jeiobs/help-links |
| `1e0de02dd` | `ae12ad510` | jeiobs: isolate VST3 scanner probes |
| `1e8aab17c` | `c1b2b2419` | Merge pull request #13 from auto44e/jeiobs/local-logs |
| `1fb405af3` | `99c722d4f` | Merge pull request #9 from auto44e/jeiobs/vst3-runtime-check |
| `1fcbce812` | `4f953e9c7` | frontend: never overwrite an earlier saved log copy |
| `2dbad5000` | `dd2078b1e` | Merge pull request #16 from auto44e/jeiobs/vst3-scanner-robustness |
| `33b31a6da` | `65d59aba2` | frontend: name the app JeiOBS in titles, version and English text |
| `372aec699` | `27a3f4b82` | tools: keep machine audio out of the VST3 check; record AGain cause |
| `46918a2ef` | `f687a650d` | docs: task 003 round 3 (bounded probe wait, 16 checks) |
| `49ad11215` | `53c8bcfaf` | cmake: Add VST3 SDK find module |
| `55dc7db76` | `f4e2e6d35` | docs: tighten the self-merge gate after peer review of #18 |
| `5a136e893` | `d9513ec62` | Merge pull request #15 from auto44e/jeiobs/building-fixes |
| `692b096fa` | `1e85e0787` | libobs: cap os_process_pipe_wait polling at 10 ms |
| `6cf380218` | `caeb38f3f` | frontend: save log copies locally instead of uploading them |
| `6f3842e6c` | `29baac073` | tools: catch a frozen probe that only the scanner can end (task 003) |
| `7302aa17c` | `b3db38c6f` | cmake: Fix cmake display of unsupported modules |
| `73b9c2190` | `6ad152a95` | tools: pin exact VST3 scanner results, close vacuous passes (task 003) |
| `76d6528db` | `3b6ef3699` | tools: turn the VST3 scanner INFO lines into checks (task 003) |
| `80aca6b22` | `6fd4338a9` | jeiobs: Record VST3 backport evidence and fetch the VST SDK |
| `80d0caa03` | `3998893fc` | frontend: point Help links at the fork and turn off What's New |
| `8c382452d` | `a88ba0bbd` | Merge pull request #8 from auto44e/jeiobs/vst3-upstream |
| `8edfb74f6` | `d1de79dd6` | obs-vst3: bound scanner probe cleanup |
| `a50fa54a1` | `0664eb7c0` | obs-vst3: Add a VST3 host |
| `bc21e3d9e` | `4ddb8166b` | docs: Expand full saving and audio roadmap |
| `be2bdff9e` | `0758c7830` | obs-vst3: harden scanner probes for Windows and review findings |
| `c0132655a` | `76f961384` | tools: add reproducible VST3 runtime check (task 001) |
| `c9cbb591e` | `227529cb8` | docs: fix the Linux build steps found broken in a clean room |
| `ca71629bb` | `0ac9d145d` | obs-vst3: do not warn about missing optional VST3 folders |
| `e3c72a9f5` | `f72d4ef3f` | docs: adopt JeiOS decision records; lead may merge after peer review |
| `e5d433625` | `5948e0474` | tools: require the invalid-bundle skip line; less timing-fragile hang checks |
| `e60d220b7` | `e702dfe7a` | Merge pull request #14 from auto44e/jeiobs/vst3-runtime-check |
| `f9d84f22e` | `d03fb6538` | Merge pull request #10 from auto44e/jeiobs/identity-ui |
