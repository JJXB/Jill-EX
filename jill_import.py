#!/usr/bin/env python3
"""
Picture / audio -> Jill converter.

Import PNG images and WAV audio into Jill (and Onesimus) archives, or build
brand new archives. Two levels of operation:

  * Replace content inside an existing archive
        replace_sha_frame(sha, entry, frame, png)   -> new .sha
        sha_entry_from_folder(sha, entry, folder)   -> new .sha (whole animation)
        replace_vcl_sfx(vcl, slot, wav)             -> new .vcl
  * Package new archives from a folder tree
        sha_from_folder(sha, folder)                -> new .sha
        vcl_from_folder(vcl, folder)                -> new .vcl

The module also has a small CLI:

    python jill_import.py sha-frame  <sha> <entry> <frame> <image.png> [-o out.sha]
    python jill_import.py sha-folder <sha> <folder> [-o out.sha]
    python jill_import.py sha-entry-folder <sha> <entry> <frames> [-o out.sha]
    python jill_import.py sha-font   <sha> <fonts/main.png> [-o out.sha]
    python jill_import.py sha-tile   <sha> <dma> <tile-id> <image.png> [-o out.sha]
    python jill_import.py tile-clone <sha> <dma> <tile-id> <image.png> [--out-sha out.sha] [--out-dma out.dma]
    python jill_import.py tile-safe  <sha> <dma> <tile-id> <image.png> [--out-sha out.sha] [--out-dma out.dma]
    python jill_import.py palette-export <sha> <entry> <out.pal|.gpl|.act>
    python jill_import.py palette-import <sha> <entry> <palette> [-o out.sha]
    python jill_import.py portrait  <sha> <portrait.png> [-o out.sha]
    python jill_import.py board-image <board> <sha> <dma> <board_render.png> [-o out.jn1]
    python jill_import.py vcl-sfx    <vcl> <slot> <audio.wav> [-o out.vcl]
    python jill_import.py vcl-folder <vcl> <folder> [-o out.vcl]
    python jill_import.py vcl-pages  <vcl> <text_pages> [-o out.vcl]
    python jill_import.py pack       <extractor-out> <external/pack> [--game ID] [--name NAME]
"""
from __future__ import annotations

import io
import json
import math
import os
import re
import shutil
import struct
import sys
import traceback
import wave
from pathlib import Path
from typing import Dict, List, Optional, Sequence, Tuple

from PIL import Image

# Import the reader/decoder implemented by the extractor.
sys.path.insert(0, str(Path(__file__).resolve().parent))
import jill_extractor as jx  # noqa: E402

# VGA colour-table shift/mask (mode 4) used to resolve entry palettes.
_VGA_SHIFT, _VGA_MASK = 16, 255


class ImportFailure(ValueError):
    """An input file could not be imported; message is safe to show to users."""


# ---------------------------------------------------------------------------
# Input validation helpers
# ---------------------------------------------------------------------------

def _file(value, what: str) -> Path:
    """Return a regular, existing input file or fail with a useful message."""
    p = Path(value)
    if not p.exists():
        raise ImportFailure(f"{what} not found: {p}")
    if not p.is_file():
        raise ImportFailure(f"{what} is not a file: {p}")
    return p


def _dir(value, what: str) -> Path:
    p = Path(value)
    if not p.exists():
        raise ImportFailure(f"{what} not found: {p}")
    if not p.is_dir():
        raise ImportFailure(f"{what} is not a directory: {p}")
    return p


def _out(value, default, what: str) -> Path:
    """Validate an output path; None means overwrite the input archive."""
    p = Path(default if value is None else value)
    if p.exists() and p.is_dir():
        raise ImportFailure(f"{what} is a directory: {p}")
    if not p.parent.is_dir():
        raise ImportFailure(f"output folder does not exist: {p.parent}")
    return p


def _index(value: int, limit: int, what: str) -> int:
    if not isinstance(value, int) or not 0 <= value < limit:
        raise ImportFailure(f"{what} must be 0..{limit - 1} (got {value})")
    return value


def _load_sha(value, what: str = "shape archive") -> "jx.SHAFile":
    p = _file(value, what)
    try:
        sha = jx.SHAFile.load(str(p))
    except Exception as e:
        raise ImportFailure(f"{what} {p.name}: {e}") from e
    data_len = len(sha.file_data)
    for i, (off, size) in enumerate(zip(sha.offsets, sha.sizes)):
        if size and (off < 0x300 or off + size > data_len):
            raise ImportFailure(
                f"{what} {p.name}: entry {i} points outside the file "
                f"(offset {off}, size {size})")
    return sha


def _load_dma(value, what: str = "tile archive") -> "jx.DMAFile":
    p = _file(value, what)
    try:
        return jx.DMAFile.load(str(p))
    except Exception as e:
        raise ImportFailure(f"{what} {p.name}: {e}") from e


def _load_vcl(value, what: str = "sound archive") -> "jx.VCLFile":
    p = _file(value, what)
    try:
        vcl = jx.VCLFile.load(str(p))
    except Exception as e:
        raise ImportFailure(f"{what} {p.name}: {e}") from e
    data_len = len(vcl.file_data)
    for name, offs, sizes in (('sfx', vcl.sfx_offsets, vcl.sfx_sizes),
                              ('music', vcl.music_offsets, vcl.music_sizes)):
        for i, (off, size) in enumerate(zip(offs, sizes)):
            if size and (off < _VCL_HEADER or off + size > data_len):
                raise ImportFailure(
                    f"{what} {p.name}: {name} entry {i} points outside "
                    f"the file (offset {off}, size {size})")
    return vcl


def build_dma_bytes(records: Sequence) -> bytes:
    """Serialise DMA records (tile id, shape ref, attrs, name) to a .dma."""
    if len(records) > 320:
        raise ImportFailure(f"too many DMA records ({len(records)} > 320)")
    out = bytearray()
    for rec in records:
        name = str(getattr(rec, 'name', '')).encode('ascii', 'replace')[:255]
        out += struct.pack('<HHHB', int(rec.tile_id) & 0xFFFF,
                           int(rec.shape_id) & 0xFFFF,
                           int(rec.attr) & 0xFFFF, len(name))
        out += name
    return bytes(out)


def save_dma(dma: "jx.DMAFile", path, out_path=None) -> Path:
    """Write an in-memory DMA archive back to disk."""
    out = _out(out_path, path, "output .dma")
    try:
        out.write_bytes(build_dma_bytes(dma.records))
    except Exception as e:
        raise ImportFailure(f"cannot write tile archive {out}: {e}") from e
    return out


def clone_tile_to_entry(sha_path, dma_path, tile_id: int, image_path,
                        out_sha=None, out_dma=None,
                        resize: bool = True) -> Tuple[Path, Path, int, int]:
    """Import tile art with a new palette without changing other tiles.

    A 4/8bpp shape entry shares one palette between all of its frames, so
    recolouring it in place changes every tile that uses any frame of it.
    Instead this copies the entry into a free shape slot, adapts the palette
    there and repoints the DMA record at the copy.  Both .sha and .dma are
    written.  DMA shape refs only have 6 bits for the entry, so the copy must
    fit in entries 0..63.
    """
    sha = _load_sha(sha_path)
    dma = _load_dma(dma_path)
    _index(tile_id, 0x10000, "tile id")
    rec = next((r for r in dma.records if r.tile_id == tile_id), None)
    if rec is None:
        raise ImportFailure(f"tile id 0x{tile_id:02X} not in "
                            f"{Path(dma_path).name}")
    entry_idx = (rec.shape_id >> 8) & 0x3F
    frame_idx = rec.shape_id & 0xFF
    entry = sha.entries[entry_idx]
    if entry is None or frame_idx >= len(entry.frames):
        raise ImportFailure(f"tile 0x{tile_id:02X} points at missing shape "
                            f"{entry_idx}:{frame_idx}")
    if entry.src_bpp >= 8:
        raise ImportFailure("tile entry uses the global palette; "
                            "palette adaptation needs a 4/8bpp entry")
    free = next((i for i in range(64) if sha.entries[i] is None), None)
    if free is None:
        raise ImportFailure("no free shape entry 0..63 to clone the tile into")

    img = _load_image(image_path)
    old = entry.frames[frame_idx]
    if resize and img.size != (old['width'], old['height']):
        img = img.resize((old['width'], old['height']), Image.NEAREST)
    colors, palette_ints = adapt_entry_palette(sha, entry_idx, img)
    frames = _entry_frames(entry)
    w, h, data = quantize(img, colors)
    frames[frame_idx] = (w, h, 0, data)
    blob = encode_entry(frames, entry.src_bpp, entry.flags & ~1, palette_ints,
                        _entry_trailing(entry))
    blobs = _all_entry_blobs(sha)
    blobs[free] = blob

    sha_out = _out(out_sha, sha_path, "output .sha")
    dma_out = _out(out_dma, dma_path, "output .dma")
    sha_out.write_bytes(build_sha_bytes(blobs))
    rec.shape_id = (free << 8) | frame_idx
    save_dma(dma, dma_path, dma_out)
    return sha_out, dma_out, free, frame_idx


