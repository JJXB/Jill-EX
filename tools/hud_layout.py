#!/usr/bin/env python3
"""Design Jill-EX HUD layouts over a reference screenshot.

The editor is deliberately standalone: it does not read game data and it does
not write engine code.  It produces a small text layout (`.hudlayout`) that the
port's alignment work can be checked against.

Features
--------
* A 320x200 (or larger) layout canvas with pixel coordinates.
* A reference image under the layout, with opacity/scale/offset controls, so
  an original screenshot or a mock-up can be traced.
* A resizeable snap grid (cell size plus origin), shown or hidden.
* Tile stamping from any PNG/JPG tileset, or straight from a Jill game's
  `.dma` tile table plus the episode's `.sha` shape bank (tile ids/names are
  shown in the palette, and runs record their tile ids on save).
* HUD/UI shape entries: load one or more `.sha` entries (3 = classic frame,
  7 = window, 14 = icons/pips) as labelled `entry:frame` sheets.
* Labelled layout parts (text/rect/box/pips/grid/image/note) that mark where
  engine-drawn elements belong; they are guides, not baked artwork.
* Undo/redo, PNG export and a plain-text summary for code alignment.

Usage
-----
    py -3 tools/hud_layout.py                     # empty editor
    py -3 tools/hud_layout.py art\\hud.hudlayout  # open an existing layout
    py -3 tools/hud_layout.py --summary art\\hud.hudlayout
    py -3 tools/hud_layout.py --render out.png art\\hud.hudlayout

The summary and render modes work without a display.  The GUI needs tkinter.

File format
-----------
UTF-8 text, INI-like sections, coordinates in screen pixels, origin top-left
of the 320x200 frame.  `[layout]` holds the canvas, grid and reference;
one `[sheet.<id>]` per tileset; one `[run]` per horizontal run of tiles; one
`[part.<name>]` per guide.  See `docs/hud-layout-tool.md` for the full
reference and an annotated example.
"""

from __future__ import annotations

import argparse
import os
import re
import sys
from dataclasses import dataclass, field, replace
from typing import Optional

try:
    from PIL import Image, ImageTk
except ImportError:  # pragma: no cover - summary works without Pillow
    Image = None
    ImageTk = None

FORMAT_ID = "jillex-hud-layout"
FORMAT_VERSION = 1
EXTENSION = ".hudlayout"

PART_KINDS = ("text", "rect", "box", "pips", "grid", "image", "note")

# The 16 EGA colours the game renders with, and their option-menu names.
EGA = (
    "#000000", "#0000AA", "#00AA00", "#00AAAA",
    "#AA0000", "#AA00AA", "#AA5500", "#AAAAAA",
    "#555555", "#5555FF", "#55FF55", "#55FFFF",
    "#FF5555", "#FF55FF", "#FFFF55", "#FFFFFF",
)
EGA_NAMES = (
    "BLACK", "BLUE", "GREEN", "CYAN",
    "RED", "PURPLE", "BROWN", "WHITE",
    "GREY", "LT BLUE", "LT GREEN", "LT CYAN",
    "LT RED", "LT PURPLE", "YELLOW", "BRIGHT WHITE",
)

PART_COLOURS = {
    "text": "#66CCFF",
    "rect": "#FFD866",
    "box": "#FFB000",
    "pips": "#FF7777",
    "grid": "#77DD77",
    "image": "#EE99EE",
    "note": "#BBBBBB",
}


# ---------------------------------------------------------------------------
# Model
# ---------------------------------------------------------------------------


@dataclass
class DmaTile:
    """One `.dma` record: the tile id, its shape and its decoded size."""

    tile_id: int = 0
    shape_id: int = 0
    name: str = ""
    w: int = 0
    h: int = 0
    missing: bool = False


@dataclass
class ShapeTile:
    """One frame of one `.sha` entry, used for HUD/UI pieces."""

    entry: int = 0
    frame: int = 0
    w: int = 0
    h: int = 0
    missing: bool = False


@dataclass
class Sheet:
    """One tileset image and its tile cell size.

    `source = "image"` uses `file` directly.  `source = "dma"` rebuilds the
    sheet from a Jill `.dma` tile table plus the episode's `.sha` shape bank.
    `source = "sha"` rebuilds one shape entry's frames (HUD/UI pieces).
    `palette` is optional and falls back to the extractor's default palette.
    """

    id: str = "sheet"
    file: str = ""
    tile_w: int = 8
    tile_h: int = 8
    source: str = "image"
    dma: str = ""
    sha: str = ""
    palette: str = ""
    columns: int = 0
    entry: int = 0
    # (column, row) -> decoded DMA record or shape frame; empty for images.
    entries: dict = field(default_factory=dict, repr=False, compare=False)
    image: object = field(default=None, repr=False, compare=False)


@dataclass
class Part:
    """A labelled guide rectangle for one engine-drawn HUD element."""

    name: str = "part"
    kind: str = "rect"
    x: int = 0
    y: int = 0
    w: int = 0
    h: int = 0
    colour: int = 7
    text: str = ""
    cell_w: int = 16
    cell_h: int = 16
    cols: int = 3
    rows: int = 3
    file: str = ""
    note: str = ""
    image: object = field(default=None, repr=False, compare=False)


@dataclass
class Layout:
    name: str = "untitled"
    style: str = "classic"  # classic / modern / custom
    width: int = 320
    height: int = 200
    grid_w: int = 8
    grid_h: int = 8
    grid_origin_x: int = 0
    grid_origin_y: int = 0
    snap: bool = True
    show_grid: bool = True
    zoom: int = 2
    reference: str = ""
    reference_opacity: int = 55
    reference_scale: int = 100
    reference_x: int = 0
    reference_y: int = 0
    show_reference: bool = True
    sheets: list[Sheet] = field(default_factory=list)
    # (x, y) pixels -> (sheet id, source column, source row)
    tiles: dict[tuple[int, int], tuple[str, int, int]] = field(
        default_factory=dict)
    parts: list[Part] = field(default_factory=list)
    path: Optional[str] = None
    reference_image: object = field(default=None, repr=False, compare=False)
    warnings: list[str] = field(default_factory=list, repr=False,
                                compare=False)


def new_layout() -> Layout:
    return Layout()


def clone_parts(parts: list[Part]) -> list[Part]:
    """Shallow copies: PIL images are immutable here and can be shared."""
    return [replace(p) for p in parts]


def part_bounds(p: Part) -> tuple[int, int]:
    """Effective width/height of a part, in layout pixels."""
    if p.kind == "grid":
        return (max(1, p.cell_w * max(1, p.cols)),
                max(1, p.cell_h * max(1, p.rows)))
    if p.kind == "pips":
        gap = max(0, p.cols - 1)
        return (max(1, p.cols * p.cell_w + gap), max(1, p.cell_h))
    if p.kind == "text":
        if p.text:
            return (max(1, 6 * len(p.text)), p.h if p.h > 0 else 8)
        return (p.w if p.w > 0 else 6, p.h if p.h > 0 else 8)
    return (max(0, p.w), max(0, p.h))


def part_contains(p: Part, x: int, y: int) -> bool:
    w, h = part_bounds(p)
    if w <= 0 or h <= 0:
        return False
    return p.x <= x < p.x + w and p.y <= y < p.y + h


