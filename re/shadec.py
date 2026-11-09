"""Decode Epic SHAPE (.SHA) banks used by Jill of the Jungle.

Derived from FUN_1069_0271 (segment 0x1069, real extent 0271..094c) and
FUN_1069_094d (_shm_do).  All multi-byte fields are little-endian.

Entry layout
------------
  +0   u8   frame count N (runtime decodes exactly N frames)
  +1   u16  always 1
  +3   u16  decoded block size, 2bpp (video modes 0/1)
  +5   u16  decoded block size, 4bpp (video modes 2/3)
  +7   u16  decoded block size, 8bpp (video mode 4)
  +9   u8   source bits per pixel: 1, 2, 3, 4 or 8
  +10  u16  flags: bit0 = colour table present / blitter mode,
                  bit2 = planar destination packing
  +12  colour table, present when flags&1 == 0 and bpp < 8:
         1<<bpp entries, 4 bytes each; the useful byte is (v >> shift) & mask
         where shift/mask depend on the video mode (see XSHIFT/XMASK).

  then N frame records:
     u8 w, u8 h, u8 opcode
       opcode 0: w*h raw 8-bit pixel bytes (byte aligned)
       opcode 1: w*h pixels of bpp bits, LSB first, bit pointer reset to 0
                 at the start of the payload
       opcode 2: empty frame, no payload
       opcode >2: treated like 2 (no payload)

  The shipped files always carry one extra record after the N decoded frames
  (parsed identically, typically opcode 0 with a stand-alone image).  The
  runtime never reads it: it allocates only N*(4 + packed frame bytes).
"""

import struct

# video mode -> (shift, mask) applied to a colour table entry
XSHIFT = {0: (0, 3), 1: (4, 3), 2: (8, 15), 3: (12, 15), 4: (16, 255)}


class Shape:
    def __init__(self):
        self.frames = []      # (w, h, opcode, raw pixel bytes)
        self.tail = None      # record after the N decoded frames
        self.bpp = 0
        self.flags = 0
        self.sizes = (0, 0, 0)
        self.table = []       # raw 4-byte colour table entries
        self.xlate = [0] * 256


def decode(data, mode=4):
    """mode = video backend id (0/1 = 2bpp, 2/3 = 4bpp, 4 = 8bpp)."""
    sh = Shape()
    if len(data) < 12:
        return sh
    n = data[0]
    sh.sizes = struct.unpack_from("<3H", data, 3)
    sh.bpp = data[9]
    sh.flags = struct.unpack_from("<H", data, 10)[0]
    pos = 12

    shift, mask = XSHIFT.get(mode, (16, 255))
    sh.xlate = [(i >> 0) & 0xFF for i in range(256)]
    if not (sh.flags & 1) and sh.bpp < 8:
        for i in range(1 << sh.bpp):
            v = struct.unpack_from("<I", data, pos)[0]
            pos += 4
            sh.table.append(v)
            sh.xlate[i] = (v >> shift) & mask

    bit = 0

    def getbits(nb):
        nonlocal pos, bit
        v = (struct.unpack_from("<H", data, pos)[0] >> bit) & ((1 << nb) - 1)
        bit += nb
        if bit >= 8:
            bit -= 8
            pos += 1
        return v

    def record():
        nonlocal pos, bit
        w, h, op = data[pos], data[pos + 1], data[pos + 2]
        pos += 3
        px = bytearray(w * h)
        if op == 0:
            px[:] = data[pos:pos + w * h]
            pos += w * h
            bit = 0
        elif op == 1:
            bit = 0
            for i in range(w * h):
                px[i] = getbits(sh.bpp)
        return (w, h, op, bytes(px))

    for _ in range(n):
        sh.frames.append(record())
    if pos + 3 <= len(data):
        sh.tail = record()
    sh.pos = pos
    return sh


def load(path):
    d = open(path, "rb").read()
    off = struct.unpack("<128I", d[:512])
    sz = struct.unpack("<128H", d[512:768])
    return d, off, sz


def pixels(sh, idx, mode=4):
    """Return (w, h, bytearray of palette indices) for one frame."""
    w, h, op, raw = sh.frames[idx]
    return w, h, bytearray(sh.xlate[p] for p in raw)


def vga_palette():
    """Default VGA 8-bit palette (6-bit DAC scaled to 8-bit)."""
    pal = []
    for i in range(256):
        r, g, b = i >> 5, (i >> 2) & 7, i & 3
        pal.append((r * 255 // 7, g * 255 // 7, b * 255 // 3))
    return pal


def sheet(sh, path, mode=4, scale=2, bg=(32, 32, 32)):
    """Write a contact sheet of every decoded frame as PNG."""
    from PIL import Image
    pal = vga_palette()
    cols = 8
    cw = max(f[0] for f in sh.frames) * scale + 2
    ch = max(f[1] for f in sh.frames) * scale + 2
    rows = (len(sh.frames) + cols - 1) // cols
    img = Image.new("RGB", (cols * cw, rows * ch), bg)
    px = img.load()
    for i in range(len(sh.frames)):
        w, h, data = pixels(sh, i, mode)
        ox, oy = (i % cols) * cw + 1, (i // cols) * ch + 1
        for y in range(h):
            for x in range(w):
                c = pal[data[y * w + x]]
                for dy in range(scale):
                    for dx in range(scale):
                        px[ox + x * scale + dx, oy + y * scale + dy] = c
    img.save(path)
    return path


if __name__ == "__main__":
    import sys
    d, off, sz = load(r"D:\Emu\DOS\C\jillt\jill1.sha")
    bad = 0
    for i in range(128):
        if not sz[i]:
            continue
        sh = decode(d[off[i]:off[i] + sz[i]])
        if sh.pos != sz[i] or len(sh.frames) != d[off[i]]:
            bad += 1
            print("MISMATCH", i, sh.pos, sz[i])
        n2 = sum(4 + ((f[0] + 3) // 4 * 4) * 2 // 8 * f[1] for f in sh.frames)
        if n2 != sh.sizes[0]:
            print("size2 differs", i, n2, sh.sizes[0])
    print("mismatches:", bad)
    for a in sys.argv[1:]:
        i = int(a)
        sh = decode(d[off[i]:off[i] + sz[i]])
        print(i, sheet(sh, r"C:\Users\jjxb_\GIT\Jill-EX\re\shape_%d.png" % i))
