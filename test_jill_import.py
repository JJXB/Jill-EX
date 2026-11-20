#!/usr/bin/env python3
"""
Self-contained checks for jill_import.py.

Builds tiny synthetic archives in a temporary folder: no game files needed.
Run with: py -3 test_jill_import.py
"""
from __future__ import annotations

import contextlib
import io
import os
import shutil
import struct
import sys
import tempfile
import wave
from pathlib import Path

from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))

import jill_extractor as jx
import jill_import as ji


class Fixtures:
    def __init__(self, root: Path):
        self.root = root
        self.png = root / 'sprite.png'
        Image.new('RGBA', (16, 16), (10, 20, 30, 255)).save(self.png)
        self.purple = root / 'purple.png'
        Image.new('RGBA', (16, 16), (180, 0, 200, 255)).save(self.purple)

        # A 4bpp entry with a 16-colour identity table, for palette adaptation.
        blob4 = ji.encode_entry(
            [(16, 16, 0, bytes(range(16)) * 16)], src_bpp=4, flags=4,
            palette_ints=[i << 16 for i in range(16)])
        entries4 = [None] * 128
        entries4[5] = blob4
        self.sha4 = root / 'palette.sha'
        self.sha4.write_bytes(ji.build_sha_bytes(entries4))
        self.dma4 = root / 'palette.dma'
        self.dma4.write_bytes(struct.pack('<HHHB', 0x10, (5 << 8), 0, 0))

        self.wav = root / 'tone.wav'
        with wave.open(str(self.wav), 'wb') as w:
            w.setnchannels(1)
            w.setsampwidth(1)
            w.setframerate(6000)
            w.writeframes(bytes([128 + (i % 32) for i in range(100)]))

        # 16x16 opaque shape entry 0.
        blob = ji.encode_entry([(16, 16, 0, b'\x10' * 256)])
        entries = [None] * 128
        entries[0] = blob
        self.sha = root / 'test.sha'
        self.sha.write_bytes(ji.build_sha_bytes(entries))

        # Font entry uses 128 cells of 1x16.
        font_frames = [(1, 16, 0, b'\x01' * 16) for _ in range(128)]
        font_entries = [None] * 128
        font_entries[2] = ji.encode_entry(font_frames)
        self.font_sha = root / 'font.sha'
        self.font_sha.write_bytes(ji.build_sha_bytes(font_entries))

        self.dma = root / 'test.dma'
        self.dma.write_bytes(struct.pack('<HHHB', 0x9E, 0, 0, 0))

        self.vcl = root / 'test.vcl'
        self.vcl.write_bytes(ji.build_vcl_bytes([None] * 50, [None] * 40))

        board = jx.BoardFile()
        board.tile_map = [0] * 8192
        board.objects = [jx.BoardObject()]
        board.header.reserved = b'\x00' * 22
        self.board = root / 'test.jn1'
        board.save(str(self.board))

        self.bad = root / 'garbage.bin'
        self.bad.write_bytes(b'\xff' * 1024)
        self.bad_image = root / 'garbage.png'
        self.bad_image.write_bytes(b'not an image')
        self.bad_anim = root / 'badanim'
        self.bad_anim.mkdir()
        (self.bad_anim / 'frame_0.png').write_bytes(b'not an image')

        self.pages = root / 'pages'
        self.pages.mkdir()
        (self.pages / 'page_0.txt').write_text('hello\r\nworld\n', encoding='ascii')

        self.wav_folder = root / 'wavs'
        self.wav_folder.mkdir()
        shutil.copy2(self.wav, self.wav_folder / 'sfx_0.wav')

        self.frames = root / 'frames'
        (self.frames / 'sprites').mkdir(parents=True)
        shutil.copy2(self.png, self.frames / 'sprites' / 'shape_000_000.png')

        self.fonts = root / 'fonts'
        self.fonts.mkdir()
        Image.new('RGBA', (128, 16), (255, 255, 255, 255)).save(
            self.fonts / 'main.png')

        self.bad_fonts = root / 'badfonts'
        self.bad_fonts.mkdir()
        Image.new('RGBA', (128, 16), (255, 255, 255, 255)).save(
            self.bad_fonts / 'main.png')
        (self.bad_fonts / 'font.json').write_text('{oops', encoding='utf-8')

        self.bad_pages = root / 'badpages'
        self.bad_pages.mkdir()
        (self.bad_pages / 'pages.json').write_text('{oops', encoding='utf-8')

        self.out = root / 'out'
        self.out.mkdir()

    def path(self, name: str) -> Path:
        return self.out / name


