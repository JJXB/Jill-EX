"""Render a board file to a PNG (tile layer + optional objects).

Usage:
  py -3 re/render_board.py <data_dir> <board_file> <sha_file> <dma_file> <out.png>
                           [--x X --y Y --w 320 --h 200]
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import jill_extractor as j  # noqa: E402
from PIL import Image  # noqa: E402


def render(data_dir, board, sha_file, dma_file, out, cx=0, cy=0, w=320, h=200):
    dma = j.DMAFile.load(os.path.join(data_dir, dma_file))
    rec = {r.tile_id: r for r in dma.records}
    sha = j.SHAFile.load(os.path.join(data_dir, sha_file))
    b = j.BoardFile.load(os.path.join(data_dir, board))
    cache = {}

    def tile_img(tid):
        if tid in cache:
            return cache[tid]
        r = rec.get(tid)
        im = None
        if r is not None:
            bank = (r.shape_id >> 8) & 0x3F
            fr = r.shape_id & 0xFF
            try:
                im = sha.decode_frame_to_image(bank, fr)
            except Exception:
                im = None
        cache[tid] = im
        return im

    canvas = Image.new('RGB', (max(1, w), max(1, h)), (0, 0, 0))
    c0, c1 = cx // 16, (cx + w + 15) // 16
    r0, r1 = cy // 16, (cy + h + 15) // 16
    for c in range(max(0, c0), min(128, c1)):
        for r in range(max(0, r0), min(64, r1)):
            tid = b.tile_map[c * 64 + r] & 0x3FFF
            im = tile_img(tid)
            if im is None:
                continue
            canvas.paste(im.convert('RGB'), (c * 16 - cx, r * 16 - cy))
    canvas.save(out)
    print('wrote', out, canvas.size, 'objects', len(b.objects))


if __name__ == '__main__':
    a = sys.argv[1:]
    if len(a) < 5:
        print(__doc__)
        sys.exit(2)
    data, board, sha, dma, out = a[:5]
    opts = {'x': 0, 'y': 0, 'w': 320, 'h': 200}
    for i in range(5, len(a) - 1, 2):
        if a[i].lstrip('-') in opts:
            opts[a[i].lstrip('-')] = int(a[i + 1])
    render(data, board, sha, dma, out, opts['x'], opts['y'], opts['w'], opts['h'])
