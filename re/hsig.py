"""Behavioural signature of every kind handler, per episode.

For each handler we collect a signature that survives recompilation:
  * multiset of small immediate constants (gameplay thresholds, scores, sounds)
  * multiset of far-call target offsets (helper identity)
  * multiset of mnemonics
and then diff the signatures across episodes.

Far-call offsets are stable because each helper module keeps its internal
layout; only the module's load segment moves.
"""
import sys, collections
from capstone import Cs, CS_ARCH_X86, CS_MODE_16
from capstone.x86 import X86_OP_IMM, X86_OP_MEM
sys.path.insert(0, "re")
import hcmp


def sig(data, seg, off):
    md = Cs(CS_ARCH_X86, CS_MODE_16)
    md.detail = True
    start = hcmp.foff(seg, off)
    todo, seen = [start], set()
    imms = collections.Counter()
    calls = collections.Counter()
    mnem = collections.Counter()
    while todo:
        a = todo.pop()
        while a not in seen and 0 <= a < len(data) - 8:
            seen.add(a)
            got = list(md.disasm(data[a:a + 16], a, 1))
            if not got:
                break
            i = got[0]
            mnem[i.mnemonic] += 1
            nxt = a + i.size
            if i.size == 1 and data[a] in (0xCB, 0xCA):
                break
            rel = i.mnemonic.startswith("j") or i.mnemonic in ("call", "loop")
            if i.mnemonic == "lcall":
                for o in i.operands:
                    if o.type == X86_OP_IMM:
                        calls[o.imm & 0xFFFF] += 1
            elif i.mnemonic == "call":
                pass                      # near call: target moves with code
            else:
                for o in i.operands:
                    if o.type == X86_OP_IMM and not rel:
                        v = o.imm & 0xFFFF
                        if v < 0x1000 or v >= 0xFF00:
                            imms[v] += 1
            if i.mnemonic == "jmp":
                t = hcmp.target(i)
                if t is not None:
                    todo.append(t)
                break
            if i.mnemonic.startswith("j") or i.mnemonic == "loop":
                t = hcmp.target(i)
                if t is not None:
                    todo.append(t)
                a = nxt
                continue
            if i.mnemonic in ("call", "lcall"):
                a = nxt
                continue
            if i.mnemonic in ("ret", "hlt"):
                break
            a = nxt
    return imms, calls, mnem


def main():
    eps = {}
    for ep in ("J1", "J2", "J3"):
        d, m, s, o = hcmp.load(ep)
        eps[ep] = (d, hcmp.kind_entries(m, s, o, 69 if ep != "J1" else 64))
    for a, b in (("J1", "J2"), ("J2", "J3"), ("J1", "J3")):
        print("== %s vs %s ==" % (a, b))
        bad = []
        for k in range(64):
            ia, ca, ma = sig(eps[a][0], *eps[a][1][k])
            ib, cb, mb = sig(eps[b][0], *eps[b][1][k])
            diffs = []
            if ia != ib:
                diffs.append("imm " + str(sorted((ia - ib).items())) +
                             " / " + str(sorted((ib - ia).items())))
            if ca != cb:
                diffs.append("calls " + str(sorted((ca - cb).items())) +
                             " / " + str(sorted((cb - ca).items())))
            if diffs:
                bad.append((k, diffs, sum(ma.values()), sum(mb.values())))
        print("  signatures equal: %d / 64" % (64 - len(bad)))
        for k, diffs, na, nb in bad:
            print("  kind %2d insns %d/%d: %s" % (k, na, nb, "; ".join(diffs)))
    # new kinds
    for k in range(64, 69):
        ia, ca, ma = sig(eps["J2"][0], *eps["J2"][1][k])
        ib, cb, mb = sig(eps["J3"][0], *eps["J3"][1][k])
        print("new kind %d J2/J3 imm=%s calls=%s" %
              (k, ia == ib, ca == cb))


if __name__ == "__main__":
    main()
