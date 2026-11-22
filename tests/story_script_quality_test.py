#!/usr/bin/env python3
"""Verify script formatting and prevent placeholder dialogue regressions."""
from pathlib import Path
import re

source = Path('src/story.c').read_text()
blocks = re.findall(r'^SCRIPT\((\w+),\s*(.*?)\);', source, flags=re.M | re.S)
assert len(blocks) >= 75, f'expected authored script scenes: {len(blocks)}'
quoted = r'"((?:[^"\\]|\\.)*)"'
page_pattern = re.compile(r'\{\s*' + quoted + r'\s*,\s*' + quoted +
                          r'\s*,\s*' + quoted + r'\s*\}')
pages = 0
for name, body in blocks:
    lines = page_pattern.findall(body)
    assert lines and len(lines) == body.count('{'), (name, len(lines),body.count('{'))
    for speaker, line1, line2 in lines:
        pages += 1
        assert 0 < len(speaker) <= 22, (name, speaker)
        for line in (line1, line2):
            # Account for a source escape like the apostrophe in 'That's'.
            line = line.replace("\\'", "'").replace('\\"', '"')
            assert len(line) <= 24, (name, line, len(line))
            assert not re.search(r'(?i)\b(?:TODO|TBD|PLACEHOLDER|LOREM IPSUM)\b', line), (name, line)
assert pages >= 300, pages
print(f'script quality: {len(blocks)} scenes, {pages} pages, no overlong or placeholder dialogue')
