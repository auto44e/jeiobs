# Fork notice, licensing and use at your own risk

**JeiOBS (based on OBS Studio)** is an independent fork of
[OBS Studio](https://github.com/obsproject/obs-studio). It is **not** OBS Studio,
it is **not** made, endorsed, supported or released by the OBS Project, and it is
**not** the original. The repository and technical identifier is `jeiobs`.
Baseline: OBS Studio 33.0.1, commit `65c320b46e1af5c7a555161aea6182cdf7a8c524`
(see [upstream.json](upstream.json)).

## Use at your own risk

JeiOBS is pre-release development software. It can crash, corrupt or lose
recordings, streams, scenes and settings, and its source still uses OBS's runtime
identity and configuration paths, so it can interfere with a normal OBS install.
Use it only in a disposable environment (see [COEXISTENCE.md](COEXISTENCE.md)).

It is provided without warranty of any kind, exactly as the GNU General Public
License provides (GPLv2 sections 11 and 12, in [COPYING](../../COPYING)). This
notice is a plain-language reminder. It adds no term, condition or restriction to
any license; if it ever conflicts with a license, the license controls.

## Licensing

- JeiOBS is distributed under the same terms as upstream: GNU General Public
  License, version 2 or (at your option) any later version. [COPYING](../../COPYING)
  is the unmodified upstream text; `tools/jeiobs/check_scaffold.py` fails if its
  hash changes.
- Upstream copyright and license headers in source files, and the
  [AUTHORS](../../AUTHORS) file, are retained. Upstream authorship is preserved in
  git history, including cherry-picks recorded with `-x`.
- Our own modifications to GPL-covered files are released under the same GPL terms.
  JeiOBS is not the Apache-2.0-licensed JeiOS kernel.
- Bundled third-party components keep their own licenses, in their own
  directories (for example `deps/*`, `libobs/graphics/libnsgif`,
  `plugins/obs-outputs/librtmp`, `plugins/obs-filters/rnnoise`,
  `plugins/decklink`, `shared/media-playback`). Any SDK added later gets a recorded
  provenance and license review first (see [task 001](tasks/001-vst3.md)).
- [COMMITMENT](../../COMMITMENT) is the GPL Cooperation Commitment made by upstream
  contributors. It is retained unchanged. Keeping it does not make any additional
  commitment on behalf of JeiOBS maintainers.

## Names and trademarks

"OBS", "OBS Studio" and the OBS logo belong to their respective owners. JeiOBS uses
the name only to say where the code comes from ("based on OBS Studio"), not to
suggest that the OBS Project backs it. Please do not describe JeiOBS as OBS or as
an official OBS build. This is a project convention, not legal advice.

## Where upstream text still appears

Several files and parts of the interface are inherited from upstream and still
describe or link to the OBS Project. They are not statements by JeiOBS:

- Repository documents: `README.rst`, `CONTRIBUTING.md`, `COC.rst` and
  `docs/sphinx` carry a fork banner but are otherwise upstream text.
  [SECURITY.md](../../SECURITY.md) is JeiOBS's own.
- Application code: the About dialog, Help links, update and crash-report
  behaviour still reference upstream services. Replacing or removing them is
  tracked under M0 in the [roadmap](ROADMAP.md); until then, do not treat any such
  link, update channel or donation page as belonging to JeiOBS, and do not send
  JeiOBS problems to the OBS Project.

## Reporting

- JeiOBS bugs: this repository's issues. JeiOBS vulnerabilities: see
  [SECURITY.md](../../SECURITY.md) (private reporting).
- If a problem also exists in upstream code, please tell upstream through its own
  channels as well.
