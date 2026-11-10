#!/usr/bin/env python3
"""
Picture / audio -> Jill converter.

Import PNG images and WAV audio into Jill (and Onesimus) archives, or build
brand new archives. Two levels of operation:

  * Replace content inside an existing archive
        replace_sha_frame(sha, entry, frame, png)   -> new .sha
        replace_vcl_sfx(vcl, slot, wav)             -> new .vcl
  * Package new archives from a folder tree
        sha_from_folder(sha, folder)                -> new .sha
        vcl_from_folder(vcl, folder)                -> new .vcl

The module also has a small CLI:

    python jill_import.py sha-frame  <sha> <entry> <frame> <image.png> [-o out.sha]
    python jill_import.py sha-folder <sha> <folder> [-o out.sha]
    python jill_import.py sha-font   <sha> <fonts/main.png> [-o out.sha]
    python jill_import.py sha-tile   <sha> <dma> <tile-id> <image.png> [-o out.sha]
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
import shutil
import struct
import sys
import wave
from pathlib import Path
from typing import Dict, List, Optional, Sequence, Tuple

from PIL import Image

# Import the reader/decoder implemented by the extractor.
sys.path.insert(0, str(Path(__file__).resolve().parent))
import jill_extractor as jx  # noqa: E402

# VGA colour-table shift/mask (mode 4) used to resolve entry palettes.
_VGA_SHIFT, _VGA_MASK = 16, 255


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
             transparent_index: int = 0, alpha_cutoff: int = 128) -> Tuple[int, int, bytes]:
    """Map an RGBA image to palette indices (one byte per pixel)."""
    img = image.convert('RGBA')
    w, h = img.size
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


def encode_entry(frames: Sequence[Tuple[int, int, int, bytes]],
                 src_bpp: int = 8, flags: int = 1,
                 palette_ints: Optional[Sequence[int]] = None,
                 trailing: Optional[Tuple[int, int, int, bytes]] = None) -> bytes:
    """Serialise one .sha entry.

    frames/trailing are (width, height, opcode, payload) tuples. New frames
    should use opcode 0 with one payload byte per pixel.
    """
    n = len(frames)
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
        out.append(w & 0xFF)
        out.append(h & 0xFF)
        out.append(op & 0xFF)
        if op == 0:
            need = w * h
            out.extend(data[:need].ljust(need, b'\x00'))
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
    entries = list(entries[:128])
    entries += [None] * (128 - len(entries))
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
    header = struct.pack('<128I', *offsets) + struct.pack('<128H', *sizes)
    assert len(header) == 0x300
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


def _load_image(path: Path) -> Image.Image:
    return Image.open(path).convert('RGBA')


def replace_sha_frame(sha_path: Path, entry_idx: int, frame_idx: int,
                      image_path: Path, out_path: Optional[Path] = None,
                      resize: bool = True) -> Path:
    """Replace one frame of an existing .sha entry with a PNG."""
    sha = jx.SHAFile.load(str(sha_path))
    entry = sha.entries[entry_idx]
    if entry is None:
        raise ValueError(f"shape entry {entry_idx} is empty")

    img = _load_image(image_path)
    colors = resolved_palette(sha, entry_idx)

    frames = _entry_frames(entry)
    if frame_idx < 0 or frame_idx >= len(frames):
        raise ValueError(f"frame {frame_idx} out of range (0..{len(frames)-1})")

    old_w, old_h = frames[frame_idx][0], frames[frame_idx][1]
    if resize and img.size != (old_w, old_h):
        img = img.resize((old_w, old_h), Image.NEAREST)
    w, h, data = quantize(img, colors)
    frames[frame_idx] = (w, h, 0, data)

    new_blob = encode_entry(frames, entry.src_bpp, entry.flags,
                            entry.palette or None, _entry_trailing(entry))
    blobs = _all_entry_blobs(sha)
    blobs[entry_idx] = new_blob

    out = Path(out_path) if out_path else Path(sha_path)
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
    sha = jx.SHAFile.load(str(sha_path)) if Path(sha_path).is_file() else None
    blobs: List[Optional[bytes]] = _all_entry_blobs(sha) if sha else [None] * 128
    if sha:
        base_palette = sha.vga_palette
        vga = [(base_palette[i], base_palette[i + 1], base_palette[i + 2])
               for i in range(0, 768, 3)]
    else:
        vga = [(i, i, i) for i in range(256)]

    folder = Path(folder)

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

    out = Path(out_path) if out_path else Path(sha_path)
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
    sha = jx.SHAFile.load(str(sha_path))
    entry = sha.entries[entry_idx] if 0 <= entry_idx < 128 else None
    if entry is None:
        raise ValueError(f"shape entry {entry_idx} is empty")

    sheet = Image.open(sheet_path).convert('RGBA')
    meta: Dict = {}
    meta_path = Path(sheet_path).parent / 'font.json'
    if meta_path.is_file():
        try:
            meta = json.loads(meta_path.read_text(encoding='utf-8'))
        except (OSError, ValueError):
            meta = {}
    cell_w = int(meta.get('cell_width') or max(1, sheet.width // 128))
    cell_h = int(meta.get('cell_height') or sheet.height)
    glyphs = {int(g['code']): g for g in meta.get('glyphs', [])
              if 'code' in g}
    frames: List[Tuple[int, int, int, bytes]] = []
    for code in range(128):
        g = glyphs.get(code, {})
        gw = int(g.get('width') or cell_w)
        gh = int(g.get('height') or cell_h)
        gw = max(1, min(gw, sheet.width - code * cell_w))
        gh = max(1, min(gh, sheet.height))
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
    out = Path(out_path) if out_path else Path(sha_path)
    out.write_bytes(build_sha_bytes(blobs))
    return out


def tile_from_png(sha_path: Path, dma_path: Path, tile_id: int,
                  image_path: Path, out_path: Optional[Path] = None,
                  resize: bool = True) -> Tuple[Path, int, int]:
    """Replace the SHA frame used by DMA `tile_id` with a PNG."""
    sha = jx.SHAFile.load(str(sha_path))
    dma = jx.DMAFile.load(str(dma_path))
    rec = next((r for r in dma.records if r.tile_id == tile_id), None)
    if rec is None:
        raise ValueError(f"tile id 0x{tile_id:02X} not in {dma_path.name}")
    entry_idx = (rec.shape_id >> 8) & 0x3F
    frame_idx = rec.shape_id & 0xFF
    entry = sha.entries[entry_idx]
    if entry is None or frame_idx >= entry.frame_count:
        raise ValueError(f"tile 0x{tile_id:02X} points at missing shape "
                         f"{entry_idx}:{frame_idx}")
    img = _load_image(image_path)
    old = entry.frames[frame_idx]
    if resize and img.size != (old['width'], old['height']):
        img = img.resize((old['width'], old['height']), Image.NEAREST)
    colors = resolved_palette(sha, entry_idx)
    w, h, data = quantize(img, colors)
    frames = _entry_frames(entry)
    frames[frame_idx] = (w, h, 0, data)
    blob = encode_entry(frames, entry.src_bpp, entry.flags,
                        entry.palette or None, _entry_trailing(entry))
    blobs = _all_entry_blobs(sha)
    blobs[entry_idx] = blob
    out = Path(out_path) if out_path else Path(sha_path)
    out.write_bytes(build_sha_bytes(blobs))
    return out, entry_idx, frame_idx


def board_from_image(board_path: Path, sha_path: Path, dma_path: Path,
                     image_path: Path,
                     out_path: Optional[Path] = None) -> Path:
    """Rebuild a board's tile map from a full 2048x1024 board render."""
    board = jx.BoardFile.load(str(board_path))
    sha = jx.SHAFile.load(str(sha_path))
    dma = jx.DMAFile.load(str(dma_path))
    img = Image.open(image_path).convert('RGB')
    if img.size != (2048, 1024):
        raise ValueError("board image must be the full 2048x1024 render "
                         "(use board_previews/<name>.png, not boards/)")

    cache: Dict[Tuple[int, int], Optional[Image.Image]] = {}
    exact: Dict[bytes, int] = {}
    sigs: List[Tuple[bytes, int]] = []
    for rec in dma.records:
        tile = jx._tile_rgba(sha, rec, cache)
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

    out = Path(out_path) if out_path else Path(board_path)
    board.save(str(out))
    return out


