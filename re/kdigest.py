"""Digest every object-kind handler for the port notes.

For each kind prints:
  - handler address and decompiled line count
  - the object fields it reads/writes (obj = DGROUP:0x96AC + i*31)
  - all callees with call counts
  - the constants it compares the state byte (+0x0d) and +0x13 against
  - string literals it references
Output is meant to be read by a human, not parsed.
"""

import os
import re
import subprocess

HERE = os.path.dirname(os.path.abspath(__file__))
DEC = os.path.join(HERE, "export", "decomp")


def kinds():
    out = subprocess.run(["python", os.path.join(HERE, "kindtable.py")],
                         capture_output=True, text=True).stdout
    rows = []
    for line in out.splitlines():
        if line.startswith("| ") and ":" in line:
            p = [x.strip() for x in line.strip("|").split("|")]
            if len(p) >= 8 and p[0].isdigit():
                rows.append(p)
    return rows


FIELDS = ["0x01 x", "0x03 y", "0x05 dx", "0x07 dy", "0x09 w", "0x0b h",
          "0x0d state", "0x0e", "0x0f", "0x11", "0x13 param", "0x15 flags",
          "0x17 str", "0x19 strseg", "0x1b", "0x1d ttl"]


def digest(k, name, addr, w, h, flags, tileset, cnt):
    seg, off = addr.split(":")
    path = os.path.join(DEC, "%s_%s_FUN_%s_%s.c" % (seg, off, seg, off))
    if not os.path.exists(path):
        return "%2d %-10s %s  <no decomp>" % (k, name, addr)
    src = open(path, encoding="utf-8", errors="replace").read()
    lines = src.splitlines()
    body = src
    calls = {}
    for m in re.finditer(r"\b(FUN_[0-9a-f]{4}_[0-9a-f]{4})", body):
        calls[m.group(1)] = calls.get(m.group(1), 0) + 1
    calls.pop("FUN_%s_%s" % (seg, off), None)
    # object field accesses: [bx+0xNN] / [bx-0xNN] in the decomp are sparse,
    # so also look at the raw pattern "0x1f" multiply then offsets
    fields = sorted(set(re.findall(r"\[\s*bx\s*([+-])0x([0-9a-f]{2})\s*\]", body)))
    cmps = sorted(set(re.findall(r"== (0x[0-9a-f]+)", body)))
    strs = sorted(set(re.findall(r"s_([A-Za-z0-9 _.,!?'-]{3,40})_2cff_", body)))
    out = []
    out.append("%2d %-10s %s w=%s h=%s flags=%s ts=%s cnt=%s lines=%d" %
               (k, name, addr, w, h, flags, tileset, cnt, len(lines)))
    if calls:
        out.append("    calls: " + ", ".join("%s x%d" % (c, n) if n > 1 else c
                                             for c, n in sorted(calls.items(), key=lambda x: -x[1])))
    if fields:
        out.append("    [bx..]: " + " ".join("%s0x%s" % f for f in fields))
    if strs:
        out.append("    strings: " + " ; ".join(strs[:6]))
    return "\n".join(out)


if __name__ == "__main__":
    out = [digest(int(p[0]), p[1], p[2].rstrip(" ?"), p[3], p[4], p[5], p[6], p[7])
           for p in kinds()]
    with open(os.path.join(HERE, "kdigest.txt"), "w", encoding="utf-8") as fh:
        fh.write("\n".join(out) + "\n")
