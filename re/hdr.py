import glob, struct, os

FIELDS = ["level", "health", "keycnt"] + ["key%d" % i for i in range(15)] + ["pad"] + [
    "score_lo", "score_hi", "dmgflash", "sav_lo", "sav_hi"
]

for f in sorted(glob.glob(r"D:\Emu\DOS\C\jillt\*.jn1")):
    d = open(f, "rb").read()
    cnt = struct.unpack("<H", d[0x4000:0x4002])[0]
    p = 0x4002 + cnt * 31
    w = struct.unpack("<35H", d[p:p + 0x46])
    print(os.path.basename(f).ljust(11), " ".join("%d:%04x" % (i, v) for i, v in enumerate(w)))
