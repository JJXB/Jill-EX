import glob, struct, os

for f in sorted(glob.glob(r"D:\Emu\DOS\C\jillt\*.jn1")):
    d = open(f, "rb").read()
    cnt = struct.unpack("<H", d[0x4000:0x4002])[0]
    p = 0x4002 + cnt * 31
    named = [i for i in range(cnt)
             if d[0x4002 + i * 31 + 23:0x4002 + i * 31 + 27] != b"\0\0\0\0"]
    q = p + 0x46
    strs = []
    while q + 2 <= len(d):
        ln = struct.unpack("<H", d[q:q + 2])[0]
        q += 2
        if q + ln + 1 > len(d):
            strs.append("<<SHORT>>")
            break
        strs.append(d[q:q + ln].decode("latin1"))
        q += ln + 1
    kinds = [d[0x4002 + i * 31] for i in named]
    print(os.path.basename(f).ljust(11), "named", len(named), "strings", len(strs),
          "end", hex(q), "len", hex(len(d)), "EXACT" if q == len(d) and len(named) == len(strs) else "MISMATCH")
    if len(named) != len(strs):
        print("   kinds", kinds)