def expect_failure(name: str, fn) -> None:
    try:
        fn()
    except ji.ImportFailure:
        return
    raise AssertionError(f"{name}: expected ImportFailure")


def check_valid_imports(f: Fixtures) -> None:
    out = ji.replace_sha_frame(f.sha, 0, 0, f.png, f.path('frame.sha'))
    sha = jx.SHAFile.load(str(out))
    assert sha.entries[0] is not None and len(sha.entries[0].frames) == 1
    assert sha.entries[0].frames[0]['opcode'] == 0

    out = ji.sha_from_folder(f.root / 'missing.sha', f.frames, f.path('folder.sha'))
    sha = jx.SHAFile.load(str(out))
    assert sha.entries[0] is not None and len(sha.entries[0].frames) == 1

    anim = f.root / 'animation'
    anim.mkdir()
    for i in range(2):
        Image.new('RGBA', (16, 16), (i * 40, 20, 30, 255)).save(
            anim / f'frame_{i}.png')
    out = ji.sha_entry_from_folder(f.sha, 0, anim, f.path('anim.sha'))
    sha = jx.SHAFile.load(str(out))
    assert len(sha.entries[0].frames) == 2
    assert all(len(frame['data']) == 256 for frame in sha.entries[0].frames)

    out = ji.font_from_sheet(f.font_sha, f.fonts / 'main.png', f.path('font.sha'))
    sha = jx.SHAFile.load(str(out))
    assert sha.entries[2] is not None and len(sha.entries[2].frames) == 128

    out, entry, frame = ji.tile_from_png(
        f.sha, f.dma, 0x9E, f.png, f.path('tile.sha'))
    assert (entry, frame) == (0, 0) and out.is_file()

    # Building an adapted palette moves towards the new image's colours, and
    # index 0 stays reserved for transparency (the VGA palette has magenta).
    colors, ints = ji.adapt_entry_palette(
        jx.SHAFile.load(str(f.sha4)), 5, ji._load_image(f.purple))
    best = min((r - 180) ** 2 + (g - 0) ** 2 + (b - 200) ** 2
               for r, g, b in colors)
    assert best < 5000, colors
    assert ints[0] & 0xFFFF == 0 and all((i >> 16) != 0 for i in ints[1:]), ints
    original_words = jx.SHAFile.load(str(f.sha4)).entries[5].palette
    assert all((new & 0xFFFF) == (old & 0xFFFF)
               for new, old in zip(ints, original_words))

    # Tile adaptation clones into a free entry and repoints the DMA record, so
    # the original entry (and every other tile using it) stays unchanged.
    sha_out, dma_out, cloned, cloned_frame = ji.clone_tile_to_entry(
        f.sha4, f.dma4, 0x10, f.purple,
        f.path('clone.sha'), f.path('clone.dma'))
    clone_sha = jx.SHAFile.load(str(sha_out))
    clone_dma = jx.DMAFile.load(str(dma_out))
    assert cloned < 64 and cloned_frame == 0
    assert [(p >> 16) & 0xFF for p in clone_sha.entries[5].palette] == list(range(16))
    new_rec = next(r for r in clone_dma.records if r.tile_id == 0x10)
    assert (new_rec.shape_id >> 8) & 0x3F == cloned
    tile = jx._tile_rgba(clone_sha, new_rec, {})
    assert tile is not None
    tpx = tile.load()
    assert tpx[0, 0][3] > 0 and tpx[0, 0][0] > 120 and tpx[0, 0][2] > 150

    out = ji.replace_vcl_sfx(f.vcl, 0, f.wav, f.path('sfx.vcl'))
    vcl = jx.VCLFile.load(str(out))
    assert vcl.sfx_sizes[0] > 0

    out = ji.vcl_from_folder(f.root / 'missing.vcl', f.wav_folder, f.path('bank.vcl'))
    vcl = jx.VCLFile.load(str(out))
    assert vcl.sfx_sizes[0] > 0

    out = ji.vcl_pages_from_folder(f.vcl, f.pages, f.path('pages.vcl'))
    vcl = jx.VCLFile.load(str(out))
    page = vcl.parse_text_page(0)
    assert page and page[0]['text'] == 'hello'

    structured = {0: [{'colour': 2, 'text': 'HI'}]}
    out = ji.vcl_set_pages(f.vcl, structured, f.path('structured.vcl'))
    assert jx.VCLFile.load(str(out)).parse_text_page(0) == structured[0]

    board = jx.BoardFile.load(str(f.board))
    out = ji.save_board(board, f.board, f.path('board_saved.jn1'))
    assert out.is_file() and len(jx.BoardFile.load(str(out)).tile_map) == 8192

    render = f.path('board.png')
    Image.new('RGB', (2048, 1024), (0, 0, 0)).save(render)
    out = ji.board_from_image(f.board, f.sha, f.dma, render, f.path('board_new.jn1'))
    assert len(jx.BoardFile.load(str(out)).tile_map) == 8192


