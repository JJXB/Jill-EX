#!/usr/bin/env python3
"""Headless checks for jill_editor.py: API round-trip + UI construction.

Run with: py -3 test_jill_editor.py
"""
from __future__ import annotations

import struct
import sys
import tempfile
import wave
from pathlib import Path

from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))

import jill_extractor as jx
import jill_import as ji
import jill_editor as je


def build_fixtures(root: Path):
    png = root / 'sprite.png'
    Image.new('RGBA', (16, 16), (10, 20, 30, 255)).save(png)
    wav = root / 'tone.wav'
    with wave.open(str(wav), 'wb') as w:
        w.setnchannels(1); w.setsampwidth(1); w.setframerate(6000)
        w.writeframes(b'\x80' * 128)

    entries = [None] * 128
    entries[0] = ji.encode_entry([(16, 16, 0, b'\x10' * 256)])
    sha = root / 'test.sha'
    sha.write_bytes(ji.build_sha_bytes(entries))
    dma = root / 'test.dma'
    dma.write_bytes(struct.pack('<HHHB', 0x9E, 0, 0, 0))
    vcl = root / 'test.vcl'
    vcl.write_bytes(ji.build_vcl_bytes([None] * 50, [None] * 40))

    purple = root / 'purple.png'
    Image.new('RGBA', (16, 16), (180, 0, 200, 255)).save(purple)
    blob4 = ji.encode_entry([(16, 16, 0, bytes(range(16)) * 16)],
                            src_bpp=4, flags=4,
                            palette_ints=[i << 16 for i in range(16)])
    entries4 = [None] * 128
    entries4[5] = blob4
    sha4 = root / 'palette.sha'
    sha4.write_bytes(ji.build_sha_bytes(entries4))
    dma4 = root / 'palette.dma'
    dma4.write_bytes(struct.pack('<HHHB', 0x10, 5 << 8, 0, 0))

    board = jx.BoardFile()
    board.tile_map = [0] * 8192
    board.objects = [jx.BoardObject()]
    board.header.reserved = b'\x00' * 22
    board.strings = ['HELLO']
    path = root / 'test.jn1'
    board.save(str(path))
    return sha, dma, vcl, path


def check_extras(root: Path) -> None:
    """Headless checks for the safe tile import, palettes, portrait and packs."""
    purple = root / 'purple_safe.png'
    Image.new('RGBA', (16, 16), (200, 0, 220, 255)).save(purple)

    entries = [None] * 128
    entries[0] = ji.encode_entry([(16, 16, 0, b'\x10' * 256)])
    sha = root / 'safe.sha'
    sha.write_bytes(ji.build_sha_bytes(entries))
    dma = root / 'safe.dma'
    dma.write_bytes(struct.pack('<HHHB', 0x9E, 0, 0, 0))
    out_sha, out_dma, host, frame = ji.tile_from_png_safe(
        sha, dma, 0x9E, purple, root / 'safe_new.sha', root / 'safe_new.dma')
    assert (out_sha.is_file() and out_dma.is_file())
    assert len(jx.SHAFile.load(str(out_sha)).entries[0].frames) == 2
    assert jx.DMAFile.load(str(out_dma)).get_record(0x9E).shape_id == (0 << 8) | 1

    # Entry palette round-trip keeps the low (CGA/EGA) palette bytes.
    pal_entries = [None] * 128
    pal_entries[5] = ji.encode_entry(
        [(16, 16, 0, bytes(range(16)) * 16)], src_bpp=4, flags=4,
        palette_ints=[(i << 16) | 0x1234 for i in range(16)])
    pal_sha = root / 'palround.sha'
    pal_sha.write_bytes(ji.build_sha_bytes(pal_entries))
    pal_file = ji.export_entry_palette(pal_sha, 5, root / 'entry5.pal')
    assert pal_file.is_file() and pal_file.read_text().startswith('JASC-PAL')
    before = jx.SHAFile.load(str(pal_sha)).entries[5].palette[:]
    ji.import_entry_palette(pal_sha, 5, pal_file, root / 'palround_new.sha')
    after = jx.SHAFile.load(str(root / 'palround_new.sha')).entries[5].palette
    assert all((a & 0xFFFF) == (b & 0xFFFF) for a, b in zip(before, after))

    # Portrait entry: sixteen tiles, composited and re-imported as one image.
    portrait_frames = []
    for i in range(16):
        h = 22 if i in (12, 13) else (20 if i in (14, 15) else 16)
        portrait_frames.append((16, h, 0, bytes([1 + (i % 15)]) * (16 * h)))
    port_entries = [None] * 128
    port_entries[jx.PORTRAIT_ENTRY] = ji.encode_entry(
        portrait_frames, src_bpp=4, flags=4,
        palette_ints=[(i << 16) | 0x4321 for i in range(16)])
    port_sha = root / 'portrait.sha'
    port_sha.write_bytes(ji.build_sha_bytes(port_entries))
    img = jx.portrait_image(jx.SHAFile.load(str(port_sha)))
    assert img is not None and img.size == jx.PORTRAIT_SIZE
    custom = root / 'custom_portrait.png'
    img.save(custom)
    out_port = ji.portrait_from_image(port_sha, custom, root / 'portrait_new.sha')
    assert len(jx.SHAFile.load(str(out_port)).entries[jx.PORTRAIT_ENTRY].frames) == 16

    # Pack section selection: only requested trees are written.
    vcl = jx.VCLFile.load(str(root / 'test.vcl')) if (root / 'test.vcl').exists() else None
    if vcl is None:
        (root / 'test.vcl').write_bytes(ji.build_vcl_bytes([None] * 50, [None] * 40))
        vcl = jx.VCLFile.load(str(root / 'test.vcl'))
    game = {'id': 'test', 'title': 'Test', 'data_dir': str(root),
            'board_ext': '.jn1', 'boards': []}
    dma_obj = jx.DMAFile.load(str(dma))
    stats = jx.export_reference_pack(game, jx.SHAFile.load(str(sha)), vcl,
                                     dma_obj, root / 'pack',
                                     include=['tiles', 'palette', 'portrait'])
    assert (root / 'pack' / 'tiles' / 'tiles.json').is_file()
    assert (root / 'pack' / 'palette.pal').is_file()
    assert 'sprites' not in stats and 'music' not in stats

    # Backup helper preserves the old contents under a timestamped name.
    target = root / 'will_change.bin'
    target.write_bytes(b'ORIGINAL')
    backup = je.backup_file(target)
    assert backup is not None and backup.read_bytes() == b'ORIGINAL'