def cell_pos(lay: Layout, x: int, y: int) -> tuple[int, int]:
    """Top-left pixel of the grid cell containing (x, y)."""
    gx = lay.grid_origin_x + ((x - lay.grid_origin_x) // lay.grid_w) * lay.grid_w
    gy = lay.grid_origin_y + ((y - lay.grid_origin_y) // lay.grid_h) * lay.grid_h
    return gx, gy


def grid_of(lay: Layout, x: int, y: int) -> tuple[int, int, int, int]:
    """(col, row, remainder_x, remainder_y) for a pixel position."""
    col, rx = divmod(x - lay.grid_origin_x, max(1, lay.grid_w))
    row, ry = divmod(y - lay.grid_origin_y, max(1, lay.grid_h))
    return col, row, rx, ry


def snap_value(value: int, origin: int, step: int) -> int:
    if step <= 0:
        return value
    return origin + round((value - origin) / step) * step


def tile_size(lay: Layout, sheet_id: str) -> tuple[int, int]:
    for s in lay.sheets:
        if s.id == sheet_id:
            return max(1, s.tile_w), max(1, s.tile_h)
    return 8, 8


def tile_at(lay: Layout, x: int, y: int) -> Optional[tuple[int, int]]:
    """Pixel key of the topmost tile covering (x, y)."""
    for key in reversed(list(lay.tiles.keys())):
        sheet_id = lay.tiles[key][0]
        tw, th = tile_size(lay, sheet_id)
        if key[0] <= x < key[0] + tw and key[1] <= y < key[1] + th:
            return key
    return None


def iter_tile_runs(lay: Layout):
    """Compact the tile map into horizontal runs.

    Each run has the same sheet, source row and tile size, and its x positions
    are contiguous multiples of the tile width.  Source columns are kept as a
    list so a repeated fill tile (`cols = 0,0,0,0`) compacts as well.
    """
    sheets = {s.id: s for s in lay.sheets}
    runs = []
    current = None
    for (x, y), (sheet_id, col, row) in sorted(
            lay.tiles.items(), key=lambda item: (item[0][1], item[0][0])):
        s = sheets.get(sheet_id)
        tw = max(1, s.tile_w) if s else 8
        th = max(1, s.tile_h) if s else 8
        if (current is not None and current["y"] == y and
                current["sheet"] == sheet_id and current["row"] == row and
                current["tw"] == tw and
                x == current["x"] + current["n"] * tw):
            current["cols"].append(col)
            current["n"] += 1
        else:
            current = {
                "sheet": sheet_id, "x": x, "y": y, "row": row,
                "cols": [col], "n": 1, "tw": tw, "th": th,
            }
            runs.append(current)
    return runs


# ---------------------------------------------------------------------------
# File format
# ---------------------------------------------------------------------------


def parse_sections(text: str) -> list[tuple[str, dict[str, str]]]:
    """Parse the INI-like format, preserving section order and repeats."""
    sections: list[tuple[str, dict[str, str]]] = []
    current: Optional[dict[str, str]] = None
    for raw in text.splitlines():
        line = raw.strip()
        if not line or line.startswith("#") or line.startswith(";"):
            continue
        if line.startswith("[") and line.endswith("]"):
            current = {}
            sections.append((line[1:-1].strip(), current))
            continue
        if current is None or "=" not in line:
            continue
        key, value = line.split("=", 1)
        current[key.strip().lower()] = value.strip()
    return sections


def _as_int(value, default: int) -> int:
    text = str(value).strip()
    try:
        return int(text, 10)
    except (TypeError, ValueError):
        try:
            return int(text, 0)
        except (TypeError, ValueError):
            return default


def _as_bool(value, default: bool) -> bool:
    if value is None:
        return default
    return str(value).strip().lower() in ("1", "true", "yes", "on")


def _as_pair(value: str, sep: str, default: tuple[int, int]) -> tuple[int, int]:
    if not value:
        return default
    parts = re.split(r"[x, ]+", value.strip())
    if len(parts) != 2:
        return default
    return _as_int(parts[0], default[0]), _as_int(parts[1], default[1])


def _clean_path(value: str) -> str:
    return value.strip().strip('"').strip("'")


def resolve_path(path: str, base: str) -> str:
    path = _clean_path(path)
    if not path:
        return ""
    path = path.replace("\\", os.sep)
    if os.path.isabs(path):
        return os.path.normpath(path)
    return os.path.normpath(os.path.join(base, path))


def store_path(path: str, base: str) -> str:
    """Prefer a path relative to the layout file, else keep it absolute."""
    path = _clean_path(path)
    if not path:
        return ""
    absolute = os.path.abspath(path)
    try:
        relative = os.path.relpath(absolute, os.path.abspath(base))
    except ValueError:
        return absolute.replace("\\", "/")
    if relative.startswith(".."):
        return absolute.replace("\\", "/")
    return relative.replace("\\", "/")


def safe_id(text: str, fallback: str) -> str:
    ident = re.sub(r"[^A-Za-z0-9_-]+", "_", text).strip("_")
    return ident or fallback


def unique_id(taken, base: str, fallback: str) -> str:
    ident = safe_id(base, fallback)
    candidate = ident
    n = 2
    while candidate in taken:
        candidate = f"{ident}_{n}"
        n += 1
    return candidate


def unique_part_name(parts: list[Part], base: str) -> str:
    taken = {p.name for p in parts}
    return unique_id(taken, base, "part")


def load_image(path: str):
    if Image is None or not path or not os.path.isfile(path):
        return None
    try:
        with Image.open(path) as opened:
            image = opened.convert("RGBA")
            image.load()
        return image
    except Exception:
        return None


# ---------------------------------------------------------------------------
# DMA tilesets
# ---------------------------------------------------------------------------

_EXTRACTOR = None


def find_extractor():
    """Import `jill_extractor` from the repo root or the release folder."""
    global _EXTRACTOR
    if _EXTRACTOR is not None:
        return _EXTRACTOR
    here = os.path.dirname(os.path.abspath(__file__))
    candidates = (here, os.path.dirname(here))
    for folder in candidates:
        if os.path.isfile(os.path.join(folder, "jill_extractor.py")):
            if folder not in sys.path:
                sys.path.insert(0, folder)
            import jill_extractor  # noqa: E402 - deliberate lazy import
            _EXTRACTOR = jill_extractor
            return _EXTRACTOR
    raise RuntimeError(
        "jill_extractor.py not found beside hud_layout.py or in its parent "
        "folder; keep the two tools together.")


def build_dma_sheet(dma_path: str, sha_path: str, palette_path: str = "",
                    columns: int = 16, cell_w: int = 16,
                    cell_h: int = 16):
    """Decode a `.dma` + `.sha` pair into a uniform tileset sheet.

    `.dma` is only a tile-id -> shape-id table; the pixels come from the
    episode's `.sha` bank.  Tiles are laid out in DMA record order, one per
    cell, so cell (0,0) is the first record, (1,0) the second, and so on.
    Returns `(image, entries, columns)` where entries maps (col, row) to a
    `DmaTile`.  Missing frames keep their cell (and are marked) so the layout
    stays stable.
    """
    if Image is None:
        raise RuntimeError("Pillow is required to decode DMA tiles")
    if not os.path.isfile(dma_path):
        raise ValueError(f"DMA file not found: {dma_path}")
    if not os.path.isfile(sha_path):
        raise ValueError(f"shape file not found: {sha_path}")
    extractor = find_extractor()
    sha = extractor.SHAFile.load(sha_path)
    if palette_path:
        if not os.path.isfile(palette_path):
            raise ValueError(f"palette file not found: {palette_path}")
        with open(palette_path, "rb") as f:
            palette = f.read()
        if len(palette) < 768:
            raise ValueError(
                f"{palette_path} is {len(palette)} bytes; expected 768")
        sha.set_palette_rgb(palette[:768], from_6bit=False)
    dma = extractor.DMAFile.load(dma_path)
    if not dma.records:
        raise ValueError(f"no tile records in {dma_path}")

    columns = max(1, min(64, columns))
    cell_w = max(4, min(128, cell_w))
    cell_h = max(4, min(128, cell_h))
    rows = (len(dma.records) + columns - 1) // columns
    sheet = Image.new("RGBA", (columns * cell_w, rows * cell_h),
                      (0, 0, 0, 0))
    entries: dict[tuple[int, int], DmaTile] = {}
    cache: dict = {}
    for index, record in enumerate(dma.records):
        col, row = index % columns, index // columns
        tile = extractor._tile_rgba(sha, record, cache)
        if tile is None:
            entries[(col, row)] = DmaTile(
                record.tile_id, record.shape_id, record.name, 0, 0, True)
            continue
        # Keep the original pixels, clipped to the cell, so oversized tiles
        # (16x20/16x22 in Jill 1) do not corrupt their neighbours.
        cell = Image.new("RGBA", (cell_w, cell_h), (0, 0, 0, 0))
        composite_clipped(cell, tile, 0, 0)
        sheet.alpha_composite(cell, (col * cell_w, row * cell_h))
        entries[(col, row)] = DmaTile(
            record.tile_id, record.shape_id, record.name,
            tile.width, tile.height, False)
    return sheet, entries, columns


def make_dma_sheet(sheet_id: str, dma_path: str, sha_path: str,
                   palette_path: str = "", columns: int = 16,
                   cell_w: int = 16, cell_h: int = 16) -> Sheet:
    image, entries, columns = build_dma_sheet(
        dma_path, sha_path, palette_path, columns, cell_w, cell_h)
    return Sheet(id=sheet_id, source="dma", dma=dma_path, sha=sha_path,
                 palette=palette_path, columns=columns, tile_w=cell_w,
                 tile_h=cell_h, entries=entries, image=image)


def build_sha_sheet(sha_path: str, entry_index: int,
                    palette_path: str = "", columns: int = 16,
                    cell_w: int = 0, cell_h: int = 0):
    """Decode every frame of one `.sha` entry into a labelled sheet.

    HUD/UI art is not in the DMA table; pieces such as the classic frame
    (entry 3), score digits (4), the nine-slice window (7) and the item icon /
    pip bank (14) are separate frames of one shape entry.  Cell size defaults
    to the largest frame in the entry; frames are top-left aligned, matching
    the engine's blitter.  Returns `(image, entries, columns, cell_w, cell_h)`.
    """
    if Image is None:
        raise RuntimeError("Pillow is required to decode shape frames")
    if not os.path.isfile(sha_path):
        raise ValueError(f"shape file not found: {sha_path}")
    if not 0 <= entry_index < 128:
        raise ValueError(f"shape entry must be 0..127, got {entry_index}")
    extractor = find_extractor()
    sha = extractor.SHAFile.load(sha_path)
    if palette_path:
        if not os.path.isfile(palette_path):
            raise ValueError(f"palette file not found: {palette_path}")
        with open(palette_path, "rb") as f:
            palette = f.read()
        if len(palette) < 768:
            raise ValueError(
                f"{palette_path} is {len(palette)} bytes; expected 768")
        sha.set_palette_rgb(palette[:768], from_6bit=False)
    entry = sha.entries[entry_index]
    if entry is None or not entry.frames:
        raise ValueError(
            f"shape entry {entry_index} has no frames in {sha_path}")

    decoded = []
    for frame in range(entry.frame_count):
        image = extractor._shape_rgba(sha, entry_index, frame, skip=(0,))
        decoded.append((frame, image))
    if not decoded:
        raise ValueError(f"shape entry {entry_index} decoded no frames")

    if cell_w <= 0:
        cell_w = max((image.width for _, image in decoded if image), default=1)
    if cell_h <= 0:
        cell_h = max((image.height for _, image in decoded if image),
                     default=1)
    cell_w = max(1, min(256, cell_w))
    cell_h = max(1, min(256, cell_h))
    columns = max(1, min(64, columns))
    rows = (len(decoded) + columns - 1) // columns
    sheet = Image.new("RGBA", (columns * cell_w, rows * cell_h),
                      (0, 0, 0, 0))
    entries: dict[tuple[int, int], ShapeTile] = {}
    for index, (frame, image) in enumerate(decoded):
        col, row = index % columns, index // columns
        if image is None:
            entries[(col, row)] = ShapeTile(
                entry_index, frame, 0, 0, True)
            continue
        cell = Image.new("RGBA", (cell_w, cell_h), (0, 0, 0, 0))
        composite_clipped(cell, image, 0, 0)
        sheet.alpha_composite(cell, (col * cell_w, row * cell_h))
        entries[(col, row)] = ShapeTile(
            entry_index, frame, image.width, image.height, False)
    return sheet, entries, columns, cell_w, cell_h


def make_sha_sheet(sheet_id: str, sha_path: str, entry_index: int,
                   palette_path: str = "", columns: int = 16,
                   cell_w: int = 0, cell_h: int = 0) -> Sheet:
    image, entries, columns, cell_w, cell_h = build_sha_sheet(
        sha_path, entry_index, palette_path, columns, cell_w, cell_h)
    return Sheet(id=sheet_id, source="sha", sha=sha_path,
                 palette=palette_path, columns=columns, entry=entry_index,
                 tile_w=cell_w, tile_h=cell_h, entries=entries, image=image)


def guess_sha_path(dma_path: str) -> str:
    """Best-effort SHA beside a DMA file: jill.dma -> jill1.sha."""
    folder = os.path.dirname(os.path.abspath(dma_path))
    stem = os.path.splitext(os.path.basename(dma_path))[0]
    candidates = [os.path.join(folder, stem + ".sha")]
    for suffix in ("1", "2", "3"):
        candidates.append(os.path.join(folder, stem + suffix + ".sha"))
    for candidate in candidates:
        if os.path.isfile(candidate):
            return candidate
    try:
        found = sorted(name for name in os.listdir(folder)
                       if name.lower().endswith(".sha"))
    except OSError:
        found = []
    if len(found) == 1:
        return os.path.join(folder, found[0])
    return ""


def tile_desc(lay: Layout, sheet_id: str, col: int, row: int) -> str:
    for sheet in lay.sheets:
        if sheet.id != sheet_id:
            continue
        entry = sheet.entries.get((col, row))
        if entry is None:
            return ""
        if isinstance(entry, ShapeTile):
            size = (f" {entry.w}x{entry.h}" if entry.w and entry.h else "")
            return f"E{entry.entry}:{entry.frame}{size}"
        show_name = entry.name and entry.name != str(entry.tile_id)
        name = f" {entry.name}" if show_name else ""
        size = (f" {entry.w}x{entry.h}" if entry.w and entry.h else "")
        return f"TILE {entry.tile_id}{name}{size}"
    return ""


def run_entries(lay: Layout, run) -> list:
    """Resolve a tile run's source cells to DMA records (if available)."""
    for sheet in lay.sheets:
        if sheet.id == run["sheet"]:
            return [sheet.entries.get((col, run["row"]))
                    for col in run["cols"]]
    return []


def load_layout(path: str) -> Layout:
    base = os.path.dirname(os.path.abspath(path))
    try:
        with open(path, "r", encoding="utf-8-sig") as f:
            text = f.read()
    except OSError as exc:
        raise ValueError(f"cannot read {path}: {exc}") from exc

    sections = parse_sections(text)
    if not sections or sections[0][0].lower() != "layout":
        raise ValueError("not a HUD layout: missing [layout] section")
    meta = sections[0][1]
    if meta.get("format", "").lower() not in (
            "", FORMAT_ID, str(FORMAT_VERSION)):
        raise ValueError(f"unsupported layout format: {meta.get('format')}")

    lay = Layout()
    lay.path = os.path.abspath(path)
    lay.name = meta.get("name", os.path.splitext(os.path.basename(path))[0])
    lay.style = meta.get("style", "classic").lower()
    canvas = _as_pair(meta.get("canvas", ""), "x", (320, 200))
    lay.width = max(16, min(960, canvas[0]))
    lay.height = max(16, min(960, canvas[1]))
    lay.grid_w = max(1, _as_pair(meta.get("grid", ""), "x", (8, 8))[0])
    lay.grid_h = max(1, _as_pair(meta.get("grid", ""), "x", (8, 8))[1])
    origin = meta.get("grid_origin", "0,0")
    lay.grid_origin_x = _as_pair(origin, ",", (0, 0))[0]
    lay.grid_origin_y = _as_pair(origin, ",", (0, 0))[1]
    lay.snap = _as_bool(meta.get("snap"), True)
    lay.show_grid = _as_bool(meta.get("show_grid"), True)
    lay.zoom = max(1, min(12, _as_int(meta.get("zoom"), 2)))
    lay.reference = resolve_path(meta.get("reference", ""), base)
    lay.reference_opacity = max(
        0, min(100, _as_int(meta.get("reference_opacity"), 55)))
    lay.reference_scale = max(
        1, min(800, _as_int(meta.get("reference_scale"), 100)))
    position = meta.get("reference_pos", "0,0")
    lay.reference_x = _as_pair(position, ",", (0, 0))[0]
    lay.reference_y = _as_pair(position, ",", (0, 0))[1]
    lay.show_reference = _as_bool(meta.get("reference_visible"), True)

    if lay.reference:
        lay.reference_image = load_image(lay.reference)
        if lay.reference_image is None:
            lay.warnings.append(f"reference image not found: {lay.reference}")

    sheets_by_id: dict[str, Sheet] = {}
    for name, body in sections[1:]:
        if not name.lower().startswith("sheet."):
            continue
        sheet = Sheet(id=name.split(".", 1)[1])
        sheet.source = body.get("source", "image").strip().lower()
        default_cell = 16 if sheet.source in ("dma", "sha") else 8
        tile_value = body.get("tile", "").strip()
        if sheet.source == "sha" and not tile_value:
            # A hand-written shape sheet may omit `tile`; auto-size the cells.
            sheet.tile_w = sheet.tile_h = 0
        else:
            tile = _as_pair(tile_value, "x",
                            (default_cell, default_cell))
            sheet.tile_w = max(1, tile[0])
            sheet.tile_h = max(1, tile[1])
        if sheet.source == "dma":
            sheet.dma = resolve_path(body.get("dma", ""), base)
            sheet.sha = resolve_path(body.get("sha", ""), base)
            sheet.palette = resolve_path(body.get("palette", ""), base)
            sheet.columns = max(1, min(64, _as_int(
                body.get("columns"), 16)))
            try:
                image, entries, columns = build_dma_sheet(
                    sheet.dma, sheet.sha, sheet.palette, sheet.columns,
                    sheet.tile_w, sheet.tile_h)
                sheet.image = image
                sheet.entries = entries
                sheet.columns = columns
            except Exception as exc:
                lay.warnings.append(
                    f"cannot rebuild DMA tileset {sheet.id}: {exc}")
        elif sheet.source == "sha":
            sheet.sha = resolve_path(body.get("sha", ""), base)
            sheet.palette = resolve_path(body.get("palette", ""), base)
            sheet.entry = max(0, min(127, _as_int(body.get("entry"), 0)))
            sheet.columns = max(1, min(64, _as_int(
                body.get("columns"), 16)))
            try:
                image, entries, columns, tile_w, tile_h = build_sha_sheet(
                    sheet.sha, sheet.entry, sheet.palette, sheet.columns,
                    sheet.tile_w, sheet.tile_h)
                sheet.image = image
                sheet.entries = entries
                sheet.columns = columns
                sheet.tile_w = tile_w
                sheet.tile_h = tile_h
            except Exception as exc:
                lay.warnings.append(
                    f"cannot rebuild shape tileset {sheet.id}: {exc}")
        else:
            sheet.source = "image"
            sheet.file = resolve_path(body.get("file", ""), base)
            sheet.image = load_image(sheet.file)
            if sheet.image is None:
                lay.warnings.append(
                    f"tileset image not found: {sheet.file or sheet.id}")
        lay.sheets.append(sheet)
        sheets_by_id[sheet.id] = sheet

    for name, body in sections[1:]:
        if name.lower() != "run":
            continue
        sheet_id = body.get("sheet", "")
        sheet = sheets_by_id.get(sheet_id)
        if sheet is None:
            lay.warnings.append(f"tile run uses unknown sheet: {sheet_id}")
            continue
        x = _as_int(body.get("x"), 0)
        y = _as_int(body.get("y"), 0)
        row = _as_int(body.get("row"), 0)
        raw_cols = body.get("cols", "").strip()
        if not raw_cols:
            continue
        cols = [_as_int(v, 0) for v in raw_cols.split(",")]
        for i, col in enumerate(cols):
            lay.tiles[(x + i * sheet.tile_w, y)] = (sheet.id, col, row)

    for name, body in sections[1:]:
        if not name.lower().startswith("part."):
            continue
        part = Part(name=name.split(".", 1)[1] or "part")
        kind = body.get("kind", "rect").lower()
        part.kind = kind if kind in PART_KINDS else "rect"
        part.x = _as_int(body.get("x"), 0)
        part.y = _as_int(body.get("y"), 0)
        size = _as_pair(body.get("size", ""), "x", (0, 0))
        part.w, part.h = max(0, size[0]), max(0, size[1])
        part.colour = max(0, min(15, _as_int(body.get("colour"), 7)))
        part.text = body.get("text", "")
        cell = _as_pair(body.get("cell", ""), "x", (16, 16))
        part.cell_w = max(1, cell[0])
        part.cell_h = max(1, cell[1])
        cells = _as_pair(body.get("cells", ""), "x", (3, 3))
        part.cols = max(1, cells[0])
        part.rows = max(1, cells[1])
        part.file = resolve_path(body.get("file", ""), base)
        part.note = body.get("note", "")
        if part.file:
            part.image = load_image(part.file)
            if part.image is None:
                lay.warnings.append(
                    f"part image not found: {part.file} ({part.name})")
        lay.parts.append(part)

    return lay


def layout_to_text(lay: Layout) -> str:
    base = os.path.dirname(os.path.abspath(lay.path)) if lay.path else os.getcwd()
    lines: list[str] = [
        "# Jill-EX HUD layout",
        "# Written by tools/hud_layout.py.  Coordinates are screen pixels in a",
        "# 320x200 frame (x,y = top-left of the element).  Grid columns count",
        "# from grid_origin.  See docs/hud-layout-tool.md.",
        "",
        "[layout]",
        f"format = {FORMAT_ID}",
        f"version = {FORMAT_VERSION}",
        f"name = {lay.name}",
        f"style = {lay.style}",
        f"canvas = {lay.width}x{lay.height}",
        f"grid = {lay.grid_w}x{lay.grid_h}",
        f"grid_origin = {lay.grid_origin_x},{lay.grid_origin_y}",
        f"snap = {1 if lay.snap else 0}",
        f"show_grid = {1 if lay.show_grid else 0}",
        f"zoom = {lay.zoom}",
        f"reference = {store_path(lay.reference, base)}".rstrip(),
        f"reference_opacity = {lay.reference_opacity}",
        f"reference_scale = {lay.reference_scale}",
        f"reference_pos = {lay.reference_x},{lay.reference_y}",
        f"reference_visible = {1 if lay.show_reference else 0}",
        "",
    ]

    for sheet in lay.sheets:
        lines.append(f"[sheet.{sheet.id}]")
        if sheet.source == "dma":
            lines.append("source = dma")
            lines.append(f"dma = {store_path(sheet.dma, base)}")
            lines.append(f"sha = {store_path(sheet.sha, base)}")
            if sheet.palette:
                lines.append(f"palette = {store_path(sheet.palette, base)}")
            lines.append(f"tile = {sheet.tile_w}x{sheet.tile_h}")
            lines.append(f"columns = {sheet.columns}")
        elif sheet.source == "sha":
            lines.append("source = sha")
            lines.append(f"sha = {store_path(sheet.sha, base)}")
            if sheet.palette:
                lines.append(f"palette = {store_path(sheet.palette, base)}")
            lines.append(f"entry = {sheet.entry}")
            lines.append(f"tile = {sheet.tile_w}x{sheet.tile_h}")
            lines.append(f"columns = {sheet.columns}")
        else:
            lines.append(f"file = {store_path(sheet.file, base)}".rstrip())
            lines.append(f"tile = {sheet.tile_w}x{sheet.tile_h}")
        lines.append("")

    for run in iter_tile_runs(lay):
        lines.extend([
            "[run]",
            f"sheet = {run['sheet']}",
            f"x = {run['x']}",
            f"y = {run['y']}",
            f"row = {run['row']}",
            "cols = " + ",".join(str(c) for c in run["cols"]),
        ])
        entries = run_entries(lay, run)
        if entries and all(entry is not None for entry in entries):
            source = next((sheet.source for sheet in lay.sheets
                           if sheet.id == run["sheet"]), "image")
            if source == "dma":
                lines.append("tiles = " + ",".join(
                    str(entry.tile_id) for entry in entries))
            elif source == "sha":
                lines.append("frames = " + ",".join(
                    str(entry.frame) for entry in entries))
        lines.append("")

    for part in lay.parts:
        w, h = part_bounds(part)
        lines.extend([
            f"[part.{part.name}]",
            f"kind = {part.kind}",
            f"x = {part.x}",
            f"y = {part.y}",
            f"size = {w}x{h}",
            f"colour = {part.colour}",
        ])
        if part.text:
            lines.append("text = " + part.text.replace("\r", " ").replace(
                "\n", " "))
        if part.kind in ("grid", "pips"):
            lines.append(f"cell = {part.cell_w}x{part.cell_h}")
            lines.append(f"cells = {part.cols}x{part.rows}")
        if part.file:
            lines.append(f"file = {store_path(part.file, base)}")
        if part.note:
            lines.append("note = " + part.note.replace("\r", " ").replace(
                "\n", " "))
        lines.append("")

    return "\n".join(lines).rstrip() + "\n"


def save_layout(lay: Layout, path: str) -> None:
    lay.path = os.path.abspath(path)
    text = layout_to_text(lay)
    with open(lay.path, "w", encoding="utf-8", newline="\n") as f:
        f.write(text)


# ---------------------------------------------------------------------------
# Rendering
# ---------------------------------------------------------------------------


def composite_clipped(base, source, x: int, y: int) -> None:
    """alpha_composite `source` at (x, y), clipping to `base`."""
    sx = max(0, -x)
    sy = max(0, -y)
    dx = max(0, x)
    dy = max(0, y)
    w = min(source.width - sx, base.width - dx)
    h = min(source.height - sy, base.height - dy)
    if w <= 0 or h <= 0:
        return
    crop = source.crop((sx, sy, sx + w, sy + h))
    base.alpha_composite(crop, (dx, dy))


def reference_picture(lay: Layout):
    """The reference image scaled/alpha-adjusted for this layout, or None."""
    image = lay.reference_image
    if image is None or lay.reference_scale <= 0:
        return None
    width = max(1, round(image.width * lay.reference_scale / 100))
    height = max(1, round(image.height * lay.reference_scale / 100))
    if (width, height) != image.size:
        image = image.resize((width, height), Image.NEAREST)
    opacity = max(0, min(100, lay.reference_opacity))
    if opacity < 100:
        r, g, b, a = image.split()
        a = a.point(lambda v: v * opacity // 100)
        image = Image.merge("RGBA", (r, g, b, a))
    return image


def render_layout(lay: Layout, *, reference: bool = True,
                  images: bool = True):
    """Render tiles (and optionally the reference/image parts) to RGBA."""
    canvas = Image.new("RGBA", (max(1, lay.width), max(1, lay.height)),
                       (0, 0, 0, 0))
    if reference and lay.show_reference:
        picture = reference_picture(lay)
        if picture is not None:
            composite_clipped(canvas, picture, lay.reference_x,
                              lay.reference_y)

    sheets = {s.id: s for s in lay.sheets}
    for (x, y), (sheet_id, col, row) in sorted(
            lay.tiles.items(), key=lambda item: (item[0][1], item[0][0])):
        sheet = sheets.get(sheet_id)
        if sheet is None or sheet.image is None:
            continue
        left = col * sheet.tile_w
        top = row * sheet.tile_h
        crop = sheet.image.crop(
            (left, top, left + sheet.tile_w, top + sheet.tile_h))
        composite_clipped(canvas, crop, x, y)

    if images:
        for part in lay.parts:
            if part.kind != "image" or part.image is None:
                continue
            w, h = part_bounds(part)
            picture = part.image
            if w > 0 and h > 0 and picture.size != (w, h):
                picture = picture.resize((w, h), Image.NEAREST)
            composite_clipped(canvas, picture, part.x, part.y)
    return canvas


def render_preview(lay: Layout, background: str = "#202020"):
    """RGBA layout composited on a flat background, for the editor canvas."""
    canvas = render_layout(lay, reference=True, images=True)
    flat = Image.new("RGBA", canvas.size, background)
    flat.alpha_composite(canvas)
    return flat


def summary_text(lay: Layout, path: Optional[str] = None) -> str:
    """A compact, coordinate-first report used to align engine drawing."""
    where = path or lay.path or "(unsaved)"
    lines = [
        "Jill-EX HUD layout summary",
        f"  file      : {where}",
        f"  name      : {lay.name}",
        f"  style     : {lay.style}",
        f"  canvas    : {lay.width}x{lay.height}",
        f"  grid      : {lay.grid_w}x{lay.grid_h}, origin "
        f"({lay.grid_origin_x},{lay.grid_origin_y}), "
        f"snap {'on' if lay.snap else 'off'}",
    ]
    if lay.reference:
        lines.append(
            f"  reference : {lay.reference}  opacity {lay.reference_opacity}%  "
            f"scale {lay.reference_scale}%  at "
            f"({lay.reference_x},{lay.reference_y})")
    if lay.warnings:
        lines.append("  warnings  :")
        for warning in lay.warnings:
            lines.append(f"    ! {warning}")

    lines.append(f"  sheets    : {len(lay.sheets)}")
    for sheet in lay.sheets:
        size = (f"{sheet.image.width}x{sheet.image.height}"
                if sheet.image is not None else "missing")
        if sheet.source == "dma":
            lines.append(
                f"    {sheet.id:<10} source=dma  tiles {sheet.tile_w}x"
                f"{sheet.tile_h}  columns {sheet.columns}  image {size}")
            lines.append(f"      dma  : {sheet.dma}")
            lines.append(f"      sha  : {sheet.sha}")
            if sheet.palette:
                lines.append(f"      pal  : {sheet.palette}")
            names = [e.name for e in sheet.entries.values() if e.name]
            sample = ", ".join(names[:6])
            lines.append(
                f"      tiles: {len(sheet.entries)}"
                + (f" ({sample}{', ...' if len(names) > 6 else ''})"
                   if sample else ""))
        elif sheet.source == "sha":
            frames = [e.frame for e in sheet.entries.values()
                      if e is not None]
            first = min(frames) if frames else 0
            last = max(frames) if frames else 0
            lines.append(
                f"    {sheet.id:<10} source=sha  entry {sheet.entry}  "
                f"tiles {sheet.tile_w}x{sheet.tile_h}  columns "
                f"{sheet.columns}  image {size}")
            lines.append(f"      sha  : {sheet.sha}")
            if sheet.palette:
                lines.append(f"      pal  : {sheet.palette}")
            lines.append(
                f"      frames: {len(sheet.entries)} (entry {sheet.entry}, "
                f"frame {first}..{last})")
        else:
            lines.append(
                f"    {sheet.id:<10} {sheet.file}  tiles {sheet.tile_w}x"
                f"{sheet.tile_h}  image {size}")

    runs = iter_tile_runs(lay)
    lines.append(f"  tiles     : {len(lay.tiles)} stamps in {len(runs)} runs")
    for run in runs:
        span = run["tw"] * run["n"]
        col, row, rx, ry = grid_of(lay, run["x"], run["y"])
        grid = f"col {col}" + (f"+{rx}" if rx else "")
        grid += f", row {row}" + (f"+{ry}" if ry else "")
        line = (
            f"    {run['sheet']:<8} row {run['row']:<3} "
            f"x {run['x']}..{run['x'] + span}  y {run['y']}.."
            f"{run['y'] + run['th']}  cols "
            f"{run['cols'][0]}..{run['cols'][-1]}  ({grid})")
        entries = run_entries(lay, run)
        source = next((sheet.source for sheet in lay.sheets
                       if sheet.id == run["sheet"]), "image")
        if entries:
            if source == "dma":
                ids = ",".join("?" if e is None else str(e.tile_id)
                               for e in entries)
                line += f"  tiles {ids}"
            elif source == "sha":
                frames = ",".join("?" if e is None else str(e.frame)
                                  for e in entries)
                line += f"  frames {frames}"
        lines.append(line)
        if entries and source == "dma":
            names = [e.name for e in entries if e is not None and e.name]
            if names:
                lines.append("      names: " + ", ".join(names))

    lines.append(f"  parts     : {len(lay.parts)}")
    for part in lay.parts:
        w, h = part_bounds(part)
        col, row, rx, ry = grid_of(lay, part.x, part.y)
        grid = f"col {col}" + (f"+{rx}" if rx else "")
        grid += f", row {row}" + (f"+{ry}" if ry else "")
        extra = []
        if part.kind == "text" and part.text:
            extra.append(f'"{part.text}"')
        if part.kind == "note" and part.note:
            extra.append(f'note "{part.note}"')
        if part.kind in ("grid", "pips"):
            extra.append(f"cell {part.cell_w}x{part.cell_h}")
        if part.kind == "grid":
            extra.append(f"cells {part.cols}x{part.rows}")
        if part.colour != 7:
            extra.append(f"colour {part.colour}")
        tail = ("  " + "  ".join(extra)) if extra else ""
        lines.append(
            f"    {part.name:<22} {part.kind:<5} "
            f"x={part.x:<4} y={part.y:<4} w={w:<4} h={h:<4} ({grid}){tail}")

    return "\n".join(lines) + "\n"


# ---------------------------------------------------------------------------
# Engine guide
# ---------------------------------------------------------------------------


def classic_guide_parts() -> list[Part]:
    """Rectangles matching the engine's CLASSIC HUD drawing.

    Values come from `Engine::render_hud_classic` and the pack-panel path in
    `port/src/engine.cpp`: the 8-px frame, the 64-px status column, the
    inventory pips and the hint strip.  They are guides for alignment, so they
    use the same pixel positions the engine uses, not a redrawn approximation.
    """
    parts: list[Part] = []

    def add(name, kind, x, y, w, h, **kwargs):
        parts.append(Part(name=name, kind=kind, x=x, y=y, w=w, h=h,
                          **kwargs))

    add("frame", "rect", 8, 12, 304, 180,
        note="patterned frame ring (E3 frames 0/3/4/6/8/10) + title strip")
    add("view", "rect", 80, 22, 232, 158, note="playfield viewport")
    add("health-box", "box", 8, 22, 64, 30, colour=12)
    add("health-label", "text", 10, 27, 0, 0, text="HEALTH")
    add("health-bar", "rect", 48, 27, 20, 7, colour=4,
        note="framed 20x7 bar; pack panels use pips at x=42")
    add("pack-pips", "pips", 42, 2, 0, 0, cols=8, cell_w=2, cell_h=6,
        rows=1, note="external classic panel health pips")
    add("level-box", "box", 8, 52, 64, 22, colour=8)
    add("level-label", "text", 9, 57, 0, 0, text="LEVEL")
    add("level-value", "text", 10, 65, 0, 0, text="0")
    add("score-label", "text", 41, 57, 0, 0, text="SCORE")
    add("score-value", "text", 34, 65, 36, 8, colour=7,
        note="score digits right-aligned to x=70")
    add("inventory-box", "box", 8, 74, 64, 84, colour=8)
    add("inventory-grid", "grid", 9, 78, 0, 0, cell_w=16, cell_h=16,
        cols=3, rows=3)
    add("inventory-caption", "box", 8, 158, 64, 14, colour=8)
    add("inventory-text", "text", 10, 162, 0, 0, text="INVENTORY")
    add("hint-strip", "rect", 0, 192, 320, 8,
        note="plain black hint strip")
    return parts


# ---------------------------------------------------------------------------
# GUI
# ---------------------------------------------------------------------------

try:
    import tkinter as tk
    from tkinter import filedialog, messagebox, simpledialog, ttk
except ImportError:  # pragma: no cover
    tk = None
    ttk = None


class DmaImportDialog(tk.Toplevel if tk is not None else object):
    """Small dialog collecting a DMA/SHA/palette triple and cell layout."""

    def __init__(self, parent, initial_dir: str = ""):
        super().__init__(parent)
        self.title("Load DMA tileset")
        self.resizable(False, False)
        self.transient(parent)
        self.result: Optional[dict] = None
        self._last_dir = initial_dir or os.getcwd()
        self.dma_var = tk.StringVar()
        self.sha_var = tk.StringVar()
        self.palette_var = tk.StringVar()
        self.columns_var = tk.StringVar(value="16")
        self.cell_var = tk.StringVar(value="16x16")
        self.match_grid = tk.BooleanVar(value=True)

        frame = ttk.Frame(self, padding=8)
        frame.pack(fill="both", expand=True)

        ttk.Label(frame, text="DMA tile table").grid(
            row=0, column=0, sticky="w")
        ttk.Entry(frame, textvariable=self.dma_var, width=52).grid(
            row=0, column=1, sticky="ew", padx=4)
        ttk.Button(frame, text="...", width=3,
                   command=self._browse_dma).grid(row=0, column=2)

        ttk.Label(frame, text="Shape bank (.sha)").grid(
            row=1, column=0, sticky="w", pady=(4, 0))
        ttk.Entry(frame, textvariable=self.sha_var, width=52).grid(
            row=1, column=1, sticky="ew", padx=4, pady=(4, 0))
        ttk.Button(frame, text="...", width=3,
                   command=self._browse_sha).grid(row=1, column=2,
                                                  pady=(4, 0))

        ttk.Label(frame, text="Palette (optional)").grid(
            row=2, column=0, sticky="w", pady=(4, 0))
        ttk.Entry(frame, textvariable=self.palette_var, width=52).grid(
            row=2, column=1, sticky="ew", padx=4, pady=(4, 0))
        ttk.Button(frame, text="...", width=3,
                   command=self._browse_palette).grid(row=2, column=2,
                                                      pady=(4, 0))

        row = ttk.Frame(frame)
        row.grid(row=3, column=0, columnspan=3, sticky="w", pady=(8, 0))
        ttk.Label(row, text="Sheet columns").pack(side="left")
        ttk.Spinbox(row, textvariable=self.columns_var, width=5, from_=1,
                    to=64).pack(side="left", padx=(3, 12))
        ttk.Label(row, text="Cell size").pack(side="left")
        ttk.Entry(row, textvariable=self.cell_var, width=8).pack(
            side="left", padx=3)
        ttk.Checkbutton(row, text="Match grid to cell",
                        variable=self.match_grid).pack(side="left", padx=(12, 0))

        note = (
            "The .dma file only maps tile ids to shape ids; the episode's "
            ".sha supplies the pixels.\n"
            "Tiles are laid out in DMA record order and labelled with their "
            "tile id.  Leave the\n"
            "palette blank for Jill 1-3 (built-in VGA palette); pick "
            "profiles/onesimus.pal for Onesimus."
        )
        ttk.Label(frame, text=note, justify="left").grid(
            row=4, column=0, columnspan=3, sticky="w", pady=(8, 0))

        buttons = ttk.Frame(frame)
        buttons.grid(row=5, column=0, columnspan=3, sticky="e", pady=(10, 0))
        ttk.Button(buttons, text="Cancel",
                   command=self.destroy).pack(side="right")
        ttk.Button(buttons, text="Load", command=self._ok).pack(
            side="right", padx=4)

        self.bind("<Escape>", lambda _e: self.destroy())
        self.bind("<Return>", lambda _e: self._ok())
        self.grab_set()

    def _browse(self, title: str, patterns: str) -> str:
        return filedialog.askopenfilename(
            parent=self, title=title, initialdir=self._last_dir,
            filetypes=((title, patterns), ("All files", "*.*")))

    def _browse_dma(self):
        path = self._browse("DMA tile table", "*.dma")
        if not path:
            return
        self.dma_var.set(path)
        self._last_dir = os.path.dirname(path)
        if not self.sha_var.get():
            guess = guess_sha_path(path)
            if guess:
                self.sha_var.set(guess)

    def _browse_sha(self):
        path = self._browse("Shape bank", "*.sha")
        if path:
            self.sha_var.set(path)
            self._last_dir = os.path.dirname(path)

    def _browse_palette(self):
        path = self._browse("Palette", "*.pal *.bin")
        if path:
            self.palette_var.set(path)
            self._last_dir = os.path.dirname(path)

    def _ok(self):
        dma = self.dma_var.get().strip()
        sha = self.sha_var.get().strip()
        if not dma or not os.path.isfile(dma):
            messagebox.showerror("DMA tileset", "Choose an existing .dma file.",
                                 parent=self)
            return
        if not sha or not os.path.isfile(sha):
            messagebox.showerror(
                "DMA tileset",
                "Choose the matching .sha shape bank for this episode.",
                parent=self)
            return
        columns = max(1, min(64, _as_int(self.columns_var.get(), 16)))
        cell = _as_pair(self.cell_var.get(), "x", (16, 16))
        self.result = {
            "dma": dma,
            "sha": sha,
            "palette": self.palette_var.get().strip(),
            "columns": columns,
            "cell_w": max(4, min(128, cell[0])),
            "cell_h": max(4, min(128, cell[1])),
            "match_grid": bool(self.match_grid.get()),
        }
        self.destroy()


class ShapeImportDialog(tk.Toplevel if tk is not None else object):
    """Collect a `.sha` plus the shape entries to turn into tilesets."""

    def __init__(self, parent, initial_dir: str = ""):
        super().__init__(parent)
        self.title("Load HUD/UI shape entries")
        self.resizable(False, False)
        self.transient(parent)
        self.result: Optional[dict] = None
        self._last_dir = initial_dir or os.getcwd()
        self.sha_var = tk.StringVar()
        self.palette_var = tk.StringVar()
        self.entries_var = tk.StringVar(value="3,4,7,14")
        self.columns_var = tk.StringVar(value="16")
        self.cell_var = tk.StringVar(value="16x16")

        frame = ttk.Frame(self, padding=8)
        frame.pack(fill="both", expand=True)

        ttk.Label(frame, text="Shape bank (.sha)").grid(
            row=0, column=0, sticky="w")
        ttk.Entry(frame, textvariable=self.sha_var, width=52).grid(
            row=0, column=1, sticky="ew", padx=4)
        ttk.Button(frame, text="...", width=3,
                   command=self._browse_sha).grid(row=0, column=2)

        ttk.Label(frame, text="Palette (optional)").grid(
            row=1, column=0, sticky="w", pady=(4, 0))
        ttk.Entry(frame, textvariable=self.palette_var, width=52).grid(
            row=1, column=1, sticky="ew", padx=4, pady=(4, 0))
        ttk.Button(frame, text="...", width=3,
                   command=self._browse_palette).grid(row=1, column=2,
                                                      pady=(4, 0))

        ttk.Label(frame, text="Entries").grid(
            row=2, column=0, sticky="w", pady=(4, 0))
        ttk.Entry(frame, textvariable=self.entries_var, width=30).grid(
            row=2, column=1, sticky="w", padx=4, pady=(4, 0))

        row = ttk.Frame(frame)
        row.grid(row=3, column=0, columnspan=3, sticky="w", pady=(8, 0))
        ttk.Label(row, text="Sheet columns").pack(side="left")
        ttk.Spinbox(row, textvariable=self.columns_var, width=5, from_=1,
                    to=64).pack(side="left", padx=(3, 12))
        ttk.Label(row, text="Cell size").pack(side="left")
        ttk.Entry(row, textvariable=self.cell_var, width=8).pack(
            side="left", padx=3)

        note = (
            "HUD/UI art is made of frames inside shape entries, not DMA "
            "tiles.\n"
            "3 = classic frame/HUD patterns, 4 = score digits, "
            "7 = window nine-slice,\n"
            "14 = item icons and health pips.  Each entry becomes its own "
            "sheet.\n"
            "Cell 16x16 suits HUD pieces; 'auto' uses each entry's largest "
            "frame."
        )
        ttk.Label(frame, text=note, justify="left").grid(
            row=4, column=0, columnspan=3, sticky="w", pady=(8, 0))

        buttons = ttk.Frame(frame)
        buttons.grid(row=5, column=0, columnspan=3, sticky="e", pady=(10, 0))
        ttk.Button(buttons, text="Cancel",
                   command=self.destroy).pack(side="right")
        ttk.Button(buttons, text="Load", command=self._ok).pack(
            side="right", padx=4)

        self.bind("<Escape>", lambda _e: self.destroy())
        self.bind("<Return>", lambda _e: self._ok())
        self.grab_set()

    def _browse_sha(self):
        path = filedialog.askopenfilename(
            parent=self, title="Shape bank", initialdir=self._last_dir,
            filetypes=(("Shape bank", "*.sha"), ("All files", "*.*")))
        if path:
            self.sha_var.set(path)
            self._last_dir = os.path.dirname(path)

    def _browse_palette(self):
        path = filedialog.askopenfilename(
            parent=self, title="Palette", initialdir=self._last_dir,
            filetypes=(("Palette", "*.pal *.bin"), ("All files", "*.*")))
        if path:
            self.palette_var.set(path)
            self._last_dir = os.path.dirname(path)

    def _ok(self):
        sha = self.sha_var.get().strip()
        if not sha or not os.path.isfile(sha):
            messagebox.showerror(
                "HUD/UI shapes", "Choose an existing .sha shape bank.",
                parent=self)
            return
        entries = []
        for part in re.split(r"[, ]+", self.entries_var.get().strip()):
            if not part:
                continue
            try:
                entries.append(int(part, 10))
            except ValueError:
                messagebox.showerror(
                    "HUD/UI shapes",
                    f"'{part}' is not a shape entry number.",
                    parent=self)
                return
        entries = sorted(set(e for e in entries if 0 <= e < 128))
        if not entries:
            messagebox.showerror(
                "HUD/UI shapes", "Enter at least one entry number (0..127).",
                parent=self)
            return
        cell_text = self.cell_var.get().strip().lower()
        if cell_text in ("", "auto"):
            cell_w = cell_h = 0
        else:
            cell = _as_pair(cell_text, "x", (0, 0))
            cell_w = max(1, min(256, cell[0])) if cell[0] else 0
            cell_h = max(1, min(256, cell[1])) if cell[1] else 0
        self.result = {
            "sha": sha,
            "palette": self.palette_var.get().strip(),
            "entries": entries,
            "columns": max(1, min(64, _as_int(self.columns_var.get(), 16))),
            "cell_w": cell_w,
            "cell_h": cell_h,
        }
        self.destroy()


class HudLayoutApp(tk.Tk if tk is not None else object):
    """Tkinter front end for the layout model above."""

    def __init__(self, layout: Optional[Layout] = None):
        super().__init__()
        self.layout = layout or new_layout()
        self.path: Optional[str] = layout.path if layout else None
        self.dirty = False
        self.zoom = max(1, min(12, self.layout.zoom))
        self.active_sheet: Optional[str] = None
        self.active_tile = (0, 0)
        self.palette_zoom = 3
        self.selected: Optional[int] = None
        self.mode = tk.StringVar(value="stamp")
        self.show_grid = tk.BooleanVar(value=self.layout.show_grid)
        self.snap = tk.BooleanVar(value=self.layout.snap)
        self.status = tk.StringVar(value="Ready")
        self.hint = tk.StringVar(value="")
        self._undo: list = []
        self._redo: list = []
        self._drag = None
        self._stroke = False
        self._redraw_job = None
        self._canvas_item = None
        self._photo = None
        self._palette_photo = None
        self._form_updating = False
        self._last_cursor: Optional[tuple[int, int]] = None

        self.title("Jill-EX HUD Layout")
        self.geometry("1320x900")
        self.minsize(1080, 720)
        self.protocol("WM_DELETE_WINDOW", self.on_close)

        self._build_menu()
        self._build_ui()
        self._load_into_ui()
        self.bind("<Control-z>", lambda _e: self.undo())
        self.bind("<Control-y>", lambda _e: self.redo())
        self.bind("<Control-s>", lambda _e: self.save())
        self.bind("<Control-Shift-S>", lambda _e: self.save_as())
        self.bind("<Control-o>", lambda _e: self.open_file())
        self.bind("<Control-n>", lambda _e: self.new_file())
        self.bind("<Delete>", self._delete_key)
        self.bind("<Escape>", lambda _e: self.select_part(None))
        self.canvas.bind("<Motion>", self._on_motion)
        self.canvas.bind("<Leave>", lambda _e: self._clear_cursor())
        self.canvas.bind("<ButtonPress-1>", self._on_press)
        self.canvas.bind("<B1-Motion>", self._on_drag)
        self.canvas.bind("<ButtonRelease-1>", self._on_release)
        self.canvas.bind("<ButtonPress-3>", self._on_right_press)
        self.canvas.bind("<Control-MouseWheel>", self._on_zoom_wheel)
        self.after(50, self.redraw)
        if self.layout.warnings:
            self.status.set("; ".join(self.layout.warnings))

    # -- construction -------------------------------------------------------

    def _build_menu(self):
        menubar = tk.Menu(self)
        file_menu = tk.Menu(menubar, tearoff=0)
        file_menu.add_command(label="New", accelerator="Ctrl+N",
                              command=self.new_file)
        file_menu.add_command(label="Open...", accelerator="Ctrl+O",
                              command=self.open_file)
        file_menu.add_command(label="Save", accelerator="Ctrl+S",
                              command=self.save)
        file_menu.add_command(label="Save As...", accelerator="Ctrl+Shift+S",
                              command=self.save_as)
        file_menu.add_separator()
        file_menu.add_command(label="Export PNG...", command=self.export_png)
        file_menu.add_command(label="Copy summary", command=self.copy_summary)
        file_menu.add_separator()
        file_menu.add_command(label="Insert classic HUD guide",
                              command=self.insert_classic_guide)
        file_menu.add_separator()
        file_menu.add_command(label="Exit", command=self.on_close)
        menubar.add_cascade(label="File", menu=file_menu)

        edit_menu = tk.Menu(menubar, tearoff=0)
        edit_menu.add_command(label="Undo", accelerator="Ctrl+Z",
                              command=self.undo)
        edit_menu.add_command(label="Redo", accelerator="Ctrl+Y",
                              command=self.redo)
        edit_menu.add_separator()
        edit_menu.add_command(label="Duplicate part", accelerator="Ctrl+D",
                              command=self.duplicate_part)
        edit_menu.add_command(label="Delete part", accelerator="Del",
                              command=lambda: self.delete_part(True))
        menubar.add_cascade(label="Edit", menu=edit_menu)

        view_menu = tk.Menu(menubar, tearoff=0)
        view_menu.add_checkbutton(label="Show grid", variable=self.show_grid,
                                  command=self._settings_changed)
        view_menu.add_checkbutton(label="Snap to grid", variable=self.snap,
                                  command=self._settings_changed)
        view_menu.add_separator()
        view_menu.add_command(label="Zoom in", accelerator="Ctrl++",
                              command=lambda: self.set_zoom(self.zoom + 1))
        view_menu.add_command(label="Zoom out", accelerator="Ctrl+-",
                              command=lambda: self.set_zoom(self.zoom - 1))
        menubar.add_cascade(label="View", menu=view_menu)
        self.config(menu=menubar)
        self.bind("<Control-d>", lambda _e: self.duplicate_part())
        self.bind("<Control-plus>", lambda _e: self.set_zoom(self.zoom + 1))
        self.bind("<Control-equal>", lambda _e: self.set_zoom(self.zoom + 1))
        self.bind("<Control-minus>", lambda _e: self.set_zoom(self.zoom - 1))

    def _build_ui(self):
        style = ttk.Style(self)
        try:
            style.theme_use("vista")
        except tk.TclError:
            pass

        toolbar = ttk.Frame(self, padding=(6, 4))
        toolbar.pack(side="top", fill="x")
        ttk.Button(toolbar, text="New", width=6,
                   command=self.new_file).pack(side="left", padx=2)
        ttk.Button(toolbar, text="Open", width=6,
                   command=self.open_file).pack(side="left", padx=2)
        ttk.Button(toolbar, text="Save", width=6,
                   command=self.save).pack(side="left", padx=2)
        ttk.Button(toolbar, text="Export PNG", width=10,
                   command=self.export_png).pack(side="left", padx=2)
        ttk.Separator(toolbar, orient="vertical").pack(side="left", fill="y",
                                                      padx=8)
        for label, value in (("Stamp", "stamp"), ("Erase", "erase"),
                             ("Select", "select")):
            ttk.Radiobutton(toolbar, text=label, value=value,
                            variable=self.mode,
                            command=self._mode_changed).pack(side="left",
                                                             padx=2)
        ttk.Separator(toolbar, orient="vertical").pack(side="left", fill="y",
                                                      padx=8)
        ttk.Button(toolbar, text="Undo", width=6,
                   command=self.undo).pack(side="left", padx=2)
        ttk.Button(toolbar, text="Redo", width=6,
                   command=self.redo).pack(side="left", padx=2)
        ttk.Label(toolbar, text="Zoom").pack(side="left", padx=(12, 2))
        self.zoom_var = tk.StringVar(value=f"{self.zoom}x")
        zoom_box = ttk.Combobox(
            toolbar, textvariable=self.zoom_var, width=5, state="readonly",
            values=[f"{v}x" for v in (1, 2, 3, 4, 6, 8, 12)])
        zoom_box.pack(side="left")
        zoom_box.bind("<<ComboboxSelected>>",
                      lambda _e: self.set_zoom(
                          int(self.zoom_var.get().rstrip("x"))))

        body = ttk.Frame(self, padding=(6, 0))
        body.pack(side="top", fill="both", expand=True)
        body.columnconfigure(1, weight=1)
        body.rowconfigure(0, weight=1)

        left = ttk.Frame(body)
        left.grid(row=0, column=0, sticky="nsw", padx=(0, 6))
        self._build_left(left)

        canvas_frame = ttk.Frame(body, relief="sunken", borderwidth=1)
        canvas_frame.grid(row=0, column=1, sticky="nsew")
        canvas_frame.rowconfigure(0, weight=1)
        canvas_frame.columnconfigure(0, weight=1)
        self.canvas = tk.Canvas(canvas_frame, background="#202020",
                                width=640, height=420, highlightthickness=0,
                                cursor="crosshair")
        self.canvas.grid(row=0, column=0, sticky="nsew")
        hbar = ttk.Scrollbar(canvas_frame, orient="horizontal",
                             command=self.canvas.xview)
        vbar = ttk.Scrollbar(canvas_frame, orient="vertical",
                             command=self.canvas.yview)
        self.canvas.configure(xscrollcommand=hbar.set,
                              yscrollcommand=vbar.set)
        hbar.grid(row=1, column=0, sticky="ew")
        vbar.grid(row=0, column=1, sticky="ns")

        right = ttk.Frame(body)
        right.grid(row=0, column=2, sticky="nse", padx=(6, 0))
        self._build_right(right)

        status_bar = ttk.Frame(self, padding=(6, 2))
        status_bar.pack(side="bottom", fill="x")
        ttk.Label(status_bar, textvariable=self.status,
                  anchor="w").pack(side="left", fill="x", expand=True)
        ttk.Label(status_bar, textvariable=self.hint,
                  anchor="e").pack(side="right")

    def _build_left(self, parent):
        layout_box = ttk.LabelFrame(parent, text="Layout", padding=4)
        layout_box.pack(fill="x", pady=(0, 4))
        self.name_var = tk.StringVar()
        self.style_var = tk.StringVar()
        self.canvas_w_var = tk.StringVar()
        self.canvas_h_var = tk.StringVar()
        ttk.Label(layout_box, text="Name").grid(row=0, column=0, sticky="w")
        ttk.Entry(layout_box, textvariable=self.name_var,
                  width=22).grid(row=0, column=1, columnspan=3, sticky="ew",
                                 padx=2)
        ttk.Label(layout_box, text="Style").grid(row=1, column=0, sticky="w")
        ttk.Combobox(layout_box, textvariable=self.style_var, width=10,
                     state="readonly",
                     values=("classic", "modern", "custom")).grid(
            row=1, column=1, sticky="w", padx=2)
        ttk.Label(layout_box, text="Canvas").grid(row=1, column=2, sticky="e")
        ttk.Spinbox(layout_box, textvariable=self.canvas_w_var, width=5,
                    from_=16, to=960).grid(row=1, column=3, sticky="w")
        ttk.Label(layout_box, text="x").grid(row=1, column=4)
        ttk.Spinbox(layout_box, textvariable=self.canvas_h_var, width=5,
                    from_=16, to=960).grid(row=1, column=5, sticky="w")
        for var in (self.name_var, self.style_var, self.canvas_w_var,
                    self.canvas_h_var):
            var.trace_add("write", lambda *_a: self._layout_fields_changed())

        grid_box = ttk.LabelFrame(parent, text="Grid", padding=4)
        grid_box.pack(fill="x", pady=(0, 4))
        self.grid_w_var = tk.StringVar()
        self.grid_h_var = tk.StringVar()
        self.grid_ox_var = tk.StringVar()
        self.grid_oy_var = tk.StringVar()
        ttk.Label(grid_box, text="Cell").grid(row=0, column=0, sticky="w")
        ttk.Spinbox(grid_box, textvariable=self.grid_w_var, width=4,
                    from_=1, to=128).grid(row=0, column=1, padx=2)
        ttk.Label(grid_box, text="x").grid(row=0, column=2)
        ttk.Spinbox(grid_box, textvariable=self.grid_h_var, width=4,
                    from_=1, to=128).grid(row=0, column=3, padx=2)
        ttk.Label(grid_box, text="Origin").grid(row=1, column=0, sticky="w")
        ttk.Spinbox(grid_box, textvariable=self.grid_ox_var, width=4,
                    from_=-256, to=256).grid(row=1, column=1, padx=2)
        ttk.Label(grid_box, text=",").grid(row=1, column=2)
        ttk.Spinbox(grid_box, textvariable=self.grid_oy_var, width=4,
                    from_=-256, to=256).grid(row=1, column=3, padx=2)
        ttk.Checkbutton(grid_box, text="Show", variable=self.show_grid,
                        command=self._settings_changed).grid(
            row=2, column=0, columnspan=2, sticky="w")
        ttk.Checkbutton(grid_box, text="Snap", variable=self.snap,
                        command=self._settings_changed).grid(
            row=2, column=2, columnspan=2, sticky="w")
        for var in (self.grid_w_var, self.grid_h_var, self.grid_ox_var,
                    self.grid_oy_var):
            var.trace_add("write", lambda *_a: self._grid_fields_changed())

        ref_box = ttk.LabelFrame(parent, text="Reference", padding=4)
        ref_box.pack(fill="x", pady=(0, 4))
        self.ref_var = tk.StringVar()
        self.ref_opacity_var = tk.DoubleVar()
        self.ref_scale_var = tk.StringVar()
        self.ref_x_var = tk.StringVar()
        self.ref_y_var = tk.StringVar()
        self.ref_show = tk.BooleanVar()
        path_row = ttk.Frame(ref_box)
        path_row.pack(fill="x")
        ttk.Entry(path_row, textvariable=self.ref_var).pack(
            side="left", fill="x", expand=True)
        ttk.Button(path_row, text="...", width=3,
                   command=self.browse_reference).pack(side="left", padx=(2, 0))
        self.ref_var.trace_add("write", lambda *_a: self._ref_path_changed())
        row = ttk.Frame(ref_box)
        row.pack(fill="x", pady=(3, 0))
        ttk.Label(row, text="Opacity").pack(side="left")
        ttk.Scale(row, from_=0, to=100, variable=self.ref_opacity_var,
                  command=lambda _v: self._ref_changed(),
                  length=110).pack(side="left", padx=3)
        ttk.Checkbutton(row, text="Show", variable=self.ref_show,
                        command=self._ref_changed).pack(side="left", padx=4)
        row2 = ttk.Frame(ref_box)
        row2.pack(fill="x", pady=(2, 0))
        ttk.Label(row2, text="Scale").pack(side="left")
        ttk.Spinbox(row2, textvariable=self.ref_scale_var, width=5,
                    from_=10, to=800).pack(side="left", padx=2)
        ttk.Label(row2, text="X").pack(side="left")
        ttk.Spinbox(row2, textvariable=self.ref_x_var, width=4,
                    from_=-512, to=512).pack(side="left", padx=2)
        ttk.Label(row2, text="Y").pack(side="left")
        ttk.Spinbox(row2, textvariable=self.ref_y_var, width=4,
                    from_=-512, to=512).pack(side="left", padx=2)
        row3 = ttk.Frame(ref_box)
        row3.pack(fill="x", pady=(3, 0))
        ttk.Button(row3, text="Fit",
                   command=self.fit_reference).pack(side="left")
        ttk.Button(row3, text="Centre",
                   command=self.centre_reference).pack(side="left", padx=3)
        for var in (self.ref_scale_var, self.ref_x_var, self.ref_y_var):
            var.trace_add("write", lambda *_a: self._ref_changed())
        self.ref_show.trace_add("write", lambda *_a: self._ref_changed())

        tile_box = ttk.LabelFrame(parent, text="Tilesets", padding=4)
        tile_box.pack(fill="both", expand=True)
        sheet_row = ttk.Frame(tile_box)
        sheet_row.pack(fill="x")
        self.sheet_list = tk.Listbox(sheet_row, height=2,
                                     exportselection=False)
        self.sheet_list.pack(side="left", fill="x", expand=True)
        sheet_buttons = ttk.Frame(sheet_row)
        sheet_buttons.pack(side="left", padx=(2, 0))
        ttk.Button(sheet_buttons, text="Add", width=5,
                   command=self.add_sheet).pack(fill="x")
        ttk.Button(sheet_buttons, text="DMA", width=5,
                   command=self.add_dma_sheet).pack(fill="x")
        ttk.Button(sheet_buttons, text="Shape", width=5,
                   command=self.add_sha_sheets).pack(fill="x")
        ttk.Button(sheet_buttons, text="Image", width=5,
                   command=self.replace_sheet_image).pack(fill="x")
        ttk.Button(sheet_buttons, text="Size", width=5,
                   command=self.edit_sheet_size).pack(fill="x")
        ttk.Button(sheet_buttons, text="Del", width=5,
                   command=self.remove_sheet).pack(fill="x")
        self.sheet_list.bind("<<ListboxSelect>>",
                             lambda _e: self._sheet_selected())

        palette_frame = ttk.Frame(tile_box)
        palette_frame.pack(fill="both", expand=True, pady=(4, 0))
        palette_frame.rowconfigure(0, weight=1)
        palette_frame.columnconfigure(0, weight=1)
        self.palette = tk.Canvas(palette_frame, background="#181818",
                                 width=230, height=190,
                                 highlightthickness=0)
        self.palette.grid(row=0, column=0, sticky="nsew")
        pv = ttk.Scrollbar(palette_frame, orient="vertical",
                           command=self.palette.yview)
        ph = ttk.Scrollbar(palette_frame, orient="horizontal",
                           command=self.palette.xview)
        self.palette.configure(yscrollcommand=pv.set, xscrollcommand=ph.set)
        pv.grid(row=0, column=1, sticky="ns")
        ph.grid(row=1, column=0, sticky="ew")
        self.palette.bind("<ButtonPress-1>", self._palette_click)
        zoom_row = ttk.Frame(tile_box)
        zoom_row.pack(fill="x", pady=(3, 0))
        ttk.Label(zoom_row, text="Palette zoom").pack(side="left")
        ttk.Button(zoom_row, text="-", width=2,
                   command=lambda: self._palette_zoom(-1)).pack(side="left",
                                                               padx=2)
        ttk.Button(zoom_row, text="+", width=2,
                   command=lambda: self._palette_zoom(1)).pack(side="left")
        self.tile_label = tk.StringVar(value="No tile selected")
        ttk.Label(zoom_row, textvariable=self.tile_label).pack(
            side="left", padx=6)
        find_row = ttk.Frame(tile_box)
        find_row.pack(fill="x", pady=(2, 0))
        ttk.Label(find_row, text="Find tile/frame").pack(side="left")
        self.find_tile_var = tk.StringVar()
        find_entry = ttk.Entry(find_row, textvariable=self.find_tile_var,
                               width=7)
        find_entry.pack(side="left", padx=4)
        find_entry.bind("<Return>", lambda _e: self.find_tile())
        ttk.Button(find_row, text="Go", width=4,
                   command=self.find_tile).pack(side="left")

    def _build_right(self, parent):
        parts_box = ttk.LabelFrame(parent, text="Parts", padding=4)
        parts_box.pack(fill="both", expand=True)
        tree_frame = ttk.Frame(parts_box)
        tree_frame.pack(fill="both", expand=True)
        self.parts_tree = ttk.Treeview(
            tree_frame, columns=("kind", "pos"), show="tree headings",
            height=8, selectmode="browse")
        self.parts_tree.heading("#0", text="Name")
        self.parts_tree.heading("kind", text="Kind")
        self.parts_tree.heading("pos", text="Position")
        self.parts_tree.column("#0", width=120)
        self.parts_tree.column("kind", width=56, anchor="w")
        self.parts_tree.column("pos", width=106, anchor="w")
        ts = ttk.Scrollbar(tree_frame, orient="vertical",
                           command=self.parts_tree.yview)
        self.parts_tree.configure(yscrollcommand=ts.set)
        self.parts_tree.pack(side="left", fill="both", expand=True)
        ts.pack(side="left", fill="y")
        self.parts_tree.bind("<<TreeviewSelect>>",
                             lambda _e: self._tree_selected())
        buttons = ttk.Frame(parts_box)
        buttons.pack(fill="x", pady=(3, 0))
        ttk.Button(buttons, text="Add", width=5,
                   command=self.add_part).pack(side="left")
        ttk.Button(buttons, text="Dup", width=5,
                   command=self.duplicate_part).pack(side="left", padx=2)
        ttk.Button(buttons, text="Del", width=5,
                   command=lambda: self.delete_part(True)).pack(side="left")
        ttk.Button(buttons, text="Up", width=4,
                   command=lambda: self.move_part(-1)).pack(side="right")
        ttk.Button(buttons, text="Down", width=5,
                   command=lambda: self.move_part(1)).pack(side="right",
                                                          padx=2)

        form = ttk.LabelFrame(parent, text="Part fields", padding=4)
        form.pack(fill="x", pady=(4, 0))
        self.f_name = tk.StringVar()
        self.f_kind = tk.StringVar()
        self.f_x = tk.StringVar()
        self.f_y = tk.StringVar()
        self.f_w = tk.StringVar()
        self.f_h = tk.StringVar()
        self.f_colour = tk.StringVar()
        self.f_text = tk.StringVar()
        self.f_cell_w = tk.StringVar()
        self.f_cell_h = tk.StringVar()
        self.f_cols = tk.StringVar()
        self.f_rows = tk.StringVar()
        self.f_file = tk.StringVar()
        self.f_note = tk.StringVar()

        ttk.Label(form, text="Name").grid(row=0, column=0, sticky="w")
        ttk.Entry(form, textvariable=self.f_name, width=14).grid(
            row=0, column=1, sticky="ew", padx=2)
        ttk.Label(form, text="Kind").grid(row=0, column=2, sticky="w")
        kind_box = ttk.Combobox(form, textvariable=self.f_kind, width=8,
                                state="readonly", values=PART_KINDS)
        kind_box.grid(row=0, column=3, sticky="ew", padx=2)

        coord = ttk.Frame(form)
        coord.grid(row=1, column=0, columnspan=4, sticky="ew", pady=(3, 0))
        for label, var in (("X", self.f_x), ("Y", self.f_y),
                           ("W", self.f_w), ("H", self.f_h)):
            ttk.Label(coord, text=label).pack(side="left")
            ttk.Spinbox(coord, textvariable=var, width=5, from_=-512,
                        to=1024).pack(side="left", padx=(1, 5))

        ttk.Label(form, text="Colour").grid(row=2, column=0, sticky="w",
                                            pady=(3, 0))
        colour_values = tuple(f"{i} {EGA_NAMES[i]}" for i in range(16))
        self.colour_box = ttk.Combobox(form, textvariable=self.f_colour,
                                       width=14, state="readonly",
                                       values=colour_values)
        self.colour_box.grid(row=2, column=1, columnspan=3, sticky="w",
                             padx=2, pady=(3, 0))
        ttk.Label(form, text="Text").grid(row=3, column=0, sticky="w",
                                          pady=(3, 0))
        ttk.Entry(form, textvariable=self.f_text, width=24).grid(
            row=3, column=1, columnspan=3, sticky="ew", padx=2, pady=(3, 0))

        cell = ttk.Frame(form)
        cell.grid(row=4, column=0, columnspan=4, sticky="ew", pady=(3, 0))
        ttk.Label(cell, text="Cell").pack(side="left")
        ttk.Spinbox(cell, textvariable=self.f_cell_w, width=4, from_=1,
                    to=128).pack(side="left", padx=2)
        ttk.Label(cell, text="x").pack(side="left")
        ttk.Spinbox(cell, textvariable=self.f_cell_h, width=4, from_=1,
                    to=128).pack(side="left", padx=2)
        ttk.Label(cell, text="Cells").pack(side="left", padx=(6, 0))
        ttk.Spinbox(cell, textvariable=self.f_cols, width=4, from_=1,
                    to=64).pack(side="left", padx=2)
        ttk.Label(cell, text="x").pack(side="left")
        ttk.Spinbox(cell, textvariable=self.f_rows, width=4, from_=1,
                    to=64).pack(side="left", padx=2)

        ttk.Label(form, text="Image").grid(row=5, column=0, sticky="w",
                                           pady=(3, 0))
        image_row = ttk.Frame(form)
        image_row.grid(row=5, column=1, columnspan=3, sticky="ew", padx=2,
                       pady=(3, 0))
        ttk.Entry(image_row, textvariable=self.f_file).pack(
            side="left", fill="x", expand=True)
        ttk.Button(image_row, text="...", width=3,
                   command=self.browse_part_image).pack(side="left", padx=2)

        ttk.Label(form, text="Note").grid(row=6, column=0, sticky="w",
                                          pady=(3, 0))
        ttk.Entry(form, textvariable=self.f_note, width=24).grid(
            row=6, column=1, columnspan=3, sticky="ew", padx=2, pady=(3, 0))

        apply_row = ttk.Frame(form)
        apply_row.grid(row=7, column=0, columnspan=4, sticky="e",
                       pady=(4, 0))
        ttk.Button(apply_row, text="Apply fields",
                   command=self.apply_part_fields).pack(side="right")
        for var in (self.f_name, self.f_kind, self.f_x, self.f_y, self.f_w,
                    self.f_h, self.f_colour, self.f_text, self.f_cell_w,
                    self.f_cell_h, self.f_cols, self.f_rows, self.f_file,
                    self.f_note):
            var.trace_add("write", lambda *_a: self._part_fields_changed())
        form.bind("<Return>", lambda _e: self.apply_part_fields())

        help_text = (
            "Stamp: click/drag paints the selected tile.  Right-click erases.\n"
            "Select: drag a part to move it, drag its corner handles to size.\n"
            "Part fields snap to the grid while dragging; edit them for exact "
            "pixels.\n"
            "File > Copy summary prints the coordinates used for engine "
            "alignment.")
        ttk.Label(parent, text=help_text, justify="left",
                  wraplength=310).pack(fill="x", pady=(6, 0))

    # -- model <-> ui -------------------------------------------------------

    def _load_into_ui(self):
        lay = self.layout
        self._form_updating = True
        try:
            self.zoom = max(1, min(12, lay.zoom))
            self.zoom_var.set(f"{self.zoom}x")
            self.name_var.set(lay.name)
            self.style_var.set(lay.style)
            self.canvas_w_var.set(str(lay.width))
            self.canvas_h_var.set(str(lay.height))
            self.grid_w_var.set(str(lay.grid_w))
            self.grid_h_var.set(str(lay.grid_h))
            self.grid_ox_var.set(str(lay.grid_origin_x))
            self.grid_oy_var.set(str(lay.grid_origin_y))
            self.show_grid.set(lay.show_grid)
            self.snap.set(lay.snap)
            self.ref_var.set(lay.reference)
            self.ref_opacity_var.set(float(lay.reference_opacity))
            self.ref_scale_var.set(str(lay.reference_scale))
            self.ref_x_var.set(str(lay.reference_x))
            self.ref_y_var.set(str(lay.reference_y))
            self.ref_show.set(lay.show_reference)
        finally:
            self._form_updating = False
        self._refresh_sheet_list()
        self._refresh_parts()
        self.select_part(None)
        self._update_title()

    def _update_title(self):
        name = os.path.basename(self.path) if self.path else "untitled"
        mark = " *" if self.dirty else ""
        self.title(f"Jill-EX HUD Layout - {name}{mark}")

    def _mark_dirty(self):
        self.dirty = True
        self._update_title()

    def _layout_fields_changed(self):
        if self._form_updating:
            return
        lay = self.layout
        lay.name = self.name_var.get().strip() or "untitled"
        style = self.style_var.get().strip().lower()
        lay.style = style if style in ("classic", "modern", "custom") else "custom"
        lay.width = max(16, min(960, _as_int(self.canvas_w_var.get(),
                                             lay.width)))
        lay.height = max(16, min(960, _as_int(self.canvas_h_var.get(),
                                              lay.height)))
        lay.zoom = self.zoom
        self._mark_dirty()
        self.schedule_redraw()

    def _grid_fields_changed(self):
        if self._form_updating:
            return
        lay = self.layout
        lay.grid_w = max(1, min(128, _as_int(self.grid_w_var.get(),
                                             lay.grid_w)))
        lay.grid_h = max(1, min(128, _as_int(self.grid_h_var.get(),
                                             lay.grid_h)))
        lay.grid_origin_x = _as_int(self.grid_ox_var.get(), lay.grid_origin_x)
        lay.grid_origin_y = _as_int(self.grid_oy_var.get(), lay.grid_origin_y)
        lay.show_grid = bool(self.show_grid.get())
        lay.snap = bool(self.snap.get())
        self._mark_dirty()
        self.schedule_redraw()

    def _settings_changed(self):
        self.layout.show_grid = bool(self.show_grid.get())
        self.layout.snap = bool(self.snap.get())
        self._mark_dirty()
        self.schedule_redraw()

    def _ref_path_changed(self):
        if self._form_updating:
            return
        path = resolve_path(self.ref_var.get(), self._base_dir())
        self.ref_show.set(True)
        self.layout.reference = path
        self.layout.reference_image = load_image(path)
        self._ref_changed()

    def _ref_changed(self):
        if self._form_updating:
            return
        lay = self.layout
        lay.reference_opacity = max(0, min(100, int(
            self.ref_opacity_var.get())))
        lay.reference_scale = max(1, min(800, _as_int(
            self.ref_scale_var.get(), lay.reference_scale)))
        lay.reference_x = _as_int(self.ref_x_var.get(), lay.reference_x)
        lay.reference_y = _as_int(self.ref_y_var.get(), lay.reference_y)
        lay.show_reference = bool(self.ref_show.get())
        lay.reference = resolve_path(self.ref_var.get(), self._base_dir())
        if (lay.reference_image is None and lay.reference and
                os.path.isfile(lay.reference)):
            lay.reference_image = load_image(lay.reference)
        self._mark_dirty()
        self.schedule_redraw()

    def _base_dir(self):
        if self.path:
            return os.path.dirname(os.path.abspath(self.path))
        return os.getcwd()

    # -- zoom / redraw ------------------------------------------------------

    def set_zoom(self, zoom: int):
        zoom = max(1, min(12, zoom))
        self.zoom = zoom
        self.layout.zoom = zoom
        self.zoom_var.set(f"{zoom}x")
        self._mark_dirty()
        self.redraw()

    def _on_zoom_wheel(self, event):
        self.set_zoom(self.zoom + (1 if event.delta > 0 else -1))

    def schedule_redraw(self):
        if self._redraw_job is None:
            self._redraw_job = self.after(20, self.redraw)

    def redraw(self):
        self._redraw_job = None
        lay = self.layout
        z = self.zoom
        try:
            image = render_preview(lay)
            if image.size != (lay.width, lay.height):
                image = image.resize((lay.width, lay.height), Image.NEAREST)
            scaled = image.resize((lay.width * z, lay.height * z),
                                  Image.NEAREST)
            self._photo = ImageTk.PhotoImage(scaled)
        except Exception as exc:  # pragma: no cover - defensive
            self.status.set(f"render failed: {exc}")
            return
        if self._canvas_item is None:
            self._canvas_item = self.canvas.create_image(
                0, 0, anchor="nw", image=self._photo, tags="page")
        else:
            self.canvas.itemconfigure(self._canvas_item, image=self._photo)
        self.canvas.configure(scrollregion=(
            0, 0, lay.width * z, lay.height * z))
        self._draw_overlay()
        self._update_hint()
        self.canvas.delete("cursor")
        if self._last_cursor is not None:
            self._draw_cursor(*self._last_cursor)

    def _draw_overlay(self):
        lay = self.layout
        z = self.zoom
        self.canvas.delete("overlay")
        if lay.show_grid:
            colour = "#3A3A3A"
            strong = "#5A5A5A"
            step_x = lay.grid_w * z
            step_y = lay.grid_h * z
            if step_x >= 2 and step_y >= 2:
                start_x = (lay.grid_origin_x % max(1, lay.grid_w)) * z
                start_y = (lay.grid_origin_y % max(1, lay.grid_h)) * z
                x = start_x
                while x <= lay.width * z:
                    self.canvas.create_line(x, 0, x, lay.height * z,
                                            fill=colour, tags="overlay")
                    x += step_x
                y = start_y
                while y <= lay.height * z:
                    self.canvas.create_line(0, y, lay.width * z, y,
                                            fill=colour, tags="overlay")
                    y += step_y
                self.canvas.create_line(lay.grid_origin_x * z, 0,
                                        lay.grid_origin_x * z, lay.height * z,
                                        fill=strong, tags="overlay")
                self.canvas.create_line(0, lay.grid_origin_y * z,
                                        lay.width * z, lay.grid_origin_y * z,
                                        fill=strong, tags="overlay")

        for index, part in enumerate(lay.parts):
            w, h = part_bounds(part)
            x0, y0 = part.x * z, part.y * z
            x1, y1 = (part.x + w) * z, (part.y + h) * z
            colour = PART_COLOURS.get(part.kind, "#CCCCCC")
            width = 1 if z <= 2 else 2
            if index == self.selected:
                self.canvas.create_rectangle(x0, y0, x1, y1, outline="#FFFFFF",
                                             width=2, dash=(3, 2),
                                             tags="overlay")
                self._draw_handles(x0, y0, x1, y1)
            else:
                self.canvas.create_rectangle(x0, y0, x1, y1, outline=colour,
                                             width=width, tags="overlay")
            if part.kind in ("rect", "box"):
                self.canvas.create_rectangle(
                    x0, y0, x1, y1, fill=colour, stipple="gray12",
                    outline="", tags="overlay")
            label = part.name
            if part.kind == "text" and part.text:
                label = part.text
            elif part.kind == "note" and part.note:
                label = part.note
            if label:
                self.canvas.create_text(
                    x0 + 2, y0 + 1, anchor="nw", text=label,
                    fill=colour, tags="overlay",
                    font=("Consolas", max(6, int(6 * z / 1.6))))


    def _draw_handles(self, x0, y0, x1, y1):
        size = 8
        for hx, hy in ((x0, y0), (x1, y1)):
            self.canvas.create_rectangle(
                hx - size // 2, hy - size // 2, hx + size // 2,
                hy + size // 2, outline="#FFFFFF", fill="#000000",
                tags="overlay")

    def _update_hint(self):
        lay = self.layout
        bits = [f"{lay.grid_w}x{lay.grid_h} grid",
                f"{len(lay.tiles)} tiles", f"{len(lay.parts)} parts"]
        if self.active_sheet:
            bits.append(f"sheet {self.active_sheet}")
        self.hint.set(" | ".join(bits))

    # -- sheets -------------------------------------------------------------

    def _refresh_sheet_list(self):
        self.sheet_list.delete(0, tk.END)
        for sheet in self.layout.sheets:
            self.sheet_list.insert(tk.END, sheet.id)
        if self.layout.sheets:
            if self.active_sheet not in {s.id for s in self.layout.sheets}:
                self.active_sheet = self.layout.sheets[0].id
            for i, sheet in enumerate(self.layout.sheets):
                if sheet.id == self.active_sheet:
                    self.sheet_list.selection_set(i)
                    break
        else:
            self.active_sheet = None
        self._draw_palette()

    def _sheet_selected(self):
        selection = self.sheet_list.curselection()
        if not selection:
            return
        self.active_sheet = self.sheet_list.get(selection[0])
        self._draw_palette()
        self.schedule_redraw()

    def _sheet_by_id(self, sheet_id: Optional[str]):
        for sheet in self.layout.sheets:
            if sheet.id == sheet_id:
                return sheet
        return None

    def add_sheet(self):
        path = filedialog.askopenfilename(
            title="Open tileset image",
            filetypes=(("Images", "*.png *.jpg *.jpeg *.gif *.bmp *.ppm"),
                       ("All files", "*.*")))
        if not path:
            return
        size = simpledialog.askstring(
            "Tile size", "Tile width x height (e.g. 8x8):",
            initialvalue=f"{self.layout.grid_w}x{self.layout.grid_h}",
            parent=self)
        if size is None:
            return
        tile_w, tile_h = _as_pair(size, "x", (8, 8))
        if tile_w < 1 or tile_h < 1:
            messagebox.showerror("Tile size", "Tile size must be positive.")
            return
        sheet_id = unique_id({s.id for s in self.layout.sheets},
                             os.path.splitext(os.path.basename(path))[0],
                             "sheet")
        image = load_image(path)
        if image is None:
            messagebox.showerror("Tileset", f"Cannot read image:\n{path}")
            return
        self.push_undo()
        self.layout.sheets.append(Sheet(id=sheet_id, file=path,
                                        tile_w=tile_w, tile_h=tile_h,
                                        image=image))
        self.active_sheet = sheet_id
        self._mark_dirty()
        self._refresh_sheet_list()

    def add_dma_sheet(self):
        dialog = DmaImportDialog(self, getattr(self, "_dma_dir", ""))
        self.wait_window(dialog)
        data = dialog.result
        if not data:
            return
        self._dma_dir = os.path.dirname(data["dma"])
        sheet_id = unique_id(
            {s.id for s in self.layout.sheets},
            os.path.splitext(os.path.basename(data["dma"]))[0], "dma")
        self.config(cursor="watch")
        self.update_idletasks()
        try:
            sheet = make_dma_sheet(
                sheet_id, data["dma"], data["sha"], data["palette"],
                data["columns"], data["cell_w"], data["cell_h"])
        except Exception as exc:
            messagebox.showerror("DMA tileset", str(exc))
            return
        finally:
            self.config(cursor="")
        self.push_undo()
        self.layout.sheets.append(sheet)
        self.active_sheet = sheet.id
        self.active_tile = (0, 0)
        self.palette_zoom = 2
        if data.get("match_grid"):
            self.layout.grid_w = sheet.tile_w
            self.layout.grid_h = sheet.tile_h
            self.grid_w_var.set(str(sheet.tile_w))
            self.grid_h_var.set(str(sheet.tile_h))
        self._mark_dirty()
        self._refresh_sheet_list()
        self.status.set(
            f"Loaded {len(sheet.entries)} DMA tiles from "
            f"{os.path.basename(data['dma'])}; tile ids are shown in the "
            f"palette.")

    def add_sha_sheets(self):
        dialog = ShapeImportDialog(self, getattr(self, "_sha_dir", ""))
        self.wait_window(dialog)
        data = dialog.result
        if not data:
            return
        self._sha_dir = os.path.dirname(data["sha"])
        added = []
        failures = []
        snapshot_done = False
        self.config(cursor="watch")
        self.update_idletasks()
        try:
            for entry in data["entries"]:
                sheet_id = unique_id(
                    {s.id for s in self.layout.sheets}, f"sha{entry}", "sha")
                try:
                    sheet = make_sha_sheet(
                        sheet_id, data["sha"], entry, data["palette"],
                        data["columns"], data["cell_w"], data["cell_h"])
                except Exception as exc:
                    failures.append(f"entry {entry}: {exc}")
                    continue
                if not snapshot_done:
                    self.push_undo()
                    snapshot_done = True
                self.layout.sheets.append(sheet)
                added.append(sheet)
        finally:
            self.config(cursor="")
        if not added:
            messagebox.showerror(
                "HUD/UI shapes",
                "No shape entries loaded.\n\n" + "\n".join(failures))
            return
        self.active_sheet = added[-1].id
        self.active_tile = (0, 0)
        self.palette_zoom = 2
        self._mark_dirty()
        self._refresh_sheet_list()
        names = ", ".join(str(s.entry) for s in added)
        self.status.set(
            f"Loaded shape entries {names}; palette labels are "
            f"entry:frame.")
        if failures:
            messagebox.showwarning(
                "HUD/UI shapes",
                "Some entries could not be loaded:\n\n" + "\n".join(failures))

    def find_tile(self):
        sheet = self._sheet_by_id(self.active_sheet)
        if sheet is None or not sheet.entries:
            self.status.set("The active tileset has no tile or frame ids.")
            return
        text = self.find_tile_var.get().strip()
        if not text:
            return
        for (col, row), entry in sheet.entries.items():
            if entry is None:
                continue
            if isinstance(entry, ShapeTile):
                wanted = (text == str(entry.frame) or
                          text == f"{entry.entry}:{entry.frame}")
            else:
                wanted = text == str(entry.tile_id)
            if wanted:
                self.active_tile = (col, row)
                self.mode.set("stamp")
                self._draw_palette()
                self._scroll_palette_to(col, row)
                self.status.set(
                    tile_desc(self.layout, sheet.id, col, row))
                return
        self.status.set(
            f"No tile/frame '{text}' in the active tileset.")

    def _scroll_palette_to(self, col: int, row: int):
        sheet = self._sheet_by_id(self.active_sheet)
        if sheet is None or sheet.image is None:
            return
        z = self.palette_zoom
        width = max(1, sheet.image.width * z)
        height = max(1, sheet.image.height * z)
        cx = (col + 0.5) * sheet.tile_w * z / width
        cy = (row + 0.5) * sheet.tile_h * z / height
        view_w = max(1, self.palette.winfo_width())
        view_h = max(1, self.palette.winfo_height())
        self.palette.xview_moveto(max(0.0, cx - view_w / (2 * width)))
        self.palette.yview_moveto(max(0.0, cy - view_h / (2 * height)))

    def replace_sheet_image(self):
        sheet = self._sheet_by_id(self.active_sheet)
        if sheet is None:
            return
        if sheet.source in ("dma", "sha"):
            messagebox.showinfo(
                "Game tileset",
                "This tileset is rebuilt from its game files.  Remove it and "
                "use DMA... or Shape... to choose different sources.")
            return
        path = filedialog.askopenfilename(
            title=f"Replace image for {sheet.id}",
            filetypes=(("Images", "*.png *.jpg *.jpeg *.gif *.bmp *.ppm"),
                       ("All files", "*.*")))
        if not path:
            return
        image = load_image(path)
        if image is None:
            messagebox.showerror("Tileset", f"Cannot read image:\n{path}")
            return
        self.push_undo()
        sheet.file = path
        sheet.image = image
        self._mark_dirty()
        self._refresh_sheet_list()

    def edit_sheet_size(self):
        sheet = self._sheet_by_id(self.active_sheet)
        if sheet is None:
            return
        size = simpledialog.askstring(
            "Tile size", f"Tile width x height for {sheet.id}:",
            initialvalue=f"{sheet.tile_w}x{sheet.tile_h}", parent=self)
        if size is None:
            return
        tile_w, tile_h = _as_pair(size, "x", (sheet.tile_w, sheet.tile_h))
        tile_w = max(1, tile_w)
        tile_h = max(1, tile_h)
        if sheet.source == "dma":
            self.config(cursor="watch")
            self.update_idletasks()
            try:
                image, entries, columns = build_dma_sheet(
                    sheet.dma, sheet.sha, sheet.palette, sheet.columns or 16,
                    tile_w, tile_h)
            except Exception as exc:
                messagebox.showerror("DMA tileset", str(exc))
                return
            finally:
                self.config(cursor="")
            self.push_undo()
            sheet.tile_w = tile_w
            sheet.tile_h = tile_h
            sheet.image = image
            sheet.entries = entries
        elif sheet.source == "sha":
            self.config(cursor="watch")
            self.update_idletasks()
            try:
                image, entries, columns, tile_w, tile_h = build_sha_sheet(
                    sheet.sha, sheet.entry, sheet.palette,
                    sheet.columns or 16, tile_w, tile_h)
            except Exception as exc:
                messagebox.showerror("HUD/UI shapes", str(exc))
                return
            finally:
                self.config(cursor="")
            self.push_undo()
            sheet.tile_w = tile_w
            sheet.tile_h = tile_h
            sheet.image = image
            sheet.entries = entries
        else:
            self.push_undo()
            sheet.tile_w = tile_w
            sheet.tile_h = tile_h
        self._mark_dirty()
        self._draw_palette()
        self.schedule_redraw()

    def remove_sheet(self):
        sheet = self._sheet_by_id(self.active_sheet)
        if sheet is None:
            return
        count = sum(1 for value in self.layout.tiles.values()
                    if value[0] == sheet.id)
        if not messagebox.askyesno(
                "Remove tileset",
                f"Remove {sheet.id} and its {count} stamped tiles?"):
            return
        self.push_undo()
        self.layout.sheets = [s for s in self.layout.sheets
                              if s.id != sheet.id]
        self.layout.tiles = {k: v for k, v in self.layout.tiles.items()
                             if v[0] != sheet.id}
        self.active_sheet = (self.layout.sheets[0].id
                             if self.layout.sheets else None)
        self._mark_dirty()
        self._refresh_sheet_list()
        self.schedule_redraw()

    def _palette_zoom(self, delta: int):
        self.palette_zoom = max(1, min(8, self.palette_zoom + delta))
        self._draw_palette()

    def _draw_palette(self):
        self.palette.delete("all")
        self._palette_photo = None
        sheet = self._sheet_by_id(self.active_sheet)
        if sheet is None or sheet.image is None:
            self.palette.create_text(
                8, 8, anchor="nw", fill="#888888",
                text="Add a PNG, DMA or shape tileset to stamp tiles.")
            self.palette.configure(scrollregion=(0, 0, 220, 180))
            self.tile_label.set("No tile selected")
            return
        z = self.palette_zoom
        scaled = sheet.image.resize(
            (sheet.image.width * z, sheet.image.height * z), Image.NEAREST)
        self._palette_photo = ImageTk.PhotoImage(scaled)
        self.palette.create_image(0, 0, anchor="nw", image=self._palette_photo)
        for x in range(0, sheet.image.width, sheet.tile_w):
            self.palette.create_line(x * z, 0, x * z, sheet.image.height * z,
                                     fill="#333333")
        for y in range(0, sheet.image.height, sheet.tile_h):
            self.palette.create_line(0, y * z, sheet.image.width * z, y * z,
                                     fill="#333333")
        col, row = self.active_tile
        col = min(max(0, col), max(0, (sheet.image.width - 1) // sheet.tile_w))
        row = min(max(0, row), max(0, (sheet.image.height - 1) // sheet.tile_h))
        self.active_tile = (col, row)
        if sheet.source in ("dma", "sha"):
            # These sheets are identified by id, so label every cell.
            for (cell_col, cell_row), entry in sheet.entries.items():
                if entry is None:
                    continue
                tx = cell_col * sheet.tile_w * z
                ty = cell_row * sheet.tile_h * z
                if isinstance(entry, ShapeTile):
                    text = f"{entry.entry}:{entry.frame}"
                    ink = "#AAEEFF"
                else:
                    text = str(entry.tile_id)
                    ink = "#FFFF55"
                tag_w = min(sheet.tile_w * z,
                            max(16, 6 * len(text) + 2))
                self.palette.create_rectangle(
                    tx, ty, tx + tag_w, ty + 10, fill="#000000",
                    outline="")
                self.palette.create_text(
                    tx + 1, ty + 1, anchor="nw", text=text,
                    fill=ink, font=("Consolas", 7))
        self.palette.create_rectangle(
            col * sheet.tile_w * z, row * sheet.tile_h * z,
            (col + 1) * sheet.tile_w * z, (row + 1) * sheet.tile_h * z,
            outline="#FFFFFF", width=2)
        self.palette.configure(scrollregion=(
            0, 0, sheet.image.width * z, sheet.image.height * z))
        if sheet.source in ("dma", "sha"):
            label = tile_desc(self.layout, sheet.id, col, row)
            self.tile_label.set(label or f"Cell {col},{row} (empty)")
        else:
            self.tile_label.set(
                f"Tile {col},{row}  ({sheet.tile_w}x{sheet.tile_h})")

    def _palette_click(self, event):
        sheet = self._sheet_by_id(self.active_sheet)
        if sheet is None or sheet.image is None:
            return
        z = self.palette_zoom
        x = int(self.palette.canvasx(event.x) / z)
        y = int(self.palette.canvasy(event.y) / z)
        col = min(max(0, x // sheet.tile_w),
                  max(0, (sheet.image.width - 1) // sheet.tile_w))
        row = min(max(0, y // sheet.tile_h),
                  max(0, (sheet.image.height - 1) // sheet.tile_h))
        self.active_tile = (col, row)
        self.mode.set("stamp")
        self._draw_palette()
        if sheet.source in ("dma", "sha"):
            self.status.set(
                tile_desc(self.layout, sheet.id, col, row)
                or f"Cell {col},{row} is empty")

    # -- parts --------------------------------------------------------------

    def _refresh_parts(self):
        self.parts_tree.delete(*self.parts_tree.get_children())
        for i, part in enumerate(self.layout.parts):
            w, h = part_bounds(part)
            self.parts_tree.insert(
                "", "end", iid=str(i),
                text=part.name,
                values=(part.kind, f"{part.x},{part.y} {w}x{h}"))
        if self.selected is not None and 0 <= self.selected < len(
                self.layout.parts):
            self.parts_tree.selection_set(str(self.selected))
            self.parts_tree.see(str(self.selected))
        else:
            self.selected = None
            self._set_part_fields(None)

    def _tree_selected(self):
        selection = self.parts_tree.selection()
        if not selection:
            return
        index = int(selection[0])
        if 0 <= index < len(self.layout.parts):
            self.selected = index
            self._set_part_fields(self.layout.parts[index])
            self.schedule_redraw()

    def select_part(self, index: Optional[int]):
        self.selected = index
        if index is None:
            self.parts_tree.selection_remove(*self.parts_tree.selection())
            self._set_part_fields(None)
        else:
            self.parts_tree.selection_set(str(index))
            self._set_part_fields(self.layout.parts[index])
        self.schedule_redraw()

    def _set_part_fields(self, part: Optional[Part]):
        self._form_updating = True
        try:
            if part is None:
                for var in (self.f_name, self.f_kind, self.f_x, self.f_y,
                            self.f_w, self.f_h, self.f_colour, self.f_text,
                            self.f_cell_w, self.f_cell_h, self.f_cols,
                            self.f_rows, self.f_file, self.f_note):
                    var.set("")
            else:
                w, h = part_bounds(part)
                self.f_name.set(part.name)
                self.f_kind.set(part.kind)
                self.f_x.set(str(part.x))
                self.f_y.set(str(part.y))
                self.f_w.set(str(w))
                self.f_h.set(str(h))
                self.f_colour.set(f"{part.colour} {EGA_NAMES[part.colour]}")
                self.f_text.set(part.text)
                self.f_cell_w.set(str(part.cell_w))
                self.f_cell_h.set(str(part.cell_h))
                self.f_cols.set(str(part.cols))
                self.f_rows.set(str(part.rows))
                self.f_file.set(part.file)
                self.f_note.set(part.note)
        finally:
            self._form_updating = False

    def _part_fields_changed(self):
        if self._form_updating:
            return

    def apply_part_fields(self):
        if self.selected is None:
            return
        part = self.layout.parts[self.selected]
        name = self.f_name.get().strip()
        if not name:
            messagebox.showerror("Part name", "A part needs a name.")
            return
        if "]" in name or "\n" in name:
            messagebox.showerror("Part name",
                                 "The name cannot contain ']' or newlines.")
            return
        for other in self.layout.parts:
            if other is not part and other.name == name:
                messagebox.showerror("Part name",
                                     f"'{name}' is already used.")
                return
        self.push_undo()
        part.name = name
        kind = self.f_kind.get().strip().lower()
        part.kind = kind if kind in PART_KINDS else part.kind
        part.x = _as_int(self.f_x.get(), part.x)
        part.y = _as_int(self.f_y.get(), part.y)
        part.w = max(0, _as_int(self.f_w.get(), part.w))
        part.h = max(0, _as_int(self.f_h.get(), part.h))
        colour = self.f_colour.get().strip().split(" ", 1)[0]
        part.colour = max(0, min(15, _as_int(colour, part.colour)))
        part.text = self.f_text.get()
        part.cell_w = max(1, _as_int(self.f_cell_w.get(), part.cell_w))
        part.cell_h = max(1, _as_int(self.f_cell_h.get(), part.cell_h))
        part.cols = max(1, _as_int(self.f_cols.get(), part.cols))
        part.rows = max(1, _as_int(self.f_rows.get(), part.rows))
        part.note = self.f_note.get()
        path = resolve_path(self.f_file.get(), self._base_dir())
        if path != part.file:
            part.file = path
            part.image = load_image(path) if path else None
        self._mark_dirty()
        self._refresh_parts()
        self.schedule_redraw()

    def browse_part_image(self):
        path = filedialog.askopenfilename(
            title="Part image",
            filetypes=(("Images", "*.png *.jpg *.jpeg *.gif *.bmp *.ppm"),
                       ("All files", "*.*")))
        if path:
            self.f_file.set(path)
            self.apply_part_fields()

    def add_part(self):
        self.push_undo()
        name = unique_part_name(self.layout.parts, "part")
        gx = self.layout.grid_origin_x
        gy = self.layout.grid_origin_y
        part = Part(name=name, kind="rect", x=gx, y=gy,
                    w=self.layout.grid_w * 4,
                    h=self.layout.grid_h * 2)
        self.layout.parts.append(part)
        self.selected = len(self.layout.parts) - 1
        self._mark_dirty()
        self._refresh_parts()
        self._set_part_fields(part)
        self.schedule_redraw()

    def duplicate_part(self):
        if self.selected is None:
            return
        self.push_undo()
        original = self.layout.parts[self.selected]
        copy = replace(original)
        copy.name = unique_part_name(self.layout.parts, original.name + "_copy")
        copy.image = original.image
        self.layout.parts.insert(self.selected + 1, copy)
        self.selected += 1
        self._mark_dirty()
        self._refresh_parts()
        self._set_part_fields(copy)
        self.schedule_redraw()

    def delete_part(self, confirm: bool = False):
        if self.selected is None:
            return
        if confirm and not messagebox.askyesno(
                "Delete part", f"Delete {self.layout.parts[self.selected].name}?"):
            return
        self.push_undo()
        del self.layout.parts[self.selected]
        self.selected = None
        self._mark_dirty()
        self._refresh_parts()
        self.select_part(None)

    def move_part(self, delta: int):
        if self.selected is None:
            return
        target = self.selected + delta
        if not 0 <= target < len(self.layout.parts):
            return
        self.push_undo()
        parts = self.layout.parts
        parts[self.selected], parts[target] = parts[target], parts[self.selected]
        self.selected = target
        self._mark_dirty()
        self._refresh_parts()
        self._set_part_fields(parts[target])
        self.schedule_redraw()

    # -- canvas events ------------------------------------------------------

    def _event_pos(self, event) -> tuple[int, int]:
        x = int(self.canvas.canvasx(event.x) / self.zoom)
        y = int(self.canvas.canvasy(event.y) / self.zoom)
        return x, y

    def _select_handle(self, event) -> Optional[str]:
        if self.selected is None:
            return None
        part = self.layout.parts[self.selected]
        w, h = part_bounds(part)
        z = self.zoom
        corners = {
            "nw": (part.x * z, part.y * z),
            "se": ((part.x + w) * z, (part.y + h) * z),
        }
        px = self.canvas.canvasx(event.x)
        py = self.canvas.canvasy(event.y)
        for name, (cx, cy) in corners.items():
            if abs(px - cx) <= 6 and abs(py - cy) <= 6:
                return name
        return None

    def _on_press(self, event):
        x, y = self._event_pos(event)
        if not (0 <= x < self.layout.width and 0 <= y < self.layout.height):
            return
        self.canvas.focus_set()
        mode = self.mode.get()
        if mode == "select":
            handle = self._select_handle(event)
            if handle is not None:
                part = self.layout.parts[self.selected]
                self.push_undo()
                self._drag = {"op": "resize", "handle": handle,
                              "start": (x, y), "part": part,
                              "orig": (part.x, part.y, *part_bounds(part))}
                return
            for index in range(len(self.layout.parts) - 1, -1, -1):
                part = self.layout.parts[index]
                if part_contains(part, x, y):
                    self.select_part(index)
                    self.push_undo()
                    self._drag = {"op": "move", "start": (x, y),
                                  "part": part,
                                  "orig": (part.x, part.y)}
                    return
            self.select_part(None)
            return
        self.push_undo()
        self._stroke = True
        self._stamp_or_erase(x, y, mode)

    def _on_right_press(self, event):
        x, y = self._event_pos(event)
        self.push_undo()
        self._stroke = True
        self._stamp_or_erase(x, y, "erase")

    def _stamp_or_erase(self, x: int, y: int, mode: str):
        if mode == "stamp":
            sheet = self._sheet_by_id(self.active_sheet)
            if sheet is None or sheet.image is None:
                self.status.set("Add and select a tileset before stamping.")
                return
            gx, gy = cell_pos(self.layout, x, y)
            if not (0 <= gx < self.layout.width and
                    0 <= gy < self.layout.height):
                return
            self.layout.tiles[(gx, gy)] = (sheet.id, self.active_tile[0],
                                           self.active_tile[1])
            self.status.set(f"Stamped tile {self.active_tile[0]},"
                            f"{self.active_tile[1]} at {gx},{gy}")
        else:
            key = tile_at(self.layout, x, y)
            if key is not None:
                del self.layout.tiles[key]
                self.status.set(f"Erased tile at {key[0]},{key[1]}")
            else:
                return
        self._mark_dirty()
        self.schedule_redraw()

    def _on_drag(self, event):
        x, y = self._event_pos(event)
        lay = self.layout
        if self._drag is not None:
            drag = self._drag
            part = drag["part"]
            if drag["op"] == "move":
                dx = x - drag["start"][0]
                dy = y - drag["start"][1]
                nx = drag["orig"][0] + dx
                ny = drag["orig"][1] + dy
                if self.snap.get():
                    nx = snap_value(nx, lay.grid_origin_x, lay.grid_w)
                    ny = snap_value(ny, lay.grid_origin_y, lay.grid_h)
                part.x, part.y = nx, ny
            elif drag["op"] == "resize":
                ox, oy, ow, oh = drag["orig"]
                nx, ny = ox, oy
                nw, nh = ow, oh
                if drag["handle"] == "se":
                    nw = max(1, x - ox)
                    nh = max(1, y - oy)
                    if self.snap.get():
                        nw = max(lay.grid_w,
                                 snap_value(nw, 0, lay.grid_w))
                        nh = max(lay.grid_h,
                                 snap_value(nh, 0, lay.grid_h))
                else:
                    nx = min(x, ox + ow - 1)
                    ny = min(y, oy + oh - 1)
                    nw = max(1, ow + (ox - nx))
                    nh = max(1, oh + (oy - ny))
                    if self.snap.get():
                        nx = snap_value(nx, lay.grid_origin_x, lay.grid_w)
                        ny = snap_value(ny, lay.grid_origin_y, lay.grid_h)
                        nw = max(lay.grid_w, ox + ow - nx)
                        nh = max(lay.grid_h, oy + oh - ny)
                part.x, part.y, part.w, part.h = nx, ny, nw, nh
            self._set_part_fields(part)
            self._mark_dirty()
            self.schedule_redraw()
            return
        if self._stroke:
            self._stamp_or_erase(x, y, self.mode.get())

    def _on_release(self, _event):
        if self._drag is not None:
            self._drag = None
            self._refresh_parts()
            self.schedule_redraw()
        self._stroke = False

    def _on_motion(self, event):
        x, y = self._event_pos(event)
        self._last_cursor = (x, y)
        col, row, rx, ry = grid_of(self.layout, x, y)
        self.status.set(
            f"x={x} y={y}   cell col {col}+{rx}, row {row}+{ry}   "
            f"mode {self.mode.get()}")
        self._draw_cursor(x, y)

    def _draw_cursor(self, x: int, y: int):
        self.canvas.delete("cursor")
        if self.mode.get() == "select":
            return
        gx, gy = cell_pos(self.layout, x, y)
        if not (0 <= gx < self.layout.width and 0 <= gy < self.layout.height):
            return
        z = self.zoom
        self.canvas.create_rectangle(
            gx * z, gy * z, (gx + self.layout.grid_w) * z,
            (gy + self.layout.grid_h) * z, outline="#DDDDDD", dash=(2, 2),
            tags="cursor")

    def _clear_cursor(self):
        self.canvas.delete("cursor")

    def _mode_changed(self):
        self.canvas.configure(
            cursor="arrow" if self.mode.get() == "select" else "crosshair")
        self._clear_cursor()

    def _delete_key(self, _event):
        widget = self.focus_get()
        if isinstance(widget, (tk.Entry, ttk.Entry, ttk.Spinbox,
                               ttk.Combobox)):
            return
        if self.mode.get() == "select" and self.selected is not None:
            self.delete_part(True)

    # -- undo ---------------------------------------------------------------

    def _snapshot(self):
        return (dict(self.layout.tiles),
                clone_parts(self.layout.parts),
                [replace(sheet) for sheet in self.layout.sheets])

    def _restore(self, snapshot):
        tiles, parts, sheets = snapshot
        self.layout.tiles = tiles
        self.layout.parts = parts
        self.layout.sheets = sheets
        self.active_sheet = sheets[0].id if sheets else None
        self.selected = None
        self._mark_dirty()
        self._refresh_sheet_list()
        self._refresh_parts()
        self.select_part(None)
        self.schedule_redraw()

    def push_undo(self):
        self._undo.append(self._snapshot())
        if len(self._undo) > 60:
            self._undo.pop(0)
        self._redo.clear()

    def undo(self):
        if not self._undo:
            return
        self._redo.append(self._snapshot())
        self._restore(self._undo.pop())

    def redo(self):
        if not self._redo:
            return
        self._undo.append(self._snapshot())
        self._restore(self._redo.pop())

    # -- reference actions --------------------------------------------------

    def browse_reference(self):
        path = filedialog.askopenfilename(
            title="Reference image",
            filetypes=(("Images", "*.png *.jpg *.jpeg *.gif *.bmp *.ppm"),
                       ("All files", "*.*")))
        if path:
            self.ref_var.set(path)

    def fit_reference(self):
        image = self.layout.reference_image
        if image is None:
            path = resolve_path(self.ref_var.get(), self._base_dir())
            image = load_image(path)
            if image is not None:
                self.layout.reference_image = image
        if image is None:
            messagebox.showerror("Reference", "Load a reference image first.")
            return
        scale = min(100 * self.layout.width / image.width,
                    100 * self.layout.height / image.height)
        self.ref_scale_var.set(str(max(1, int(round(scale)))))
        self.centre_reference()

    def centre_reference(self):
        image = self.layout.reference_image
        if image is None:
            return
        scale = max(1, _as_int(self.ref_scale_var.get(), 100)) / 100.0
        self.ref_x_var.set(str(int((self.layout.width -
                                    image.width * scale) / 2)))
        self.ref_y_var.set(str(int((self.layout.height -
                                    image.height * scale) / 2)))

    # -- files --------------------------------------------------------------

    def new_file(self):
        if not self._confirm_discard():
            return
        self.layout = new_layout()
        self.path = None
        self.dirty = False
        self._undo.clear()
        self._redo.clear()
        self.selected = None
        self.active_sheet = None
        self.active_tile = (0, 0)
        self._load_into_ui()
        self.redraw()

    def open_file(self, path: Optional[str] = None):
        if not self._confirm_discard():
            return
        if path is None:
            path = filedialog.askopenfilename(
                title="Open HUD layout",
                filetypes=(("HUD layout", "*" + EXTENSION),
                           ("All files", "*.*")))
        if not path:
            return
        try:
            layout = load_layout(path)
        except ValueError as exc:
            messagebox.showerror("Open HUD layout", str(exc))
            return
        self.layout = layout
        self.path = layout.path
        self.dirty = False
        self._undo.clear()
        self._redo.clear()
        self.selected = None
        self.active_tile = (0, 0)
        self._load_into_ui()
        self.redraw()
        if layout.warnings:
            self.status.set("; ".join(layout.warnings))

    def _confirm_discard(self) -> bool:
        if not self.dirty:
            return True
        answer = messagebox.askyesnocancel(
            "Unsaved layout", "Save changes before continuing?")
        if answer is None:
            return False
        if answer:
            return self.save()
        return True

    def save(self) -> bool:
        if not self.path:
            return self.save_as()
        try:
            save_layout(self.layout, self.path)
        except OSError as exc:
            messagebox.showerror("Save HUD layout", str(exc))
            return False
        self.dirty = False
        self._update_title()
        self.status.set(f"Saved {self.path}")
        return True

    def save_as(self) -> bool:
        path = filedialog.asksaveasfilename(
            title="Save HUD layout", defaultextension=EXTENSION,
            initialfile=(os.path.basename(self.path) if self.path
                         else self.layout.name + EXTENSION),
            filetypes=(("HUD layout", "*" + EXTENSION), ("All files", "*.*")))
        if not path:
            return False
        try:
            save_layout(self.layout, path)
        except OSError as exc:
            messagebox.showerror("Save HUD layout", str(exc))
            return False
        self.path = path
        self.dirty = False
        self._update_title()
        self.status.set(f"Saved {path}")
        return True

    def on_close(self):
        if not self._confirm_discard():
            return
        self.destroy()

    def export_png(self):
        path = filedialog.asksaveasfilename(
            title="Export HUD PNG", defaultextension=".png",
            initialfile=(self.layout.name + ".png"),
            filetypes=(("PNG", "*.png"), ("All files", "*.*")))
        if not path:
            return
        try:
            image = render_layout(self.layout, reference=False, images=True)
            image.save(path)
        except OSError as exc:
            messagebox.showerror("Export PNG", str(exc))
            return
        self.status.set(f"Exported {path}")

    def copy_summary(self):
        text = summary_text(self.layout, self.path)
        self.clipboard_clear()
        self.clipboard_append(text)
        self.status.set("Summary copied to the clipboard.")

    def insert_classic_guide(self):
        """Add the current engine classic-HUD rectangles as guide parts."""
        existing = {p.name for p in self.layout.parts}
        new_parts = [p for p in classic_guide_parts() if p.name not in existing]
        if not new_parts:
            self.status.set("The classic HUD guide is already present.")
            return
        self.push_undo()
        self.layout.parts.extend(new_parts)
        self._mark_dirty()
        self._refresh_parts()
        self.schedule_redraw()


def run_gui(path: Optional[str] = None) -> int:
    if tk is None:
        print("tkinter is not available; install Python with Tk support.",
              file=sys.stderr)
        return 2
    if Image is None:
        print("Pillow is required for the editor: pip install pillow",
              file=sys.stderr)
        return 2
    layout = None
    if path:
        try:
            layout = load_layout(path)
        except ValueError as exc:
            print(f"error: {exc}", file=sys.stderr)
            return 2
    app = HudLayoutApp(layout)
    if layout is not None:
        app.path = layout.path
        app.dirty = False
        app._update_title()
    app.mainloop()
    return 0


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description="Jill-EX HUD layout designer (.hudlayout)")
    parser.add_argument("layout", nargs="?", help="layout file to open")
    parser.add_argument("--summary", action="store_true",
                        help="print coordinates and exit (no GUI)")
    parser.add_argument("--render", metavar="PNG",
                        help="render tiles to a PNG and exit (no GUI)")
    args = parser.parse_args(argv)

    if args.summary:
        if not args.layout:
            parser.error("--summary needs a layout file")
        try:
            layout = load_layout(args.layout)
        except ValueError as exc:
            print(f"error: {exc}", file=sys.stderr)
            return 2
        sys.stdout.write(summary_text(layout))
        return 0

    if args.render:
        if not args.layout:
            parser.error("--render needs a layout file")
        if Image is None:
            print("error: Pillow is required to render", file=sys.stderr)
            return 2
        try:
            layout = load_layout(args.layout)
        except ValueError as exc:
            print(f"error: {exc}", file=sys.stderr)
            return 2
        image = render_layout(layout, reference=False, images=True)
        image.save(args.render)
        print(f"rendered {args.render} ({image.width}x{image.height})")
        return 0

    return run_gui(args.layout)


if __name__ == "__main__":
    sys.exit(main())