def install_pack(src_dir: Path, pack_dir: Path,
                 game: Optional[str] = None,
                 name: Optional[str] = None) -> Dict[str, int]:
    """Copy an extractor/reference-pack tree into an external pack folder."""
    src = Path(src_dir)
    dst = Path(pack_dir)
    dst.mkdir(parents=True, exist_ok=True)
    stats: Dict[str, int] = {}
    dirs = ['sprites', 'fonts', 'tiles', 'boards', 'themes', 'hud',
            'pages', 'sfx', 'music']
    for sub in dirs:
        s = src / sub
        if not s.is_dir():
            continue
        shutil.copytree(s, dst / sub, dirs_exist_ok=True)
        stats[sub] = sum(1 for p in (dst / sub).rglob('*') if p.is_file())
    for fname in ['portrait.png', 'palette.pal', 'palette_6bit.bin',
                  'palette.json']:
        s = src / fname
        if s.is_file():
            shutil.copy2(s, dst / fname)
            stats[fname] = 1

    warnings: List[str] = []
    def check(path: Path, size: Tuple[int, int]) -> None:
        if path.is_file():
            try:
                with Image.open(path) as im:
                    if im.size != size:
                        warnings.append(f"{path.name}: {im.size} (expected {size})")
            except OSError:
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
        label = name or dst.name
        lines = ['[pack]', f'name = {label}']
        if game:
            lines.append(f'game = {game}')
        ini.write_text('\n'.join(lines) + '\n', encoding='ascii')
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
    with wave.open(io.BytesIO(data), 'rb') as w:
        nch = w.getnchannels()
        sw = w.getsampwidth()
        rate = w.getframerate()
        raw = w.readframes(w.getnframes())
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
        raise ValueError(f"unsupported WAV sample width: {sw}")
    if nch > 1:
        mono = []
        for i in range(0, len(samples), nch):
            chunk = samples[i:i + nch]
            mono.append(sum(chunk) / len(chunk))
        samples = mono
    return samples, rate, 1