def check_ui(root: Path) -> None:
    if je.tk is None:
        print('  UI construction: SKIP (Tkinter unavailable)')
        return
    top = je.tk.Tk()
    errors = []
    original_error = je.messagebox.showerror
    try:
        top.withdraw()
        # Any action that reports an error dialog fails the smoke test.
        je.messagebox.showerror = lambda *args, **kwargs: errors.append(args)

        app = je.JillEditorApp(top, root)
        sha, dma, vcl, board_path = build_fixtures(root)
        app.sha_path, app.dma_path, app.vcl_path = sha, dma, vcl
        app.data_dir = root
        app.load_archives()

        app.sprite_list.selection_set(0)
        app.on_sprite_selected()
        app.sprite_grid_var.set(True)
        app.refresh_sprite_view()
        assert app._sprite_grid_cells
        app.sprite_grid_var.set(False)
        app.refresh_sprite_view()
        app.tile_list.selection_set(0)
        app.on_tile_selected()
        app.tile_grid_var.set(True)
        app.refresh_tile_view()
        assert app._tile_grid_cells
        app.tile_grid_var.set(False)
        app.refresh_tile_view()
        app.sfx_list.selection_set(0)
        app.on_sfx_selected()
        app.page_list.selection_set(0)
        app.on_page_selected()

        app.open_board(board_path)
        app.edit_tile_var.set('158')
        app.use_tile_in_levels()

        class Event:
            x, y, state = 16, 16, 0
            x_root, y_root = 0, 0

        app.paint_at(Event())
        assert app.board.tile_map[0] == 158
        shift = Event()
        shift.x = shift.y = 4
        shift.state = 1
        app.on_canvas_press(shift)
        assert len(app.board.objects) == 0  # Shift removed the fixture object
        ctrl = Event()
        ctrl.state = 2
        app.on_canvas_press(ctrl)
        assert app.board.tile_map[0] == 0  # Ctrl used the erase-tile value
        app.zoom_in()
        assert app.zoom == 4
        app.zoom_out()
        assert app.zoom == 2
        app.show_kinds_var.set(True)
        app.show_links_var.set(True)
        app.draw_board_view()
        app.set_view_c('moveto', '0.5')
        app.set_view_r('scroll', '1')

        anim = root / 'anim'
        anim.mkdir()
        for i in range(2):
            Image.new('RGBA', (16, 16), (i * 40, 20, 30, 255)).save(
                anim / f'frame_{i}.png')
        old_dir = je.filedialog.askdirectory
        old_out = app.ask_out
        try:
            je.filedialog.askdirectory = lambda **kwargs: str(anim)
            app.ask_out = lambda *args, **kwargs: root / 'anim.sha'
            app.import_sprite_animation()
        finally:
            je.filedialog.askdirectory = old_dir
            app.ask_out = old_out
        assert len(jx.SHAFile.load(str(root / 'anim.sha')).entries[0].frames) == 2

        app.line_var.set('TEST')
        app.apply_line()
        assert app.pages[app.page_idx][-1]['text'] == 'TEST'
        app.line_var.set('EXTRA')
        app.add_line()
        assert [line['text'] for line in app.pages[app.page_idx]][-2:] == [
            'TEST', 'EXTRA']
        app.line_var.set('THIRD')
        app.add_line()  # a selected line must not turn Add Line into Replace
        assert app.pages[app.page_idx][-1]['text'] == 'THIRD'

        # Dithered tile import keeps the palette and writes only the .sha.
        app.sha_path, app.dma_path = root / 'palette.sha', root / 'palette.dma'
        app.load_archives()
        app.tile_var.set('16')
        app.dither_var.set(True)
        app.tile_safe_var.set(False)
        old_open = je.filedialog.askopenfilename
        try:
            je.filedialog.askopenfilename = lambda **kwargs: str(root / 'purple.png')
            app.ask_out = lambda *args, **kwargs: root / 'dither_gui.sha'
            app.import_tile_png()
        finally:
            je.filedialog.askopenfilename = old_open
        assert (root / 'dither_gui.sha').is_file()
        assert not errors, errors
        print('  UI construction: OK')
    finally:
        je.messagebox.showerror = original_error
        top.destroy()


def main() -> int:
    print(je.self_check())
    with tempfile.TemporaryDirectory(prefix='jill_editor_test_') as td:
        try:
            check_ui(Path(td))
            check_extras(Path(td))
        except Exception as e:
            import traceback
            traceback.print_exc()
            print(f'  UI construction: FAIL - {type(e).__name__}: {e}')
            return 1
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