def _nearest_vga_index(sha: "jx.SHAFile", rgb: Tuple[int, int, int],
                       allow_zero: bool = True) -> int:
    """Snap an RGB triplet to the closest index in the game's global palette."""
    base = sha.vga_palette
    r, g, b = rgb
    best, bestd = (0 if allow_zero else 1), 1 << 30
    start = 0 if allow_zero else 1
    for i in range(start, 256):
        dr, dg, db = r - base[i * 3], g - base[i * 3 + 1], b - base[i * 3 + 2]
        d = dr * dr + dg * dg + db * db
        if d < bestd:
            best, bestd = i, d
            if d == 0:
                break
    return best


def tile_from_png_safe(sha_path, dma_path, tile_id: int, image_path,
                       out_sha=None, out_dma=None,
                       resize: bool = True) -> Tuple[Path, Path, int, int]:
    """Import tile art with new colours without touching any entry palette.

    The original DOS engine only loads the shape entries it needs and shares
    one colour table between every frame of an entry.  Rewriting that table (or
    adding a brand-new entry) therefore shows up as corruption in the original
    game.  Instead this:

    * quantises the image against the fixed 256-colour VGA palette,
    * appends the frame to an **existing 8bpp (identity-mapped) shape entry**,
      so no per-entry colour table exists at all, and
    * repoints only that tile's DMA record at the new frame.

    Both archives are written.  If the game bank has no 8bpp entry with a free
    frame slot, ``ImportFailure`` explains that the in-place/dithered import
    should be used instead.
    """
    sha = _load_sha(sha_path)
    dma = _load_dma(dma_path)
    _index(tile_id, 0x10000, "tile id")
    rec = next((r for r in dma.records if r.tile_id == tile_id), None)
    if rec is None:
        raise ImportFailure(f"tile id 0x{tile_id:02X} not in "
                            f"{Path(dma_path).name}")
    entry_idx = (rec.shape_id >> 8) & 0x3F
    frame_idx = rec.shape_id & 0xFF
    entry = sha.entries[entry_idx]
    if entry is None or frame_idx >= len(entry.frames):
        raise ImportFailure(f"tile 0x{tile_id:02X} points at missing shape "
                            f"{entry_idx}:{frame_idx}")

    img = _load_image(image_path)
    old = entry.frames[frame_idx]
    if resize and img.size != (old['width'], old['height']):
        img = img.resize((old['width'], old['height']), Image.NEAREST)
    w, h, data = quantize(img, _vga_colors(sha))

    # Prefer an 8bpp entry below 64 (the DMA shape field's limit).  Appending
    # to an entry the original loader already uses is what keeps the old EXE
    # happy: no new directory entry and no palette table are introduced.
    host = None
    for i in range(64):
        candidate = sha.entries[i]
        if (candidate is not None and candidate.src_bpp >= 8
                and len(candidate.frames) < 255):
            host = i
            break
    if host is None:
        raise ImportFailure(
            "no 8bpp shape entry below 64 has a free frame slot; use the "
            "dithered in-place import instead (it stays original-safe but "
            "approximates new colours with the tile's current palette)")

    host_entry = sha.entries[host]
    frames = _entry_frames(host_entry)
    new_frame = len(frames)
    frames.append((w, h, 0, data))
    blob = encode_entry(frames, 8, host_entry.flags, None,
                        _entry_trailing(host_entry))
    blobs = _all_entry_blobs(sha)
    blobs[host] = blob

    sha_out = _out(out_sha, sha_path, "output .sha")
    dma_out = _out(out_dma, dma_path, "output .dma")
    sha_out.write_bytes(build_sha_bytes(blobs))
    rec.shape_id = (host << 8) | new_frame
    save_dma(dma, dma_path, dma_out)
    return sha_out, dma_out, host, new_frame


# ---------------------------------------------------------------------------
# Palette import/export (for image editors)
# ---------------------------------------------------------------------------

def export_entry_palette(sha_path, entry_idx: int, out_path) -> Path:
    """Write one shape entry's resolved palette for an image editor.

    The format follows the output extension: ``.pal`` (JASC-PAL text),
    ``.gpl`` (GIMP) or ``.act`` (Adobe Colour Table, fixed 256 entries).
    """
    sha = _load_sha(sha_path)
    _index(entry_idx, 128, "shape entry")
    if sha.entries[entry_idx] is None:
        raise ImportFailure(f"shape entry {entry_idx} is empty")
    colors = resolved_palette(sha, entry_idx)
    out = Path(out_path)
    ext = out.suffix.lower()
    if ext == '.gpl':
        lines = ["GIMP Palette", f"Name: entry {entry_idx}", "Columns: 8", "#"]
        for i, (r, g, b) in enumerate(colors):
            lines.append(f"{r:3d} {g:3d} {b:3d}\tindex {i}")
        out.write_text("\n".join(lines) + "\n", encoding='ascii')
    elif ext == '.act':
        data = bytearray(768)
        for i, (r, g, b) in enumerate(colors[:256]):
            data[i * 3:i * 3 + 3] = bytes((
                max(0, min(255, r)), max(0, min(255, g)),
                max(0, min(255, b))))
        out.write_bytes(bytes(data))
    else:
        lines = ["JASC-PAL", "0100", str(len(colors))]
        lines += [f"{r} {g} {b}" for r, g, b in colors]
        out.write_text("\n".join(lines) + "\n", encoding='ascii')
    return out


def _read_palette_colors(path: Path) -> List[Tuple[int, int, int]]:
    """Parse JASC-PAL, GIMP .gpl, RIFF .pal or Adobe .act colour tables."""
    data = path.read_bytes()
    ext = path.suffix.lower()
    if ext == '.act' or (len(data) in (768, 772) and b'\n' not in data[:64]):
        if len(data) < 768:
            raise ImportFailure(f"palette {path.name}: .act needs 768 bytes")
        return [tuple(data[i * 3:i * 3 + 3]) for i in range(256)]
    if data[:4] == b'RIFF' and data[8:12] == b'PAL ':
        pos = 12
        while pos + 8 <= len(data):
            tag = data[pos:pos + 4]
            size = struct.unpack('<I', data[pos + 4:pos + 8])[0]
            body = data[pos + 8:pos + 8 + size]
            if tag == b'data':
                count = struct.unpack('<H', body[2:4])[0] if len(body) >= 4 else 0
                return [tuple(body[4 + i * 3:7 + i * 3]) for i in range(count)]
            pos += 8 + size + (size & 1)
        raise ImportFailure(f"palette {path.name}: RIFF PAL has no data chunk")
    try:
        text = data.decode('utf-8', errors='replace')
    except Exception as e:
        raise ImportFailure(f"palette {path.name}: unreadable ({e})") from e
    lines = [ln.strip() for ln in text.splitlines()]
    if lines and lines[0].upper() == 'JASC-PAL':
        lines = lines[3:]
    elif lines and lines[0].lower().startswith('gimp palette'):
        lines = [ln for ln in lines[1:]
                 if ln and not ln.startswith('#')
                 and not ln.lower().startswith(('name:', 'columns:'))]
    colors: List[Tuple[int, int, int]] = []
    for ln in lines:
        if not ln or ln.startswith('#'):
            continue
        parts = ln.replace('\t', ' ').split()
        if len(parts) < 3:
            continue
        try:
            r, g, b = (max(0, min(255, int(parts[i]))) for i in range(3))
        except ValueError:
            continue
        colors.append((r, g, b))
    if not colors:
        raise ImportFailure(f"palette {path.name}: no RGB entries found")
    return colors


