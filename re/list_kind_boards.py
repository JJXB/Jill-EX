import glob
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import jill_extractor as j

wanted = set(int(x) for x in sys.argv[1:]) if len(sys.argv) > 1 else set(range(3, 64))
for ext in ('.jn1', '.jn2', '.jn3', '.one'):
    for f in sorted(glob.glob(os.path.join('D:/Emu/DOS/C/jillt', '*' + ext))):
        try:
            b = j.BoardFile.load(f)
        except Exception:
            continue
        kinds = {}
        for o in b.objects:
            if o.kind in wanted:
                kinds.setdefault(o.kind, []).append((o.x, o.y, o.state, o.primary_param, o.x_delta))
        if kinds:
            print(os.path.basename(f), {k: len(v) for k, v in sorted(kinds.items())})
            for k, v in sorted(kinds.items()):
                print('   ', k, v[:2])