def check_bad_inputs(f: Fixtures) -> None:
    expect_failure('garbage .sha', lambda: ji.replace_sha_frame(
        f.bad, 0, 0, f.png, f.path('bad1.sha')))
    expect_failure('missing .sha', lambda: ji.replace_sha_frame(
        f.root / 'nope.sha', 0, 0, f.png, f.path('bad2.sha')))
    expect_failure('bad image', lambda: ji.replace_sha_frame(
        f.sha, 0, 0, f.bad_image, f.path('bad3.sha')))
    expect_failure('negative entry', lambda: ji.replace_sha_frame(
        f.sha, -1, 0, f.png, f.path('bad4.sha')))
    expect_failure('large entry', lambda: ji.replace_sha_frame(
        f.sha, 128, 0, f.png, f.path('bad5.sha')))
    expect_failure('bad frame', lambda: ji.replace_sha_frame(
        f.sha, 0, 3, f.png, f.path('bad6.sha')))
    expect_failure('garbage folder ref', lambda: ji.sha_from_folder(
        f.bad, f.frames, f.path('bad7.sha')))
    expect_failure('missing folder', lambda: ji.sha_from_folder(
        f.root / 'missing.sha', f.root / 'nope', f.path('bad8.sha')))
    expect_failure('empty animation folder', lambda: ji.sha_entry_from_folder(
        f.sha, 0, f.pages, f.path('bad8b.sha')))
    expect_failure('bad animation image', lambda: ji.sha_entry_from_folder(
        f.sha, 0, f.bad_anim, f.path('bad8c.sha')))
    expect_failure('bad font json', lambda: ji.font_from_sheet(
        f.font_sha, f.bad_fonts / 'main.png', f.path('bad9.sha')))
    expect_failure('bad tile id', lambda: ji.tile_from_png(
        f.sha, f.dma, 0x7F, f.png, f.path('bad10.sha')))
    expect_failure('negative tile id', lambda: ji.tile_from_png(
        f.sha, f.dma, -1, f.png, f.path('bad11.sha')))
    expect_failure('bad board render', lambda: ji.board_from_image(
        f.board, f.sha, f.dma, f.png, f.path('bad12.jn1')))
    expect_failure('bad wav', lambda: ji.replace_vcl_sfx(
        f.vcl, 0, f.bad, f.path('bad1.vcl')))
    expect_failure('bad slot', lambda: ji.replace_vcl_sfx(
        f.vcl, 50, f.wav, f.path('bad2.vcl')))
    expect_failure('garbage folder ref', lambda: ji.vcl_from_folder(
        f.bad, f.wav_folder, f.path('bad3.vcl')))
    expect_failure('bad pages json', lambda: ji.vcl_pages_from_folder(
        f.vcl, f.bad_pages, f.path('bad4.vcl')))
    expect_failure('bad pages mapping', lambda: ji.vcl_set_pages(
        f.vcl, [1, 2, 3], f.path('bad5.vcl')))
    expect_failure('missing pack src', lambda: ji.install_pack(
        f.root / 'nope', f.path('pack')))
    expect_failure('pack into itself', lambda: ji.install_pack(
        f.frames, f.frames))

    for name in ('bad1.sha', 'bad1.vcl', 'bad4.vcl'):
        assert not f.path(name).exists(), f"{name} was written on failure"


