import struct

d = open(r"D:\Emu\DOS\C\jillt\jill1.sha", "rb").read()
off = struct.unpack("<128I", d[:512])
sz = struct.unpack("<128H", d[512:768])

print(" idx  off    size  hdr bytes")
for i in range(128):
    if sz[i] == 0:
        continue
    h = d[off[i]:off[i] + min(sz[i], 16)]
    b0 = h[0]
    w1, w3, w5, w7 = struct.unpack("<4H", h[1:9])
    b9 = h[9]
    w10 = struct.unpack("<H", h[10:12])[0]
    print("%3d %6x %5d  b0=%3d w1=%5d w3=%5d w5=%5d w7=%5d b9=%3d w10=%04x" %
          (i, off[i], sz[i], b0, w1, w3, w5, w7, b9, w10))
