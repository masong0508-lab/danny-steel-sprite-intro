#!/usr/bin/env python3
"""Check in-game dialogue length, three-field formatting and dummy filler."""
from pathlib import Path
import re

source = Path('src/story.c').read_text()
blocks = re.findall(r'^SCRIPT\((\w+),\s*(.*?)\);', source, flags=re.M | re.S)
assert len(blocks) >= 75, f'expected 75+ authored scenes: {len(blocks)}'
quoted = r'"((?:[^"\\]|\\.)*)"'
page_pattern = re.compile(r'\{\s*' + quoted + r'\s*,\s*' + quoted +
                          r'\s*,\s*' + quoted + r'\s*\}')
pages = 0
for name, body in blocks:
    lines = page_pattern.findall(body)
    assert lines and len(lines) == body.count('{'), (name, len(lines), body.count('{'))
    for speaker, line1, line2 in lines:
        pages += 1
        assert 0 < len(speaker) <= 22, (name, speaker)
        for line in (line1, line2):
            line = line.replace('\\\'', "'").replace('\\"', '"')
            assert len(line) <= 24, (name, line, len(line))
            assert not re.search(r'(?i)\b(?:TODO|TBD|PLACEHOLDER|LOREM IPSUM)\b', line), (name,line)
assert pages >= 300, pages
print(f'Script quality: {len(blocks)} authored scenes, {pages} dialogue pages, no formatting or placeholder errors')