def check_edge_cases(f: Fixtures) -> None:
    expect_failure('oversized sha entry', lambda: ji.build_sha_bytes([b'x' * 0x10000]))
    expect_failure('oversized vcl entry', lambda: ji.build_vcl_bytes(
        [(b'x' * 0x10000, 6000)], []))
    expect_failure('frame too large', lambda: ji.encode_entry([(256, 1, 0, b'x' * 256)]))
    expect_failure('short frame data', lambda: ji.encode_entry([(2, 2, 0, b'x')]))
    expect_failure('zero sample rate', lambda: ji.resample([0.0], 0, 6000))
    expect_failure('bad wav blob', lambda: ji.decode_wav(b'not a wav'))

    full_entries = [ji.encode_entry([(1, 1, 0, b'\x00')]) for _ in range(64)]
    full_entries += [None] * 64
    full_sha = f.root / 'full.sha'
    full_sha.write_bytes(ji.build_sha_bytes(full_entries))
    expect_failure('no free tile entry', lambda: ji.clone_tile_to_entry(
        full_sha, f.dma4, 0x10, f.purple,
        f.path('full_out.sha'), f.path('full_out.dma')))

    # Unicode pack names are written as UTF-8, not rejected.
    src = f.root / 'packsrc'
    (src / 'sprites').mkdir(parents=True)
    shutil.copy2(f.png, src / 'sprites' / 'kind_1.png')
    dst = f.path('CaféPack')
    ji.install_pack(src, dst, game='jill1', name='Café Pack')
    assert 'Café Pack' in (dst / 'pack.ini').read_text(encoding='utf-8')

    # Junk names in a folder are ignored; the real flat sprite is imported.
    junk = f.root / 'junkfolder'
    (junk / 'sprites').mkdir(parents=True)
    (junk / 'sprites' / 'shape_x_0.png').write_bytes(b'junk')
    (junk / 'shape_nope').mkdir()
    shutil.copy2(f.png, junk / 'sprites' / 'shape_000_000.png')
    out = ji.sha_from_folder(f.root / 'missing.sha', junk, f.path('junk.sha'))
    assert jx.SHAFile.load(str(out)).entries[0] is not None


def check_cli(f: Fixtures) -> None:
    err = io.StringIO()
    with contextlib.redirect_stderr(err):
        rc = ji._cli(['vcl-sfx', str(f.vcl), '0', str(f.bad),
                      '-o', str(f.path('cli_bad.vcl'))])
    assert rc == 1 and 'Traceback' not in err.getvalue()
    assert not f.path('cli_bad.vcl').exists()

    err = io.StringIO()
    with contextlib.redirect_stderr(err):
        rc = ji._cli(['sha-entry-folder', str(f.sha), '0', str(f.pages),
                      '-o', str(f.path('cli_bad_anim.sha'))])
    assert rc == 1 and 'Traceback' not in err.getvalue()
    assert not f.path('cli_bad_anim.sha').exists()

    err = io.StringIO()
    with contextlib.redirect_stderr(err):
        rc = ji._cli(['vcl-sfx', str(f.vcl), '0', str(f.wav),
                      '-o', str(f.path('cli_ok.vcl'))])
    assert rc == 0 and f.path('cli_ok.vcl').is_file()


def main() -> int:
    root = Path(tempfile.mkdtemp(prefix='jill_import_test_'))
    try:
        f = Fixtures(root)
        checks = [
            ('valid imports', lambda: check_valid_imports(f)),
            ('bad inputs', lambda: check_bad_inputs(f)),
            ('edge cases', lambda: check_edge_cases(f)),
            ('CLI errors', lambda: check_cli(f)),
        ]
        failed = 0
        for name, fn in checks:
            try:
                fn()
                print(f"  {name}: OK")
            except Exception as e:
                failed += 1
                print(f"  {name}: FAIL - {type(e).__name__}: {e}")
                if os.environ.get('JILL_TEST_DEBUG'):
                    import traceback
                    traceback.print_exc()
        print(f"{len(checks) - failed}/{len(checks)} checks passed "
              f"(fixtures: {root})")
        return 1 if failed else 0
    finally:
        pass  # keep fixtures on disk for inspection; remove manually if wanted


if __name__ == '__main__':
    raise SystemExit(main())
