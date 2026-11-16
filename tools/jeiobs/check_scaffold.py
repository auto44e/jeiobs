#!/usr/bin/env python3
"""Cheap repository checks; not a runtime, license-law or isolation audit."""
import hashlib
import json
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[2]
errors = []

def require(condition, message):
    if not condition:
        errors.append(message)

pin = json.loads((ROOT / 'docs/jeiobs/upstream.json').read_text())
require(bool(re.fullmatch(r'[0-9a-f]{40}', pin['commit'])), 'Invalid upstream commit')
require(hashlib.sha256((ROOT / 'COPYING').read_bytes()).hexdigest() == pin['copying_sha256'],
        'COPYING changed: preserve upstream license text')
for path in [ROOT / 'README.md', ROOT / 'AGENTS.md', *ROOT.glob('docs/jeiobs/**/*.md')]:
    text = path.read_text()
    # Links inside fenced code blocks (e.g. verbatim diffs in debate
    # transcripts) are not navigable, so they are not checked.
    text = re.sub(r'^(`{3,}|~{3,}).*?^\1[ \t]*$', '', text, flags=re.M | re.S)
    for target in re.findall(r'\[[^\]]*\]\(([^)]+)\)', text):
        if '://' in target or target.startswith('#'):
            continue
        require((path.parent / target.split('#')[0]).exists(), f'{path}: broken link {target}')
# This narrow checker intentionally covers only the inherited workflow shape.
# Unknown shapes must fail closed and receive review, not be silently accepted.
for name in ['push', 'pr-pull', 'publish', 'scheduled', 'dispatch', 'sign-windows']:
    text = (ROOT / f'.github/workflows/{name}.yaml').read_text()
    jobs = text.split('jobs:\n', 1)[1]
    chunks = re.split(r'(?=^  [\w-]+:\n)', jobs, flags=re.M)
    for chunk in filter(str.strip, chunks):
        require(bool(re.search(r"^    if: github\.repository == 'obsproject/obs-studio'(?: && \(.+\))?$", chunk, re.M)),
                f'{name}: unguarded job {chunk.splitlines()[0]}')
build = (ROOT / '.github/workflows/build-project.yaml').read_text()
require('if [[ "${GITHUB_REPOSITORY}" != "obsproject/obs-studio" ]]; then' in build,
        'Missing fork unsigned override')
wrapper = (ROOT / '.github/workflows/jeiobs-build.yaml').read_text()
require('secrets:' not in wrapper, 'Manual wrapper must not pass secrets')
require('workflow_dispatch:' in wrapper and 'pull_request:' not in wrapper,
        'Full matrix must remain explicitly requested during scaffold phase')
# Fork-identity notices: inherited upstream documents must keep their JeiOBS
# banner, and SECURITY.md must never route JeiOBS reports to the OBS Project.
MARK = 'JeiOBS (based on OBS Studio)'
for rel in ['README.md', 'README.rst', 'CONTRIBUTING.md', 'COC.rst', 'SECURITY.md',
            'docs/sphinx/index.rst', 'docs/jeiobs/NOTICE.md']:
    head = '\n'.join((ROOT / rel).read_text().splitlines()[:30])
    require(MARK in head, f'{rel}: missing fork banner "{MARK}" near the top')
notice = (ROOT / 'docs/jeiobs/NOTICE.md').read_text()
require('Use at your own risk' in notice and 'without warranty' in notice,
        'NOTICE.md: missing own-risk / no-warranty statement')
security = (ROOT / 'SECURITY.md').read_text()
require('security@obsproject.com' not in security and 'obsproject.com/discord' not in security,
        'SECURITY.md: must not route JeiOBS reports to OBS Project channels')
require('github.com/auto44e/jeiobs/security/advisories/new' in security,
        'SECURITY.md: missing private reporting link')
for message in errors:
    print(f'FAIL: {message}', file=sys.stderr)
if errors:
    sys.exit(1)
print('PASS: upstream pin shape, COPYING integrity, local doc links, scaffold workflow guards and fork notices')