def resample(samples: List[float], src_rate: int, dst_rate: int) -> List[float]:
    if not samples or src_rate == dst_rate or dst_rate <= 0:
        return samples
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
    sfx = list(sfx[:50]) + [None] * (50 - len(sfx[:50]))
    music = list(music[:40]) + [None] * (40 - len(music[:40]))
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
    assert len(out) == _VCL_HEADER, len(out)
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
    if not (0 <= slot < 50):
        raise ValueError("slot must be 0..49")
    vcl = jx.VCLFile.load(str(vcl_path))
    sfx, music = _vcl_parts(vcl)
    data, rate = wav_to_sample(Path(wav_path).read_bytes(),
                               target_rate=vcl.sfx_rates[slot] or 6000)
    sfx[slot] = (data, rate)
    out = Path(out_path) if out_path else Path(vcl_path)
    out.write_bytes(build_vcl_bytes(sfx, music))
    return out


def vcl_from_folder(vcl_path: Path, folder: Path,
                    out_path: Optional[Path] = None) -> Path:
    """Build a .vcl from <folder>/sfx_NN.wav (+ optional music_NN.raw)."""
    vcl = jx.VCLFile.load(str(vcl_path)) if Path(vcl_path).is_file() else None
    if vcl:
        sfx, music = _vcl_parts(vcl)
    else:
        sfx, music = [None] * 50, [None] * 40

    folder = Path(folder)
    for wav_file in sorted(folder.glob('sfx_*.wav')):
        try:
            slot = int(wav_file.stem.split('_', 1)[1])
        except (IndexError, ValueError):
            continue
        if not (0 <= slot < 50):
            continue
        rate = (vcl.sfx_rates[slot] if vcl else 0) or 6000
        data, rate = wav_to_sample(wav_file.read_bytes(), target_rate=rate)
        sfx[slot] = (data, rate)
    for raw_file in sorted(folder.glob('music_*.raw')):
        try:
            slot = int(raw_file.stem.split('_', 1)[1])
        except (IndexError, ValueError):
            continue
        if 0 <= slot < 40:
            music[slot] = raw_file.read_bytes()

    out = Path(out_path) if out_path else Path(vcl_path)
    out.write_bytes(build_vcl_bytes(sfx, music))
    return out


def vcl_pages_from_folder(vcl_path: Path, folder: Path,
                          out_path: Optional[Path] = None) -> Path:
    """Replace the VCL text-page bank from `text_pages/`.

    Uses `pages.json` when present so the per-line colours are preserved;
    otherwise reads `page_NN.txt` with the default colour.
    """
    vcl = jx.VCLFile.load(str(vcl_path))
    sfx, pages = _vcl_parts(vcl)
    folder = Path(folder)
    meta_path = folder / 'pages.json'
    if meta_path.is_file():
        entries = json.loads(meta_path.read_text(encoding='utf-8'))
        for entry in entries:
            try:
                idx = int(entry['index'])
            except (KeyError, TypeError, ValueError):
                continue
            if not (0 <= idx < 40):
                continue
            data = bytearray()
            for line in entry.get('lines', []):
                colour = int(line.get('colour', 7))
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
            lines = page_file.read_text(encoding='ascii',
                                        errors='replace').splitlines()
            pages[idx] = b'\r'.join(line.encode('ascii', 'replace')
                                    for line in lines) + b'\r'
    out = Path(out_path) if out_path else Path(vcl_path)
    out.write_bytes(build_vcl_bytes(sfx, pages))
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

    g = sub.add_parser('board-image', help='rebuild a board from a 2048x1024 render')
    g.add_argument('board'); g.add_argument('sha'); g.add_argument('dma')
    g.add_argument('image'); g.add_argument('-o', '--out')

    h = sub.add_parser('pack', help='copy extractor output into an external pack')
    h.add_argument('src'); h.add_argument('pack')
    h.add_argument('--game'); h.add_argument('--name')

    i = sub.add_parser('vcl-pages', help='replace VCL text pages from text_pages/')
    i.add_argument('vcl'); i.add_argument('folder'); i.add_argument('-o', '--out')

    args = p.parse_args(argv)
    if args.cmd == 'sha-frame':
        out = replace_sha_frame(Path(args.sha), args.entry, args.frame,
                                Path(args.image), Path(args.out) if args.out else None)
    elif args.cmd == 'sha-folder':
        out = sha_from_folder(Path(args.sha), Path(args.folder),
                              Path(args.out) if args.out else None)
    elif args.cmd == 'sha-font':
        out = font_from_sheet(Path(args.sha), Path(args.sheet),
                              Path(args.out) if args.out else None,
                              args.entry, args.fg)
    elif args.cmd == 'sha-tile':
        out, entry, frame = tile_from_png(
            Path(args.sha), Path(args.dma), args.tile, Path(args.image),
            Path(args.out) if args.out else None)
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
