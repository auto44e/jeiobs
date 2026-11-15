# jeiobs

An independent OBS Studio fork for professional audio workflows.
Not affiliated with or endorsed by the OBS Project.

**Development scaffold, not a separately installable application yet.** The source
still uses OBS's runtime identity and configuration paths. Do not install or run
these builds against your normal OBS setup; use a disposable environment.

Baseline: OBS Studio **33.0.1**, commit
`65c320b46e1af5c7a555161aea6182cdf7a8c524`.
Upstream history and GPL-2.0-or-later notices are retained (see [COPYING](COPYING)
and the original [upstream README](README.rst)). This is not Apache-licensed JeiOS.

## Start here

- [Current handoff](docs/jeiobs/HANDOFF.md)
- [First coding task: upstream VST3 backport](docs/jeiobs/tasks/001-vst3.md)
- [Roadmap](docs/jeiobs/ROADMAP.md)
- [Contribution workflow](docs/jeiobs/CONTRIBUTING.md) and [agent instructions](AGENTS.md)
- [Build and CI guide](docs/jeiobs/BUILDING.md)
- [Coexistence audit](docs/jeiobs/COEXISTENCE.md)

Goals: independent installation and data, explicit OBS imports, master-bus audio
processing, modern audio plugin hosting, advanced routing and professional I/O.
These are planned capabilities, not claims about this scaffold.

Quick repository check: `python3 tools/jeiobs/check_scaffold.py`.
