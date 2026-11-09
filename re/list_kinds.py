import re

s = open('port/src/gamedata.inc').read()
for tag in ('kKinds1', 'kKinds2', 'kKinds3'):
    m = re.search(tag + r'\[\] = \{(.*?)\n\};', s, re.S)
    print('===', tag)
    out = []
    for line in m.group(1).strip().splitlines():
        mm = re.match(r'\s*\{\s*(\d+),\s*"([^"]+)"', line)
        if mm:
            out.append(mm.group(1) + ' ' + mm.group(2))
    print(' | '.join(out))
