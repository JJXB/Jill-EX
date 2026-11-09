"""List the text pages stored in a .vcl sampled-sound bank.

The bank header is: 50 x u32 sound offsets (0x000), 50 x u16 sizes (0x0C8),
50 x u16 rates (0x12C), 40 x u32 page offsets (0x190), 40 x u16 page sizes
(0x230).  Page numbers are the 0-based index in that last table.
"""
import struct
import sys


def main():
    for path in sys.argv[1:]:
        data = open(path, "rb").read()
        print("==", path, len(data), "bytes")
        if len(data) < 0x280:
            print("   too small for a page table")
            continue
        for n in range(40):
            off = struct.unpack_from("<I", data, 0x190 + 4 * n)[0]
            size = struct.unpack_from("<H", data, 0x230 + 2 * n)[0]
            if not size:
                continue
            head = data[off:off + 40].split(b"\r")[0]
            head = head[1:] if head[:1].isdigit() else head
            print("   page %2d  off=%#08x size=%4d  %r"
                  % (n, off, size, head.decode("latin1")))


if __name__ == "__main__":
    main()
