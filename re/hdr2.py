import glob, struct, os

def hdr(path):
    d = open(path, "rb").read()
    cnt = struct.unpack("<H", d[0x4000:0x4002])[0]
    p = 0x4002 + cnt * 31
    return cnt, struct.unpack("<35H", d[p:p + 0x46]), len(d), p

for f in sorted(glob.glob(r"D:\Emu\DOS\C\jillt\*.dem")) + sorted(
        glob.glob(r"D:\Emu\DOS\C\jillt\*.sav")) + sorted(
        glob.glob(r"D:\Emu\DOS\C\jillt\JN1SAVE*")):
    cnt, w, n, p = hdr(f)
    print(os.path.basename(f).ljust(12), "objs", cnt, "hdr@", hex(p),
          " ".join("%d:%04x" % (i, v) for i, v in enumerate(w)))
