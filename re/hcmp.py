"""Compare object-kind handler bodies across episodes.

Recursively disassembles each handler from its entry, following intra-module
branches, stopping at far returns.  Compares the mnemonic stream (so absolute
DGROUP offsets and moved module segments do not matter) and reports the first
difference.

The kind table lives in BSS; its contents come from the initializer function
(`c7 06 <dgroup_addr> <value>`), so handler pointers are parsed from there.

Usage: py -3 re/hcmp.py
"""
import struct
from capstone import Cs, CS_ARCH_X86, CS_MODE_16

# episode -> (image, initializer file offset, seg-array DGROUP addr, off-array addr)
IMGS = {
    "J1": ("re/img.bin",   0x6442, 0xB850, 0xB84E),
    "J2": ("re/img2.bin",  0x6717, 0xB58A, 0xB588),
    "J3": ("re/img3.bin",  0x6500, 0xB2EC, 0xB2EA),
}
TERM = {0xCB, 0xCA}          # retf / retf imm16


def load(ep):
    path, init, segbase, offbase = IMGS[ep]
    data = open(path, "rb").read()
    m = {}
    p = init
    while p < len(data) - 8:
        if data[p] in (0xC3, 0xCB):
            break
        if data[p:p + 2] == b"\xc7\x06":
            a = struct.unpack_from("<H", data, p + 2)[0]
            v = struct.unpack_from("<H", data, p + 4)[0]
            m[a] = v
            p += 6
            continue
        if data[p:p + 2] == b"\x8c\x1e":
            m[struct.unpack_from("<H", data, p + 2)[0]] = -1
            p += 4
            continue
        p += 1
    return data, m, segbase, offbase


def kind_entries(m, segbase, offbase, n=64):
    out = []
    for k in range(n):
        seg = m[segbase + 4 * k] + 0x1000
        off = m[offbase + 4 * k]
        out.append((seg, off))
    return out


def foff(seg, off):
    return (seg - 0x1000) * 16 + off


def body(data, seg, off, limit=4000):
    """Recursive sweep, returns list of (addr, mnemonic, op_str)."""
    md = Cs(CS_ARCH_X86, CS_MODE_16)
    start = foff(seg, off)
    todo, seen, insns = [start], set(), []
    while todo:
        a = todo.pop()
        while a not in seen and 0 <= a < len(data) - 8:
            seen.add(a)
            got = list(md.disasm(data[a:a + 16], a, 1))
            if not got:
                break
            i = got[0]
            insns.append((a, i.mnemonic, i.op_str))
            nxt = a + i.size
            if i.size == 1 and data[a] in TERM:
                break
            if i.mnemonic == "jmp":
                tgt = target(i)
                if tgt is not None:
                    todo.append(tgt)
                break
            if i.mnemonic.startswith("j") or i.mnemonic == "loop":
                tgt = target(i)
                if tgt is not None:
                    todo.append(tgt)
                a = nxt
                continue
            if i.mnemonic in ("call", "lcall"):
                a = nxt
                continue
            if i.mnemonic in ("ret", "hlt"):
                break
            a = nxt
            if len(insns) > limit:
                return insns
    insns.sort()
    return insns


def target(i):
    s = i.op_str.split(",")[-1].strip()
    try:
        if ":" in s:
            return None
        return int(s, 16)
    except ValueError:
        return None


def seq(insns):
    return [m for _, m, _ in insns]


def compare(epA, epB, k, n=64):
    dA, mA, sA, oA = load(epA)
    dB, mB, sB, oB = load(epB)
    a = kind_entries(mA, sA, oA, n)[k]
    b = kind_entries(mB, sB, oB, n)[k]
    ia, ib = body(dA, *a), body(dB, *b)
    sa, sb = seq(ia), seq(ib)
    if sa == sb:
        return True, len(sa), None
    for n, (x, y) in enumerate(zip(sa, sb)):
        if x != y:
            return False, min(len(sa), len(sb)), (n, x, y, len(sa), len(sb))
    return False, min(len(sa), len(sb)), ("len", len(sa), len(sb))


if __name__ == "__main__":
    import statistics
    for ep in ("J1", "J2", "J3"):
        d, m, s, o = load(ep)
        e = kind_entries(m, s, o)
        c = [len(body(d, *x)) for x in e]
        print("%s: handlers=%d insns total=%d min=%d med=%d max=%d" %
              (ep, len(e), sum(c), min(c), int(statistics.median(c)), max(c)))
    for epA, epB in (("J1", "J2"), ("J2", "J3"), ("J1", "J3")):
        print("== %s vs %s ==" % (epA, epB))
        same = diff = 0
        for k in range(64):
            ok, n, info = compare(epA, epB, k)
            if ok:
                same += 1
            else:
                diff += 1
                print("  kind %2d DIFF n=%s %s" % (k, n, info))
        print("  identical=%d different=%d" % (same, diff))
    # new kinds exist only in J2/J3
    dA, mA, sA, oA = load("J2")
    dB, mB, sB, oB = load("J3")
    ea = kind_entries(mA, sA, oA, 69)
    eb = kind_entries(mB, sB, oB, 69)
    for k in range(64, 69):
        ia, ib = body(dA, *ea[k]), body(dB, *eb[k])
        print("  kind %d %s insns %d/%d" %
              (k, "same" if seq(ia) == seq(ib) else "DIFF", len(ia), len(ib)))
