#!/usr/bin/env python3
"""
Jill asset editor (Tkinter + Pillow).

A small GUI for editing the archives read by jill_extractor.py and written by
jill_import.py: shape frames, DMA tiles, board levels (tile map, objects,
strings, header), VCL text pages and VCL sound slots.

    py -3 jill_editor.py [base_dir]
    py -3 jill_editor.py --check

`base_dir` is the folder holding jill.ini / games/*.game (defaults to the
script's folder).  See EDITOR_README.md for the workflow.
"""
from __future__ import annotations

import argparse
import json
import os
import sys
import tempfile
import traceback
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

from PIL import Image, ImageDraw

BASE_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(BASE_DIR))

import jill_extractor as jx  # noqa: E402
import jill_import as ji  # noqa: E402

try:
    import tkinter as tk
    from tkinter import filedialog, messagebox, scrolledtext, ttk
    from PIL import ImageTk
except ImportError:  # --check and --help must work without a display
    tk = None
    filedialog = messagebox = scrolledtext = ttk = ImageTk = None


BOARD_FIELDS = (
    ('kind', 'Kind'), ('x', 'X'), ('y', 'Y'), ('x_delta', 'X delta'),
    ('y_delta', 'Y delta'), ('width', 'Width'), ('height', 'Height'),
    ('state', 'State'), ('primary_param', 'Param'), ('flags', 'Flags'),
    ('ttl', 'TTL'), ('string_offset', 'String off'),
    ('string_segment', 'Segment'), ('kind_specific_2', 'KS2'),
)
HEADER_FIELDS = (
    ('level_id', 'Level'), ('health', 'Health'), ('key_count', 'Keys'),
    ('score', 'Score'), ('damage_flash', 'Flash'), ('score_copy', 'Score copy'),
)
SFX_COUNT = 50
PAGE_COUNT = 40
KIND_COLORS = ('#FF5555', '#55FF55', '#5555FF', '#FFFF55', '#FF55FF',
               '#55FFFF', '#FFAA55', '#AAFF55', '#55AAFF', '#FF55AA')
ZOOM_LEVELS = (1, 2, 4)


# ---------------------------------------------------------------------------
# Non-GUI helpers (also used by the self-check)
# ---------------------------------------------------------------------------

def load_palette(sha: "jx.SHAFile", game: Optional[Dict[str, Any]],
                 base_dir: Path) -> None:
    """Apply a game's optional palette file to an in-memory SHA."""
    pal = game.get('palette') if game else None
    if not pal:
        return
    pal_path = Path(pal)
    if not pal_path.is_absolute():
        pal_path = Path(base_dir) / pal_path
    if pal_path.is_file():
        try:
            data = pal_path.read_bytes()
            sha.set_palette_rgb(data, from_6bit=(max(data) <= 63))
        except Exception:
            pass


def tile_image(sha, dma, tile_id: int, cache: Dict[str, Image.Image]):
    key = int(tile_id) & 0x3FFF
    if key not in cache:
        rec = jx._dma_by_tile(dma).get(key)
        cache[key] = jx._tile_rgba(sha, rec, {}) if rec else None
    return cache[key]


def page_lines(vcl, index: int) -> List[Dict[str, Any]]:
    return list(vcl.parse_text_page(index) or [])


def self_check() -> str:
    """Headless API/round-trip check; raises on failure."""
    for name in ('load', 'parse'):
        assert hasattr(jx.SHAFile, name), name
    for name in ('adapt_entry_palette', 'replace_sha_frame',
                 'sha_entry_from_folder', 'tile_from_png', 'clone_tile_to_entry',
                 'build_dma_bytes', 'save_dma',
                 'board_from_image',
                 'save_board', 'vcl_set_pages', 'replace_vcl_sfx',
                 'vcl_pages_from_folder', 'vcl_from_folder'):
        assert hasattr(ji, name), name
    with tempfile.TemporaryDirectory(prefix='jill_editor_check_') as td:
        root = Path(td)
        vcl_path = root / 'test.vcl'
        vcl_path.write_bytes(ji.build_vcl_bytes([None] * 50, [None] * 40))
        pages = {1: [{'colour': 3, 'text': 'HELLO'},
                     {'colour': 7, 'text': 'WORLD'}]}
        out = ji.vcl_set_pages(vcl_path, pages, root / 'pages.vcl')
        vcl = jx.VCLFile.load(str(out))
        assert vcl.parse_text_page(1) == pages[1], vcl.parse_text_page(1)
        board = jx.BoardFile()
        board.tile_map = [0] * 8192
        board.objects = [jx.BoardObject()]
        board.header.reserved = b'\x00' * 22
        bpath = ji.save_board(board, root / 'test.jn1')
        assert len(jx.BoardFile.load(str(bpath)).tile_map) == 8192
    return ('editor API OK: parsers, importer writers, VCL pages and board '
            'save round-trip')


# ---------------------------------------------------------------------------
# GUI
# ---------------------------------------------------------------------------

