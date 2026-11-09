import os
import re
import sys

DEC = r"C:\Users\jjxb_\GIT\Jill-EX\re\export\decomp"
FIELD = {"96ad": "x", "96af": "y", "96b1": "dx", "96b3": "dy",
         "96b5": "w", "96b7": "h", "96b9": "state", "96bb": "p0f",
         "96bd": "p11", "96bf": "p13", "96c1": "flags15",
         "96c3": "s17", "96c5": "s19", "96c7": "p1b", "96c9": "ttl"}
DECL = re.compile(r"^\s*(int|uint|byte|bool|char|code|long|ulong|undefined\d?|"
                  r"void|size_t|ptrdiff_t|\w+ \*)\b[^=;]*;\s*$")


def condense(src):
    out = []
    for line in src.splitlines():
        s = line.strip()
        if not s or s.startswith("//") or s in "{}":
            continue
        if DECL.match(s):
            continue
        # object field accesses: <cast>&DAT_2cff_96xx + PARAM * 0x1f
        def rep(m):
            base = m.group("base")
            idx = m.group("idx")
            name = FIELD.get(base, "f" + base[-2:])
            return "O(%s).%s" % (idx, name)
        s = re.sub(r"\(?(?:int|uint|undefined1|undefined2|char|byte) ?\*?\)?"
                   r"(?:&|\(int\)&)?DAT_2cff_(?P<base>96[0-9a-f]{2})\s*\+\s*"
                   r"(?P<idx>\w+)\s*\*\s*0x1f", rep, s)
        s = re.sub(r"\(\(undefined1 \*\)&DAT_2cff_96ac\)\[(\w+) \* 0x1f\]",
                   r"kind(\1)", s)
        s = re.sub(r"\(\(undefined2 \*\)&DAT_2cff_b6b6\)\[", "kwidth[", s)
        s = re.sub(r"\(\(undefined2 \*\)&DAT_2cff_b740\)\[", "kheight[", s)
        s = re.sub(r"\(\(undefined2 \*\)&DAT_2cff_be3c\)\[", "kflags[", s)
        s = re.sub(r"\(\(undefined2 \*\)&DAT_2cff_bbc8\)\[", "kscore[", s)
        s = re.sub(r"\(\(undefined2 \*\)&DAT_2cff_43a6\)\[", "map[", s)
        s = re.sub(r"DAT_2cff_", "", s)
        s = re.sub(r"\s+", " ", s)
        out.append(s)
    return out


for f in sys.argv[1:]:
    seg, off = f.split("_")[1], f.split("_")[2]
    p = os.path.join(DEC, "%s_%s_%s.c" % (seg, off, f))
    if not os.path.exists(p):
        print("== %s MISSING" % f)
        continue
    lines = condense(open(p, encoding="utf-8", errors="replace").read())
    print("== %s (%d lines) %s" % (f, len(lines), lines[0]))
    for l in lines[1:]:
        print("  " + l[:170])
