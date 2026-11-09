import re

SRC = open(r"C:\Users\jjxb_\GIT\Jill-EX\re\export\decomp\1644_0002_FUN_1644_0002.c").read()
rows = {int(m.group(1), 16): m.group(2) for m in re.finditer(r"DAT_2cff_([0-9a-f]{4}) = ([^;]+);", SRC)}
exe = open(r"D:\Emu\DOS\C\jillt\jill1.exe", "rb").read()


def cstr(off):
    p = 0x1EBF0 + off
    e = exe.find(b"\0", p)
    return exe[p:e].decode("latin1")


def raw(a):
    v = rows.get(a)
    if v is None:
        return None
    v = v.strip()
    if v == "ds":
        return "ds"
    m = re.match(r"^0x([0-9a-f]+)$", v)
    if m:
        return int(m.group(1), 16)
    m = re.match(r"^(\d+)$", v)
    if m:
        return int(m.group(1))
    m = re.search(r"_2cff_([0-9a-f]{4})(?:\s*\+\s*(\d+))?", v)
    if m:
        return int(m.group(1), 16) + (int(m.group(2)) if m.group(2) else 0)
    return v


funcs = set()
for line in open(r"C:\Users\jjxb_\GIT\Jill-EX\re\export\functions.txt"):
    m = re.match(r"([0-9a-f]{4}):([0-9a-f]{4}) (FUN_\S+)", line)
    if m:
        funcs.add((int(m.group(1), 16), int(m.group(2), 16)))

print("| kind | name | handler | w | h | flags | tileset | cnt |")
print("|--|--|--|--|--|--|--|--|")
miss = 0
kinds = []
for k in range(64):
    off = raw(0xB84E + 4 * k)
    seg = raw(0xB850 + 4 * k)
    if isinstance(seg, int):
        pass
    w = raw(0xB6B6 + 2 * k)
    h = raw(0xB740 + 2 * k)
    nm = cstr(raw(0xB99C + 4 * k))
    fl = raw(0xBE3C + 2 * k)
    ts = raw(0xBB3E + 2 * k)
    cnt = raw(0xBBC8 + 2 * k)
    ok = (seg, off) in funcs
    miss += 0 if ok else 1
    kinds.append((k, nm, seg, off, w, h, fl, ts, cnt, ok))
    print("| %d | %s | %04x:%04x%s | %s | %s | 0x%04x | 0x%02x | %s |" %
          (k, nm, seg, off, "" if ok else " ?", w, h, fl, ts, cnt))
print("unmatched handlers:", miss)