class JillEditorApp:
    """Tkinter application.  `root` is supplied by run_gui()."""

    def __init__(self, root, base_dir: Optional[Path] = None):
        self.root = root
        self.base_dir = Path(base_dir or BASE_DIR)
        self.root.title('Jill Editor - sprites, tiles, levels, text and SFX')
        self.root.geometry('1180x760')

        self.games = jx.discover_games(self.base_dir)
        self.current_game: Optional[Dict[str, Any]] = None
        self.data_dir: Optional[Path] = None
        self.sha_path: Optional[Path] = None
        self.dma_path: Optional[Path] = None
        self.vcl_path: Optional[Path] = None
        self.sha = None
        self.dma = None
        self.vcl = None

        self.board_path: Optional[Path] = None
        self.board = None
        self.board_dirty = False
        self.view_c = 0
        self.view_r = 0
        self.tile_id = 0
        self.object_idx: Optional[int] = None
        self.line_idx: Optional[int] = None
        self.page_idx = 0
        self.pages: Dict[int, List[Dict[str, Any]]] = {}

        self._tile_cache: Dict[int, Optional[Image.Image]] = {}
        self._tile_photos: Dict[int, Any] = {}
        self._shape_cache: Dict[Tuple[int, int], Any] = {}
        self._preview_photo = None
        self._board_photo = None
        self._sfx_preview = None

        self.status_var = tk.StringVar(value='Select a game or open archive files.')
        self.game_var = tk.StringVar()
        self.entry_var = tk.StringVar()
        self.frame_var = tk.StringVar(value='0')
        self.tile_var = tk.StringVar(value='0')
        self.sprite_info = tk.StringVar(value='No shape archive loaded.')
        self.tile_info = tk.StringVar(value='No tile archive loaded.')
        self.board_info = tk.StringVar(value='No board open.')
        self.sfx_info = tk.StringVar(value='No sound archive loaded.')
        self.edit_tile_var = tk.StringVar(value='0')
        self.grid_var = tk.BooleanVar(value=True)
        self.show_kinds_var = tk.BooleanVar(value=True)
        self.show_links_var = tk.BooleanVar(value=True)
        self.adapt_palette_var = tk.BooleanVar(value=False)
        self.dither_var = tk.BooleanVar(value=False)
        self.bg_tile_var = tk.StringVar(value='0')
        self.zoom = 2
        self.colour_var = tk.StringVar(value='7')
        self.line_var = tk.StringVar(value='')

        self.setup_ui()
        if self.games:
            self.game_var.set(self.games[0]['title'])
            self.load_game(self.games[0])

    # -- UI construction ----------------------------------------------------

    def setup_ui(self):
        self.root.config(menu=self._build_menu())

        bar = ttk.Frame(self.root, padding=(6, 4))
        bar.pack(fill='x')
        ttk.Label(bar, text='Game:').pack(side='left')
        self.game_combo = ttk.Combobox(
            bar, textvariable=self.game_var,
            values=[g['title'] for g in self.games], width=28, state='readonly')
        self.game_combo.pack(side='left', padx=(4, 6))
        self.game_combo.bind('<<ComboboxSelected>>', self.on_game_selected)
        ttk.Button(bar, text='Load', command=self.on_load_game).pack(side='left')
        ttk.Button(bar, text='Rescan', command=self.on_rescan).pack(side='left', padx=4)
        ttk.Button(bar, text='Open Files...', command=self.open_files).pack(side='left')

        self.tabs = ttk.Notebook(self.root)
        self.tabs.pack(fill='both', expand=True, padx=6, pady=4)
        self.sprites_tab = ttk.Frame(self.tabs)
        self.tiles_tab = ttk.Frame(self.tabs)
        self.levels_tab = ttk.Frame(self.tabs)
        self.text_tab = ttk.Frame(self.tabs)
        self.sfx_tab = ttk.Frame(self.tabs)
        for frame, label in ((self.sprites_tab, 'Sprites'),
                             (self.tiles_tab, 'Tiles'),
                             (self.levels_tab, 'Levels'),
                             (self.text_tab, 'Text'),
                             (self.sfx_tab, 'SFX')):
            self.tabs.add(frame, text=label)

        self._build_sprites_tab(self.sprites_tab)
        self._build_tiles_tab(self.tiles_tab)
        self._build_levels_tab(self.levels_tab)
        self._build_text_tab(self.text_tab)
        self._build_sfx_tab(self.sfx_tab)

        ttk.Label(self.root, textvariable=self.status_var, relief='sunken',
                  anchor='w', padding=(4, 2)).pack(fill='x', side='bottom')

    def _build_menu(self):
        menu = tk.Menu(self.root)
        file_menu = tk.Menu(menu, tearoff=0)
        file_menu.add_command(label='Open Files...', command=self.open_files)
        file_menu.add_command(label='Save Current...', command=self.save_current)
        file_menu.add_separator()
        file_menu.add_command(label='Exit', command=self.root.quit)
        menu.add_cascade(label='File', menu=file_menu)

        view_menu = tk.Menu(menu, tearoff=0)
        view_menu.add_command(label='Export Reference Pack...',
                              command=self.export_pack)
        menu.add_cascade(label='Tools', menu=view_menu)
        return menu

    def _listbox(self, parent, height=16, width=32):
        frame = ttk.Frame(parent)
        frame.pack(side='left', fill='y', padx=(4, 2), pady=4)
        lb = tk.Listbox(frame, height=height, width=width, font=('Consolas', 9),
                        exportselection=False)
        sb = ttk.Scrollbar(frame, orient='vertical', command=lb.yview)
        lb.config(yscrollcommand=sb.set)
        lb.pack(side='left', fill='y')
        sb.pack(side='left', fill='y')
        return lb

    def _photo_label(self, parent, width=320, height=200):
        label = tk.Label(parent, width=width, height=height, bg='#202020')
        label.pack(anchor='nw', padx=4, pady=4)
        return label

    def _build_sprites_tab(self, parent):
        left = ttk.Frame(parent)
        left.pack(side='left', fill='y')
        ttk.Label(left, text='Shape entries').pack(anchor='w', padx=4)
        self.sprite_list = self._listbox(left, height=28)
        self.sprite_list.bind('<<ListboxSelect>>', self.on_sprite_selected)

        right = ttk.Frame(parent)
        right.pack(side='left', fill='both', expand=True)
        self.sprite_preview = self._photo_label(right, 320, 200)
        nav = ttk.Frame(right)
        nav.pack(fill='x', padx=4)
        ttk.Label(nav, textvariable=self.sprite_info).pack(side='left')
        ttk.Button(nav, text='Prev', command=lambda: self.step_frame(-1)).pack(side='right')
        ttk.Button(nav, text='Next', command=lambda: self.step_frame(1)).pack(side='right')
        ttk.Entry(nav, textvariable=self.frame_var, width=5).pack(side='right', padx=2)

        actions = ttk.LabelFrame(right, text='Shape frame', padding=6)
        actions.pack(fill='x', padx=4, pady=4)
        ttk.Button(actions, text='Export PNG...', command=self.export_sprite_png).grid(row=0, column=0, padx=2, pady=2, sticky='w')
        ttk.Button(actions, text='Replace from PNG...', command=self.import_sprite_png).grid(row=0, column=1, padx=2, pady=2, sticky='w')
        ttk.Button(actions, text='Rebuild from Folder...', command=self.import_sprites_folder).grid(row=1, column=0, padx=2, pady=2, sticky='w')
        ttk.Button(actions, text='Font from Sheet...', command=self.import_font_sheet).grid(row=1, column=1, padx=2, pady=2, sticky='w')
        ttk.Button(actions, text='Replace Animation from Folder...',
                   command=self.import_sprite_animation).grid(
                       row=2, column=0, columnspan=2, padx=2, pady=2, sticky='w')
        ttk.Checkbutton(actions, text='Adapt palette (recolours whole entry)',
                        variable=self.adapt_palette_var).grid(
                            row=3, column=0, columnspan=2, sticky='w')

    def _build_tiles_tab(self, parent):
        left = ttk.Frame(parent)
        left.pack(side='left', fill='y')
        ttk.Label(left, text='DMA tiles').pack(anchor='w', padx=4)
        self.tile_list = self._listbox(left, height=28)
        self.tile_list.bind('<<ListboxSelect>>', self.on_tile_selected)

        right = ttk.Frame(parent)
        right.pack(side='left', fill='both', expand=True)
        self.tile_preview = self._photo_label(right, 160, 160)
        ttk.Label(right, textvariable=self.tile_info, wraplength=480,
                  justify='left').pack(anchor='w', padx=4)
        ttk.Button(right, text='Use in Level Editor',
                   command=self.use_tile_in_levels).pack(anchor='w', padx=4, pady=2)
        actions = ttk.LabelFrame(right, text='Tile art', padding=6)
        actions.pack(fill='x', padx=4, pady=4)
        ttk.Button(actions, text='Export PNG...', command=self.export_tile_png).grid(row=0, column=0, padx=2, pady=2, sticky='w')
        ttk.Button(actions, text='Replace from PNG...', command=self.import_tile_png).grid(row=0, column=1, padx=2, pady=2, sticky='w')
        ttk.Checkbutton(actions,
                        text='Dither to existing palette (original-game safe)',
                        variable=self.dither_var).grid(
                            row=1, column=0, columnspan=2, sticky='w')

    def _build_levels_tab(self, parent):
        top = ttk.Frame(parent)
        top.pack(fill='x', padx=4, pady=2)
        ttk.Label(top, text='Board:').pack(side='left')
        self.board_combo = ttk.Combobox(top, width=24, state='readonly')
        self.board_combo.pack(side='left', padx=4)
        self.board_combo.bind('<<ComboboxSelected>>', self.on_board_selected)
        ttk.Button(top, text='Open', command=self.open_selected_board).pack(side='left')
        ttk.Button(top, text='Save As...', command=self.save_board_file).pack(side='left', padx=4)
        ttk.Button(top, text='Import Render...', command=self.import_board_render).pack(side='left')
        ttk.Button(top, text='Export Render...', command=self.export_board_render).pack(side='left', padx=4)

        mid = ttk.Frame(parent)
        mid.pack(fill='both', expand=True)
        board_frame = ttk.Frame(mid)
        board_frame.pack(side='left', fill='both', expand=True, padx=4)
        controls = ttk.Frame(board_frame)
        controls.pack(fill='x')
        ttk.Label(controls, text='Paint tile:').pack(side='left')
        ttk.Entry(controls, textvariable=self.edit_tile_var, width=6).pack(side='left', padx=2)
        ttk.Button(controls, text='From Tiles tab',
                   command=self.use_tile_in_levels).pack(side='left', padx=2)
        ttk.Checkbutton(controls, text='Grid', variable=self.grid_var,
                        command=self.draw_board_view).pack(side='left', padx=8)

        controls2 = ttk.Frame(board_frame)
        controls2.pack(fill='x')
        ttk.Checkbutton(controls2, text='Kinds', variable=self.show_kinds_var,
                        command=self.draw_board_view).pack(side='left')
        ttk.Checkbutton(controls2, text='Links', variable=self.show_links_var,
                        command=self.draw_board_view).pack(side='left', padx=4)
        ttk.Label(controls2, text='Erase tile:').pack(side='left', padx=(8, 2))
        ttk.Entry(controls2, textvariable=self.bg_tile_var,
                  width=6).pack(side='left')
        ttk.Button(controls2, text='Zoom -', command=self.zoom_out).pack(side='left', padx=(8, 2))
        self.zoom_label = ttk.Label(controls2, text='2x', width=4)
        self.zoom_label.pack(side='left')
        ttk.Button(controls2, text='Zoom +', command=self.zoom_in).pack(side='left', padx=2)
        ttk.Label(controls2, text='L-click paint, Shift erase kind, '
                                'Ctrl erase tile, R-click properties').pack(
                                    side='left', padx=8)

        self.board_canvas = tk.Canvas(board_frame,
                                      width=320 * self.zoom,
                                      height=200 * self.zoom,
                                      bg='#202020')
        self.board_canvas.pack(side='left', anchor='nw')
        self.board_canvas.bind('<Button-1>', self.on_canvas_press)
        self.board_canvas.bind('<B1-Motion>', self.on_canvas_drag)
        self.board_canvas.bind('<Button-3>', self.on_canvas_right_click)
        self.hscroll = ttk.Scrollbar(board_frame, orient='horizontal',
                                     command=self.set_view_c)
        self.vscroll = ttk.Scrollbar(board_frame, orient='vertical',
                                     command=self.set_view_r)
        self.hscroll.pack(side='bottom', fill='x')
        self.vscroll.pack(side='right', fill='y')
        ttk.Label(board_frame, textvariable=self.board_info,
                  wraplength=620).pack(fill='x')

        side = ttk.Notebook(mid)
        side.pack(side='left', fill='y', padx=4)
        self.side_notebook = side
        self._build_object_editor(side)
        self._build_string_editor(side)
        self._build_header_editor(side)

    def _build_object_editor(self, notebook):
        page = ttk.Frame(notebook)
        notebook.add(page, text='Objects')
        self.object_list = self._listbox(page, height=10, width=24)
        form = ttk.Frame(page)
        form.pack(side='left', fill='both', expand=True, padx=4)
        self.object_vars: Dict[str, tk.StringVar] = {}
        for i, (key, label) in enumerate(BOARD_FIELDS):
            ttk.Label(form, text=label).grid(row=i // 2, column=(i % 2) * 2,
                                             sticky='w', padx=2)
            var = tk.StringVar(value='0')
            self.object_vars[key] = var
            ttk.Entry(form, textvariable=var, width=10).grid(
                row=i // 2, column=(i % 2) * 2 + 1, sticky='w', padx=2)
        self.object_list.bind('<<ListboxSelect>>', self.on_object_selected)
        buttons = ttk.Frame(page)
        buttons.pack(side='bottom', fill='x', pady=4)
        ttk.Button(buttons, text='Apply', command=self.apply_object).pack(side='left', padx=2)
        ttk.Button(buttons, text='Add', command=self.add_object).pack(side='left', padx=2)
        ttk.Button(buttons, text='Delete', command=self.delete_object).pack(side='left', padx=2)

    def _build_string_editor(self, notebook):
        page = ttk.Frame(notebook)
        notebook.add(page, text='Strings')
        self.string_list = self._listbox(page, height=12, width=30)
        self.string_list.bind('<<ListboxSelect>>', self.on_string_selected)
        form = ttk.Frame(page)
        form.pack(side='left', fill='both', expand=True, padx=4)
        self.string_var = tk.StringVar(value='')
        ttk.Entry(form, textvariable=self.string_var, width=28).pack(anchor='w', pady=4)
        ttk.Button(form, text='Apply', command=self.apply_string).pack(anchor='w', pady=2)
        ttk.Button(form, text='Add', command=self.add_string).pack(anchor='w', pady=2)
        ttk.Button(form, text='Delete', command=self.delete_string).pack(anchor='w', pady=2)
        ttk.Label(form, text='Strings are written in list order; point an\n'
                             'object at one with its String off/Seg fields.',
                  justify='left').pack(anchor='w', pady=8)

    def _build_header_editor(self, notebook):
        page = ttk.Frame(notebook)
        notebook.add(page, text='Header')
        self.header_vars: Dict[str, tk.StringVar] = {}
        row = 0
        for key, label in HEADER_FIELDS:
            ttk.Label(page, text=label).grid(row=row, column=0, sticky='w', padx=4, pady=2)
            var = tk.StringVar(value='0')
            self.header_vars[key] = var
            ttk.Entry(page, textvariable=var, width=12).grid(row=row, column=1, padx=4)
            row += 1
        ttk.Label(page, text='Key IDs (16, comma separated)').grid(
            row=row, column=0, sticky='w', padx=4, pady=2)
        self.keys_var = tk.StringVar(value=','.join(['0'] * 16))
        ttk.Entry(page, textvariable=self.keys_var, width=40).grid(
            row=row, column=1, padx=4)
        ttk.Button(page, text='Apply Header', command=self.apply_header).grid(
            row=row + 1, column=0, columnspan=2, pady=8)

    def _build_text_tab(self, parent):
        left = ttk.Frame(parent)
        left.pack(side='left', fill='y')
        ttk.Label(left, text='VCL text pages').pack(anchor='w', padx=4)
        self.page_list = self._listbox(left, height=28, width=22)
        self.page_list.bind('<<ListboxSelect>>', self.on_page_selected)

        right = ttk.Frame(parent)
        right.pack(side='left', fill='both', expand=True)
        self.page_preview = self._photo_label(right, 320, 200)
        ttk.Label(right, text='Lines (colour code + text)').pack(anchor='w', padx=4)
        self.line_list = self._listbox(right, height=10, width=56)
        self.line_list.bind('<<ListboxSelect>>', self.on_line_selected)

        form = ttk.Frame(right)
        form.pack(fill='x', padx=4, pady=4)
        ttk.Label(form, text='Colour').pack(side='left')
        self.colour_combo = ttk.Combobox(
            form, textvariable=self.colour_var,
            values=[str(i) for i in range(8)], width=4, state='readonly')
        self.colour_combo.pack(side='left', padx=4)
        self.line_entry = ttk.Entry(form, textvariable=self.line_var, width=44)
        self.line_entry.pack(side='left', padx=2)
        self.line_entry.bind('<Return>', lambda _event: self.add_line())
        ttk.Button(form, text='Apply', command=self.apply_line).pack(side='left', padx=2)
        ttk.Button(form, text='Add Line', command=self.add_line).pack(side='left', padx=2)
        ttk.Button(form, text='Delete', command=self.delete_line).pack(side='left', padx=2)

        actions = ttk.LabelFrame(right, text='Text pages', padding=6)
        actions.pack(fill='x', padx=4, pady=4)
        ttk.Button(actions, text='Save .vcl...', command=self.save_pages_vcl).grid(row=0, column=0, padx=2, pady=2, sticky='w')
        ttk.Button(actions, text='Import pages folder...', command=self.import_pages_folder).grid(row=0, column=1, padx=2, pady=2, sticky='w')
        ttk.Button(actions, text='Export page TXT...', command=self.export_page_txt).grid(row=1, column=0, padx=2, pady=2, sticky='w')

    def _build_sfx_tab(self, parent):
        left = ttk.Frame(parent)
        left.pack(side='left', fill='y')
        ttk.Label(left, text='VCL sampled sounds').pack(anchor='w', padx=4)
        self.sfx_list = self._listbox(left, height=28, width=30)
        self.sfx_list.bind('<<ListboxSelect>>', self.on_sfx_selected)

        right = ttk.Frame(parent)
        right.pack(side='left', fill='both', expand=True)
        ttk.Label(right, textvariable=self.sfx_info, wraplength=560,
                  justify='left').pack(anchor='w', padx=4, pady=4)
        actions = ttk.LabelFrame(right, text='Sound conversion', padding=6)
        actions.pack(fill='x', padx=4, pady=4)
        ttk.Button(actions, text='Play', command=self.play_sfx).grid(row=0, column=0, padx=2, pady=2, sticky='w')
        ttk.Button(actions, text='Export WAV...', command=self.export_sfx_wav).grid(row=0, column=1, padx=2, pady=2, sticky='w')
        ttk.Button(actions, text='Import WAV into Slot...', command=self.import_sfx_wav).grid(row=1, column=0, padx=2, pady=2, sticky='w')
        ttk.Button(actions, text='Export All WAVs...', command=self.export_all_sfx).grid(row=1, column=1, padx=2, pady=2, sticky='w')
        ttk.Button(actions, text='Import WAV Folder...', command=self.import_sfx_folder).grid(row=2, column=0, padx=2, pady=2, sticky='w')
        ttk.Button(actions, text='Save .vcl As...', command=self.save_vcl_file).grid(row=2, column=1, padx=2, pady=2, sticky='w')
        ttk.Label(right, text='Imports use jill_import: WAV -> unsigned 8-bit mono\n'
                              'at the slot rate (normally 6000 Hz).  Exports use\n'
                              'VCLFile.create_wav().',
                  justify='left').pack(anchor='w', padx=4, pady=8)

    # -- status / error handling --------------------------------------------

    def status(self, text: str) -> None:
        self.status_var.set(text)

    def run_action(self, fn, ok=None):
        """Run an operation, showing a friendly error instead of a crash."""
        try:
            result = fn()
            if ok is not None:
                self.status(ok(result) if callable(ok) else ok)
            return result
        except ji.ImportFailure as e:
            self.status(str(e))
            messagebox.showerror('Jill Editor', str(e), parent=self.root)
        except Exception as e:
            if os.environ.get('JILL_EDITOR_DEBUG'):
                traceback.print_exc()
            self.status(f'{type(e).__name__}: {e}')
            messagebox.showerror('Jill Editor', f'{type(e).__name__}: {e}',
                                 parent=self.root)
        return None

    def ask_out(self, title: str, source: Optional[Path], suffix: str,
                ext: str) -> Optional[Path]:
        initial = str(source.parent) if source else str(self.data_dir or self.base_dir)
        initial_file = ''
        if source:
            initial_file = f'{source.stem}_{suffix}{ext}'
        value = filedialog.asksaveasfilename(
            title=title, initialdir=initial, initialfile=initial_file,
            defaultextension=ext,
            filetypes=[('Jill data', f'*{ext}'), ('All', '*.*')],
            parent=self.root)
        return Path(value) if value else None

    # -- loading ------------------------------------------------------------

    def on_game_selected(self, _event=None):
        title = self.game_var.get()
        game = next((g for g in self.games if g['title'] == title), None)
        if game:
            self.load_game(game)

    def on_load_game(self):
        self.on_game_selected()

    def on_rescan(self):
        self.games = jx.discover_games(self.base_dir)
        self.game_combo.config(values=[g['title'] for g in self.games])
        if self.games:
            self.game_var.set(self.games[0]['title'])
        self.status(f'Found {len(self.games)} game(s).')

    def load_game(self, game: Dict[str, Any]):
        if self.board_dirty and not messagebox.askyesno(
                'Unsaved changes', 'Discard unsaved board changes?', parent=self.root):
            return
        self.current_game = game
        self.data_dir = Path(game['data_dir'])
        self.board_path = None
        self.board = None
        self.board_dirty = False
        self.sha_path = self.data_dir / game['sha']
        self.dma_path = self.data_dir / game['dma']
        self.vcl_path = self.data_dir / game['vcl']
        self.load_archives()
        self.refresh_boards()

    def open_files(self):
        start = str(self.data_dir or self.base_dir)
        sha = filedialog.askopenfilename(
            title='Shape archive (.sha)', initialdir=start,
            filetypes=[('Shape archive', '*.sha *.XR1 *.XR2 *.XR3'), ('All', '*.*')],
            parent=self.root)
        if not sha:
            return
        dma = filedialog.askopenfilename(
            title='Tile archive (.dma)', initialdir=str(Path(sha).parent),
            filetypes=[('Tile archive', '*.dma *.XR1 *.XR2 *.XR3'), ('All', '*.*')],
            parent=self.root)
        vcl = filedialog.askopenfilename(
            title='Sound/text archive (.vcl)', initialdir=str(Path(sha).parent),
            filetypes=[('Sound archive', '*.vcl *.XR1 *.XR2 *.XR3'), ('All', '*.*')],
            parent=self.root)
        self.sha_path = Path(sha)
        self.dma_path = Path(dma) if dma else None
        self.vcl_path = Path(vcl) if vcl else None
        self.data_dir = self.sha_path.parent
        self.load_archives()
        self.refresh_boards()

    def load_archives(self):
        self.sha = self.dma = self.vcl = None
        self._tile_cache.clear()
        self._tile_photos.clear()
        self._shape_cache.clear()
        self.pages = {}
        if self.sha_path and self.sha_path.is_file():
            try:
                self.sha = jx.SHAFile.load(str(self.sha_path))
                load_palette(self.sha, self.current_game, self.base_dir)
            except Exception as e:
                self.status(f'Cannot load shape archive: {e}')
        if self.dma_path and self.dma_path.is_file():
            try:
                self.dma = jx.DMAFile.load(str(self.dma_path))
            except Exception as e:
                self.status(f'Cannot load tile archive: {e}')
        if self.vcl_path and self.vcl_path.is_file():
            try:
                self.vcl = jx.VCLFile.load(str(self.vcl_path))
            except Exception as e:
                self.status(f'Cannot load sound archive: {e}')
        self.refresh_sprites()
        self.refresh_tiles()
        self.refresh_pages()
        self.refresh_sfx()
        if self.board:
            self.draw_board_view()
        self.status(f'Loaded {self.sha_path.name if self.sha_path else "no .sha"}'
                    f' / {self.dma_path.name if self.dma_path else "no .dma"}'
                    f' / {self.vcl_path.name if self.vcl_path else "no .vcl"}')

    def save_current(self):
        tab = self.tabs.index(self.tabs.select())
        labels = [self.tabs.tab(i, 'text') for i in range(self.tabs.index('end'))]
        name = labels[tab]
        actions = {'Sprites': self.import_sprite_png, 'Tiles': self.import_tile_png,
                   'Levels': self.save_board_file, 'Text': self.save_pages_vcl,
                   'SFX': self.save_vcl_file}
        actions.get(name, self.save_board_file)()

    def export_pack(self):
        if not all((self.sha, self.dma, self.vcl, self.current_game)):
            messagebox.showinfo('Export Pack', 'Load a full game first.',
                                parent=self.root)
            return
        out = filedialog.askdirectory(title='Export reference pack into',
                                      parent=self.root)
        if not out:
            return
        def work():
            return jx.export_reference_pack(self.current_game, self.sha, self.vcl,
                                            self.dma, Path(out))
        self.run_action(work, lambda stats: f'Exported pack: {stats}')

    # -- sprites ------------------------------------------------------------

    def refresh_sprites(self):
        self.sprite_list.delete(0, 'end')
        if not self.sha:
            self.sprite_info.set('No shape archive loaded.')
            return
        for i, entry in enumerate(self.sha.entries):
            if entry is None:
                self.sprite_list.insert('end', f'Entry {i:03d}: EMPTY')
            else:
                self.sprite_list.insert(
                    'end', f'Entry {i:03d}: {entry.frame_count} frames, '
                           f'{entry.src_bpp}bpp')
        self.status(f'{len(self.sha.entries)} shape entries loaded.')

    def on_sprite_selected(self, _event=None):
        sel = self.sprite_list.curselection()
        if not sel or not self.sha:
            return
        idx = sel[0]
        entry = self.sha.entries[idx]
        self.entry_var.set(str(idx))
        frames = len(entry.frames) if entry else 0
        try:
            frame = max(0, min(int(self.frame_var.get() or 0), frames - 1))
        except ValueError:
            frame = 0
        self.frame_var.set(str(frame))
        self.show_sprite_frame(idx, frame)

    def step_frame(self, delta: int):
        try:
            frame = int(self.frame_var.get() or 0) + delta
        except ValueError:
            frame = 0
        frames = len(self.sha.entries[int(self.entry_var.get() or 0)].frames) \
            if self.sha and self.sha.entries[int(self.entry_var.get() or 0)] else 0
        frame = max(0, min(frame, frames - 1))
        self.frame_var.set(str(frame))
        self.show_sprite_frame(int(self.entry_var.get() or 0), frame)

    def show_sprite_frame(self, entry_idx: int, frame_idx: int):
        if not self.sha:
            return
        entry = self.sha.entries[entry_idx]
        self.sprite_info.set(
            f'Entry {entry_idx} frame {frame_idx}'
            + (f' ({len(entry.frames)} frames)' if entry else ' (empty)'))
        if entry is None or frame_idx >= len(entry.frames):
            self.sprite_preview.config(image='')
            return
        img = jx._shape_rgba(self.sha, entry_idx, frame_idx)
        if img is None:
            self.sprite_preview.config(image='')
            return
        scale = max(1, min(8, 256 // max(1, img.width)))
        self._preview_photo = ImageTk.PhotoImage(
            img.resize((img.width * scale, img.height * scale), Image.NEAREST))
        self.sprite_preview.config(image=self._preview_photo)

    def selected_sprite(self) -> Tuple[int, int]:
        try:
            return int(self.entry_var.get()), int(self.frame_var.get() or 0)
        except ValueError:
            return 0, 0

    def export_sprite_png(self):
        if not self.sha:
            return
        entry, frame = self.selected_sprite()
        img = jx._shape_rgba(self.sha, entry, frame)
        if img is None:
            self.status(f'No frame at {entry}:{frame}.')
            return
        out = filedialog.asksaveasfilename(
            title='Export frame as PNG', defaultextension='.png',
            initialfile=f'shape_{entry:03d}_{frame:03d}.png',
            filetypes=[('PNG', '*.png')], parent=self.root)
        if out:
            self.run_action(lambda: img.save(out), f'Exported {out}')

    def import_sprite_png(self):
        if not self.sha or not self.sha_path:
            return
        entry, frame = self.selected_sprite()
        src = filedialog.askopenfilename(title='PNG for the selected frame',
                                         filetypes=[('Images', '*.png *.bmp *.gif *.jpg'),
                                                    ('All', '*.*')], parent=self.root)
        if not src:
            return
        out = self.ask_out('Save modified .sha', self.sha_path, 'mod', '.sha')
        if not out:
            return
        def work():
            path = ji.replace_sha_frame(self.sha_path, entry, frame, Path(src), out,
                                        adapt_palette=self.adapt_palette_var.get())
            self.sha_path = path
            self.load_archives()
            return path
        self.run_action(work, lambda path: f'Replaced {entry}:{frame} -> {path}')

    def import_sprites_folder(self):
        if not self.sha_path:
            return
        folder = filedialog.askdirectory(title='Folder of shape_NNN/frame_NNN.png',
                                         parent=self.root)
        if not folder:
            return
        out = self.ask_out('Save rebuilt .sha', self.sha_path, 'new', '.sha')
        if not out:
            return
        def work():
            path = ji.sha_from_folder(self.sha_path, Path(folder), out)
            self.sha_path = path
            self.load_archives()
            return path
        self.run_action(work, lambda path: f'Rebuilt shapes -> {path}')

    def import_sprite_animation(self):
        """Replace every frame of the selected entry with a folder of images."""
        if not self.sha or not self.sha_path:
            return
        entry, _frame = self.selected_sprite()
        folder = filedialog.askdirectory(
            title=f'Folder of frames for shape entry {entry} '
                  '(frame_0.png, frame_1.png, ...)', parent=self.root)
        if not folder:
            return
        out = self.ask_out('Save modified .sha', self.sha_path, 'anim', '.sha')
        if not out:
            return
        def work():
            path = ji.sha_entry_from_folder(
                self.sha_path, entry, Path(folder), out,
                adapt_palette=self.adapt_palette_var.get())
            self.sha_path = path
            self.load_archives()
            return path
        self.run_action(work, lambda path: f'Animation imported -> {path}')

    def import_font_sheet(self):
        if not self.sha_path:
            return
        sheet = filedialog.askopenfilename(title='fonts/main.png',
                                           filetypes=[('PNG', '*.png')],
                                           parent=self.root)
        if not sheet:
            return
        out = self.ask_out('Save .sha with new font', self.sha_path, 'font', '.sha')
        if not out:
            return
        def work():
            path = ji.font_from_sheet(self.sha_path, Path(sheet), out)
            self.sha_path = path
            self.load_archives()
            return path
        self.run_action(work, lambda path: f'Font imported -> {path}')

    # -- tiles --------------------------------------------------------------

    def refresh_tiles(self):
        self.tile_list.delete(0, 'end')
        if not self.dma:
            self.tile_info.set('No tile archive loaded.')
            return
        for rec in self.dma.records:
            entry = (rec.shape_id >> 8) & 0x3F
            frame = rec.shape_id & 0xFF
            self.tile_list.insert(
                'end', f'0x{rec.tile_id:03X}: {rec.name[:14]:14s} {entry:02X}:{frame:02X}')
        self.status(f'{len(self.dma.records)} DMA tiles loaded.')

    def on_tile_selected(self, _event=None):
        sel = self.tile_list.curselection()
        if not sel or not self.dma or not self.sha:
            return
        rec = self.dma.records[sel[0]]
        self.tile_id = rec.tile_id
        self.tile_var.set(str(rec.tile_id))
        entry = (rec.shape_id >> 8) & 0x3F
        frame = rec.shape_id & 0xFF
        self.tile_info.set(f'tile 0x{rec.tile_id:02X} -> shape {entry}:{frame}, '
                           f'attr 0x{rec.attr:04X}, name "{rec.name}"')
        img = tile_image(self.sha, self.dma, rec.tile_id, self._tile_cache)
        if img is None:
            self.tile_preview.config(image='')
            return
        scale = max(1, min(8, 128 // max(1, img.width)))
        self._preview_photo = ImageTk.PhotoImage(
            img.resize((img.width * scale, img.height * scale), Image.NEAREST))
        self.tile_preview.config(image=self._preview_photo)

    def use_tile_in_levels(self):
        try:
            value = int(self.tile_var.get(), 0)
        except ValueError:
            value = self.tile_id
        self.edit_tile_var.set(str(value))
        self.status(f'Paint tile set to 0x{value:02X}; switch to Levels.')

    def export_tile_png(self):
        if not all((self.sha, self.dma)):
            return
        try:
            tile_id = int(self.tile_var.get(), 0)
        except ValueError:
            return
        img = tile_image(self.sha, self.dma, tile_id, self._tile_cache)
        if img is None:
            self.status(f'No art for tile 0x{tile_id:02X}.')
            return
        out = filedialog.asksaveasfilename(
            title='Export tile as PNG', defaultextension='.png',
            initialfile=f'{tile_id}.png', filetypes=[('PNG', '*.png')],
            parent=self.root)
        if out:
            self.run_action(lambda: img.save(out), f'Exported {out}')

    def import_tile_png(self):
        if not all((self.sha, self.dma, self.sha_path, self.dma_path)):
            return
        try:
            tile_id = int(self.tile_var.get(), 0)
        except ValueError:
            return
        src = filedialog.askopenfilename(title='PNG for tile art',
                                         filetypes=[('Images', '*.png *.bmp *.gif *.jpg'),
                                                    ('All', '*.*')], parent=self.root)
        if not src:
            return
        out = self.ask_out('Save modified .sha', self.sha_path, 'tile', '.sha')
        if not out:
            return

        def work():
            path, entry, frame = ji.tile_from_png(
                self.sha_path, self.dma_path, tile_id, Path(src), out,
                dither=self.dither_var.get())
            self.sha_path = path
            self.load_archives()
            return f'{path} (shape {entry}:{frame})'
        self.run_action(work, lambda msg: f'Tile art imported -> {msg}')

    # -- levels -------------------------------------------------------------

    def refresh_boards(self):
        names: List[str] = []
        if self.current_game and self.data_dir:
            ext = self.current_game.get('board_ext', '.jn1')
            names = sorted(p.name for p in self.data_dir.glob(f'*{ext}'))
        elif self.data_dir:
            names = sorted(p.name for p in self.data_dir.glob('*.jn*'))
        self.board_combo.config(values=names)
        if names:
            self.board_combo.current(0)

    def on_board_selected(self, _event=None):
        self.open_selected_board()

    def open_selected_board(self):
        name = self.board_combo.get()
        if not name or not self.data_dir:
            return
        if self.board_dirty and not messagebox.askyesno(
                'Unsaved changes', 'Discard unsaved board changes?', parent=self.root):
            return
        self.open_board(self.data_dir / name)

    def open_board(self, path: Path):
        def work():
            board = jx._load_board_for_game(self.current_game or {}, path)
            self.board = board
            self.board_path = path
            self.board_dirty = False
            self.view_c = self.view_r = 0
            self.object_idx = None
            self.board_info.set(f'{path.name}: {len(board.objects)} objects, '
                                f'{len(board.strings)} strings')
            self.refresh_objects()
            self.refresh_strings()
            self.load_header()
            self.draw_board_view()
            return path
        self.run_action(work, lambda p: f'Opened board {p}')

    def draw_board_view(self):
        if not self.board or not self.sha or not self.dma:
            return
        z = self.zoom
        self.board_canvas.config(width=320 * z, height=200 * z)
        view = Image.new('RGBA', (320, 200), (*jx._palette_rgb(self.sha, 0), 255))
        for dy in range(min(12, 64 - self.view_r)):
            for dx in range(min(20, 128 - self.view_c)):
                c, r = self.view_c + dx, self.view_r + dy
                img = tile_image(self.sha, self.dma,
                                 self.board.get_tile_at(c, r), self._tile_cache)
                if img is not None:
                    view.paste(img, (dx * 16, dy * 16), img)
        if self.grid_var.get():
            draw = ImageDraw.Draw(view)
            for x in range(0, 321, 16):
                draw.line((x, 0, x, 199), fill=(90, 90, 90, 90))
            for y in range(0, 201, 16):
                draw.line((0, y, 319, y), fill=(90, 90, 90, 90))
        self._board_photo = ImageTk.PhotoImage(
            view.resize((320 * z, 200 * z), Image.NEAREST))
        self.board_canvas.delete('all')
        self.board_canvas.create_image(0, 0, anchor='nw', image=self._board_photo)
        if self.show_links_var.get():
            self._draw_object_links(z)
        if self.show_kinds_var.get():
            self._draw_object_kinds(z)
        self.hscroll.set(self.view_c / 108, min(1.0, (self.view_c + 20) / 108))
        self.vscroll.set(self.view_r / 52, min(1.0, (self.view_r + 12) / 52))

    def _view_pos(self, bx: int, by: int, z: int):
        """Board pixels -> canvas pixels for the current scroll offset."""
        return ((bx - self.view_c * 16) * z, (by - self.view_r * 16) * z)

    def _draw_object_kinds(self, z: int):
        for i, obj in enumerate(self.board.objects):
            x, y = self._view_pos(obj.x, obj.y, z)
            w, h = max(4, obj.width * z), max(4, obj.height * z)
            if x + w < -8 or y + h < -8 or x > 320 * z + 8 or y > 200 * z + 8:
                continue
            colour = KIND_COLORS[obj.kind % len(KIND_COLORS)]
            self.board_canvas.create_rectangle(
                x, y, x + w, y + h, outline=colour, width=2)
            label = obj.get_kind_name()[:3].upper() or f'K{obj.kind}'
            self.board_canvas.create_text(
                x + 2, y + 1, text=f'{i}:{label}', anchor='nw', fill=colour,
                font=('Consolas', max(7, 7 + z)))

    def _draw_object_links(self, z: int):
        groups: Dict[int, List[int]] = {}
        for i, obj in enumerate(self.board.objects):
            if obj.primary_param:
                groups.setdefault(obj.primary_param, []).append(i)
        for param, indexes in groups.items():
            if len(indexes) < 2:
                continue
            points = []
            for i in indexes:
                obj = self.board.objects[i]
                points.append((self._view_pos(
                    obj.x + max(4, obj.width) // 2,
                    obj.y + max(4, obj.height) // 2, z), i))
            for (a, ia), (b, ib) in zip(points, points[1:]):
                if (min(a[0], b[0]) > 320 * z or max(a[0], b[0]) < 0 or
                        min(a[1], b[1]) > 200 * z or max(a[1], b[1]) < 0):
                    continue
                self.board_canvas.create_line(
                    a[0], a[1], b[0], b[1], fill='#FFFF00', width=2,
                    dash=(4, 2))

    def zoom_in(self):
        self.zoom = ZOOM_LEVELS[min(ZOOM_LEVELS.index(self.zoom) + 1,
                                    len(ZOOM_LEVELS) - 1)]
        self.zoom_label.config(text=f'{self.zoom}x')
        self.draw_board_view()

    def zoom_out(self):
        self.zoom = ZOOM_LEVELS[max(ZOOM_LEVELS.index(self.zoom) - 1, 0)]
        self.zoom_label.config(text=f'{self.zoom}x')
        self.draw_board_view()

    def set_view_c(self, *args):
        mode = args[0] if args else 'set'
        value = args[1] if len(args) > 1 else '0'
        if mode == 'scroll':
            delta = -1 if value == '1' else 1
            self.view_c = max(0, min(108, self.view_c + delta))
        elif mode in ('set', 'moveto'):
            try:
                self.view_c = max(0, min(108, int(float(value) * 108)))
            except ValueError:
                return
        self.draw_board_view()

    def set_view_r(self, *args):
        mode = args[0] if args else 'set'
        value = args[1] if len(args) > 1 else '0'
        if mode == 'scroll':
            delta = -1 if value == '1' else 1
            self.view_r = max(0, min(52, self.view_r + delta))
        elif mode in ('set', 'moveto'):
            try:
                self.view_r = max(0, min(52, int(float(value) * 52)))
            except ValueError:
                return
        self.draw_board_view()

    def _board_pixel(self, event):
        z = self.zoom
        return (self.view_c * 16 + event.x // z,
                self.view_r * 16 + event.y // z)

    def object_at(self, event) -> Optional[int]:
        """Index of the object under the cursor, or None."""
        if not self.board:
            return None
        bx, by = self._board_pixel(event)
        for i in range(len(self.board.objects) - 1, -1, -1):
            obj = self.board.objects[i]
            if (obj.x - 2 <= bx <= obj.x + max(4, obj.width) + 2 and
                    obj.y - 2 <= by <= obj.y + max(4, obj.height) + 2):
                return i
        return None

    def paint_at(self, event, erase_tile: bool = False):
        if not self.board:
            return
        c = self.view_c + event.x // (16 * self.zoom)
        r = self.view_r + event.y // (16 * self.zoom)
        if not (0 <= c < 128 and 0 <= r < 64):
            return
        if erase_tile:
            try:
                tile_id = int(self.bg_tile_var.get(), 0) & 0x3FFF
            except ValueError:
                return
        else:
            try:
                tile_id = int(self.edit_tile_var.get(), 0) & 0x3FFF
            except ValueError:
                return
        idx = c * 64 + r
        if self.board.tile_map[idx] == tile_id:
            return
        self.board.tile_map[idx] = tile_id
        self.board_dirty = True
        self.board_info.set(f'{self.board_path.name if self.board_path else "board"}*: '
                            f'{"erased tile" if erase_tile else "painted"} '
                            f'0x{tile_id:02X} at ({c},{r})')
        self.draw_board_view()

    def on_canvas_press(self, event):
        if not self.board:
            return
        if event.state & 0x0001:  # Shift: remove the object under the cursor
            idx = self.object_at(event)
            if idx is not None:
                self.delete_object_at(idx)
            return
        if event.state & 0x0002:  # Ctrl: paint the erase/background tile
            self.paint_at(event, erase_tile=True)
            return
        self.paint_at(event)

    def on_canvas_drag(self, event):
        if event.state & 0x0001 or event.state & 0x0002:
            return
        self.paint_at(event)

    def on_canvas_right_click(self, event):
        idx = self.object_at(event)
        menu = tk.Menu(self.root, tearoff=0)
        if idx is None:
            menu.add_command(label='No object here', state='disabled')
        else:
            obj = self.board.objects[idx]
            menu.add_command(
                label=f'Object {idx}: {obj.get_kind_name()} '
                      f'({obj.x},{obj.y})', state='disabled')
            menu.add_separator()
            menu.add_command(label='Edit properties',
                             command=lambda: self.edit_object_from_canvas(idx))
            if obj.primary_param:
                menu.add_command(
                    label=f'Select links (param {obj.primary_param})',
                    command=lambda: self.select_linked_objects(obj.primary_param))
            menu.add_command(label='Delete object',
                             command=lambda: self.delete_object_at(idx))
        menu.tk_popup(event.x_root, event.y_root)

    def edit_object_from_canvas(self, idx: int):
        self.side_notebook.select(0)
        self.object_list.selection_clear(0, 'end')
        self.object_list.selection_set(idx)
        self.object_list.see(idx)
        self.on_object_selected()

    def select_linked_objects(self, param: int):
        self.side_notebook.select(0)
        self.object_list.selection_clear(0, 'end')
        found = False
        for i, obj in enumerate(self.board.objects):
            if obj.primary_param == param:
                self.object_list.selection_set(i)
                found = True
        if found:
            self.object_list.see(0)
            self.on_object_selected()
            self.board_info.set(f'Selected {self.object_list.size()} '
                                f'objects linked to param {param}')

    def delete_object_at(self, idx: int):
        def work():
            del self.board.objects[idx]
            self.object_idx = None
            self.board_dirty = True
            self.refresh_objects()
            self.refresh_strings()
            self.draw_board_view()
            return len(self.board.objects)
        self.run_action(work, lambda n: f'Deleted object; {n} remain')

    def save_board_file(self):
        if not self.board:
            return
        out = self.ask_out('Save board', self.board_path, 'mod', '.jn1')
        if not out:
            return
        def work():
            path = ji.save_board(self.board, self.board_path or out, out)
            self.board_path = path
            self.board_dirty = False
            self.board_info.set(f'Saved {path.name}: {len(self.board.objects)} objects')
            return path
        self.run_action(work, lambda path: f'Saved board {path}')

    def import_board_render(self):
        if not all((self.board, self.board_path, self.sha_path, self.dma_path)):
            return
        src = filedialog.askopenfilename(
            title='Full 2048x1024 board render',
            filetypes=[('PNG', '*.png')], parent=self.root)
        if not src:
            return
        out = self.ask_out('Save rebuilt board', self.board_path, 'render', '.jn1')
        if not out:
            return
        def work():
            path = ji.board_from_image(self.board_path, self.sha_path,
                                       self.dma_path, Path(src), out)
            self.open_board(path)
            return path
        self.run_action(work, lambda path: f'Imported render -> {path}')

    def export_board_render(self):
        if not all((self.board, self.sha, self.dma)):
            return
        out = filedialog.asksaveasfilename(
            title='Export full board render', defaultextension='.png',
            initialfile=f'{(self.board_path or Path("board")).stem}_full.png',
            filetypes=[('PNG', '*.png')], parent=self.root)
        if not out:
            return
        def work():
            img = jx._render_board(self.board, self.sha, self.dma)
            img.save(out)
            return out
        self.run_action(work, lambda p: f'Exported render {p}')

    # -- board objects / strings / header -----------------------------------

    def refresh_objects(self):
        self.object_list.delete(0, 'end')
        if not self.board:
            return
        for i, obj in enumerate(self.board.objects):
            self.object_list.insert(
                'end', f'{i:03d} {obj.get_kind_name():10s} '
                       f'({obj.x},{obj.y}) {obj.width}x{obj.height}')
        if self.board.objects:
            self.object_list.selection_set(0)
            self.on_object_selected()

    def on_object_selected(self, _event=None):
        if not self.board:
            return
        sel = self.object_list.curselection()
        if not sel:
            return
        self.object_idx = sel[0]
        obj = self.board.objects[self.object_idx]
        for key, _label in BOARD_FIELDS:
            self.object_vars[key].set(str(getattr(obj, key, 0)))

    def apply_object(self):
        if not self.board or self.object_idx is None:
            return
        def work():
            obj = self.board.objects[self.object_idx]
            for key, _label in BOARD_FIELDS:
                setattr(obj, key, int(self.object_vars[key].get(), 0))
            self.board_dirty = True
            self.refresh_objects()
            self.object_list.selection_set(self.object_idx)
            return self.object_idx
        self.run_action(work, lambda i: f'Applied object {i}')

    def add_object(self):
        if not self.board:
            return
        def work():
            obj = jx.BoardObject()
            for key, _label in BOARD_FIELDS:
                if key in self.object_vars:
                    try:
                        setattr(obj, key, int(self.object_vars[key].get(), 0))
                    except ValueError:
                        pass
            self.board.objects.append(obj)
            self.object_idx = len(self.board.objects) - 1
            self.board_dirty = True
            self.refresh_objects()
            self.object_list.selection_set(self.object_idx)
            return self.object_idx
        self.run_action(work, lambda i: f'Added object {i}')

    def delete_object(self):
        if not self.board or self.object_idx is None:
            return
        def work():
            del self.board.objects[self.object_idx]
            self.object_idx = None
            self.board_dirty = True
            self.refresh_objects()
            return len(self.board.objects)
        self.run_action(work, lambda n: f'{n} objects remain')

    def refresh_strings(self):
        self.string_list.delete(0, 'end')
        if not self.board:
            return
        for i, text in enumerate(self.board.strings):
            self.string_list.insert('end', f'{i:02d}: {text[:40]}')
        if self.board.strings:
            self.string_list.selection_set(0)
            self.on_string_selected()

    def on_string_selected(self, _event=None):
        if not self.board:
            return
        sel = self.string_list.curselection()
        if sel:
            self.string_var.set(self.board.strings[sel[0]])

    def apply_string(self):
        sel = self.string_list.curselection()
        if not self.board or not sel:
            return
        def work():
            self.board.strings[sel[0]] = self.string_var.get()
            self.board_dirty = True
            self.refresh_strings()
            self.string_list.selection_set(sel[0])
            return sel[0]
        self.run_action(work, lambda i: f'Applied string {i}')

    def add_string(self):
        if not self.board:
            return
        def work():
            self.board.strings.append(self.string_var.get())
            self.board_dirty = True
            self.refresh_strings()
            return len(self.board.strings) - 1
        self.run_action(work, lambda i: f'Added string {i}')

    def delete_string(self):
        sel = self.string_list.curselection()
        if not self.board or not sel:
            return
        def work():
            del self.board.strings[sel[0]]
            self.board_dirty = True
            self.refresh_strings()
            return len(self.board.strings)
        self.run_action(work, lambda n: f'{n} strings remain')

    def load_header(self):
        if not self.board:
            return
        h = self.board.header
        for key, _label in HEADER_FIELDS:
            self.header_vars[key].set(str(getattr(h, key, 0)))
        self.keys_var.set(','.join(str(i) for i in h.key_ids[:16]))

    def apply_header(self):
        if not self.board:
            return
        def work():
            h = self.board.header
            for key, _label in HEADER_FIELDS:
                setattr(h, key, int(self.header_vars[key].get(), 0))
            ids = [int(p.strip(), 0) for p in self.keys_var.get().split(',')
                   if p.strip()]
            if len(ids) != 16:
                raise ji.ImportFailure('Key IDs must contain exactly 16 values')
            h.key_ids = ids[:16]
            self.board_dirty = True
            return h.level_id
        self.run_action(work, lambda level: f'Header applied (level {level})')

    # -- text pages ---------------------------------------------------------

    def refresh_pages(self):
        self.page_list.delete(0, 'end')
        if not self.vcl:
            return
        self.pages = {i: page_lines(self.vcl, i) for i in range(PAGE_COUNT)}
        for i, lines in self.pages.items():
            marker = '*' if lines else ' '
            self.page_list.insert('end', f'{marker} Page {i:02d} ({len(lines)} lines)')
        self.page_list.selection_set(0)
        self.on_page_selected()

    def on_page_selected(self, _event=None):
        sel = self.page_list.curselection()
        if not sel:
            return
        self.page_idx = sel[0]
        self.show_page()

    def show_page(self):
        lines = self.pages.get(self.page_idx, [])
        self.line_list.delete(0, 'end')
        for i, line in enumerate(lines):
            self.line_list.insert('end', f'[{line["colour"]}] {line["text"]}')
        self.line_idx = None
        self.line_var.set('')
        self.colour_var.set('7')
        self._render_page_preview(lines)

    def _render_page_preview(self, lines):
        if not self.sha:
            return
        img = Image.new('RGBA', (320, 200), (*jx._palette_rgb(self.sha, 0), 255))
        draw = ImageDraw.Draw(img)
        draw.rectangle([6, 4, 313, 195], fill=(*jx._palette_rgb(self.sha, 8), 255))
        draw.rectangle([7, 5, 312, 194], fill=(*jx._palette_rgb(self.sha, 0), 255))
        for i, line in enumerate(lines[:18]):
            colour = jx._page_colour(line['colour'])
            rgb = jx._palette_rgb(self.sha, colour)
            x = max(10, (320 - len(line['text']) * 6) // 2)
            jx._render_text(img, self.sha, x, 10 + i * 9, line['text'], rgb)
        self._page_photo = ImageTk.PhotoImage(img.resize((640, 400), Image.NEAREST))
        self.page_preview.config(image=self._page_photo)

    def _select_line(self, index: Optional[int]):
        """Restore a line selection after show_page() rebuilt the list."""
        if index is None or index < 0:
            return
        self.line_list.selection_clear(0, 'end')
        self.line_list.selection_set(index)
        self.line_list.see(index)
        self.line_idx = index
        self.on_line_selected()

    def add_line(self):
        """Append a new line to the current page, regardless of selection."""
        lines = self.pages.setdefault(self.page_idx, [])
        entry = {'colour': int(self.colour_var.get() or 7),
                 'text': self.line_var.get()}
        def work():
            lines.append(entry)
            self.show_page()
            self._select_line(len(lines) - 1)
            return self.page_idx
        self.run_action(work, lambda idx: f'Added line to page {idx}')

    def apply_line(self):
        lines = self.pages.setdefault(self.page_idx, [])
        text = self.line_var.get()
        colour = int(self.colour_var.get() or 7)
        def work():
            entry = {'colour': colour, 'text': text}
            chosen = self.line_idx
            if chosen is None or chosen >= len(lines):
                lines.append(entry)
                chosen = len(lines) - 1
            else:
                lines[chosen] = entry
            self.show_page()
            # show_page() resets line_idx, so restore the selection afterwards.
            self._select_line(chosen)
            return self.page_idx
        self.run_action(work, lambda idx: f'Page {idx} updated')

    def delete_line(self):
        sel = self.line_list.curselection()
        lines = self.pages.get(self.page_idx, [])
        if not sel or not lines:
            return
        def work():
            del lines[sel[0]]
            self.show_page()
            return self.page_idx
        self.run_action(work, lambda idx: f'Line removed from page {idx}')

    def on_line_selected(self, _event=None):
        # Hook for external callers; the listbox is informational by design.
        sel = self.line_list.curselection()
        if not sel:
            return
        line = self.pages[self.page_idx][sel[0]]
        self.line_idx = sel[0]
        self.line_var.set(line['text'])
        self.colour_var.set(str(line['colour']))

    def save_pages_vcl(self):
        if not self.vcl_path:
            return
        out = self.ask_out('Save VCL with edited pages', self.vcl_path, 'text', '.vcl')
        if not out:
            return
        def work():
            path = ji.vcl_set_pages(self.vcl_path, self.pages, out)
            self.vcl_path = path
            self.load_archives()
            return path
        self.run_action(work, lambda path: f'Saved text pages -> {path}')

    def import_pages_folder(self):
        if not self.vcl_path:
            return
        folder = filedialog.askdirectory(title='text_pages folder (pages.json)',
                                         parent=self.root)
        if not folder:
            return
        out = self.ask_out('Save VCL with imported pages', self.vcl_path,
                           'pages', '.vcl')
        if not out:
            return
        def work():
            path = ji.vcl_pages_from_folder(self.vcl_path, Path(folder), out)
            self.vcl_path = path
            self.load_archives()
            return path
        self.run_action(work, lambda path: f'Imported pages -> {path}')

    def export_page_txt(self):
        lines = self.pages.get(self.page_idx, [])
        out = filedialog.asksaveasfilename(
            title='Export page as TXT', defaultextension='.txt',
            initialfile=f'page_{self.page_idx:02d}.txt',
            filetypes=[('Text', '*.txt')], parent=self.root)
        if not out:
            return
        def work():
            Path(out).write_text('\n'.join(l['text'] for l in lines) + '\n',
                                 encoding='ascii', errors='replace')
            return out
        self.run_action(work, lambda p: f'Exported {p}')

    # -- SFX ----------------------------------------------------------------

    def refresh_sfx(self):
        self.sfx_list.delete(0, 'end')
        if not self.vcl:
            self.sfx_info.set('No sound archive loaded.')
            return
        for i in range(SFX_COUNT):
            data = self.vcl.extract_sfx(i)
            size = len(data) if data else 0
            rate = self.vcl.sfx_rates[i] or 6000
            self.sfx_list.insert(
                'end', f'SFX {i:02d}: {size:6d} B @ {rate} Hz'
                       + ('' if data else ' (empty)'))
        self.status(f'{sum(1 for i in range(SFX_COUNT) if self.vcl.extract_sfx(i))} SFX slots loaded.')

    def on_sfx_selected(self, _event=None):
        sel = self.sfx_list.curselection()
        if not sel or not self.vcl:
            return
        i = sel[0]
        data = self.vcl.extract_sfx(i)
        rate = self.vcl.sfx_rates[i] or 6000
        self.sfx_info.set(f'Slot {i}: {len(data) if data else 0} bytes, '
                          f'{rate} Hz, '
                          f'~{(len(data) / rate if data else 0):.2f}s')

    def selected_sfx(self) -> int:
        sel = self.sfx_list.curselection()
        return sel[0] if sel else 0

    def play_sfx(self):
        if not self.vcl or not jx.HAS_WINSOUND:
            self.status('Audio playback needs winsound (Windows only).')
            return
        i = self.selected_sfx()
        data = self.vcl.create_wav(i, 'sfx')
        if not data:
            self.status(f'Slot {i} is empty.')
            return
        path = Path(tempfile.gettempdir()) / f'jill_editor_sfx_{i}.wav'
        path.write_bytes(data)
        self.run_action(lambda: jx.play_wav_file(path), f'Playing SFX {i}')

    def export_sfx_wav(self):
        if not self.vcl:
            return
        i = self.selected_sfx()
        data = self.vcl.create_wav(i, 'sfx')
        if not data:
            self.status(f'Slot {i} is empty.')
            return
        out = filedialog.asksaveasfilename(
            title=f'Export SFX {i} as WAV', defaultextension='.wav',
            initialfile=f'sfx_{i:02d}.wav', filetypes=[('WAV', '*.wav')],
            parent=self.root)
        if out:
            def work():
                Path(out).write_bytes(data)
                return out
            self.run_action(work, lambda p: f'Exported {p}')

    def export_all_sfx(self):
        if not self.vcl:
            return
        out = filedialog.askdirectory(title='Folder for exported WAVs',
                                      parent=self.root)
        if not out:
            return
        def work():
            count = 0
            for i in range(SFX_COUNT):
                data = self.vcl.create_wav(i, 'sfx')
                if data:
                    (Path(out) / f'sfx_{i:02d}.wav').write_bytes(data)
                    count += 1
            return count
        self.run_action(work, lambda n: f'Exported {n} WAV files to {out}')

    def import_sfx_wav(self):
        if not self.vcl or not self.vcl_path:
            return
        slot = self.selected_sfx()
        src = filedialog.askopenfilename(
            title=f'WAV for SFX slot {slot}',
            filetypes=[('WAV', '*.wav'), ('All', '*.*')], parent=self.root)
        if not src:
            return
        out = self.ask_out('Save modified .vcl', self.vcl_path, 'sfx', '.vcl')
        if not out:
            return
        def work():
            path = ji.replace_vcl_sfx(self.vcl_path, slot, Path(src), out)
            self.vcl_path = path
            self.load_archives()
            return path
        self.run_action(work, lambda path: f'Imported WAV into SFX {slot} -> {path}')

    def import_sfx_folder(self):
        if not self.vcl_path:
            return
        folder = filedialog.askdirectory(title='Folder of sfx_NN.wav files',
                                         parent=self.root)
        if not folder:
            return
        out = self.ask_out('Save rebuilt .vcl', self.vcl_path, 'new', '.vcl')
        if not out:
            return
        def work():
            path = ji.vcl_from_folder(self.vcl_path, Path(folder), out)
            self.vcl_path = path
            self.load_archives()
            return path
        self.run_action(work, lambda path: f'Imported SFX folder -> {path}')

    def save_vcl_file(self):
        """Save the current text-page edits as a .vcl (same as Save Pages)."""
        self.save_pages_vcl()


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

def run_gui(base_dir: Optional[Path] = None) -> int:
    if tk is None:
        print('Tkinter is required for the GUI (python3-tk on Linux).',
              file=sys.stderr)
        return 2
    root = tk.Tk()
    JillEditorApp(root, base_dir)
    root.mainloop()
    return 0


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description='Jill asset editor')
    parser.add_argument('base_dir', nargs='?', default=None,
                        help='folder with jill.ini / games (default: script folder)')
    parser.add_argument('--check', action='store_true',
                        help='run a headless API round-trip check and exit')
    args = parser.parse_args(argv)
    if args.check:
        print(self_check())
        return 0
    return run_gui(Path(args.base_dir) if args.base_dir else BASE_DIR)


if __name__ == '__main__':
    raise SystemExit(main())