def import_entry_palette(sha_path, entry_idx: int, palette_path,
                         out_path=None) -> Path:
    """Replace one 4/8bpp entry's colour table from an image-editor palette.

    Imported colours are snapped to the game's fixed global VGA palette and
    the unused low 16 bits of every palette word are preserved, so the
    original game's CGA/EGA tables stay intact.  All frames sharing the entry
    will change colour (that is what an entry palette is).
    """
    sha = _load_sha(sha_path)
    _index(entry_idx, 128, "shape entry")
    entry = sha.entries[entry_idx]
    if entry is None:
        raise ImportFailure(f"shape entry {entry_idx} is empty")
    if entry.src_bpp >= 8 or (entry.flags & 1):
        raise ImportFailure(
            f"shape entry {entry_idx} has no per-entry colour table "
            f"({entry.src_bpp}bpp, flags 0x{entry.flags:02X})")
    count = 1 << entry.src_bpp
    colors = _read_palette_colors(_file(palette_path, "palette"))
    ints: List[int] = []
    for slot in range(count):
        old = entry.palette[slot] if slot < len(entry.palette) else 0
        if slot == 0:
            # Slot 0 is the transparent/key colour; leave it exactly as it was.
            ints.append(old & 0xFFFFFFFF)
            continue
        rgb = colors[slot] if slot < len(colors) else (0, 0, 0)
        vga = _nearest_vga_index(sha, rgb, allow_zero=False)
        ints.append(((vga << _VGA_SHIFT) | (old & 0xFFFF)) & 0xFFFFFFFF)
    blob = encode_entry(_entry_frames(entry), entry.src_bpp, entry.flags & ~1,
                        ints, _entry_trailing(entry))
    blobs = _all_entry_blobs(sha)
    blobs[entry_idx] = blob
    out = _out(out_path, sha_path, "output .sha")
    out.write_bytes(build_sha_bytes(blobs))
    return out


# ---------------------------------------------------------------------------
# Portrait (shape entry 24, one 64x70 image instead of 16 tiles)
# ---------------------------------------------------------------------------

