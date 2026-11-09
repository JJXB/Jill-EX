import glob, struct, os, collections

for f in sorted(glob.glob(r"D:\Emu\DOS\C\jillt\*.jn1")):
    d = open(f, "rb").read()
    cnt = struct.unpack("<H", d[0x4000:0x4002])[0]
    p = 0x4002 + cnt * 31
    w = struct.unpack("<35H", d[p:p + 0x46])
    tiles = struct.unpack("<8192H", d[:0x4000])
    freq = collections.Counter(t & 0x3fff for t in tiles)
    common = [t for t, _ in freq.most_common(40)]
    hdr = list(w)
    # count how many of the 32 words from w3..w34 are in the top-8 common tiles
    vals = hdr[3:35]
    hit = sum(1 for v in vals if v & 0x3fff in common[:8])
    print(os.path.basename(f).ljust(11), "vals in top8:", hit, "/32", " top tiles:",
          [hex(t) for t in common[:6]])
    print("   hdr :", " ".join("%04x" % v for v in vals))
    print("   freq:", [("%04x:%d" % (t, freq[t])) for t in common[:8]])
