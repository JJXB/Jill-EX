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
    python jill_import.py vcl-sfx    <vcl> <slot> <audio.wav> [-o out.vcl]
    python jill_import.py vcl-folder <vcl> <folder> [-o out.vcl]
"""
from __future__ import annotations

import io
import math
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
    """Build a .sha archive from <folder>/shape_NNN/frame_NNN.png trees.

    Any entry folder that exists replaces that entry; the rest are copied from
    the reference .sha (when given and readable). New entries are written as
    8bpp identity-mapped sprites.
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
    for entry_dir in sorted(folder.glob('shape_*')):
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
        frames = []
        for im in images:
            if im.size != (w, h):
                im = im.resize((w, h), Image.NEAREST)
            fw, fh, data = quantize(im, vga)
            frames.append((fw, fh, 0, data))
        trailing = None
        tpath = entry_dir / 'trailing.png'
        if tpath.is_file():
            tim = _load_image(tpath)
            tw, th, tdata = quantize(tim, vga)
            trailing = (tw, th, 0, tdata)
        blobs[entry_idx] = encode_entry(frames, 8, 1, None, trailing)

    out = Path(out_path) if out_path else Path(sha_path)
    out.write_bytes(build_sha_bytes(blobs))
    return out


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

    args = p.parse_args(argv)
    if args.cmd == 'sha-frame':
        out = replace_sha_frame(Path(args.sha), args.entry, args.frame,
                                Path(args.image), Path(args.out) if args.out else None)
    elif args.cmd == 'sha-folder':
        out = sha_from_folder(Path(args.sha), Path(args.folder),
                              Path(args.out) if args.out else None)
    elif args.cmd == 'vcl-sfx':
        out = replace_vcl_sfx(Path(args.vcl), args.slot, Path(args.wav),
                              Path(args.out) if args.out else None)
    elif args.cmd == 'vcl-folder':
        out = vcl_from_folder(Path(args.vcl), Path(args.folder),
                              Path(args.out) if args.out else None)
    else:
        return 2
    print(f"wrote {out}")
    return 0


if __name__ == '__main__':
    raise SystemExit(_cli(sys.argv[1:]))