def portrait_from_image(sha_path, image_path, out_path=None) -> Path:
    """Replace Jill's dialog portrait from a single 64x70 image.

    The picture is split into the sixteen tile frames of shape entry 24 using
    each frame's existing width/height, so the irregular bottom row keeps its
    original layout.  Neighbouring pixel colours are quantised to the entry
    palette.
    """
    sha = _load_sha(sha_path)
    entry = sha.entries[jx.PORTRAIT_ENTRY] if jx.PORTRAIT_ENTRY < 128 else None
    if entry is None or not entry.frames:
        raise ImportFailure(f"shape entry {jx.PORTRAIT_ENTRY} has no portrait")
    if len(entry.frames) < 16:
        raise ImportFailure(
            f"shape entry {jx.PORTRAIT_ENTRY} has only {len(entry.frames)} "
            "frames (expected 16 portrait tiles)")
    img = _load_image(image_path)
    if img.size != jx.PORTRAIT_SIZE:
        img = img.resize(jx.PORTRAIT_SIZE, Image.NEAREST)
    colors = resolved_palette(sha, jx.PORTRAIT_ENTRY)
    frames = _entry_frames(entry)
    for i in range(16):
        w, h = frames[i][0], frames[i][1]
        x, y = (i % 4) * 16, (i // 4) * 16
        tile = img.crop((x, y, x + w, y + h))
        tw, th, data = quantize(tile, colors)
        frames[i] = (tw, th, 0, data)
    blob = encode_entry(frames, entry.src_bpp, entry.flags,
                        entry.palette or None, _entry_trailing(entry))
    blobs = _all_entry_blobs(sha)
    blobs[jx.PORTRAIT_ENTRY] = blob
    out = _out(out_path, sha_path, "output .sha")
    out.write_bytes(build_sha_bytes(blobs))
    return out


def _load_board(value, what: str = "board file") -> "jx.BoardFile":
    p = _file(value, what)
    try:
        return jx.BoardFile.load(str(p))
    except Exception as e:
        raise ImportFailure(f"{what} {p.name}: {e}") from e


def _load_image(value, what: str = "image") -> Image.Image:
    p = _file(value, what)
    try:
        with Image.open(p) as im:
            img = im.convert('RGBA')
    except Exception as e:
        raise ImportFailure(f"{what} {p.name}: not a readable image ({e})") from e
    if img.width < 1 or img.height < 1:
        raise ImportFailure(f"{what} {p.name}: empty image")
    return img


def _read_json(value, what: str, optional: bool = False):
    p = Path(value)
    if optional and not p.is_file():
        return None
    p = _file(p, what)
    try:
        return json.loads(p.read_text(encoding='utf-8'))
    except Exception as e:
        raise ImportFailure(f"{what} {p.name}: invalid JSON ({e})") from e


# ---------------------------------------------------------------------------
# Palette helpers
# ---------------------------------------------------------------------------

def resolved_palette(sha: "jx.SHAFile", entry_idx: int) -> List[Tuple[int, int, int]]:
    """Return the resolved RGB colour for every source index of an entry."""
    base = sha.vga_palette
    entry = sha.entries[entry_idx] if 0 <= entry_idx < 128 else None
    n = 1 << (entry.src_bpp if entry and entry.src_bpp < 8 else 8)
    colors: List[Tuple[int, int, int]] = []
    if entry is None or (entry.flags & 1) or entry.src_bpp >= 8:
        # Identity: source index == VGA index.
        for i in range(n):
            vi = i & 0xFF
            colors.append((base[vi * 3], base[vi * 3 + 1], base[vi * 3 + 2]))
        return colors
    for i in range(n):
        if i < len(entry.palette):
            vi = (entry.palette[i] >> _VGA_SHIFT) & _VGA_MASK
        else:
            vi = i & 0xFF
        vi &= 0xFF
        colors.append((base[vi * 3], base[vi * 3 + 1], base[vi * 3 + 2]))
    return colors


def quantize(image: Image.Image, colors: Sequence[Tuple[int, int, int]],
             transparent_index: int = 0, alpha_cutoff: int = 128,
             dither: bool = False) -> Tuple[int, int, bytes]:
    """Map an RGBA image to palette indices (one byte per pixel)."""
    img = image.convert('RGBA')
    w, h = img.size
    if w < 1 or h < 1:
        raise ImportFailure("cannot quantise an empty image")
    if dither:
        palette = bytearray(768)
        for i, (r, g, b) in enumerate(colors[:256]):
            palette[i * 3:i * 3 + 3] = bytes(
                (max(0, min(255, r)), max(0, min(255, g)), max(0, min(255, b))))
        pal_img = Image.new('P', (1, 1))
        pal_img.putpalette(bytes(palette))
        indices = bytearray(img.convert('RGB').quantize(
            palette=pal_img, dither=Image.FLOYDSTEINBERG).tobytes())
        for i, a in enumerate(img.getchannel('A').tobytes()):
            if a < alpha_cutoff:
                indices[i] = transparent_index
        return w, h, bytes(indices)
    px = img.load()
    # Small cache RGB -> nearest index.
    cache: Dict[Tuple[int, int, int], int] = {}
    out = bytearray(w * h)
    for y in range(h):
        row = y * w
        for x in range(w):
            r, g, b, a = px[x, y]
            if a < alpha_cutoff:
                out[row + x] = transparent_index
                continue
            key = (r, g, b)
            idx = cache.get(key)
            if idx is None:
                best = 0
                bestd = 1 << 30
                for i, (cr, cg, cb) in enumerate(colors):
                    dr = r - cr
                    dg = g - cg
                    db = b - cb
                    d = dr * dr + dg * dg + db * db
                    if d < bestd:
                        bestd = d
                        best = i
                        if d == 0:
                            break
                idx = best
                cache[key] = idx
            out[row + x] = idx
    return w, h, bytes(out)


# ---------------------------------------------------------------------------
# SHA encoding
# ---------------------------------------------------------------------------

def _padded4(w: int) -> int:
    return ((w + 3) // 4) * 4


def _decoded_sizes(frames: Sequence[Tuple[int, int, int, bytes]]) -> Tuple[int, int, int]:
    """Recompute the three decoded-block size words documented for .sha."""
    s2 = s4 = s8 = 0
    for (w, h, _op, _data) in frames:
        s2 += 4 + ((w + 3) // 4) * h
        s4 += 4 + ((w + 1) // 2) * h
        s8 += 4 + _padded4(w) * h
    return s2 & 0xFFFF, s4 & 0xFFFF, s8 & 0xFFFF


def adapt_entry_palette(sha: "jx.SHAFile", entry_idx: int,
                        image: Image.Image,
                        count: Optional[int] = None) -> Tuple[List[Tuple[int, int, int]], List[int]]:
    """Build a new per-entry palette from an image (for 4/8-entry recolouring).

    Returns (RGB colours to quantise against, 32-bit palette words to store).
    Each chosen colour is snapped to the nearest entry in the global VGA
    palette, since a .sha entry stores palette indices, not RGB triplets.
    """
    entry = sha.entries[entry_idx] if 0 <= entry_idx < 128 else None
    if entry is None:
        raise ImportFailure(f"shape entry {entry_idx} is empty")
    if count is None:
        count = 1 << entry.src_bpp if 0 < entry.src_bpp < 8 else 0
    if not 2 <= count <= 256:
        raise ImportFailure(f"cannot adapt the palette of a {entry.src_bpp}bpp entry")
    # Index 0 is the transparency slot for shapes, so leave it out of the
    # colours sampled from the image (otherwise a solid-colour import would
    # map every opaque pixel to 0 and render invisible).
    slots = count - 1
    try:
        quantized = image.convert('RGB').quantize(colors=slots,
                                                  method=Image.MEDIANCUT)
        palette = list(quantized.getpalette() or [])
        # Pillow returns only the colours actually used (a flat image may
        # report just one) — pad with black so every sampled slot is valid.
        palette += [0] * max(0, slots * 3 - len(palette))
        targets = [tuple(palette[i * 3:i * 3 + 3]) for i in range(slots)]
    except Exception as e:
        raise ImportFailure(f"cannot build a palette from the image ({e})") from e

    base = sha.vga_palette
    # Keep the original low 16 bits of every palette word.  The port only uses
    # bits 16..23 (VGA mode 4), but the original engine's EGA/CGA paths read
    # the lower bytes at shifts 0/4/8/12; zeroing them changes those modes.
    def word(slot: int, vga_index: int) -> int:
        old = entry.palette[slot] if slot < len(entry.palette) else 0
        return ((vga_index << _VGA_SHIFT) | (old & 0xFFFF)) & 0xFFFFFFFF

    reserved = ((entry.palette[0] >> _VGA_SHIFT) & 0xFF) if entry.palette else 0
    colors: List[Tuple[int, int, int]] = [
        (base[reserved * 3], base[reserved * 3 + 1], base[reserved * 3 + 2])]
    ints: List[int] = [word(0, reserved)]
    for slot, (r, g, b) in enumerate(targets, start=1):
        best, bestd = 1, 1 << 30
        for i in range(1, 256):
            dr, dg, db = r - base[i * 3], g - base[i * 3 + 1], b - base[i * 3 + 2]
            d = dr * dr + dg * dg + db * db
            if d < bestd:
                best, bestd = i, d
        colors.append((base[best * 3], base[best * 3 + 1], base[best * 3 + 2]))
        ints.append(word(slot, best))
    return colors, ints


def encode_entry(frames: Sequence[Tuple[int, int, int, bytes]],
                 src_bpp: int = 8, flags: int = 1,
                 palette_ints: Optional[Sequence[int]] = None,
                 trailing: Optional[Tuple[int, int, int, bytes]] = None) -> bytes:
    """Serialise one .sha entry.

    frames/trailing are (width, height, opcode, payload) tuples. New frames
    should use opcode 0 with one payload byte per pixel.
    """
    n = len(frames)
    if n > 255:
        raise ImportFailure(f"too many frames in one shape entry ({n} > 255)")
    s2, s4, s8 = _decoded_sizes(frames)
    out = bytearray()
    out.append(n & 0xFF)
    out += struct.pack('<H', 1)
    out += struct.pack('<HHH', s2, s4, s8)
    out.append(src_bpp & 0xFF)
    out += struct.pack('<H', flags & 0xFFFF)

    if not (flags & 1) and src_bpp < 8:
        count = 1 << src_bpp
        if palette_ints and len(palette_ints) >= count:
            for i in range(count):
                out += struct.pack('<I', palette_ints[i] & 0xFFFFFFFF)
        else:
            # Identity table: source index i -> VGA index i.
            for i in range(count):
                out += struct.pack('<I', (i << _VGA_SHIFT) & 0xFFFFFFFF)

    def emit(w: int, h: int, op: int, data: bytes):
        if not (0 <= w <= 255 and 0 <= h <= 255 and 0 <= op <= 255):
            raise ImportFailure(f"frame dimensions/opcode out of range: "
                                f"{w}x{h} op {op}")
        out.append(w)
        out.append(h)
        out.append(op)
        if op == 0:
            need = w * h
            if len(data) != need:
                raise ImportFailure(f"raw {w}x{h} frame needs {need} pixels, "
                                    f"got {len(data)}")
            out.extend(data)
        elif op == 1:
            out.extend(data)
        # opcode >= 2 carries no payload

    for (w, h, op, data) in frames:
        emit(w, h, op, data)
    if trailing is not None:
        emit(*trailing)
    return bytes(out)


def build_sha_bytes(entries: Sequence[Optional[bytes]]) -> bytes:
    """Assemble a .sha file from up to 128 raw entry blobs."""
    if len(entries) > 128:
        raise ImportFailure(f"too many shape entries ({len(entries)} > 128)")
    entries = list(entries) + [None] * (128 - len(entries))
    offsets: List[int] = []
    sizes: List[int] = []
    body = bytearray()
    cur = 0x300
    for blob in entries:
        if blob:
            offsets.append(cur)
            sizes.append(len(blob))
            body += blob
            cur += len(blob)
        else:
            offsets.append(0)
            sizes.append(0)
    if any(size > 0xFFFF for size in sizes):
        raise ImportFailure("a shape entry is too large (>65535 bytes) for .sha")
    header = struct.pack('<128I', *offsets) + struct.pack('<128H', *sizes)
    return header + bytes(body)


def _entry_frames(entry: "jx.ShapeEntry") -> List[Tuple[int, int, int, bytes]]:
    frames = []
    for f in entry.frames:
        frames.append((f['width'], f['height'], f['opcode'], f['data']))
    return frames


def _entry_trailing(entry: "jx.ShapeEntry"):
    tf = entry.trailing_frame
    if not tf:
        return None
    return (tf['width'], tf['height'], tf['opcode'], tf.get('data', b''))


def _all_entry_blobs(sha: "jx.SHAFile") -> List[Optional[bytes]]:
    blobs: List[Optional[bytes]] = []
    for e in sha.entries:
        if e is None:
            blobs.append(None)
        else:
            blobs.append(bytes(e.raw_data))
    return blobs


def replace_sha_frame(sha_path: Path, entry_idx: int, frame_idx: int,
                      image_path: Path, out_path: Optional[Path] = None,
                      resize: bool = True, adapt_palette: bool = False,
                      dither: bool = False) -> Path:
    """Replace one frame of an existing .sha entry with a PNG."""
    sha = _load_sha(sha_path)
    _index(entry_idx, 128, "shape entry")
    entry = sha.entries[entry_idx]
    if entry is None:
        raise ImportFailure(f"shape entry {entry_idx} is empty")

    img = _load_image(image_path)

    frames = _entry_frames(entry)
    if not isinstance(frame_idx, int) or not 0 <= frame_idx < len(frames):
        raise ImportFailure(f"frame {frame_idx} out of range (0..{len(frames)-1})")

    old_w, old_h = frames[frame_idx][0], frames[frame_idx][1]
    if resize and img.size != (old_w, old_h):
        img = img.resize((old_w, old_h), Image.NEAREST)
    flags, palette = entry.flags, entry.palette or None
    if adapt_palette and entry.src_bpp < 8:
        # Recolouring a shared per-entry palette affects every frame of it.
        colors, palette = adapt_entry_palette(sha, entry_idx, img)
        flags &= ~1
    else:
        colors = resolved_palette(sha, entry_idx)
    w, h, data = quantize(img, colors, dither=dither)
    frames[frame_idx] = (w, h, 0, data)

    new_blob = encode_entry(frames, entry.src_bpp, flags, palette,
                            _entry_trailing(entry))
    blobs = _all_entry_blobs(sha)
    blobs[entry_idx] = new_blob

    out = _out(out_path, sha_path, "output .sha")
    out.write_bytes(build_sha_bytes(blobs))
    return out


def sha_from_folder(sha_path: Path, folder: Path,
                    out_path: Optional[Path] = None) -> Path:
    """Build a .sha archive from extracted frames.

    Two layouts are accepted:

      * ``shape_NNN/frame_NNN.png`` (plus optional ``trailing.png``), and
      * the extractor's flat ``sprites/shape_NNN_FFF.png`` names.

    Entries that exist keep their source bpp, flags and palette; new entries
    are written as 8bpp identity-mapped sprites.  Entries not present in the
    folder are copied from the reference .sha when one is supplied.
    """
    sha_path = Path(sha_path)
    # A missing reference path is allowed (build a fresh archive); an existing
    # path must be a readable .sha, never silently treated as "no reference".
    sha = _load_sha(sha_path) if sha_path.exists() else None
    blobs: List[Optional[bytes]] = _all_entry_blobs(sha) if sha else [None] * 128
    if sha:
        base_palette = sha.vga_palette
        vga = [(base_palette[i], base_palette[i + 1], base_palette[i + 2])
               for i in range(0, 768, 3)]
    else:
        vga = [(i, i, i) for i in range(256)]

    folder = _dir(folder, "source folder")

    def write_entry(entry_idx: int, images: Sequence[Image.Image],
                    trailing: Optional[Image.Image] = None) -> None:
        entry = sha.entries[entry_idx] if sha and sha.entries[entry_idx] else None
        if entry is not None:
            bpp = entry.src_bpp
            flags = entry.flags
            palette = entry.palette or None
            colors = resolved_palette(sha, entry_idx)
        else:
            bpp, flags, palette, colors = 8, 1, None, vga
        frames: List[Tuple[int, int, int, bytes]] = []
        for im in images:
            fw, fh, data = quantize(im, colors)
            frames.append((fw, fh, 0, data))
        trail = None
        if trailing is not None:
            tw, th, tdata = quantize(trailing, colors)
            trail = (tw, th, 0, tdata)
        blobs[entry_idx] = encode_entry(frames, bpp, flags, palette, trail)

    for entry_dir in sorted(folder.glob('shape_*')):
        if not entry_dir.is_dir():
            continue
        try:
            entry_idx = int(entry_dir.name.split('_', 1)[1])
        except (IndexError, ValueError):
            continue
        if not (0 <= entry_idx < 128):
            continue
        pngs = sorted(entry_dir.glob('frame_*.png'))
        if not pngs:
            continue
        images = [_load_image(p) for p in pngs]
        w, h = images[0].size
        images = [im if im.size == (w, h) else im.resize((w, h), Image.NEAREST)
                  for im in images]
        trailing = None
        tpath = entry_dir / 'trailing.png'
        if tpath.is_file():
            trailing = _load_image(tpath)
        write_entry(entry_idx, images, trailing)

    # Flat extractor layout: sprites/shape_NNN_FFF.png grouped by entry.
    flat = folder / 'sprites'
    if flat.is_dir():
        groups: Dict[int, Dict[int, Path]] = {}
        for png in flat.glob('shape_*_*.png'):
            parts = png.stem.split('_')
            if len(parts) != 3:
                continue
            try:
                entry_idx = int(parts[1])
                frame_idx = int(parts[2])
            except ValueError:
                continue
            if 0 <= entry_idx < 128:
                groups.setdefault(entry_idx, {})[frame_idx] = png
        for entry_idx, frame_map in groups.items():
            # The flat sprite dump contains the font as white masks; use the
            # dedicated sha-font command so its foreground index is preserved.
            if entry_idx == jx.FONT_ENTRY:
                continue
            images = [_load_image(frame_map[k]) for k in sorted(frame_map)]
            write_entry(entry_idx, images)

    out = _out(out_path, sha_path, "output .sha")
    out.write_bytes(build_sha_bytes(blobs))
    return out


_IMAGE_EXTS = {'.png', '.bmp', '.gif', '.jpg', '.jpeg', '.tga'}


def _natural_key(path: Path):
    """Sort frame_2.png before frame_10.png."""
    return [int(part) if part.isdigit() else part.lower()
            for part in re.split(r'(\d+)', path.name)]


def _vga_colors(sha: Optional["jx.SHAFile"]) -> List[Tuple[int, int, int]]:
    if sha is None:
        return [(i, i, i) for i in range(256)]
    base = sha.vga_palette
    return [(base[i], base[i + 1], base[i + 2]) for i in range(0, 768, 3)]


def sha_entry_from_folder(sha_path, entry_idx: int, folder,
                          out_path=None, resize: bool = True,
                          adapt_palette: bool = False) -> Path:
    """Replace one shape entry's whole animation with images from a folder.

    Images (PNG/BMP/GIF/JPEG/TGA) are sorted naturally by file name, all
    scaled to the first image's size and quantised to the entry's palette.
    The entry's source bpp, flags, palette and trailing frame are preserved;
    other entries are copied from the reference archive.  A missing reference
    path builds a fresh archive, as with sha_from_folder().
    """
    sha_path = Path(sha_path)
    sha = _load_sha(sha_path) if sha_path.exists() else None
    _index(entry_idx, 128, "shape entry")
    folder = _dir(folder, "frame folder")
    frames = sorted((p for p in folder.iterdir()
                     if p.is_file() and p.suffix.lower() in _IMAGE_EXTS),
                    key=_natural_key)
    if not frames:
        raise ImportFailure(f"no images found in {folder}")
    images = [_load_image(p, "frame image") for p in frames]
    w, h = images[0].size
    if resize:
        images = [im if im.size == (w, h) else im.resize((w, h), Image.NEAREST)
                  for im in images]

    if sha and sha.entries[entry_idx] is not None:
        entry = sha.entries[entry_idx]
        bpp, flags, palette = entry.src_bpp, entry.flags, entry.palette or None
        colors = resolved_palette(sha, entry_idx)
        trailing = _entry_trailing(entry)
        blobs = _all_entry_blobs(sha)
    else:
        bpp, flags, palette = 8, 1, None
        colors = _vga_colors(sha)
        trailing = None
        blobs = [None] * 128
    if adapt_palette and bpp < 8 and sha is not None:
        colors, palette = adapt_entry_palette(sha, entry_idx, images[0])
        flags &= ~1

    encoded: List[Tuple[int, int, int, bytes]] = []
    for im in images:
        fw, fh, data = quantize(im, colors)
        encoded.append((fw, fh, 0, data))
    blobs[entry_idx] = encode_entry(encoded, bpp, flags, palette, trailing)

    out = _out(out_path, sha_path, "output .sha")
    out.write_bytes(build_sha_bytes(blobs))
    return out


# ---------------------------------------------------------------------------
# Extracted asset importers (fonts, tiles, boards, packs)
# ---------------------------------------------------------------------------

def font_from_sheet(sha_path: Path, sheet_path: Path,
                    out_path: Optional[Path] = None,
                    entry_idx: int = 2, foreground: int = 1) -> Path:
    """Replace a SHA font entry from `fonts/main.png` (128-cell strip).

    Opaque pixels become `foreground`; transparent pixels become index 0.
    Reads `font.json` beside the sheet for the cell/glyph sizes when present.
    """
    sha = _load_sha(sha_path)
    _index(entry_idx, 128, "shape entry")
    _index(foreground, 256, "foreground colour")
    entry = sha.entries[entry_idx]
    if entry is None:
        raise ImportFailure(f"shape entry {entry_idx} is empty")

    sheet = _load_image(sheet_path, "font sheet")
    meta = _read_json(Path(sheet_path).parent / 'font.json',
                      "font metadata", optional=True) or {}
    if not isinstance(meta, dict):
        raise ImportFailure("font.json: expected an object")
    try:
        cell_w = int(meta.get('cell_width') or max(1, sheet.width // 128))
        cell_h = int(meta.get('cell_height') or sheet.height)
        glyphs = {int(g['code']): g for g in meta.get('glyphs', [])
                  if 'code' in g}
    except Exception as e:
        raise ImportFailure(f"font.json: invalid metadata ({e})") from e
    if cell_w < 1 or cell_h < 1 or cell_w * 128 > sheet.width or cell_h > sheet.height:
        raise ImportFailure(
            f"font sheet {sheet.width}x{sheet.height} is too small for "
            f"128 cells of {cell_w}x{cell_h}")
    frames: List[Tuple[int, int, int, bytes]] = []
    for code in range(128):
        g = glyphs.get(code, {})
        try:
            gw = int(g.get('width') or cell_w)
            gh = int(g.get('height') or cell_h)
        except Exception as e:
            raise ImportFailure(f"font.json glyph {code}: invalid size ({e})") from e
        if not (1 <= gw <= cell_w and 1 <= gh <= cell_h):
            raise ImportFailure(
                f"font.json glyph {code}: size {gw}x{gh} outside "
                f"cell {cell_w}x{cell_h}")
        cell = sheet.crop((code * cell_w, 0, code * cell_w + gw, gh))
        px = cell.load()
        data = bytearray(gw * gh)
        for y in range(gh):
            for x in range(gw):
                _r, _g, _b, a = px[x, y]
                data[y * gw + x] = foreground if a >= 128 else 0
        frames.append((gw, gh, 0, bytes(data)))

    blob = encode_entry(frames, entry.src_bpp, entry.flags,
                        entry.palette or None, _entry_trailing(entry))
    blobs = _all_entry_blobs(sha)
    blobs[entry_idx] = blob
    out = _out(out_path, sha_path, "output .sha")
    out.write_bytes(build_sha_bytes(blobs))
    return out


def tile_from_png(sha_path: Path, dma_path: Path, tile_id: int,
                  image_path: Path, out_path: Optional[Path] = None,
                  resize: bool = True, dither: bool = False) -> Tuple[Path, int, int]:
    """Replace the SHA frame used by DMA `tile_id` with a PNG."""
    sha = _load_sha(sha_path)
    dma = _load_dma(dma_path)
    _index(tile_id, 0x10000, "tile id")
    rec = next((r for r in dma.records if r.tile_id == tile_id), None)
    if rec is None:
        raise ImportFailure(f"tile id 0x{tile_id:02X} not in "
                            f"{Path(dma_path).name}")
    entry_idx = (rec.shape_id >> 8) & 0x3F
    frame_idx = rec.shape_id & 0xFF
    entry = sha.entries[entry_idx]
    if entry is None or frame_idx >= len(entry.frames):
        raise ImportFailure(f"tile 0x{tile_id:02X} points at missing shape "
                            f"{entry_idx}:{frame_idx}")
    img = _load_image(image_path)
    old = entry.frames[frame_idx]
    if resize and img.size != (old['width'], old['height']):
        img = img.resize((old['width'], old['height']), Image.NEAREST)
    colors = resolved_palette(sha, entry_idx)
    w, h, data = quantize(img, colors, dither=dither)
    frames = _entry_frames(entry)
    frames[frame_idx] = (w, h, 0, data)
    blob = encode_entry(frames, entry.src_bpp, entry.flags,
                        entry.palette or None, _entry_trailing(entry))
    blobs = _all_entry_blobs(sha)
    blobs[entry_idx] = blob
    out = _out(out_path, sha_path, "output .sha")
    out.write_bytes(build_sha_bytes(blobs))
    return out, entry_idx, frame_idx


def board_from_image(board_path: Path, sha_path: Path, dma_path: Path,
                     image_path: Path,
                     out_path: Optional[Path] = None) -> Path:
    """Rebuild a board's tile map from a full 2048x1024 board render."""
    board = _load_board(board_path)
    sha = _load_sha(sha_path)
    dma = _load_dma(dma_path)
    img = _load_image(image_path, "board render").convert('RGB')
    if img.size != (2048, 1024):
        raise ImportFailure("board image must be the full 2048x1024 render "
                            "(use board_previews/<name>.png, not boards/)")

    cache: Dict[Tuple[int, int], Optional[Image.Image]] = {}
    exact: Dict[bytes, int] = {}
    sigs: List[Tuple[bytes, int]] = []
    for rec in dma.records:
        try:
            tile = jx._tile_rgba(sha, rec, cache)
        except Exception as e:
            raise ImportFailure(f"cannot decode tile 0x{rec.tile_id:02X}: {e}") from e
        if tile is None:
            continue
        rgb = tile.convert('RGB')
        exact.setdefault(rgb.tobytes(), rec.tile_id)
        sigs.append((rgb.resize((4, 4), Image.NEAREST).tobytes(),
                     rec.tile_id))

    for c in range(128):
        for r in range(64):
            block = img.crop((c * 16, r * 16, c * 16 + 16, r * 16 + 16))
            tid = exact.get(block.tobytes())
            if tid is None:
                # A uniform block is background showing through, not a tile.
                block_colors = block.getcolors(2)
                if block_colors is not None and len(block_colors) <= 1:
                    tid = board.get_tile_at(c, r)
                else:
                    sig = block.resize((4, 4), Image.NEAREST).tobytes()
                    best = 1 << 30
                    for tsig, ttid in sigs:
                        d = sum((a - b) * (a - b) for a, b in zip(sig, tsig))
                        if d < best:
                            best = d
                            tid = ttid
                    # A poor match is more likely background/alpha than a
                    # genuinely new tile; keep the old cell in that case.
                    if tid is None or best > 60000:
                        tid = board.get_tile_at(c, r)
            if tid is None:
                tid = board.get_tile_at(c, r)
            board.tile_map[c * 64 + r] = tid & 0x3FFF

    return save_board(board, out_path or board_path)


def save_board(board: "jx.BoardFile", path, out_path=None) -> Path:
    """Write an in-memory board (e.g. one edited by a GUI) back to .jn1."""
    out = _out(out_path, path, "output board")
    try:
        board.save(str(out))
    except Exception as e:
        raise ImportFailure(f"cannot write board {out}: {e}") from e
    return out


def install_pack(src_dir: Path, pack_dir: Path,
                 game: Optional[str] = None,
                 name: Optional[str] = None) -> Dict[str, int]:
    """Copy an extractor/reference-pack tree into an external pack folder."""
    src = _dir(src_dir, "pack source")
    dst = Path(pack_dir)
    if dst.resolve() == src.resolve():
        raise ImportFailure("pack destination must differ from the source")
    try:
        dst.mkdir(parents=True, exist_ok=True)
    except OSError as e:
        raise ImportFailure(f"cannot create pack folder {dst}: {e}") from e
    stats: Dict[str, int] = {}
    dirs = ['sprites', 'fonts', 'tiles', 'boards', 'themes', 'hud',
            'pages', 'sfx', 'music']
    for sub in dirs:
        s = src / sub
        if not s.is_dir():
            continue
        try:
            shutil.copytree(s, dst / sub, dirs_exist_ok=True)
        except OSError as e:
            raise ImportFailure(f"cannot copy {s.name} into pack: {e}") from e
        stats[sub] = sum(1 for p in (dst / sub).rglob('*') if p.is_file())
    for fname in ['portrait.png', 'palette.pal', 'palette_6bit.bin',
                  'palette.json']:
        s = src / fname
        if s.is_file():
            try:
                shutil.copy2(s, dst / fname)
            except OSError as e:
                raise ImportFailure(f"cannot copy {s.name} into pack: {e}") from e
            stats[fname] = 1

    warnings: List[str] = []
    def check(path: Path, size: Tuple[int, int]) -> None:
        if path.is_file():
            try:
                with Image.open(path) as im:
                    if im.size != size:
                        warnings.append(f"{path.name}: {im.size} (expected {size})")
            except Exception:
                warnings.append(f"{path.name}: not a readable image")

    check(dst / 'portrait.png', (64, 84))
    check(dst / 'hud' / 'classic.png', (64, 84))
    check(dst / 'hud' / 'modern.png', (320, 12))
    for p in (dst / 'boards').glob('*.png'):
        check(p, (320, 200))
    for p in (dst / 'pages').glob('*.png'):
        check(p, (320, 200))
    for p in (dst / 'tiles').glob('*.png'):
        check(p, (16, 16))

    ini = dst / 'pack.ini'
    if not ini.exists():
        def ini_value(text: str) -> str:
            return ' '.join(str(text).splitlines())

        label = ini_value(name or dst.name)
        lines = ['[pack]', f'name = {label}']
        if game:
            lines.append(f'game = {ini_value(game)}')
        try:
            ini.write_text('\n'.join(lines) + '\n', encoding='utf-8')
        except OSError as e:
            raise ImportFailure(f"cannot write {ini}: {e}") from e
        stats['pack.ini'] = 1
    for w in warnings:
        print(f"warning: {w}")
    return stats


# ---------------------------------------------------------------------------
# VCL / WAV
# ---------------------------------------------------------------------------

_VCL_HEADER = 0xC8 + 0x64 + 0x64 + 0xA0 + 0x50  # 0x280


def decode_wav(data: bytes) -> Tuple[List[float], int, int]:
    """Return (samples in -1..1, sample_rate, channels) for a WAV blob."""
    try:
        with wave.open(io.BytesIO(data), 'rb') as w:
            nch = w.getnchannels()
            sw = w.getsampwidth()
            rate = w.getframerate()
            raw = w.readframes(w.getnframes())
    except Exception as e:
        raise ImportFailure(f"invalid WAV data ({e})") from e
    if nch < 1 or rate < 1:
        raise ImportFailure(f"invalid WAV header: {nch} channels, {rate} Hz")
    samples: List[float] = []
    if sw == 1:
        for b in raw:
            samples.append((b - 128) / 128.0)
    elif sw == 2:
        n = len(raw) // 2
        ints = struct.unpack('<%dh' % n, raw)
        samples = [v / 32768.0 for v in ints]
    elif sw == 3:
        n = len(raw) // 3
        for i in range(n):
            b0, b1, b2 = raw[i * 3], raw[i * 3 + 1], raw[i * 3 + 2]
            v = b0 | (b1 << 8) | (b2 << 16)
            if v & 0x800000:
                v -= 0x1000000
            samples.append(v / 8388608.0)
    elif sw == 4:
        n = len(raw) // 4
        ints = struct.unpack('<%di' % n, raw)
        samples = [v / 2147483648.0 for v in ints]
    else:
        raise ImportFailure(f"unsupported WAV sample width: {sw}")
    if nch > 1:
        mono = []
        for i in range(0, len(samples), nch):
            chunk = samples[i:i + nch]
            mono.append(sum(chunk) / len(chunk))
        samples = mono
    return samples, rate, 1


def resample(samples: List[float], src_rate: int, dst_rate: int) -> List[float]:
    if not samples or src_rate == dst_rate:
        return samples
    if src_rate <= 0 or dst_rate <= 0:
        raise ImportFailure(f"invalid sample rate: {src_rate} -> {dst_rate} Hz")
    ratio = dst_rate / src_rate
    out_len = max(1, int(len(samples) * ratio))
    out: List[float] = []
    last = len(samples) - 1
    for i in range(out_len):
        pos = i / ratio
        i0 = int(pos)
        frac = pos - i0
        i1 = min(last, i0 + 1)
        out.append(samples[i0] * (1.0 - frac) + samples[i1] * frac)
    return out


def to_unsigned8(samples: List[float]) -> bytes:
    out = bytearray(len(samples))
    for i, s in enumerate(samples):
        v = int(round(s * 127.5 + 127.5))
        out[i] = 0 if v < 0 else 255 if v > 255 else v
    return bytes(out)


def wav_to_sample(data: bytes, target_rate: int = 6000) -> Tuple[bytes, int]:
    """Convert a WAV blob to a Jill .vcl sample (unsigned 8-bit mono)."""
    samples, rate, _ = decode_wav(data)
    if target_rate and rate != target_rate:
        samples = resample(samples, rate, target_rate)
        rate = target_rate
    return to_unsigned8(samples), rate


def build_vcl_bytes(sfx: Sequence[Optional[Tuple[bytes, int]]],
                    music: Sequence[Optional[bytes]]) -> bytes:
    if len(sfx) > 50 or len(music) > 40:
        raise ImportFailure("too many VCL sfx/music entries")
    if (any(len(item[0]) > 0xFFFF or item[1] > 0xFFFF for item in sfx if item)
            or any(len(data) > 0xFFFF for data in music if data)):
        raise ImportFailure("a VCL entry is too large (>65535 bytes) for the bank")
    sfx = list(sfx) + [None] * (50 - len(sfx))
    music = list(music) + [None] * (40 - len(music))
    offs: List[int] = []
    sizes: List[int] = []
    rates: List[int] = []
    moffs: List[int] = []
    msizes: List[int] = []
    body = bytearray()
    cur = _VCL_HEADER
    for item in sfx:
        if not item:
            offs.append(0); sizes.append(0); rates.append(0)
            continue
        data, rate = item
        offs.append(cur); sizes.append(len(data)); rates.append(rate)
        body += data
        cur += len(data)
    for data in music:
        if not data:
            moffs.append(0); msizes.append(0)
            continue
        moffs.append(cur); msizes.append(len(data))
        body += data
        cur += len(data)
    out = bytearray()
    out += struct.pack('<50I', *offs)
    out += struct.pack('<50H', *sizes)
    out += struct.pack('<50H', *rates)
    out += struct.pack('<40I', *moffs)
    out += struct.pack('<40H', *msizes)
    out += body
    return bytes(out)


def _vcl_parts(vcl: "jx.VCLFile"):
    sfx: List[Optional[Tuple[bytes, int]]] = []
    for i in range(50):
        data = vcl.extract_sfx(i)
        sfx.append((data, vcl.sfx_rates[i] or 6000) if data else None)
    music: List[Optional[bytes]] = []
    for i in range(40):
        data = vcl.extract_music(i)
        music.append(data if data else None)
    return sfx, music


def replace_vcl_sfx(vcl_path: Path, slot: int, wav_path: Path,
                    out_path: Optional[Path] = None) -> Path:
    """Replace one sampled-sound slot with audio from a WAV file."""
    _index(slot, 50, "SFX slot")
    vcl = _load_vcl(vcl_path)
    wav_file = _file(wav_path, "WAV file")
    sfx, music = _vcl_parts(vcl)
    data, rate = wav_to_sample(wav_file.read_bytes(),
                               target_rate=vcl.sfx_rates[slot] or 6000)
    sfx[slot] = (data, rate)
    out = _out(out_path, vcl_path, "output .vcl")
    out.write_bytes(build_vcl_bytes(sfx, music))
    return out


def vcl_from_folder(vcl_path: Path, folder: Path,
                    out_path: Optional[Path] = None) -> Path:
    """Build a .vcl from <folder>/sfx_NN.wav (+ optional music_NN.raw)."""
    vcl_path = Path(vcl_path)
    # As with sha-folder: a missing reference starts a fresh bank; an existing
    # path must be a readable .vcl.
    vcl = _load_vcl(vcl_path) if vcl_path.exists() else None
    if vcl:
        sfx, music = _vcl_parts(vcl)
    else:
        sfx, music = [None] * 50, [None] * 40

    folder = _dir(folder, "source folder")
    for wav_file in sorted(folder.glob('sfx_*.wav')):
        try:
            slot = int(wav_file.stem.split('_', 1)[1])
        except (IndexError, ValueError):
            continue
        if not (0 <= slot < 50):
            continue
        rate = (vcl.sfx_rates[slot] if vcl else 0) or 6000
        source = _file(wav_file, "WAV file")
        data, rate = wav_to_sample(source.read_bytes(), target_rate=rate)
        sfx[slot] = (data, rate)
    for raw_file in sorted(folder.glob('music_*.raw')):
        try:
            slot = int(raw_file.stem.split('_', 1)[1])
        except (IndexError, ValueError):
            continue
        if 0 <= slot < 40:
            music[slot] = _file(raw_file, "raw audio file").read_bytes()

    out = _out(out_path, vcl_path, "output .vcl")
    out.write_bytes(build_vcl_bytes(sfx, music))
    return out


def vcl_pages_from_folder(vcl_path: Path, folder: Path,
                          out_path: Optional[Path] = None) -> Path:
    """Replace the VCL text-page bank from `text_pages/`.

    Uses `pages.json` when present so the per-line colours are preserved;
    otherwise reads `page_NN.txt` with the default colour.
    """
    vcl = _load_vcl(vcl_path)
    sfx, pages = _vcl_parts(vcl)
    folder = _dir(folder, "source folder")
    meta_path = folder / 'pages.json'
    if meta_path.is_file():
        entries = _read_json(meta_path, "page metadata")
        if not isinstance(entries, list):
            raise ImportFailure("pages.json: expected a list of pages")
        for entry in entries:
            if not isinstance(entry, dict):
                raise ImportFailure("pages.json: each page must be an object")
            try:
                idx = int(entry['index'])
            except (KeyError, TypeError, ValueError):
                continue
            if not (0 <= idx < 40):
                continue
            lines = entry.get('lines', [])
            if not isinstance(lines, list):
                raise ImportFailure(f"pages.json page {idx}: lines must be a list")
            data = bytearray()
            for line in lines:
                if not isinstance(line, dict):
                    raise ImportFailure(f"pages.json page {idx}: line must be an object")
                try:
                    colour = int(line.get('colour', 7))
                except (TypeError, ValueError) as e:
                    raise ImportFailure(
                        f"pages.json page {idx}: invalid colour ({e})") from e
                if not 0 <= colour <= 7:
                    raise ImportFailure(
                        f"pages.json page {idx}: colour {colour} outside 0..7")
                text = str(line.get('text', ''))
                if colour != 7:
                    data.extend(str(colour).encode('ascii', 'replace'))
                data.extend(text.encode('ascii', 'replace'))
                data.append(0x0D)
            pages[idx] = bytes(data)
    else:
        for page_file in sorted(folder.glob('page_*.txt')):
            try:
                idx = int(page_file.stem.split('_', 1)[1])
            except (IndexError, ValueError):
                continue
            if not (0 <= idx < 40):
                continue
            source = _file(page_file, "page file")
            try:
                lines = source.read_text(encoding='ascii',
                                         errors='replace').splitlines()
            except Exception as e:
                raise ImportFailure(f"cannot read {source.name}: {e}") from e
            pages[idx] = b'\r'.join(line.encode('ascii', 'replace')
                                    for line in lines) + b'\r'
    out = _out(out_path, vcl_path, "output .vcl")
    out.write_bytes(build_vcl_bytes(sfx, pages))
    return out


def vcl_set_pages(vcl_path, pages, out_path=None) -> Path:
    """Replace text pages from structured data: {index: [{'colour', 'text'}]}.

    This is the in-memory counterpart of vcl_pages_from_folder(), used by the
    GUI editor.  Lines are written as colour code + text + 0x0D, like the
    extractor's pages.json export.
    """
    vcl = _load_vcl(vcl_path)
    sfx, bank = _vcl_parts(vcl)
    if not isinstance(pages, dict):
        raise ImportFailure("pages must be a {index: lines} mapping")
    for idx, lines in pages.items():
        _index(idx, 40, "text page")
        if not isinstance(lines, (list, tuple)):
            raise ImportFailure(f"text page {idx}: lines must be a list")
        data = bytearray()
        for line in lines:
            if not isinstance(line, dict):
                raise ImportFailure(f"text page {idx}: line must be an object")
            try:
                colour = int(line.get('colour', 7))
            except (TypeError, ValueError) as e:
                raise ImportFailure(
                    f"text page {idx}: invalid colour ({e})") from e
            if not 0 <= colour <= 7:
                raise ImportFailure(f"text page {idx}: colour {colour} outside 0..7")
            text = str(line.get('text', ''))
            if colour != 7:
                data.extend(str(colour).encode('ascii', 'replace'))
            data.extend(text.encode('ascii', 'replace'))
            data.append(0x0D)
        bank[idx] = bytes(data)
    out = _out(out_path, vcl_path, "output .vcl")
    out.write_bytes(build_vcl_bytes(sfx, bank))
    return out


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def _cli(argv: Sequence[str]) -> int:
    import argparse
    p = argparse.ArgumentParser(description="Picture/audio -> Jill converter")
    sub = p.add_subparsers(dest='cmd', required=True)

    a = sub.add_parser('sha-frame', help='replace a frame in a .sha entry')
    a.add_argument('sha'); a.add_argument('entry', type=int); a.add_argument('frame', type=int)
    a.add_argument('image'); a.add_argument('-o', '--out')
    a.add_argument('--adapt-palette', action='store_true',
                   help='recolour the entry palette from the image (bpp < 8)')
    a.add_argument('--dither', action='store_true',
                   help='dither to the existing palette instead of nearest colour')

    b = sub.add_parser('sha-folder', help='build a .sha from a PNG folder tree')
    b.add_argument('sha'); b.add_argument('folder'); b.add_argument('-o', '--out')

    c = sub.add_parser('vcl-sfx', help='replace a .vcl sampled-sound slot')
    c.add_argument('vcl'); c.add_argument('slot', type=int); c.add_argument('wav')
    c.add_argument('-o', '--out')

    d = sub.add_parser('vcl-folder', help='build a .vcl from a WAV folder')
    d.add_argument('vcl'); d.add_argument('folder'); d.add_argument('-o', '--out')

    e = sub.add_parser('sha-font', help='replace a .sha font entry from fonts/main.png')
    e.add_argument('sha'); e.add_argument('sheet'); e.add_argument('-o', '--out')
    e.add_argument('--entry', type=int, default=2)
    e.add_argument('--fg', type=int, default=1,
                   help='palette index for opaque pixels (default 1)')

    f = sub.add_parser('sha-tile', help='replace the SHA frame used by a DMA tile')
    f.add_argument('sha'); f.add_argument('dma'); f.add_argument('tile', type=lambda s: int(s, 0))
    f.add_argument('image'); f.add_argument('-o', '--out')
    f.add_argument('--dither', action='store_true',
                   help='dither to the existing palette instead of nearest colour')

    g = sub.add_parser('board-image', help='rebuild a board from a 2048x1024 render')
    g.add_argument('board'); g.add_argument('sha'); g.add_argument('dma')
    g.add_argument('image'); g.add_argument('-o', '--out')

    h = sub.add_parser('pack', help='copy extractor output into an external pack')
    h.add_argument('src'); h.add_argument('pack')
    h.add_argument('--game'); h.add_argument('--name')

    i = sub.add_parser('vcl-pages', help='replace VCL text pages from text_pages/')
    i.add_argument('vcl'); i.add_argument('folder'); i.add_argument('-o', '--out')

    j = sub.add_parser('sha-entry-folder',
                       help='replace one shape entry from a folder of frames')
    j.add_argument('sha'); j.add_argument('entry', type=int)
    j.add_argument('folder'); j.add_argument('-o', '--out')

    k = sub.add_parser('tile-clone',
                       help='import tile art with a fresh palette into a free '
                            'shape entry (writes .sha and .dma)')
    k.add_argument('sha'); k.add_argument('dma')
    k.add_argument('tile', type=lambda s: int(s, 0)); k.add_argument('image')
    k.add_argument('--out-sha'); k.add_argument('--out-dma')

    k2 = sub.add_parser(
        'tile-safe',
        help='import tile art with new colours by appending to an existing '
             '8bpp entry (no palette changes; writes .sha and .dma)')
    k2.add_argument('sha'); k2.add_argument('dma')
    k2.add_argument('tile', type=lambda s: int(s, 0)); k2.add_argument('image')
    k2.add_argument('--out-sha'); k2.add_argument('--out-dma')

    pal = sub.add_parser('palette-export',
                         help='export one shape entry palette (.pal/.gpl/.act)')
    pal.add_argument('sha'); pal.add_argument('entry', type=int)
    pal.add_argument('out')

    palm = sub.add_parser('palette-import',
                          help='replace one entry palette from .pal/.gpl/.act')
    palm.add_argument('sha'); palm.add_argument('entry', type=int)
    palm.add_argument('palette'); palm.add_argument('-o', '--out')

    port = sub.add_parser('portrait',
                          help='replace the 64x70 dialog portrait from one image')
    port.add_argument('sha'); port.add_argument('image'); port.add_argument('-o', '--out')

    args = p.parse_args(argv)
    try:
        return _run(args)
    except Exception as e:
        # Any bad input file is reported as a short message, never a traceback.
        if os.environ.get('JILL_IMPORT_DEBUG'):
            traceback.print_exc()
        message = str(e) or type(e).__name__
        print(f"error: {message}", file=sys.stderr)
        return 1


def _run(args) -> int:
    if args.cmd == 'sha-frame':
        out = replace_sha_frame(Path(args.sha), args.entry, args.frame,
                                Path(args.image), Path(args.out) if args.out else None,
                                adapt_palette=args.adapt_palette, dither=args.dither)
    elif args.cmd == 'sha-folder':
        out = sha_from_folder(Path(args.sha), Path(args.folder),
                              Path(args.out) if args.out else None)
    elif args.cmd == 'sha-entry-folder':
        out = sha_entry_from_folder(Path(args.sha), args.entry, Path(args.folder),
                                    Path(args.out) if args.out else None)
    elif args.cmd == 'tile-clone':
        sha_out, dma_out, entry, frame = clone_tile_to_entry(
            Path(args.sha), Path(args.dma), args.tile, Path(args.image),
            Path(args.out_sha) if args.out_sha else None,
            Path(args.out_dma) if args.out_dma else None)
        print(f"tile -> cloned shape {entry}:{frame}")
        print(f"wrote {sha_out}")
        print(f"wrote {dma_out}")
        return 0
    elif args.cmd == 'tile-safe':
        sha_out, dma_out, entry, frame = tile_from_png_safe(
            Path(args.sha), Path(args.dma), args.tile, Path(args.image),
            Path(args.out_sha) if args.out_sha else None,
            Path(args.out_dma) if args.out_dma else None)
        print(f"tile -> global-palette shape {entry}:{frame}")
        print(f"wrote {sha_out}")
        print(f"wrote {dma_out}")
        return 0
    elif args.cmd == 'palette-export':
        out = export_entry_palette(Path(args.sha), args.entry, Path(args.out))
    elif args.cmd == 'palette-import':
        out = import_entry_palette(Path(args.sha), args.entry,
                                   Path(args.palette),
                                   Path(args.out) if args.out else None)
    elif args.cmd == 'portrait':
        out = portrait_from_image(Path(args.sha), Path(args.image),
                                  Path(args.out) if args.out else None)
    elif args.cmd == 'sha-font':
        out = font_from_sheet(Path(args.sha), Path(args.sheet),
                              Path(args.out) if args.out else None,
                              args.entry, args.fg)
    elif args.cmd == 'sha-tile':
        out, entry, frame = tile_from_png(
            Path(args.sha), Path(args.dma), args.tile, Path(args.image),
            Path(args.out) if args.out else None, dither=args.dither)
        print(f"tile -> shape {entry}:{frame}")
    elif args.cmd == 'board-image':
        out = board_from_image(Path(args.board), Path(args.sha), Path(args.dma),
                               Path(args.image),
                               Path(args.out) if args.out else None)
    elif args.cmd == 'vcl-sfx':
        out = replace_vcl_sfx(Path(args.vcl), args.slot, Path(args.wav),
                              Path(args.out) if args.out else None)
    elif args.cmd == 'vcl-folder':
        out = vcl_from_folder(Path(args.vcl), Path(args.folder),
                              Path(args.out) if args.out else None)
    elif args.cmd == 'pack':
        stats = install_pack(Path(args.src), Path(args.pack),
                             args.game, args.name)
        print(', '.join(f"{k}={v}" for k, v in stats.items()))
        print(f"wrote {args.pack}")
        return 0
    elif args.cmd == 'vcl-pages':
        out = vcl_pages_from_folder(Path(args.vcl), Path(args.folder),
                                    Path(args.out) if args.out else None)
    else:
        return 2
    print(f"wrote {out}")
    return 0


if __name__ == '__main__':
    raise SystemExit(_cli(sys.argv[1:]))
