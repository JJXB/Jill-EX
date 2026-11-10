#!/usr/bin/env python3
"""
Jill of the Jungle - Game File Viewer and Extractor
A comprehensive GUI utility for viewing and extracting game assets from Jill 1, 2, and 3.

Supports:
- Board files (.jn1, .jn2, .jn3, .dem)
- Shape files (.sha), including the text font
- Tile definitions (jill.dma) and rendered tile images
- Sound banks (.vcl), sampled effects and text pages
- Music files (.ddt - CMF format)
- Config files (.cfg)
- Input macros (.mac)
- Enhanced pack export: fonts, HUD panels, text pages, palettes,
  board previews and ready-to-edit external asset packs
"""

import sys
import os
import shutil
import struct
import json
import tkinter as tk
from tkinter import ttk, filedialog, messagebox, scrolledtext
from PIL import Image, ImageTk, ImageDraw
import io
import zlib
import tempfile
import wave
import subprocess
from pathlib import Path
from typing import Optional, List, Tuple, Dict, Any, BinaryIO
from dataclasses import dataclass, field
from enum import IntEnum
import threading

# Sound playback - use winsound on Windows
try:
    import winsound
    HAS_WINSOUND = True
except ImportError:
    HAS_WINSOUND = False
    winsound = None


BASE_DIR = Path(__file__).resolve().parent
# The port's OPL2/CMF renderer is used to turn .ddt music into WAV.  This is
# the only reliable CMF synthesiser available to the extractor.
MUSIC_EXPORT_SECONDS = 60.0


def find_port_exe() -> Optional[Path]:
    """Locate the built port executable used to render CMF music."""
    candidates = [
        BASE_DIR / "port" / "build" / "Release" / "jill.exe",
        BASE_DIR / "port" / "build" / "Debug" / "jill.exe",
        BASE_DIR / "port" / "build" / "jill.exe",
        BASE_DIR / "jill.exe",
    ]
    for p in candidates:
        if p.is_file():
            return p
    which = shutil.which("jill.exe")
    return Path(which) if which else None


def render_cmf_wav(game: Dict[str, Any], track: Path, out_wav: Path,
                   seconds: float = MUSIC_EXPORT_SECONDS) -> Tuple[bool, str]:
    """Render a .ddt/.cmf track to WAV with the port's OPL2 renderer.

    Returns (ok, message).  The extractor can still read/export the raw CMF
    when no built port executable is available.
    """
    exe = find_port_exe()
    if exe is None:
        return False, ("no built jill.exe found (build the port, or use "
                       "Export Raw / --dump-music)")
    out_wav = Path(out_wav)
    out_wav.parent.mkdir(parents=True, exist_ok=True)
    cmd = [str(exe), "--game", game['id'], "--data",
           str(Path(game['data_dir'])), "--dump-music", str(out_wav),
           "--music-file", track.name, "--music-seconds",
           f"{max(1.0, seconds):.1f}"]
    try:
        proc = subprocess.run(cmd, cwd=str(BASE_DIR), capture_output=True,
                              text=True, timeout=max(120.0, seconds * 4.0))
    except Exception as e:
        return False, f"music render failed: {e}"
    if proc.returncode != 0 or not out_wav.is_file() or out_wav.stat().st_size <= 44:
        detail = (proc.stderr or proc.stdout or "").strip().splitlines()
        return False, (detail[-1] if detail else f"music render failed "
                       f"(exit {proc.returncode})")
    return True, str(out_wav)


def is_cmf_file(path: Path) -> bool:
    try:
        with open(path, 'rb') as f:
            return f.read(4) == b'CTMF'
    except OSError:
        return False


def export_music_wavs(game: Dict[str, Any], out_dir: Path,
                      seconds: float = MUSIC_EXPORT_SECONDS) -> int:
    """Render every CMF track (.ddt or .ARK) to `music/<stem>.wav`."""
    data_dir = Path(game['data_dir'])
    music_dir = Path(out_dir) / "music"
    music_dir.mkdir(parents=True, exist_ok=True)
    count = 0
    files = []
    try:
        files.extend(data_dir.glob("*.ddt"))
        files.extend(data_dir.glob("*.ark"))
        files.extend(data_dir.glob("*.ARK"))
    except OSError:
        pass
    seen = set()
    for f in sorted(files, key=lambda p: p.name.lower()):
        if f.name.lower() in seen or not is_cmf_file(f):
            continue
        seen.add(f.name.lower())
        out = music_dir / f"{f.stem}.wav"
        ok, msg = render_cmf_wav(game, f, out, seconds)
        if ok:
            print(f"  Rendered: {f.name} -> {out.name}")
            count += 1
        else:
            print(f"  Skipped {f.name}: {msg}")
    return count


def play_wav_file(path: Path, async_: bool = True) -> bool:
    """Play a temporary WAV with winsound (Windows only)."""
    if not HAS_WINSOUND:
        return False
    try:
        flags = winsound.SND_FILENAME
        if async_:
            flags |= winsound.SND_ASYNC
        winsound.PlaySound(str(path), flags)
        if async_:
            def cleanup():
                import time
                time.sleep(120)
                try:
                    Path(path).unlink()
                except OSError:
                    pass
            threading.Thread(target=cleanup, daemon=True).start()
        return True
    except Exception as e:
        print(f"Playback error: {e}")
        return False


# ============================================================================
# DATA STRUCTURES AND FORMAT PARSERS
# ============================================================================

@dataclass
class BoardHeader:
    """70-byte board header (live HUD state)"""
    level_id: int = 0
    health: int = 6
    key_count: int = 0
    key_ids: List[int] = field(default_factory=lambda: [0]*16)
    score: int = 0
    damage_flash: int = 0
    score_copy: int = 0
    reserved: bytes = b''
    
    @classmethod
    def from_bytes(cls, data: bytes) -> 'BoardHeader':
        if len(data) < 70:
            raise ValueError("Board header too small")
        h = cls()
        h.level_id = struct.unpack('<H', data[0:2])[0]
        h.health = struct.unpack('<H', data[2:4])[0]
        h.key_count = struct.unpack('<H', data[4:6])[0]
        h.key_ids = list(struct.unpack('<16H', data[6:38]))
        h.score = struct.unpack('<I', data[38:42])[0]
        h.damage_flash = struct.unpack('<H', data[42:44])[0]
        h.score_copy = struct.unpack('<I', data[44:48])[0]
        h.reserved = data[48:70]
        return h
    
    def to_bytes(self) -> bytes:
        out = bytearray(70)
        struct.pack_into('<H', out, 0, self.level_id)
        struct.pack_into('<H', out, 2, self.health)
        struct.pack_into('<H', out, 4, self.key_count)
        struct.pack_into('<16H', out, 6, *self.key_ids)
        struct.pack_into('<I', out, 38, self.score)
        struct.pack_into('<H', out, 42, self.damage_flash)
        struct.pack_into('<I', out, 44, self.score_copy)
        out[48:70] = self.reserved
        return bytes(out)


@dataclass
class BoardObject:
    """31-byte object record"""
    kind: int = 0
    x: int = 0
    y: int = 0
    x_delta: int = 0
    y_delta: int = 0
    width: int = 0
    height: int = 0
    state: int = 0
    kind_specific_1: bytes = b'\x00'*5
    primary_param: int = 0
    flags: int = 0
    string_offset: int = 0
    string_segment: int = 0
    kind_specific_2: int = 0
    ttl: int = 0
    attached_string: str = ""
    
    KIND_NAMES = {
        0: "PLAYER", 1: "APPLE", 2: "KNIFE", 3: "KILLME", 4: "BIGANT",
        5: "FLY", 6: "MACROTRIG", 7: "DEMON", 8: "BUNNY", 9: "INCHWORM",
        10: "ZAPPER", 11: "BOBSLUG", 12: "CHECKPT", 13: "PAUL", 14: "KEY",
        15: "PAD", 16: "WISEMAN", 17: "FATSO", 18: "FIREBALL", 19: "CLOUD",
        20: "TEXT6", 21: "TEXT8", 22: "FROG", 23: "TINY", 24: "DOOR",
        25: "FALLDOOR", 26: "BRIDGER", 27: "SCORE", 28: "TOKEN", 29: "ANT",
        30: "PHOENIX", 31: "FIRE", 32: "SWITCH", 33: "GEM", 34: "TXTMSG",
        35: "BOULDER", 36: "EXPL1", 37: "EXPL2", 38: "STALAG", 39: "SNAKE",
        40: "SEAROCK", 41: "BOLL", 42: "MEGA", 43: "BAT", 44: "KNIGHT",
        45: "BEENEST", 46: "BEESWARM", 47: "CRAB", 48: "CROC", 49: "EPIC",
        50: "SPINBLAD", 51: "SKULL", 52: "BUTTON", 53: "PAC", 54: "JILLFISH",
        55: "JILLSPIDER", 56: "JILLBIRD", 57: "JILLFROG", 58: "BUBBLE",
        59: "JELLYFISH", 60: "BADFISH", 61: "ELEV", 62: "FIREBULLET", 63: "FISHBULLET",
        64: "EYE", 65: "VINECLIMB", 66: "FLAG", 67: "MAPDEMO", 68: "ROMAN"
    }
    
    FLAG_BITS = {
        0x0008: "HIT_SCAN",
        0x0040: "HAS_STRING",
        0x0080: "KILLABLE",
        0x0100: "RECEIVES_TRIGGER",
        0x0400: "SPINBLAD_TARGET",
        0x4000: "IS_PROJECTILE",
    }
    
    @classmethod
    def from_bytes(cls, data: bytes) -> 'BoardObject':
        if len(data) < 31:
            raise ValueError("Object record too small")
        obj = cls()
        obj.kind = data[0]
        obj.x = struct.unpack('<h', data[1:3])[0]
        obj.y = struct.unpack('<h', data[3:5])[0]
        obj.x_delta = struct.unpack('<h', data[5:7])[0]
        obj.y_delta = struct.unpack('<h', data[7:9])[0]
        obj.width = struct.unpack('<H', data[9:11])[0]
        obj.height = struct.unpack('<H', data[11:13])[0]
        obj.state = data[13]
        obj.kind_specific_1 = data[14:19]
        obj.primary_param = struct.unpack('<H', data[19:21])[0]
        obj.flags = struct.unpack('<H', data[21:23])[0]
        obj.string_offset = struct.unpack('<H', data[23:25])[0]
        obj.string_segment = struct.unpack('<H', data[25:27])[0]
        obj.kind_specific_2 = struct.unpack('<H', data[27:29])[0]
        obj.ttl = struct.unpack('<H', data[29:31])[0]
        return obj
    
    def to_bytes(self) -> bytes:
        out = bytearray(31)
        out[0] = self.kind
        struct.pack_into('<h', out, 1, self.x)
        struct.pack_into('<h', out, 3, self.y)
        struct.pack_into('<h', out, 5, self.x_delta)
        struct.pack_into('<h', out, 7, self.y_delta)
        struct.pack_into('<H', out, 9, self.width)
        struct.pack_into('<H', out, 11, self.height)
        out[13] = self.state
        out[14:19] = self.kind_specific_1
        struct.pack_into('<H', out, 19, self.primary_param)
        struct.pack_into('<H', out, 21, self.flags)
        struct.pack_into('<H', out, 23, self.string_offset)
        struct.pack_into('<H', out, 25, self.string_segment)
        struct.pack_into('<H', out, 27, self.kind_specific_2)
        struct.pack_into('<H', out, 29, self.ttl)
        return bytes(out)
    
    def get_kind_name(self) -> str:
        return self.KIND_NAMES.get(self.kind, f"UNKNOWN_{self.kind}")
    
    def get_flag_names(self) -> List[str]:
        flags = []
        for bit, name in self.FLAG_BITS.items():
            if self.flags & bit:
                flags.append(name)
        return flags


@dataclass
class BoardFile:
    """Complete board file (.jn1, .jn2, .jn3, .dem)"""
    tile_map: List[int] = field(default_factory=list)  # 128*64 = 8192 u16
    objects: List[BoardObject] = field(default_factory=list)
    header: BoardHeader = field(default_factory=BoardHeader)
    strings: List[str] = field(default_factory=list)
    filename: str = ""
    
    @classmethod
    def load(cls, filepath: str) -> 'BoardFile':
        with open(filepath, 'rb') as f:
            data = f.read()
        return cls.parse(data, os.path.basename(filepath))
    
    @classmethod
    def parse(cls, data: bytes, filename: str = "") -> 'BoardFile':
        if len(data) < 0x4000 + 2:
            raise ValueError("File too small for board")
        
        board = cls()
        board.filename = filename
        
        # Tile map: 128*64 u16, column-major
        board.tile_map = list(struct.unpack('<8192H', data[0:0x4000]))
        
        # Object count
        obj_count = struct.unpack('<H', data[0x4000:0x4002])[0]
        if obj_count > 192 or obj_count == 0:
            raise ValueError(f"Invalid object count: {obj_count}")
        
        # Objects
        offset = 0x4002
        for i in range(obj_count):
            obj_data = data[offset:offset+31]
            obj = BoardObject.from_bytes(obj_data)
            board.objects.append(obj)
            offset += 31
        
        # Board header (70 bytes)
        board.header = BoardHeader.from_bytes(data[offset:offset+70])
        offset += 70
        
        # Strings - length prefixed (u16 len + bytes + NUL)
        # Only objects with string pointers have strings
        for obj in board.objects:
            if obj.string_offset != 0 or obj.string_segment != 0:
                if offset + 2 <= len(data):
                    str_len = struct.unpack('<H', data[offset:offset+2])[0]
                    offset += 2
                    if offset + str_len <= len(data):
                        s = data[offset:offset+str_len].decode('ascii', errors='replace')
                        board.strings.append(s)
                        offset += str_len + 1  # +1 for NUL
                    else:
                        board.strings.append("")
                else:
                    board.strings.append("")
        
        return board
    
    def save(self, filepath: str):
        """Write board file"""
        with open(filepath, 'wb') as f:
            # Tile map
            f.write(struct.pack('<8192H', *self.tile_map))
            # Object count
            f.write(struct.pack('<H', len(self.objects)))
            # Objects
            for obj in self.objects:
                f.write(obj.to_bytes())
            # Header
            f.write(self.header.to_bytes())
            # Strings
            for s in self.strings:
                encoded = s.encode('ascii')
                f.write(struct.pack('<H', len(encoded)))
                f.write(encoded)
                f.write(b'\x00')
    
    def get_tile_at(self, x: int, y: int) -> int:
        """Get tile ID at column-major position"""
        if 0 <= x < 128 and 0 <= y < 64:
            return self.tile_map[x * 64 + y] & 0x3FFF
        return 0
    
    def get_tile_flags(self, x: int, y: int) -> int:
        if 0 <= x < 128 and 0 <= y < 64:
            return (self.tile_map[x * 64 + y] >> 14) & 3
        return 0


@dataclass
class DMARecord:
    """jill.dma tile definition record"""
    tile_id: int = 0
    shape_id: int = 0
    attr: int = 0
    name: str = ""
    
    @classmethod
    def parse(cls, data: bytes, offset: int) -> Tuple['DMARecord', int]:
        if offset + 7 > len(data):
            return None, offset
        tile_id = struct.unpack('<H', data[offset:offset+2])[0]
        shape_id = struct.unpack('<H', data[offset+2:offset+4])[0]
        attr = struct.unpack('<H', data[offset+4:offset+6])[0]
        name_len = data[offset+6]
        if offset + 7 + name_len > len(data):
            return None, offset
        name = data[offset+7:offset+7+name_len].decode('ascii', errors='replace')
        rec = cls(tile_id, shape_id, attr, name)
        return rec, offset + 7 + name_len


class DMAFile:
    """jill.dma - tile definitions (320 records)"""
    def __init__(self):
        self.records: List[DMARecord] = []
    
    @classmethod
    def load(cls, filepath: str) -> 'DMAFile':
        with open(filepath, 'rb') as f:
            data = f.read()
        return cls.parse(data)
    
    @classmethod
    def parse(cls, data: bytes) -> 'DMAFile':
        dma = cls()
        offset = 0
        while offset < len(data):
            rec, offset = DMARecord.parse(data, offset)
            if rec is None:
                break
            dma.records.append(rec)
            if len(dma.records) >= 320:
                break
        return dma
    
    def get_record(self, tile_id: int) -> Optional[DMARecord]:
        for rec in self.records:
            if rec.tile_id == tile_id:
                return rec
        return None


@dataclass
class ShapeEntry:
    """Shape entry in .sha file"""
    frame_count: int = 0
    unk1: int = 1
    size_2bpp: int = 0
    size_4bpp: int = 0
    size_8bpp: int = 0
    src_bpp: int = 0
    flags: int = 0
    palette: List[int] = field(default_factory=list)  # 4 bytes per entry
    frames: List[Dict] = field(default_factory=list)  # decoded frame info
    trailing_frame: Optional[Dict] = None
    raw_data: bytes = b''


class SHAFile:
    """jill1.sha / jill2.sha / jill3.sha - Shape archive"""
    def __init__(self):
        self.offsets: List[int] = [0]*128
        self.sizes: List[int] = [0]*128
        self.entries: List[Optional[ShapeEntry]] = [None]*128
        self.file_data: bytes = b''
        # Base 256-colour palette (768 8-bit ints). Defaults to the game's
        # 6-bit palette expanded to 8-bit; can be replaced with a .pal file.
        self.vga_palette: List[int] = [int(c * 255 / 63) for c in self.DEFAULT_VGA_PALETTE]

    def set_palette_rgb(self, data: bytes, from_6bit: bool = False) -> bool:
        """Set the base DAC palette from 768 bytes of RGB data."""
        if len(data) < 768:
            return False
        idx = range(768)
        if from_6bit:
            self.vga_palette = [int(data[i] * 255 / 63) for i in idx]
        else:
            self.vga_palette = [data[i] for i in idx]
        return True
    
    @classmethod
    def load(cls, filepath: str) -> 'SHAFile':
        with open(filepath, 'rb') as f:
            data = f.read()
        return cls.parse(data)
    
    @classmethod
    def parse(cls, data: bytes) -> 'SHAFile':
        sha = cls()
        sha.file_data = data
        
        if len(data) < 0x300:
            raise ValueError("SHA file too small")
        
        # Directory: 128 u32 offsets
        sha.offsets = list(struct.unpack('<128I', data[0:0x200]))
        # Directory: 128 u16 sizes
        sha.sizes = list(struct.unpack('<128H', data[0x200:0x300]))
        
        # Parse each entry
        for i in range(128):
            if sha.sizes[i] > 0 and sha.offsets[i] < len(data):
                sha.entries[i] = sha._parse_entry(i, data)
        
        return sha
    
    def _parse_entry(self, index: int, data: bytes) -> ShapeEntry:
        entry = ShapeEntry()
        offset = self.offsets[index]
        size = self.sizes[index]
        entry.raw_data = data[offset:offset+size]
        
        if size < 11:
            return entry
        
        # Header
        ptr = 0
        entry.frame_count = entry.raw_data[ptr]; ptr += 1
        entry.unk1 = struct.unpack('<H', entry.raw_data[ptr:ptr+2])[0]; ptr += 2
        entry.size_2bpp = struct.unpack('<H', entry.raw_data[ptr:ptr+2])[0]; ptr += 2
        entry.size_4bpp = struct.unpack('<H', entry.raw_data[ptr:ptr+2])[0]; ptr += 2
        entry.size_8bpp = struct.unpack('<H', entry.raw_data[ptr:ptr+2])[0]; ptr += 2
        entry.src_bpp = entry.raw_data[ptr]; ptr += 1
        entry.flags = struct.unpack('<H', entry.raw_data[ptr:ptr+2])[0]; ptr += 2
        
        # Palette if flags & 1 == 0 and bpp < 8
        if (entry.flags & 1) == 0 and entry.src_bpp < 8:
            palette_entries = 1 << entry.src_bpp
            entry.palette = []
            for _ in range(palette_entries):
                if ptr + 4 <= len(entry.raw_data):
                    pal_entry = struct.unpack('<I', entry.raw_data[ptr:ptr+4])[0]
                    entry.palette.append(pal_entry)
                    ptr += 4
        
        # Frames
        for f in range(entry.frame_count):
            if ptr + 3 > len(entry.raw_data):
                break
            w = entry.raw_data[ptr]; ptr += 1
            h = entry.raw_data[ptr]; ptr += 1
            opcode = entry.raw_data[ptr]; ptr += 1
            
            frame = {'width': w, 'height': h, 'opcode': opcode, 'data': b'', 'offset': ptr}
            
            if opcode == 0:  # Raw 8-bit pixels
                payload_size = w * h
                if ptr + payload_size <= len(entry.raw_data):
                    frame['data'] = entry.raw_data[ptr:ptr+payload_size]
                    ptr += payload_size
            elif opcode == 1:  # Packed bits
                # Bit-packed: width*height pixels of src_bpp bits
                bit_count = w * h * entry.src_bpp
                byte_count = (bit_count + 7) // 8
                if ptr + byte_count <= len(entry.raw_data):
                    frame['data'] = entry.raw_data[ptr:ptr+byte_count]
                    ptr += byte_count
            elif opcode == 2:  # Empty frame
                frame['data'] = b''
            else:
                frame['data'] = b''
            
            entry.frames.append(frame)
        
        # Trailing frame (editor preview). Capture its payload so a re-encoded
        # entry can be written back losslessly.
        if ptr + 3 <= len(entry.raw_data):
            w = entry.raw_data[ptr]; ptr += 1
            h = entry.raw_data[ptr]; ptr += 1
            opcode = entry.raw_data[ptr]; ptr += 1
            tf = {'width': w, 'height': h, 'opcode': opcode, 'data': b'', 'offset': ptr}
            if opcode == 0:
                n = w * h
                if ptr + n <= len(entry.raw_data):
                    tf['data'] = entry.raw_data[ptr:ptr+n]
                    ptr += n
            elif opcode == 1:
                n = (w * h * entry.src_bpp + 7) // 8
                if ptr + n <= len(entry.raw_data):
                    tf['data'] = entry.raw_data[ptr:ptr+n]
                    ptr += n
            entry.trailing_frame = tf
        
        return entry
    
    def frame_indices(self, entry_idx: int, frame_idx: int) -> Optional[Tuple[int, int, bytes]]:
        """Raw palette indices for one frame (no RGB conversion).

        Used by the font/HUD exporters, which need to know the key colour
        (index 0/3 for the text font) rather than only the mapped RGB.
        """
        if entry_idx < 0 or entry_idx >= 128 or self.entries[entry_idx] is None:
            return None
        entry = self.entries[entry_idx]
        if frame_idx < 0 or frame_idx >= len(entry.frames):
            return None
        frame = entry.frames[frame_idx]
        w, h = frame['width'], frame['height']
        if w <= 0 or h <= 0:
            return None
        if frame['opcode'] == 0:
            raw = bytearray(frame['data'][:w * h])
            if len(raw) < w * h:
                raw.extend(b'\x00' * (w * h - len(raw)))
            return w, h, bytes(raw)
        if frame['opcode'] == 1:
            return w, h, self._unpack_pixels(frame['data'], w, h, entry.src_bpp)
        return w, h, bytes(w * h)

    def decode_frame_to_image(self, entry_idx: int, frame_idx: int, video_mode: int = 4) -> Optional[Image.Image]:
        """Decode a frame to PIL Image. video_mode: 0=CGA, 2=EGA, 4=VGA"""
        if entry_idx >= 128 or self.entries[entry_idx] is None:
            return None
        entry = self.entries[entry_idx]
        if frame_idx >= len(entry.frames):
            return None
        
        frame = entry.frames[frame_idx]
        w, h = frame['width'], frame['height']
        if w == 0 or h == 0:
            return None
        
        # Build palette for this entry/video mode
        palette = self._build_palette(entry, video_mode)
        
        # Decode based on opcode and source BPP.
        # opcode 0 stores exactly one byte per pixel (values already 0..2^bpp-1);
        # opcode 1 bit-packs src_bpp bits per pixel, LSB first.
        if frame['opcode'] == 0:
            needed = w * h
            raw = frame['data'][:needed]
            if len(raw) < needed:
                raw = raw + b'\x00' * (needed - len(raw))
            img = Image.frombytes('P', (w, h), raw)
            if palette:
                img.putpalette(palette)
            return img.convert('RGBA')
        
        elif frame['opcode'] == 1:
            # Bit-packed: width*height pixels of src_bpp bits, LSB first
            pixels = self._unpack_pixels(frame['data'], w, h, entry.src_bpp)
            img = Image.frombytes('P', (w, h), pixels)
            if palette:
                img.putpalette(palette)
            return img.convert('RGBA')
        
        elif frame['opcode'] == 2:
            # Empty frame
            return Image.new('RGBA', (w, h), (0, 0, 0, 0))
        
        return None
    
    # Default VGA palette from DGROUP:0x0194 (768 bytes = 256 RGB triplets, 6-bit)
    DEFAULT_VGA_PALETTE = [
        0,   0,   0,   0,   0,  42,   0,  42,   0,   0,  42,  42,
        42,   0,   0,  42,   0,  42,  42,  21,   0,  42,  42,  42,
        21,  21,  21,  21,  21,  63,  21,  63,  21,  21,  63,  63,
        63,  21,  21,  63,  21,  63,  63,  63,  21,  63,  63,  63,
         7,   7,   7,  14,  14,  14,  27,  27,  27,  32,  32,  32,
        37,  37,  37,  47,  47,  47,  52,  52,  52,  57,  57,  57,
        25,  10,   0,  37,  10,   0,  50,  10,   0,  63,  10,   0,
         0,  21,   0,  12,  21,   0,  25,  21,   0,  37,  21,   0,
        50,  21,   0,  63,  21,   0,   0,  31,   0,  12,  31,   0,
        25,  31,   0,  37,  31,   0,  50,  31,   0,  63,  31,   0,
         0,  42,   0,  12,  42,   0,  25,  42,   0,  37,  42,   0,
        50,  42,   0,  63,  42,   0,  12,  52,   0,  25,  52,   0,
        37,  52,   0,  50,  52,   0,  63,  52,   0,  12,  63,   0,
        25,  63,   0,  37,  63,   0,  50,  63,   0,  63,  63,   0,
         0,   0,  12,  12,   0,  12,  25,   0,  12,  37,   0,  12,
        50,   0,  12,  63,   0,  12,   0,  10,  12,  12,  10,  12,
        25,  10,  12,  37,  10,  12,  50,  10,  12,  63,  10,  12,
         0,  21,  12,  12,  21,  12,  25,  21,  12,  37,  21,  12,
        50,  21,  12,  63,  21,  12,   0,  31,  12,  12,  31,  12,
        25,  31,  12,  37,  31,  12,  50,  31,  12,  63,  31,  12,
         0,  42,  12,  12,  42,  12,  25,  42,  12,  37,  42,  12,
        50,  42,  12,  63,  42,  12,  12,  52,  12,  25,  52,  12,
        37,  52,  12,  50,  52,  12,  63,  52,  12,  12,  63,  12,
        25,  63,  12,  37,  63,  12,  50,  63,  12,  63,  63,  12,
         0,   0,  25,  12,   0,  25,  25,   0,  25,  37,   0,  25,
        50,   0,  25,  63,   0,  25,   0,  10,  25,  12,  10,  25,
        25,  10,  25,  37,  10,  25,  50,  10,  25,  63,  10,  25,
         0,  21,  25,  12,  21,  25,  25,  21,  25,  37,  21,  25,
        50,  21,  25,  63,  21,  25,   0,  31,  25,  12,  31,  25,
        25,  31,  25,  37,  31,  25,  50,  31,  25,  63,  31,  25,
         0,  42,  25,  12,  42,  25,  25,  42,  25,  37,  42,  25,
        50,  42,  25,  63,  42,  25,  12,  52,  25,  25,  52,  25,
        37,  52,  25,  50,  52,  25,  63,  52,  25,  12,  63,  25,
        25,  63,  25,  37,  63,  25,  50,  63,  25,  63,  63,  25,
         0,   0,  37,  12,   0,  37,  25,   0,  37,  37,   0,  37,
        50,   0,  37,  63,   0,  37,   0,  10,  37,  12,  10,  37,
        25,  10,  37,  37,  10,  37,  50,  10,  37,  63,  10,  37,
         0,  21,  37,  12,  21,  37,  25,  21,  37,  37,  21,  37,
        50,  21,  37,  63,  21,  37,   0,  31,  37,  12,  31,  37,
        25,  31,  37,  37,  31,  37,  50,  31,  37,  63,  31,  37,
         0,  42,  37,  12,  42,  37,  25,  42,  37,  37,  42,  37,
        50,  42,  37,  63,  42,  37,  12,  52,  37,  25,  52,  37,
        37,  52,  37,  50,  52,  37,  63,  52,  37,  12,  63,  37,
        25,  63,  37,  37,  63,  37,  50,  63,  37,  63,  63,  37,
         0,   0,  50,  12,   0,  50,  25,   0,  50,  37,   0,  50,
        50,   0,  50,  63,   0,  50,   0,  10,  50,  12,  10,  50,
        25,  10,  50,  37,  10,  50,  50,  10,  50,  63,  10,  50,
         0,  21,  50,  12,  21,  50,  25,  21,  50,  37,  21,  50,
        50,  21,  50,  63,  21,  50,   0,  31,  50,  12,  31,  50,
        25,  31,  50,  37,  31,  50,  50,  31,  50,  63,  31,  50,
         0,  42,  50,  12,  42,  50,  25,  42,  50,  37,  42,  50,
        50,  42,  50,  63,  42,  50,  12,  52,  50,  25,  52,  50,
        37,  52,  50,  50,  52,  50,  63,  52,  50,  12,  63,  50,
        25,  63,  50,  37,  63,  50,  50,  63,  50,  63,  63,  50,
         0,   0,  63,  12,   0,  63,  25,   0,  63,  37,   0,  63,
        50,   0,  63,  63,   0,  63,   0,  10,  63,  12,  10,  63,
        25,  10,  63,  37,  10,  63,  50,  10,  63,  63,  10,  63,
         0,  21,  63,  12,  21,  63,  25,  21,  63,  37,  21,  63,
        50,  21,  63,  63,  21,  63,   0,  31,  63,  12,  31,  63,
        25,  31,  63,  37,  31,  63,  50,  31,  63,  63,  31,  63,
         0,  42,  63,  12,  42,  63,  25,  42,  63,  37,  42,  63,
        50,  42,  63,  63,  42,  63,  12,  52,  63,  25,  52,  63,
        37,  52,  63,  50,  52,  63,  63,  52,  63,  12,  63,  63,
        25,  63,  63,  37,  63,  63,  50,  63,  63,  63,  63,  63,
    ]
    
    def _build_palette(self, entry: 'ShapeEntry', video_mode: int) -> Optional[List[int]]:
        """Build a 256*3 RGB palette for the given entry and video mode"""
        vga_palette_8bit = self.vga_palette
        
        if entry.flags & 1:
            # No palette - identity mapping
            if video_mode == 4:  # VGA 8bpp
                return vga_palette_8bit  # Use default VGA palette
            elif video_mode == 2:  # EGA 4bpp - 16 colors
                ega_colors = [
                    (0,0,0), (0,0,170), (0,170,0), (0,170,170),
                    (170,0,0), (170,0,170), (170,85,0), (170,170,170),
                    (85,85,85), (85,85,255), (85,255,85), (85,255,255),
                    (255,85,85), (255,85,255), (255,255,85), (255,255,255)
                ]
                palette = []
                for r,g,b in ega_colors:
                    palette.extend([r,g,b])
                palette.extend([0]* (768 - len(palette)))
                return palette
            elif video_mode == 0:  # CGA 2bpp - 4 colors
                cga_colors = [(0,0,0), (85,255,255), (255,85,255), (255,255,255)]
                palette = []
                for r,g,b in cga_colors:
                    palette.extend([r,g,b])
                palette.extend([0]* (768 - len(palette)))
                return palette
        else:
            # Has palette in entry - entries are indices into the VGA palette
            if entry.palette:
                palette = [0] * 768
                for i, pal_entry in enumerate(entry.palette):
                    # Each palette entry is 4 bytes, useful byte depends on video mode
                    if video_mode == 4:  # VGA - use byte at offset 2 (bits 16-23)
                        vga_index = (pal_entry >> 16) & 0xFF
                    elif video_mode == 2:  # EGA - use byte at offset 1 (bits 8-11)
                        vga_index = (pal_entry >> 8) & 0x0F
                    elif video_mode == 0:  # CGA - use byte at offset 0 (bits 0-1)
                        vga_index = pal_entry & 0x03
                    else:
                        vga_index = (pal_entry >> 16) & 0xFF
                    
                    # Map through VGA palette
                    if vga_index < 256:
                        r = vga_palette_8bit[vga_index * 3]
                        g = vga_palette_8bit[vga_index * 3 + 1]
                        b = vga_palette_8bit[vga_index * 3 + 2]
                    else:
                        r = g = b = 0
                    
                    if i < 256:
                        palette[i * 3] = r
                        palette[i * 3 + 1] = g
                        palette[i * 3 + 2] = b
                
                return palette
        
        return None
    
    def _unpack_pixels(self, data: bytes, width: int, height: int, bpp: int) -> bytes:
        """Unpack bit-packed pixel data to bytes (one byte per pixel)"""
        total_pixels = width * height
        pixels = bytearray(total_pixels)
        
        bit_pos = 0
        byte_idx = 0
        
        for i in range(total_pixels):
            if bpp == 1:
                # 1 bit per pixel
                if byte_idx < len(data):
                    pixels[i] = (data[byte_idx] >> bit_pos) & 1
                    bit_pos += 1
                    if bit_pos >= 8:
                        bit_pos = 0
                        byte_idx += 1
            elif bpp == 2:
                # 2 bits per pixel
                if byte_idx < len(data):
                    pixels[i] = (data[byte_idx] >> bit_pos) & 3
                    bit_pos += 2
                    if bit_pos >= 8:
                        bit_pos = 0
                        byte_idx += 1
            elif bpp == 3:
                # 3 bits per pixel
                if byte_idx + 1 < len(data):
                    val = data[byte_idx] | (data[byte_idx + 1] << 8)
                    pixels[i] = (val >> bit_pos) & 7
                    bit_pos += 3
                    if bit_pos >= 8:
                        bit_pos -= 8
                        byte_idx += 1
            elif bpp == 4:
                # 4 bits per pixel (nibbles)
                if byte_idx < len(data):
                    if bit_pos == 0:
                        pixels[i] = data[byte_idx] & 0x0F
                        bit_pos = 4
                    else:
                        pixels[i] = (data[byte_idx] >> 4) & 0x0F
                        bit_pos = 0
                        byte_idx += 1
            elif bpp == 8:
                # 8 bits per pixel (should be opcode 0 but handle anyway)
                if byte_idx < len(data):
                    pixels[i] = data[byte_idx]
                    byte_idx += 1
            else:
                pixels[i] = 0
        
        return bytes(pixels)
    
    def get_entry_info(self, index: int) -> str:
        if index >= 128 or self.entries[index] is None:
            return f"Entry {index}: EMPTY"
        e = self.entries[index]
        return (f"Entry {index}: {e.frame_count} frames, "
                f"src_bpp={e.src_bpp}, flags=0x{e.flags:04X}, "
                f"sizes: 2bpp={e.size_2bpp}, 4bpp={e.size_4bpp}, 8bpp={e.size_8bpp}")


class VCLFile:
    """jill1.vcl / jill2.vcl / jill3.vcl - Sampled sound bank"""
    def __init__(self):
        self.sfx_offsets: List[int] = [0]*50
        self.sfx_sizes: List[int] = [0]*50
        self.sfx_rates: List[int] = [0]*50
        self.music_offsets: List[int] = [0]*40
        self.music_sizes: List[int] = [0]*40
        self.file_data: bytes = b''
    
    @classmethod
    def load(cls, filepath: str) -> 'VCLFile':
        with open(filepath, 'rb') as f:
            data = f.read()
        return cls.parse(data)
    
    @classmethod
    def parse(cls, data: bytes) -> 'VCLFile':
        vcl = cls()
        vcl.file_data = data
        
        if len(data) < 0x230:
            raise ValueError("VCL file too small")
        
        # First bank: 50 SFX
        vcl.sfx_offsets = list(struct.unpack('<50I', data[0:0xC8]))
        vcl.sfx_sizes = list(struct.unpack('<50H', data[0xC8:0x12C]))
        vcl.sfx_rates = list(struct.unpack('<50H', data[0x12C:0x190]))
        
        # Second bank: 40 music/pattern entries
        vcl.music_offsets = list(struct.unpack('<40I', data[0x190:0x190+0xA0]))
        vcl.music_sizes = list(struct.unpack('<40H', data[0x230:0x230+0x50]))
        
        return vcl
    
    def extract_sfx(self, index: int) -> Optional[bytes]:
        if 0 <= index < 50 and self.sfx_sizes[index] > 0:
            offset = self.sfx_offsets[index]
            size = self.sfx_sizes[index]
            if offset + size <= len(self.file_data):
                return self.file_data[offset:offset+size]
        return None
    
    def extract_music(self, index: int) -> Optional[bytes]:
        if 0 <= index < 40 and self.music_sizes[index] > 0:
            offset = self.music_offsets[index]
            size = self.music_sizes[index]
            if offset + size <= len(self.file_data):
                return self.file_data[offset:offset+size]
        return None

    # The second bank (0x190 / 0x230) is the text-page table, not music.
    # Keep the historical names above for API compatibility.
    def extract_page(self, index: int) -> Optional[bytes]:
        return self.extract_music(index)

    def parse_text_page(self, index: int) -> Optional[List[Dict[str, Any]]]:
        """Return [{'colour': 0..7, 'text': str}] for one text page."""
        data = self.extract_page(index)
        if not data:
            return None
        lines: List[Dict[str, Any]] = []
        line = bytearray()

        def flush():
            while line and (line[-1] < 0x20 or line[-1] >= 0x7F):
                line.pop()
            if not line:
                lines.append({'colour': 7, 'text': ''})
                return
            text = line.decode('ascii', errors='replace')
            colour = 7
            pos = 0
            if len(text) > 1 and '/' < text[0] < '8':
                colour = ord(text[0]) - ord('0')
                pos = 1
            lines.append({'colour': colour, 'text': text[pos:]})

        for b in data:
            if b == 0x0D:
                flush()
                line.clear()
            elif b == 0x0A:
                pass
            elif 0x20 <= b < 0x7F:
                line.append(b)
        if line:
            flush()
        while lines and not lines[-1]['text']:
            lines.pop()
        return lines or None

    def text_page_indexes(self) -> List[int]:
        return [i for i in range(40) if self.music_sizes[i] > 0]

    
    def get_info(self) -> str:
        sfx_count = sum(1 for s in self.sfx_sizes if s > 0)
        music_count = sum(1 for s in self.music_sizes if s > 0)
        return (f"VCL: {sfx_count} SFX, {music_count} music entries\n"
                f"Sample rates: {set(r for r in self.sfx_rates if r > 0)}")
    
    def create_wav(self, index: int, type_: str = "sfx") -> Optional[bytes]:
        """Create a WAV file from raw PCM data (8-bit unsigned, mono)"""
        if type_ == "sfx":
            data = self.extract_sfx(index)
            rate = self.sfx_rates[index] if index < 50 else 6000
        else:
            data = self.extract_music(index)
            rate = 6000  # Music doesn't have rate in header
        
        if not data:
            return None
        
        # The .VCL banks store unsigned 8-bit PCM (silence = 128), which is
        # exactly what an 8-bit WAV expects, so the bytes are written as-is.
        wav_buffer = io.BytesIO()
        with wave.open(wav_buffer, 'wb') as wav:
            wav.setnchannels(1)  # Mono
            wav.setsampwidth(1)  # 8-bit unsigned
            wav.setframerate(rate if rate > 0 else 6000)
            wav.writeframes(data)
        
        return wav_buffer.getvalue()
    
    def save_wav(self, index: int, filepath: str, type_: str = "sfx") -> bool:
        """Save sound as WAV file"""
        wav_data = self.create_wav(index, type_)
        if wav_data:
            with open(filepath, 'wb') as f:
                f.write(wav_data)
            return True
        return False
    
    def play_sound(self, index: int, type_: str = "sfx", async_: bool = True) -> bool:
        """Play sound using winsound (Windows only)"""
        if not HAS_WINSOUND:
            return False
        
        wav_data = self.create_wav(index, type_)
        if not wav_data:
            return False
        
        # Save to temp file and play
        try:
            with tempfile.NamedTemporaryFile(suffix='.wav', delete=False) as tmp:
                tmp.write(wav_data)
                tmp_path = tmp.name
            
            flags = winsound.SND_FILENAME
            if async_:
                flags |= winsound.SND_ASYNC
            winsound.PlaySound(tmp_path, flags)
            
            # Clean up temp file after a delay (for async)
            if async_:
                def cleanup():
                    import time
                    time.sleep(2)
                    try:
                        os.unlink(tmp_path)
                    except:
                        pass
                threading.Thread(target=cleanup, daemon=True).start()
            
            return True
        except Exception as e:
            print(f"Playback error: {e}")
            return False


class DDTFile:
    """*.ddt - Creative Music File (CMF)"""
    def __init__(self):
        self.data: bytes = b''
        self.is_cmf: bool = False
        self.version: int = 0
        self.instrument_offset: int = 0
        self.music_offset: int = 0
        self.ticks_per_beat: int = 0
    
    @classmethod
    def load(cls, filepath: str) -> 'DDTFile':
        with open(filepath, 'rb') as f:
            data = f.read()
        return cls.parse(data)
    
    @classmethod
    def parse(cls, data: bytes) -> 'DDTFile':
        ddt = cls()
        ddt.data = data
        
        if len(data) >= 4 and data[0:4] == b'CTMF':
            ddt.is_cmf = True
            if len(data) >= 0xF8:
                ddt.version = struct.unpack('<H', data[4:6])[0]
                ddt.instrument_offset = struct.unpack('<I', data[0x24:0x28])[0]
                ddt.music_offset = struct.unpack('<I', data[0xF4:0xF8])[0]
                ddt.ticks_per_beat = struct.unpack('<H', data[0xFA:0xFC])[0]
        
        return ddt


class CFGFile:
    """jill1.cfg / jill2.cfg / jill3.cfg - Config file (254 bytes)"""
    def __init__(self):
        self.high_score_names: List[str] = [""]*10
        self.high_scores: List[int] = [0]*10
        self.save_slot_labels: List[str] = [""]*6
        self.options: List[int] = [0]*11
    
    @classmethod
    def load(cls, filepath: str) -> 'CFGFile':
        with open(filepath, 'rb') as f:
            data = f.read()
        return cls.parse(data)
    
    @classmethod
    def parse(cls, data: bytes) -> 'CFGFile':
        cfg = cls()
        if len(data) < 254:
            return cfg
        
        # 10 x 10-byte names at offset 0
        for i in range(10):
            name_bytes = data[i*10:(i+1)*10].rstrip(b'\x00')
            cfg.high_score_names[i] = name_bytes.decode('ascii', errors='replace')
        
        # 10 x u32 scores at offset 0x78
        for i in range(10):
            cfg.high_scores[i] = struct.unpack('<I', data[0x78+i*4:0x7C+i*4])[0]
        
        # 6 x 12-byte save labels at offset 0xA0
        for i in range(6):
            label_bytes = data[0xA0+i*12:0xAC+i*12].rstrip(b'\x00')
            cfg.save_slot_labels[i] = label_bytes.decode('ascii', errors='replace')
        
        # 11 u16 options at offset 0xE8
        for i in range(11):
            if 0xE8 + i*2 + 2 <= len(data):
                cfg.options[i] = struct.unpack('<H', data[0xE8+i*2:0xEA+i*2])[0]
        
        return cfg


class MACFile:
    """*.mac - Recorded input macros/demos"""
    def __init__(self):
        self.records: List[Dict] = []
    
    @classmethod
    def load(cls, filepath: str) -> 'MACFile':
        with open(filepath, 'rb') as f:
            data = f.read()
        return cls.parse(data)
    
    @classmethod
    def parse(cls, data: bytes) -> 'MACFile':
        mac = cls()
        ptr = 0
        
        while ptr < len(data):
            # Read mask
            if ptr >= len(data):
                break
            mask = data[ptr]
            ptr += 1
            
            time = 0
            if mac.records:  # Not first record
                # Varint time
                time = 0
                shift = 0
                while ptr < len(data):
                    b = data[ptr]
                    ptr += 1
                    time |= (b & 0x7F) << shift
                    if (b & 0x80) == 0:
                        break
                    shift += 7
            
            # Read values for each set bit in mask
            values = {}
            bit_names = ['horiz_dir', 'vert_dir', 'action1', 'action2', 'ascii_key']
            for bit in range(5):
                if mask & (1 << bit):
                    if ptr < len(data):
                        values[bit_names[bit]] = data[ptr]
                        ptr += 1
            
            mac.records.append({
                'time': time,
                'mask': mask,
                'values': values
            })
        
        return mac


# ============================================================================
# ENHANCED ASSET EXPORTERS (fonts, text pages, palettes, tiles, HUD, packs)
# ============================================================================

FONT_ENTRY = 2
FONT_TRANSPARENT = (0, 3)  # the original text blitter skips 0 and 3
HUD_ICON_BANK = 0x0E


def _palette_rgb(sha: SHAFile, index: int) -> Tuple[int, int, int]:
    pal = sha.vga_palette
    i = max(0, min(255, index)) * 3
    if i + 2 < len(pal):
        return int(pal[i]), int(pal[i + 1]), int(pal[i + 2])
    return 0, 0, 0


def _entry_xlate(sha: SHAFile, entry_idx: int) -> List[int]:
    """Map raw frame indices to VGA palette indices.

    Shapes whose `flags & 1` is clear carry a small per-entry colour table
    (4, 8 or 16 entries).  The original blitter converts each raw pixel
    through it before writing to the 8-bit framebuffer, exactly like the
    C++ port's `ShapeBank::decode`.  Without this, 4bpp tiles/shapes get
    mapped straight to the global palette and change colour.
    """
    table = list(range(256))
    if entry_idx < 0 or entry_idx >= 128:
        return table
    entry = sha.entries[entry_idx]
    if entry is None or (entry.flags & 1) or entry.src_bpp >= 8:
        return table
    for i, pal_entry in enumerate(entry.palette):
        if i < 256:
            table[i] = (pal_entry >> 16) & 0xFF
    return table


def _shape_rgba(sha: SHAFile, entry_idx: int, frame_idx: int,
                skip=(0,), colour: Optional[Tuple[int, int, int]] = None) -> Optional[Image.Image]:
    """Decode one frame to RGBA, making `skip` palette indices transparent."""
    raw = sha.frame_indices(entry_idx, frame_idx)
    if not raw:
        return None
    w, h, px = raw
    xlate = _entry_xlate(sha, entry_idx)
    img = Image.new('RGBA', (w, h), (0, 0, 0, 0))
    out = img.load()
    for y in range(h):
        for x in range(w):
            idx = px[y * w + x]
            if idx in skip:
                continue
            r, g, b = _palette_rgb(sha, xlate[idx])
            if colour is not None:
                r, g, b = colour
            out[x, y] = (r, g, b, 255)
    return img


def _render_text(img: Image.Image, sha: SHAFile, x: int, y: int,
                 text: str, colour: Tuple[int, int, int],
                 advance: int = 6) -> None:
    """Draw text using the game font (shape entry 2), as the engine does."""
    out = img.load()
    for i, ch in enumerate(text):
        code = ord(ch) & 0x7F
        raw = sha.frame_indices(FONT_ENTRY, code)
        if not raw:
            continue
        w, h, px = raw
        for gy in range(h):
            for gx in range(w):
                idx = px[gy * w + gx]
                if idx in FONT_TRANSPARENT:
                    continue
                tx, ty = x + i * advance + gx, y + gy
                if 0 <= tx < img.width and 0 <= ty < img.height:
                    out[tx, ty] = (colour[0], colour[1], colour[2], 255)


def export_font_assets(sha: SHAFile, out_dir: Path) -> int:
    """Export `fonts/main.png` (128-cell strip) plus per-glyph PNGs."""
    entry = sha.entries[FONT_ENTRY]
    if entry is None or not entry.frames:
        return 0
    frames = []
    for code in range(128):
        raw = sha.frame_indices(FONT_ENTRY, code) if code < entry.frame_count else None
        frames.append(raw)
    widths = [f[0] for f in frames if f]
    heights = [f[1] for f in frames if f]
    cell_w = max(widths) if widths else 6
    cell_h = max(heights) if heights else 6

    font_dir = Path(out_dir) / "fonts"
    font_dir.mkdir(parents=True, exist_ok=True)
    sheet = Image.new('RGBA', (cell_w * 128, cell_h), (0, 0, 0, 0))
    sheet_px = sheet.load()
    glyph_dir = font_dir / "glyphs"
    glyph_dir.mkdir(exist_ok=True)

    meta = {'entry': FONT_ENTRY, 'frames': entry.frame_count,
            'cell_width': cell_w, 'cell_height': cell_h,
            'transparent_indices': list(FONT_TRANSPARENT), 'glyphs': []}
    count = 0
    for code, raw in enumerate(frames):
        if not raw:
            continue
        w, h, px = raw
        glyph = Image.new('RGBA', (w, h), (0, 0, 0, 0))
        gpx = glyph.load()
        for y in range(h):
            for x in range(w):
                if px[y * w + x] not in FONT_TRANSPARENT:
                    gpx[x, y] = (255, 255, 255, 255)
                    sheet_px[code * cell_w + x, y] = (255, 255, 255, 255)
        glyph.save(glyph_dir / f"glyph_{code:03d}.png")
        meta['glyphs'].append({'code': code, 'width': w, 'height': h})
        count += 1
    sheet.save(font_dir / "main.png")
    with open(font_dir / "font.json", 'w', encoding='utf-8') as f:
        json.dump(meta, f, indent=2)
    return count


def _page_colour(code: int) -> int:
    if code == 1:
        return 9
    if code == 0:
        return 8
    return max(0, min(15, code))


def export_text_page_assets(vcl: VCLFile, sha: Optional[SHAFile],
                            out_dir: Path) -> int:
    """Export text pages as .txt, JSON and optional font-rendered previews."""
    page_dir = Path(out_dir) / "text_pages"
    preview_dir = page_dir / "preview"
    page_dir.mkdir(parents=True, exist_ok=True)
    preview_dir.mkdir(exist_ok=True)
    entries = []
    count = 0
    for n in vcl.text_page_indexes():
        lines = vcl.parse_text_page(n)
        if not lines:
            continue
        title = lines[0]['text'] if lines else ''
        with open(page_dir / f"page_{n:02d}.txt", 'w', encoding='ascii',
                  errors='replace') as f:
            for line in lines:
                f.write(line['text'] + "\n")
        entries.append({'index': n, 'title': title, 'lines': lines})
        if sha is not None and sha.entries[FONT_ENTRY] is not None:
            bg = _palette_rgb(sha, 0)
            frame = _palette_rgb(sha, 8)
            img = Image.new('RGBA', (320, 200), (*bg, 255))
            d = ImageDraw.Draw(img)
            d.rectangle([6, 4, 313, 195], fill=(*frame, 255))
            d.rectangle([7, 5, 312, 194], fill=(*bg, 255))
            title_col = _palette_rgb(sha, _page_colour(lines[0]['colour']))
            tx = max(12, (320 - len(title) * 6) // 2)
            _render_text(img, sha, tx, 11, title, title_col)
            d.line([14, 21, 305, 21], fill=(*frame, 255))
            y = 27
            for line in lines[1:]:
                if line['text']:
                    col = _palette_rgb(sha, _page_colour(line['colour']))
                    x = max(10, (320 - len(line['text']) * 6) // 2)
                    _render_text(img, sha, x, y, line['text'], col)
                y += 9
            _render_text(img, sha, 14, 187, f"PAGE {n}", _palette_rgb(sha, 8))
            img.save(preview_dir / f"page_{n:02d}.png")
        count += 1
    with open(page_dir / "pages.json", 'w', encoding='utf-8') as f:
        json.dump(entries, f, indent=2)
    return count


def export_palette_assets(sha: SHAFile, out_dir: Path) -> bool:
    """Write the 8-bit DAC palette as .pal, .bin (6-bit) and JSON."""
    pal_dir = Path(out_dir)
    pal_dir.mkdir(parents=True, exist_ok=True)
    data = bytes(max(0, min(255, int(v))) for v in sha.vga_palette[:768])
    if len(data) < 768:
        data = data + b'\x00' * (768 - len(data))
    (pal_dir / "palette.pal").write_bytes(data)
    six = bytes(max(0, min(63, round(v * 63 / 255))) for v in data)
    (pal_dir / "palette_6bit.bin").write_bytes(six)
    entries = [{'index': i, 'rgb': list(data[i * 3:i * 3 + 3]),
                'hex': '#%02X%02X%02X' % tuple(data[i * 3:i * 3 + 3])}
               for i in range(256)]
    with open(pal_dir / "palette.json", 'w', encoding='utf-8') as f:
        json.dump(entries, f, indent=2)
    return True


def _dma_by_tile(dma: DMAFile) -> Dict[int, DMARecord]:
    return {rec.tile_id: rec for rec in dma.records}


def _tile_rgba(sha: SHAFile, rec: DMARecord,
               cache: Dict[Tuple[int, int], Optional[Image.Image]]) -> Optional[Image.Image]:
    shape = rec.shape_id
    entry = (shape >> 8) & 0x3F
    index = shape & 0xFF
    key = (entry, index)
    if key not in cache:
        cache[key] = _shape_rgba(sha, entry, index, skip=(0,))
    return cache[key]


def export_tile_assets(sha: SHAFile, dma: DMAFile, out_dir: Path) -> int:
    """Export every DMA tile as `tiles/<tile id>.png` plus tiles.json."""
    tile_dir = Path(out_dir) / "tiles"
    tile_dir.mkdir(parents=True, exist_ok=True)
    cache: Dict[Tuple[int, int], Optional[Image.Image]] = {}
    meta = []
    count = 0
    for rec in dma.records:
        img = _tile_rgba(sha, rec, cache)
        if img is None:
            continue
        img.save(tile_dir / f"{rec.tile_id}.png")
        meta.append({'tile_id': rec.tile_id, 'index': dma.records.index(rec),
                     'shape_id': rec.shape_id, 'attr': rec.attr,
                     'name': rec.name, 'width': img.width, 'height': img.height})
        count += 1
    with open(tile_dir / "tiles.json", 'w', encoding='utf-8') as f:
        json.dump(meta, f, indent=2)
    return count


def _render_board(board: BoardFile, sha: SHAFile, dma: DMAFile) -> Image.Image:
    """Composite a board's tile layer into a 2048x1024 RGBA image."""
    canvas = Image.new('RGBA', (128 * 16, 64 * 16), (*_palette_rgb(sha, 0), 255))
    dma_map = _dma_by_tile(dma)
    cache: Dict[Tuple[int, int], Optional[Image.Image]] = {}
    for c in range(128):
        for r in range(64):
            tid = board.get_tile_at(c, r)
            rec = dma_map.get(tid)
            if rec is None:
                continue
            tile = _tile_rgba(sha, rec, cache)
            if tile is not None:
                canvas.paste(tile, (c * 16, r * 16), tile)
    return canvas


def export_board_previews(game: Dict[str, Any], sha: SHAFile, dma: DMAFile,
                          out_dir: Path) -> int:
    """Write full board renders and 320x200 pack-ready backgrounds."""
    data_dir = Path(game['data_dir'])
    ext = game['board_ext']
    full_dir = Path(out_dir) / "board_previews"
    pack_dir = Path(out_dir) / "boards"
    full_dir.mkdir(parents=True, exist_ok=True)
    pack_dir.mkdir(parents=True, exist_ok=True)
    targets = []
    for name in game.get('boards', []):
        p = data_dir / f"{name}{ext}"
        if p.exists():
            targets.append(p)
    try:
        for p in sorted(data_dir.glob(f"*{ext}")):
            if p not in targets:
                targets.append(p)
    except OSError:
        pass
    resample = getattr(Image, 'Resampling', Image).NEAREST
    meta = []
    count = 0
    for p in targets:
        try:
            board = BoardFile.load(str(p))
            img = _render_board(board, sha, dma)
            img.save(full_dir / f"{p.stem}.png")
            # Nearest-neighbour keeps every pixel on the game palette; a
            # filtered downscale would invent blended colours that the engine
            # then quantises again (the "mangled colours" seen in packs).
            small = img.resize((320, 200), resample)
            small.save(pack_dir / f"{p.stem}.png")
            meta.append({'board': p.stem, 'width': img.width, 'height': img.height})
            count += 1
        except Exception as e:
            print(f"  ERROR rendering {p.name}: {e}")
    with open(pack_dir / "boards.json", 'w', encoding='utf-8') as f:
        json.dump(meta, f, indent=2)
    return count


def export_theme_assets(game: Dict[str, Any], sha: SHAFile, dma: DMAFile,
                        out_dir: Path) -> bool:
    """Export the intro-board menu panel as `themes/intro.png`."""
    data_dir = Path(game['data_dir'])
    ext = game['board_ext']
    intro = None
    for name in game.get('boards', []):
        if name.lower() == 'intro':
            p = data_dir / f"{name}{ext}"
            if p.exists():
                intro = p
                break
    if intro is None:
        p = data_dir / f"intro{ext}"
        if p.exists():
            intro = p
    if intro is None:
        return False
    try:
        board = BoardFile.load(str(intro))
        full = _render_board(board, sha, dma)
        # Port menu panel: tiles (36,0)..(55,13), centred vertically.
        crop = full.crop((36 * 16, 12, 36 * 16 + 320, 12 + 200))
        theme_dir = Path(out_dir) / "themes"
        theme_dir.mkdir(parents=True, exist_ok=True)
        crop.save(theme_dir / "intro.png")
        return True
    except Exception as e:
        print(f"  ERROR rendering intro theme: {e}")
        return False


def export_hud_assets(sha: SHAFile, out_dir: Path,
                      icon_bank: int = HUD_ICON_BANK) -> int:
    """Export HUD panel backgrounds, previews and icon frames."""
    hud_dir = Path(out_dir) / "hud"
    icon_dir = hud_dir / "icons"
    icon_dir.mkdir(parents=True, exist_ok=True)
    bg = (16, 24, 48)
    edge = (90, 160, 230)
    text = (235, 235, 235)

    classic = Image.new('RGBA', (64, 84), (*bg, 255))
    d = ImageDraw.Draw(classic)
    d.rectangle([0, 0, 63, 83], outline=(*edge, 255))
    classic.save(hud_dir / "classic.png")
    classic_prev = classic.copy()
    _render_text(classic_prev, sha, 2, 2, "HEALTH", text)
    for i in range(6):
        pip = _shape_rgba(sha, icon_bank, 0x2A, skip=(0,))
        if pip:
            classic_prev.paste(pip, (0x2A + i * 3, 2), pip)
    last = _shape_rgba(sha, icon_bank, 0x2B, skip=(0,))
    if last:
        classic_prev.paste(last, (0x28 + 5 * 3, 2), last)
    _render_text(classic_prev, sha, 0x21, 10, "SCORE", text)
    _render_text(classic_prev, sha, 0x40 - 7, 16, "0", text)
    _render_text(classic_prev, sha, 1, 10, "LEVEL", text)
    _render_text(classic_prev, sha, 1, 16, "0", text)
    classic_prev.save(hud_dir / "classic_preview.png")

    modern = Image.new('RGBA', (320, 12), (*bg, 255))
    ImageDraw.Draw(modern).rectangle([0, 10, 319, 11], fill=(*edge, 255))
    modern.save(hud_dir / "modern.png")
    modern_prev = modern.copy()
    _render_text(modern_prev, sha, 2, 3, "LEVEL 0", text)
    _render_text(modern_prev, sha, 163, 3, "HEALTH", text)
    for i in range(7):
        pip = _shape_rgba(sha, icon_bank, 0x2A, skip=(0,))
        if pip:
            modern_prev.paste(pip, (203 + 2 + i * 3, 3), pip)
    first = _shape_rgba(sha, icon_bank, 0x2B, skip=(0,))
    if first:
        modern_prev.paste(first, (203, 3), first)
    _render_text(modern_prev, sha, 276, 3, "SCORE 0", text)
    modern_prev.save(hud_dir / "modern_preview.png")

    icons = sha.entries[icon_bank]
    count = 0
    if icons is not None:
        for frame in range(icons.frame_count):
            img = _shape_rgba(sha, icon_bank, frame, skip=(0,))
            if img:
                img.save(icon_dir / f"icon_{frame:03d}.png")
                count += 1
    with open(hud_dir / "hud.json", 'w', encoding='utf-8') as f:
        json.dump({'classic_size': [64, 84], 'modern_size': [320, 12],
                   'icon_bank': icon_bank, 'icons': count}, f, indent=2)
    return count


def export_portrait_asset(sha: SHAFile, out_dir: Path) -> bool:
    """Export `portrait.png` (64x84) used beside level-title dialogs.

    The default is composed from the player's idle sprite (the same art the
    original title screen uses for Jill), framed and scaled 2x.
    """
    sprite = _shape_rgba(sha, 8, 0x10, skip=(0,))
    if sprite is None:
        return False
    resample = getattr(Image, 'Resampling', Image).NEAREST
    big = sprite.resize((sprite.width * 2, sprite.height * 2), resample)
    frame = Image.new('RGBA', (64, 84), (16, 24, 48, 255))
    d = ImageDraw.Draw(frame)
    d.rectangle([0, 0, 63, 83], outline=(90, 160, 230, 255))
    x = (64 - big.width) // 2
    y = (84 - big.height) // 2
    frame.paste(big, (x, y), big)
    out_dir.mkdir(parents=True, exist_ok=True)
    frame.save(out_dir / "portrait.png")
    with open(out_dir / "portrait.json", 'w', encoding='utf-8') as f:
        json.dump({'size': [64, 84], 'source': 'shape entry 8 frame 0x10',
                   'scale': 2}, f, indent=2)
    return True


def export_reference_pack(game: Dict[str, Any], sha: SHAFile, vcl: VCLFile,
                          dma: DMAFile, out_dir: Path,
                          music_seconds: float = MUSIC_EXPORT_SECONDS) -> Dict[str, int]:
    """Write a ready-to-edit `external/<id>` style pack tree."""
    out = Path(out_dir)
    sprite_dir = out / "sprites"
    sprite_dir.mkdir(parents=True, exist_ok=True)
    stats: Dict[str, int] = {}

    # Every shape frame under the pack's shape_<entry>_<frame> name.
    shapes = 0
    for entry_idx in range(128):
        entry = sha.entries[entry_idx]
        if entry is None:
            continue
        for frame_idx in range(entry.frame_count):
            skip = FONT_TRANSPARENT if entry_idx == FONT_ENTRY else (0,)
            img = _shape_rgba(sha, entry_idx, frame_idx, skip=skip)
            if img:
                img.save(sprite_dir / f"shape_{entry_idx:03d}_{frame_idx:03d}.png")
                shapes += 1
    stats['sprites'] = shapes

    stats['fonts'] = export_font_assets(sha, out)
    stats['pages'] = export_text_page_assets(vcl, sha, out)
    stats['tiles'] = export_tile_assets(sha, dma, out)
    stats['boards'] = export_board_previews(game, sha, dma, out)
    export_theme_assets(game, sha, dma, out)
    export_palette_assets(sha, out)

    # Pack page art: blank framed pages, with the text kept beside them.
    pages_dir = out / "pages"
    pages_dir.mkdir(exist_ok=True)
    src_pages = out / "text_pages"
    for p in sorted(src_pages.glob("page_*.txt")):
        n = int(p.stem.split('_')[1])
        bg = _palette_rgb(sha, 0)
        frame = _palette_rgb(sha, 8)
        img = Image.new('RGBA', (320, 200), (*bg, 255))
        d = ImageDraw.Draw(img)
        d.rectangle([6, 4, 313, 195], fill=(*frame, 255))
        d.rectangle([7, 5, 312, 194], fill=(*bg, 255))
        d.line([14, 21, 305, 21], fill=(*frame, 255))
        img.save(pages_dir / f"page_{n:02d}.png")

    # Sound effects as WAV, the format the pack loader expects.
    sfx_dir = out / "sfx"
    sfx_dir.mkdir(exist_ok=True)
    sfx_count = 0
    for i in range(50):
        wav = vcl.create_wav(i, "sfx")
        if wav:
            (sfx_dir / f"{i}.wav").write_bytes(wav)
            sfx_count += 1
    stats['sfx'] = sfx_count
    # Music: OPL2-rendered WAVs named after the original .ddt track stems.
    stats['music'] = export_music_wavs(game, out, music_seconds)

    icon_bank = game.get('hud_icon_bank', HUD_ICON_BANK)
    stats['hud_icons'] = export_hud_assets(sha, out, icon_bank)
    stats['portrait'] = 1 if export_portrait_asset(sha, out) else 0

    with open(out / "pack.ini", 'w', encoding='ascii') as f:
        f.write("; Generated reference pack - edit or replace these files.\n")
        f.write("[pack]\n")
        f.write(f"name = {game.get('title', game['id'])} Reference\n")
        f.write(f"game = {game['id']}\n")
    return stats


# ============================================================================
# EPISODE PROFILES
# ============================================================================

EPISODE_PROFILES = {
    1: {
        'name': 'Jill of the Jungle',
        'exe': 'jill1.exe',
        'sha': 'jill1.sha',
        'vcl': 'jill1.vcl',
        'cfg': 'jill1.cfg',
        'dma': 'jill.dma',
        'board_ext': '.jn1',
        'save_prefix': 'jn1save',
        'title_music': 'funky.ddt',
        'boards': ['1', '2', '3', '4', '6', '9', '50', 'map', 'intro'],
        'kind_count': 64,
    },
    2: {
        'name': 'Jill Goes Underground',
        'exe': 'jill2.exe',
        'sha': 'jill2.sha',
        'vcl': 'jill2.vcl',
        'cfg': 'jill2.cfg',
        'dma': 'jill.dma',
        'board_ext': '.jn2',
        'save_prefix': 'jn2save',
        'title_music': 'peaks.ddt',
        'boards': ['0', '1', '3', '9', '10', '11', '13', '15', '17', '20', 'intro'],
        'kind_count': 69,
    },
    3: {
        'name': 'Jill Saves The Prince',
        'exe': 'jill3.exe',
        'sha': 'jill3.sha',
        'vcl': 'jill3.vcl',
        'cfg': 'jill3.cfg',
        'dma': 'jill.dma',
        'board_ext': '.jn3',
        'save_prefix': 'jn3save',
        'title_music': 'wild.ddt',
        'boards': [str(i) for i in range(1, 15)] + ['map', 'intro'],
        'kind_count': 69,
    }
}


# ============================================================================
# GAME DISCOVERY (port layout: jill.ini + games/*.game manifests)
# ============================================================================

def parse_ini(path: Path) -> Dict[str, Dict[str, str]]:
    """Parse a simple INI file into {section: {key: value}} (keys lowercased)."""
    out: Dict[str, Dict[str, str]] = {}
    section = ""
    try:
        text = path.read_text(encoding='utf-8', errors='replace')
    except OSError:
        return out
    for raw in text.splitlines():
        line = raw.strip()
        if not line or line[0] in '#;':
            continue
        if line.startswith('[') and line.endswith(']'):
            section = line[1:-1].strip().lower()
            out.setdefault(section, {})
            continue
        if '=' in line:
            k, v = line.split('=', 1)
            out.setdefault(section, {})[k.strip().lower()] = v.strip()
    return out


def _episode_for_profile(profile: str) -> int:
    p = (profile or "").lower()
    if p.endswith("1") or p == "jill1":
        return 1
    if p.endswith("2") or p == "jill2":
        return 2
    if p.endswith("3") or p == "jill3":
        return 3
    return 3  # Onesimus is a Jill 3 conversion


def _game_from_episode(episode: int, data_dir: Path) -> Dict[str, Any]:
    prof = EPISODE_PROFILES[episode]
    return {
        'id': f"jill{episode}",
        'title': prof['name'],
        'profile': f"jill{episode}",
        'episode': episode,
        'data_dir': data_dir,
        'sha': prof['sha'],
        'dma': prof['dma'],
        'vcl': prof['vcl'],
        'cfg': prof['cfg'],
        'board_ext': prof['board_ext'],
        'boards': list(prof['boards']),
        'palette': None,
        'title_music': prof['title_music'],
    }


def discover_games(base_dir: Path) -> List[Dict[str, Any]]:
    """Discover installed games from jill.ini + games/*.game manifests.

    Falls back to the legacy <base>/jillt directory with the three built-in
    episode profiles when no manifests are present.
    """
    games: List[Dict[str, Any]] = []
    base_dir = Path(base_dir)
    ini = parse_ini(base_dir / 'jill.ini')
    path_sec = ini.get('paths', {})
    games_dir = base_dir / path_sec.get('games', 'games')

    manifests = sorted(games_dir.glob('*.game')) if games_dir.is_dir() else []
    seen_ids = set()

    for mf in manifests:
        # .game manifests are flat key=value files (no sections).
        m = parse_ini(mf).get('', {})
        gid = m.get('id') or mf.stem
        if gid in seen_ids:
            continue
        seen_ids.add(gid)
        profile = m.get('profile', gid)
        episode = _episode_for_profile(profile)
        prof = EPISODE_PROFILES.get(episode, EPISODE_PROFILES[1])

        data_field = m.get('data', 'jillt')
        candidates = []
        if path_sec.get(gid):
            candidates.append(Path(path_sec[gid]))
        candidates.append(base_dir / data_field)
        candidates.append(base_dir / 'jillt')
        data_dir = None
        for c in candidates:
            if c and c.is_dir():
                data_dir = c
                break
        if data_dir is None:
            data_dir = candidates[0] if candidates else (base_dir / 'jillt')

        boards = [b.strip() for b in m.get('boards', '').split(',') if b.strip()]
        pal = m.get('palette')
        games.append({
            'id': gid,
            'title': m.get('title', prof['name']),
            'profile': profile,
            'episode': episode,
            'data_dir': data_dir,
            'sha': m.get('shapes', prof['sha']),
            'dma': m.get('tiles', prof['dma']),
            'vcl': m.get('pages', prof['vcl']),
            'cfg': prof['cfg'] if profile in (f"jill{e}" for e in (1, 2, 3)) else None,
            'board_ext': m.get('board_ext', prof['board_ext']),
            'boards': boards or list(prof['boards']),
            'palette': (base_dir / pal) if pal else None,
            'title_music': m.get('title_music', prof['title_music']),
        })

    if games:
        return games

    # Legacy / fallback layout
    for e in (1, 2, 3):
        d = base_dir / 'jillt'
        if d.is_dir():
            games.append(_game_from_episode(e, d))
    if not games:
        games.append(_game_from_episode(1, base_dir / 'jillt'))
    return games


# ============================================================================
# GUI APPLICATION
# ============================================================================

class JillExtractorApp:
    def __init__(self, root, base_dir: Optional[Path] = None):
        self.root = root
        self.root.title("Jill of the Jungle - Game File Viewer & Extractor")
        self.root.geometry("1200x800")
        
        # State
        self.base_dir = Path(base_dir) if base_dir else Path(__file__).resolve().parent
        self.games = discover_games(self.base_dir)
        self.current_game_idx = 0
        self.current_game = self.games[0]
        self.current_episode = self.current_game['episode']
        self.game_path = Path(self.current_game['data_dir'])
        self.board_paths: List[Path] = []
        self.sha_file: Optional[SHAFile] = None
        self.dma_file: Optional[DMAFile] = None
        self.vcl_file: Optional[VCLFile] = None
        self.cfg_file: Optional[CFGFile] = None
        self.current_board: Optional[BoardFile] = None
        self.current_ddt: Optional[DDTFile] = None
        self.current_mac: Optional[MACFile] = None
        
        # Image cache
        self.tile_images: Dict[Any, ImageTk.PhotoImage] = {}
        self.shape_images: Dict[Tuple[int, int], ImageTk.PhotoImage] = {}
        
        self.setup_ui()
        self.load_game_files()
    
    def setup_ui(self):
        # Menu bar
        menubar = tk.Menu(self.root)
        self.root.config(menu=menubar)
        
        file_menu = tk.Menu(menubar, tearoff=0)
        menubar.add_cascade(label="File", menu=file_menu)
        file_menu.add_command(label="Open Game Directory...", command=self.choose_game_dir)
        file_menu.add_separator()
        file_menu.add_command(label="Exit", command=self.root.quit)
        
        game_menu = tk.Menu(menubar, tearoff=0)
        menubar.add_cascade(label="Game", menu=game_menu)
        for i, g in enumerate(self.games):
            game_menu.add_command(
                label=g['title'],
                command=lambda idx=i: self.select_game(idx)
            )
        game_menu.add_separator()
        game_menu.add_command(label="Rescan Games", command=self.rescan_games)
        
        tools_menu = tk.Menu(menubar, tearoff=0)
        menubar.add_cascade(label="Tools", menu=tools_menu)
        tools_menu.add_command(label="Batch Extract All Boards...", command=self.batch_extract_boards)
        tools_menu.add_command(label="Batch Extract All Shapes...", command=self.batch_extract_shapes)
        tools_menu.add_command(label="Batch Extract All Sounds...", command=self.batch_extract_sounds)
        tools_menu.add_separator()
        tools_menu.add_command(label="Batch Extract Fonts...", command=self.batch_extract_fonts)
        tools_menu.add_command(label="Batch Extract Text Pages...", command=self.batch_extract_text_pages)
        tools_menu.add_command(label="Batch Extract Tile Images...", command=self.batch_extract_tiles)
        tools_menu.add_command(label="Batch Extract HUD + Theme...", command=self.batch_extract_hud)
        tools_menu.add_command(label="Batch Extract Music (WAV)...", command=self.batch_extract_music_wavs)
        tools_menu.add_command(label="Batch Extract Board Previews...", command=self.batch_extract_board_previews)
        tools_menu.add_command(label="Export Palette...", command=self.export_palette)
        tools_menu.add_separator()
        tools_menu.add_command(label="Export Reference Asset Pack...", command=self.export_reference_pack)
        
        import_menu = tk.Menu(menubar, tearoff=0)
        menubar.add_cascade(label="Convert", menu=import_menu)
        import_menu.add_command(label="Import Image into Shape Frame...",
                                command=self.import_image_to_shape)
        import_menu.add_command(label="Build .sha from PNG Folder...",
                                command=self.build_sha_from_folder)
        import_menu.add_separator()
        import_menu.add_command(label="Import WAV into Sound Slot...",
                                command=self.import_wav_to_vcl)
        import_menu.add_command(label="Build .vcl from WAV Folder...",
                                command=self.build_vcl_from_folder)
        
        # Main layout
        main_paned = ttk.PanedWindow(self.root, orient=tk.HORIZONTAL)
        main_paned.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        # Left panel - File browser
        left_frame = ttk.Frame(main_paned)
        main_paned.add(left_frame, weight=1)
        
        # Episode indicator
        self.episode_label = ttk.Label(left_frame, text="Episode: Jill 1", font=('Arial', 10, 'bold'))
        self.episode_label.pack(anchor=tk.W, padx=5, pady=5)
        
        # Notebook for file categories
        self.left_notebook = ttk.Notebook(left_frame)
        self.left_notebook.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        # Boards tab
        self.setup_boards_tab()
        
        # Shapes tab
        self.setup_shapes_tab()
        
        # Sounds tab
        self.setup_sounds_tab()
        
        # Config/DMA tab
        self.setup_config_tab()
        
        # Right panel - Viewer
        right_frame = ttk.Frame(main_paned)
        main_paned.add(right_frame, weight=2)
        
        self.right_notebook = ttk.Notebook(right_frame)
        self.right_notebook.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        # Board viewer
        self.setup_board_viewer()
        
        # Shape viewer
        self.setup_shape_viewer()
        
        # Sound viewer
        self.setup_sound_viewer()
        
        # Hex viewer
        self.setup_hex_viewer()
        
        # Keep the viewer tab in step with the file category chosen on the left.
        self.left_notebook.bind("<<NotebookTabChanged>>",
                                self.on_left_tab_changed)
        
        # Status bar
        self.status_var = tk.StringVar(value="Ready")
        status_bar = ttk.Label(self.root, textvariable=self.status_var, relief=tk.SUNKEN, anchor=tk.W)
        status_bar.pack(fill=tk.X, side=tk.BOTTOM, padx=5, pady=5)
    
    def on_left_tab_changed(self, event=None):
        """Mirror the left file-category tab on the right viewer notebook."""
        try:
            idx = self.left_notebook.index("current")
        except tk.TclError:
            return
        # Boards, Shapes, Sounds, Config/DMA -> Board, Shape, Sound, Hex.
        mapping = {0: 0, 1: 1, 2: 2, 3: 3}
        target = mapping.get(idx)
        if target is None:
            return
        try:
            if target < len(self.right_notebook.tabs()):
                self.right_notebook.select(target)
        except tk.TclError:
            pass

    def setup_boards_tab(self):
        frame = ttk.Frame(self.left_notebook)
        self.left_notebook.add(frame, text="Boards")
        
        # Toolbar
        toolbar = ttk.Frame(frame)
        toolbar.pack(fill=tk.X, padx=5, pady=5)
        ttk.Button(toolbar, text="Refresh", command=self.refresh_boards).pack(side=tk.LEFT, padx=2)
        ttk.Button(toolbar, text="Extract Selected", command=self.extract_selected_board).pack(side=tk.LEFT, padx=2)
        ttk.Button(toolbar, text="Extract All", command=self.batch_extract_boards).pack(side=tk.LEFT, padx=2)
        
        # Board list
        list_frame = ttk.Frame(frame)
        list_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        self.board_listbox = tk.Listbox(list_frame, font=('Consolas', 9))
        self.board_listbox.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        self.board_listbox.bind('<<ListboxSelect>>', self.on_board_select)
        self.board_listbox.bind('<Double-Button-1>', self.on_board_double_click)
        
        scrollbar = ttk.Scrollbar(list_frame, orient=tk.VERTICAL, command=self.board_listbox.yview)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        self.board_listbox.config(yscrollcommand=scrollbar.set)
        
        # Board info
        self.board_info_text = scrolledtext.ScrolledText(frame, height=8, font=('Consolas', 9))
        self.board_info_text.pack(fill=tk.X, padx=5, pady=5)
    
    def setup_shapes_tab(self):
        frame = ttk.Frame(self.left_notebook)
        self.left_notebook.add(frame, text="Shapes")
        
        toolbar = ttk.Frame(frame)
        toolbar.pack(fill=tk.X, padx=5, pady=5)
        ttk.Button(toolbar, text="Refresh", command=self.refresh_shapes).pack(side=tk.LEFT, padx=2)
        ttk.Button(toolbar, text="Extract Selected", command=self.extract_selected_shape).pack(side=tk.LEFT, padx=2)
        ttk.Button(toolbar, text="Extract All", command=self.batch_extract_shapes).pack(side=tk.LEFT, padx=2)
        
        # Shape list with info
        list_frame = ttk.Frame(frame)
        list_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        self.shape_listbox = tk.Listbox(list_frame, font=('Consolas', 9))
        self.shape_listbox.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        self.shape_listbox.bind('<<ListboxSelect>>', self.on_shape_select)
        
        scrollbar = ttk.Scrollbar(list_frame, orient=tk.VERTICAL, command=self.shape_listbox.yview)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        self.shape_listbox.config(yscrollcommand=scrollbar.set)
        
        # Shape info
        self.shape_info_text = scrolledtext.ScrolledText(frame, height=8, font=('Consolas', 9))
        self.shape_info_text.pack(fill=tk.X, padx=5, pady=5)
    
    def setup_sounds_tab(self):
        frame = ttk.Frame(self.left_notebook)
        self.left_notebook.add(frame, text="Sounds")
        
        toolbar = ttk.Frame(frame)
        toolbar.pack(fill=tk.X, padx=5, pady=5)
        ttk.Button(toolbar, text="Refresh", command=self.refresh_sounds).pack(side=tk.LEFT, padx=2)
        ttk.Button(toolbar, text="Extract Selected", command=self.extract_selected_sound).pack(side=tk.LEFT, padx=2)
        ttk.Button(toolbar, text="Extract All", command=self.batch_extract_sounds).pack(side=tk.LEFT, padx=2)
        
        # Sound list
        list_frame = ttk.Frame(frame)
        list_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        self.sound_listbox = tk.Listbox(list_frame, font=('Consolas', 9))
        self.sound_listbox.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        self.sound_listbox.bind('<<ListboxSelect>>', self.on_sound_select)
        
        scrollbar = ttk.Scrollbar(list_frame, orient=tk.VERTICAL, command=self.sound_listbox.yview)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        self.sound_listbox.config(yscrollcommand=scrollbar.set)
        
        # Sound info
        self.sound_info_text = scrolledtext.ScrolledText(frame, height=8, font=('Consolas', 9))
        self.sound_info_text.pack(fill=tk.X, padx=5, pady=5)
    
    def setup_config_tab(self):
        frame = ttk.Frame(self.left_notebook)
        self.left_notebook.add(frame, text="Config/DMA")
        
        # DMA list
        ttk.Label(frame, text="jill.dma - Tile Definitions", font=('Arial', 9, 'bold')).pack(anchor=tk.W, padx=5, pady=5)
        
        dma_frame = ttk.Frame(frame)
        dma_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        self.dma_listbox = tk.Listbox(dma_frame, font=('Consolas', 9), height=15)
        self.dma_listbox.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        self.dma_listbox.bind('<<ListboxSelect>>', self.on_dma_select)
        
        scrollbar = ttk.Scrollbar(dma_frame, orient=tk.VERTICAL, command=self.dma_listbox.yview)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        self.dma_listbox.config(yscrollcommand=scrollbar.set)
        
        # Config info
        ttk.Label(frame, text="Config File", font=('Arial', 9, 'bold')).pack(anchor=tk.W, padx=5, pady=5)
        self.config_text = scrolledtext.ScrolledText(frame, height=10, font=('Consolas', 9))
        self.config_text.pack(fill=tk.X, padx=5, pady=5)
    
    def setup_board_viewer(self):
        frame = ttk.Frame(self.right_notebook)
        self.right_notebook.add(frame, text="Board Viewer")
        
        # Toolbar
        toolbar = ttk.Frame(frame)
        toolbar.pack(fill=tk.X, padx=5, pady=5)
        ttk.Label(toolbar, text="Zoom:").pack(side=tk.LEFT)
        self.zoom_var = tk.DoubleVar(value=1.0)
        zoom_scale = ttk.Scale(toolbar, from_=0.5, to=4.0, variable=self.zoom_var, 
                              orient=tk.HORIZONTAL, length=150, command=self.on_zoom_change)
        zoom_scale.pack(side=tk.LEFT, padx=5)
        ttk.Button(toolbar, text="Show Grid", command=self.toggle_grid).pack(side=tk.LEFT, padx=5)
        ttk.Button(toolbar, text="Show Objects", command=self.toggle_objects).pack(side=tk.LEFT, padx=5)
        ttk.Button(toolbar, text="Export PNG", command=self.export_board_png).pack(side=tk.LEFT, padx=5)
        
        # Canvas with scrollbars
        canvas_frame = ttk.Frame(frame)
        canvas_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        self.board_canvas = tk.Canvas(canvas_frame, bg='#202020', highlightthickness=0)
        canvas_frame.rowconfigure(0, weight=1)
        canvas_frame.columnconfigure(0, weight=1)
        self.board_canvas.grid(row=0, column=0, sticky="nsew")
        
        h_scroll = ttk.Scrollbar(canvas_frame, orient=tk.HORIZONTAL,
                                 command=self.board_canvas.xview)
        h_scroll.grid(row=1, column=0, sticky="ew")
        v_scroll = ttk.Scrollbar(canvas_frame, orient=tk.VERTICAL,
                                 command=self.board_canvas.yview)
        v_scroll.grid(row=0, column=1, sticky="ns")
        
        self.board_canvas.config(xscrollcommand=h_scroll.set, yscrollcommand=v_scroll.set)
        self.board_canvas.bind('<Button-1>', self.on_board_canvas_click)
        self.board_canvas.bind('<Motion>', self.on_board_canvas_motion)
        
        self.show_grid = True
        self.show_objects = True
        
        # Tile info panel
        self.tile_info_text = scrolledtext.ScrolledText(frame, height=6, font=('Consolas', 9))
        self.tile_info_text.pack(fill=tk.X, padx=5, pady=5)
    
    def setup_shape_viewer(self):
        frame = ttk.Frame(self.right_notebook)
        self.right_notebook.add(frame, text="Shape Viewer")
        
        # Animation state
        self.anim_playing = False
        self.anim_timer = None
        self.anim_fps = 10
        self.anim_entry_idx = 0
        self.anim_frame = 0
        # Cache of decoded+scaled PhotoImages: (entry, frame, mode, w, h)
        self._shape_frame_cache: Dict[Tuple[int, int, int, int, int], ImageTk.PhotoImage] = {}
        
        toolbar = ttk.Frame(frame)
        toolbar.pack(fill=tk.X, padx=5, pady=5)
        ttk.Label(toolbar, text="Frame:").pack(side=tk.LEFT)
        self.frame_var = tk.IntVar(value=0)
        self.frame_spin = ttk.Spinbox(toolbar, from_=0, to=0, textvariable=self.frame_var,
                                       width=6, command=self.on_frame_change)
        self.frame_spin.pack(side=tk.LEFT, padx=5)
        # Keep the spinbox and the animation position in sync for every input
        # path (arrow buttons, typed value, mouse wheel, keyboard).
        self.frame_spin.bind('<Return>', lambda e: self.on_frame_change())
        self.frame_spin.bind('<FocusOut>', lambda e: self.on_frame_change())
        self.frame_spin.bind('<KeyRelease>', lambda e: self.on_frame_change())
        self.frame_spin.bind('<Up>', lambda e: self.step_frame(1))
        self.frame_spin.bind('<Down>', lambda e: self.step_frame(-1))
        self.frame_status = ttk.Label(toolbar, text="0/0", width=8)
        self.frame_status.pack(side=tk.LEFT, padx=2)

        # Step buttons make frame navigation independent of spinbox quirks.
        ttk.Button(toolbar, text="-", width=2,
                   command=lambda: self.step_frame(-1)).pack(side=tk.LEFT)
        ttk.Button(toolbar, text="+", width=2,
                   command=lambda: self.step_frame(1)).pack(side=tk.LEFT, padx=(0, 4))
        self.grid_var = tk.IntVar(value=0)
        self.grid_mode = False
        self._grid_cells = []
        self._grid_photos = []
        ttk.Checkbutton(toolbar, text="Grid", variable=self.grid_var,
                        command=self.on_grid_toggle).pack(side=tk.LEFT, padx=4)
        
        ttk.Label(toolbar, text="Mode:").pack(side=tk.LEFT, padx=5)
        self.mode_var = tk.IntVar(value=4)
        ttk.Radiobutton(toolbar, text="VGA (8bpp)", variable=self.mode_var, value=4, 
                       command=self.on_mode_change).pack(side=tk.LEFT)
        ttk.Radiobutton(toolbar, text="EGA (4bpp)", variable=self.mode_var, value=2, 
                       command=self.on_mode_change).pack(side=tk.LEFT)
        ttk.Radiobutton(toolbar, text="CGA (2bpp)", variable=self.mode_var, value=0, 
                       command=self.on_mode_change).pack(side=tk.LEFT)
        
        # Animation controls
        ttk.Separator(toolbar, orient=tk.VERTICAL).pack(side=tk.LEFT, fill=tk.Y, padx=10)
        self.anim_btn = ttk.Button(toolbar, text="> Play", command=self.toggle_animation, width=8)
        self.anim_btn.pack(side=tk.LEFT, padx=5)
        ttk.Label(toolbar, text="FPS:").pack(side=tk.LEFT)
        self.fps_var = tk.IntVar(value=10)
        ttk.Spinbox(toolbar, from_=1, to=60, textvariable=self.fps_var, width=4, 
                   command=self.on_fps_change).pack(side=tk.LEFT, padx=2)
        
        ttk.Button(toolbar, text="Export Frame", command=self.export_shape_frame).pack(side=tk.LEFT, padx=10)
        ttk.Button(toolbar, text="Export All Frames", command=self.export_all_shape_frames).pack(side=tk.LEFT, padx=5)
        
        # Shape preview with checkerboard background for transparency
        canvas_frame = ttk.Frame(frame)
        canvas_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        self.shape_canvas = tk.Canvas(canvas_frame, highlightthickness=0)
        self.shape_canvas.pack(fill=tk.BOTH, expand=True)
        self.shape_canvas.bind('<Configure>', self.on_shape_canvas_resize)
        self.shape_canvas.bind('<Button-1>', self.on_shape_canvas_click)
        
        # Draw checkerboard pattern for transparency visualization
        self.draw_checkerboard()
        
        # Shape details
        self.shape_detail_text = scrolledtext.ScrolledText(frame, height=8, font=('Consolas', 9))
        self.shape_detail_text.pack(fill=tk.X, padx=5, pady=5)
    
    def draw_checkerboard(self, force: bool = False):
        """Draw (or refresh) the transparency checkerboard once per size.

        The animation loop redraws sprites many times per second, so this must
        not rebuild thousands of canvas rectangles on every frame.
        """
        w = self.shape_canvas.winfo_width()
        h = self.shape_canvas.winfo_height()
        if w <= 1 or h <= 1:
            self.shape_canvas.after(100, self.draw_checkerboard)
            return
        if not force and getattr(self, '_checker_size', None) == (w, h):
            return
        self._checker_size = (w, h)
        self.shape_canvas.delete("checkerboard")
        square_size = 8
        colors = ['#303030', '#404040']
        for y in range(0, h, square_size):
            for x in range(0, w, square_size):
                color = colors[((x // square_size) + (y // square_size)) % 2]
                self.shape_canvas.create_rectangle(x, y, x + square_size, y + square_size,
                                                  fill=color, outline='', tags="checkerboard")
        self.shape_canvas.tag_lower("checkerboard")
    
    def on_fps_change(self):
        try:
            self.anim_fps = max(1, min(60, int(self.fps_var.get())))
        except (tk.TclError, ValueError):
            self.anim_fps = 10
        if getattr(self, 'anim_playing', False):
            # Restart the timer so the new speed takes effect immediately.
            if self.anim_timer:
                try:
                    self.root.after_cancel(self.anim_timer)
                except Exception:
                    pass
                self.anim_timer = None
            self._schedule_animation()
    
    def toggle_animation(self):
        if not self.sha_file:
            return
        if getattr(self, 'grid_mode', False):
            self.grid_var.set(0)
            self.on_grid_toggle()
        selection = self.shape_listbox.curselection()
        if not selection:
            return
        idx = selection[0]
        if idx >= 128 or not self.sha_file.entries[idx]:
            return
        
        entry = self.sha_file.entries[idx]
        if entry.frame_count <= 1:
            self.status_var.set("This shape has only one frame - nothing to animate")
            return
        
        if self.anim_playing:
            self.stop_animation()
        else:
            self.start_animation(idx)
    
    def start_animation(self, entry_idx: int):
        # Cancel any previous timer first so we never run two loops.
        if self.anim_timer:
            try:
                self.root.after_cancel(self.anim_timer)
            except Exception:
                pass
            self.anim_timer = None
        self.anim_playing = True
        self.anim_entry_idx = entry_idx
        # Start from the frame currently shown in the spinbox, keeping the
        # animation position and the frame number connected.
        entry = self.sha_file.entries[entry_idx]
        try:
            start = int(self.frame_var.get())
        except (tk.TclError, ValueError):
            start = 0
        self.anim_frame = max(0, min(entry.frame_count - 1, start))
        self.anim_btn.config(text="|| Stop")
        self._draw_anim_frame()
        self._schedule_animation()
    
    def stop_animation(self):
        self.anim_playing = False
        self.anim_btn.config(text="> Play")
        if self.anim_timer:
            try:
                self.root.after_cancel(self.anim_timer)
            except Exception:
                pass
            self.anim_timer = None
    
    def _schedule_animation(self):
        if not self.anim_playing:
            return
        delay = max(1, int(1000 / max(1, self.anim_fps)))
        self.anim_timer = self.root.after(delay, self._anim_tick)
    
    def _anim_tick(self):
        if not self.anim_playing:
            return
        entry = self.sha_file.entries[self.anim_entry_idx]
        if not entry:
            self.stop_animation()
            return
        self.anim_frame = (self.anim_frame + 1) % entry.frame_count
        self._draw_anim_frame()
        self._schedule_animation()
    
    def _draw_anim_frame(self):
        self.frame_var.set(self.anim_frame)
        self._update_frame_status()
        # Skip the (more expensive) detail/hex panel while animating.
        self.draw_shape(self.anim_entry_idx, self.anim_frame, update_detail=False)
    
    def _update_frame_status(self):
        try:
            if self.sha_file and self.anim_entry_idx < 128 and self.sha_file.entries[self.anim_entry_idx]:
                n = self.sha_file.entries[self.anim_entry_idx].frame_count
                if getattr(self, 'anim_playing', False):
                    frame = self.anim_frame
                else:
                    frame = int(self.frame_var.get())
                self.frame_status.config(text=f"{frame + 1}/{n}")
        except Exception:
            pass
    
    def draw_shape(self, entry_idx: int, frame_idx: int, update_detail: bool = True):
        if not self.sha_file or entry_idx >= 128 or not self.sha_file.entries[entry_idx]:
            return
        
        entry = self.sha_file.entries[entry_idx]
        if frame_idx >= len(entry.frames):
            return
        
        mode = self.mode_var.get()
        
        # Grid mode: show every frame of this entry at once.
        if getattr(self, 'grid_mode', False):
            self.draw_frame_grid(entry_idx, frame_idx, update_detail)
            return
        self.shape_canvas.delete("grid")
        self._grid_cells = []
        self._grid_photos = []
        
        # Only ensure the checkerboard exists; never rebuild it per frame.
        self.draw_checkerboard()
        
        # Determine target size from the canvas, then use a cached PhotoImage
        # so animation playback does not re-encode the image every tick.
        canvas_w = self.shape_canvas.winfo_width()
        canvas_h = self.shape_canvas.winfo_height()
        if canvas_w <= 1 or canvas_h <= 1:
            canvas_w = canvas_h = 400
        frame = entry.frames[frame_idx]
        img_w = max(1, frame['width'])
        img_h = max(1, frame['height'])
        scale = min(canvas_w / img_w, canvas_h / img_h, 8.0)
        new_w = max(1, int(img_w * scale))
        new_h = max(1, int(img_h * scale))
        
        cache_key = (entry_idx, frame_idx, mode, new_w, new_h)
        photo = self._shape_frame_cache.get(cache_key)
        if photo is None:
            img = self.sha_file.decode_frame_to_image(entry_idx, frame_idx, mode)
            if img is None:
                self.shape_canvas.delete("sprite")
                if update_detail:
                    self.shape_detail_text.delete(1.0, tk.END)
                    self.shape_detail_text.insert(tk.END, "Could not decode frame")
                return
            img = img.resize((new_w, new_h), Image.NEAREST)
            photo = ImageTk.PhotoImage(img)
            self._shape_frame_cache[cache_key] = photo
        
        if photo:
            self._shape_photo = photo  # keep a strong reference
            item = self.shape_canvas.find_withtag("sprite")
            if item:
                self.shape_canvas.itemconfig(item[0], image=photo)
                self.shape_canvas.coords(item[0], canvas_w // 2, canvas_h // 2)
            else:
                self.shape_canvas.create_image(canvas_w // 2, canvas_h // 2,
                                               anchor=tk.CENTER, image=photo, tags="sprite")
            self.shape_canvas.image = photo
            
            if update_detail:
                # Update detail text (skipped while animating for performance)
                frame = entry.frames[frame_idx]
                detail = []
                detail.append(f"Frame {frame_idx}/{entry.frame_count-1}")
                detail.append(f"  Size: {frame['width']}x{frame['height']}")
                detail.append(f"  Opcode: {frame['opcode']}")
                detail.append(f"  Data length: {len(frame['data'])} bytes")
                detail.append(f"  Video mode: {'VGA 8bpp' if mode==4 else 'EGA 4bpp' if mode==2 else 'CGA 2bpp'}")
                
                if frame['opcode'] == 0 and frame['data']:
                    detail.append(f"  First 32 bytes: {frame['data'][:32].hex()}")
                
                self.shape_detail_text.delete(1.0, tk.END)
                self.shape_detail_text.insert(tk.END, '\n'.join(detail))
        else:
            self.shape_canvas.delete("sprite")
            self.shape_detail_text.delete(1.0, tk.END)
            self.shape_detail_text.insert(tk.END, "Could not decode frame (format not fully supported)")
    
    def step_frame(self, delta: int):
        """Move one frame forward/back without relying on Spinbox internals."""
        idx = self._selected_shape_index()
        if idx is None:
            return "break"
        try:
            current = int(self.frame_var.get())
        except (tk.TclError, ValueError):
            try:
                current = int(self.frame_spin.get())
            except (tk.TclError, ValueError):
                current = 0
        frame = self._clamp_frame(idx, current + delta)
        self.frame_var.set(frame)
        if getattr(self, 'anim_playing', False):
            self.anim_frame = frame
            self._update_frame_status()
            if self.anim_timer:
                try:
                    self.root.after_cancel(self.anim_timer)
                except Exception:
                    pass
                self.anim_timer = None
            self._draw_anim_frame()
            self._schedule_animation()
        else:
            self._update_frame_status()
            self.draw_shape(idx, frame)
        return "break"

    def on_grid_toggle(self):
        self.grid_mode = bool(self.grid_var.get())
        if self.grid_mode:
            self.stop_animation()
        idx = self._selected_shape_index()
        if idx is None:
            return
        try:
            frame = self._clamp_frame(idx, int(self.frame_spin.get()))
        except (tk.TclError, ValueError):
            frame = 0
        self.frame_var.set(frame)
        self.draw_shape(idx, frame)

    def on_shape_canvas_click(self, event):
        if not getattr(self, 'grid_mode', False):
            return
        idx = self._selected_shape_index()
        if idx is None:
            return
        for x0, y0, cw, ch, fi in self._grid_cells:
            if x0 <= event.x < x0 + cw and y0 <= event.y < y0 + ch:
                self.frame_var.set(fi)
                self.anim_frame = fi
                self._update_frame_status()
                self.draw_shape(idx, fi)
                break

    def draw_frame_grid(self, entry_idx: int, frame_idx: int,
                        update_detail: bool = True):
        entry = self.sha_file.entries[entry_idx]
        frames = entry.frames
        n = len(frames)
        if n == 0:
            return
        mode = self.mode_var.get()
        self.draw_checkerboard()
        self.shape_canvas.delete("sprite")
        self.shape_canvas.delete("grid")
        cw = self.shape_canvas.winfo_width()
        ch = self.shape_canvas.winfo_height()
        if cw <= 2 or ch <= 2:
            cw, ch = 640, 400
        max_fw = max(1, max(f['width'] for f in frames))
        max_fh = max(1, max(f['height'] for f in frames))
        # Pick the column count that gives the largest per-frame scale.
        best = None
        for cols in range(1, min(n, 16) + 1):
            rows = (n + cols - 1) // cols
            cell_w = cw / cols
            cell_h = ch / rows
            scale = min((cell_w - 8) / max_fw, (cell_h - 18) / max_fh)
            if best is None or scale > best[0] + 1e-6:
                best = (scale, cols, rows)
        _, cols, rows = best
        cell_w = max(1, cw // cols)
        cell_h = max(1, ch // rows)
        label_h = 10
        pad = 4
        self._grid_cells = []
        self._grid_photos = []
        for i, f in enumerate(frames):
            r, c = divmod(i, cols)
            x0 = c * cell_w
            y0 = r * cell_h
            img = self.sha_file.decode_frame_to_image(entry_idx, i, mode)
            if img is None:
                continue
            avail_w = max(1, cell_w - 2 * pad)
            avail_h = max(1, cell_h - 2 * pad - label_h)
            scale = min(avail_w / max(1, img.width),
                        avail_h / max(1, img.height), 4.0)
            scale = max(scale, 0.05)
            nw = max(1, int(img.width * scale))
            nh = max(1, int(img.height * scale))
            img = img.resize((nw, nh), Image.NEAREST)
            photo = ImageTk.PhotoImage(img)
            self._grid_photos.append(photo)
            self.shape_canvas.create_image(
                x0 + cell_w // 2, y0 + pad + (cell_h - label_h) // 2,
                image=photo, anchor=tk.CENTER, tags="grid")
            self.shape_canvas.create_text(
                x0 + 3, y0 + cell_h - label_h, text=f"0x{i:02X}",
                anchor=tk.NW, fill="#dddddd", font=("Consolas", 7),
                tags="grid")
            self._grid_cells.append((x0, y0, cell_w, cell_h, i))
            if i == frame_idx:
                self.shape_canvas.create_rectangle(
                    x0 + 1, y0 + 1, x0 + cell_w - 2, y0 + cell_h - 2,
                    outline="#ffcc00", width=2, tags="grid")
        if update_detail:
            f = frames[min(frame_idx, n - 1)]
            detail = [f"Grid view: {n} frame(s), {cols}x{rows}",
                      f"Selected: {frame_idx}  {f['width']}x{f['height']}",
                      "Click a cell to select that frame."]
            self.shape_detail_text.delete(1.0, tk.END)
            self.shape_detail_text.insert(tk.END, "\n".join(detail))
        self._update_frame_status()

    def setup_sound_viewer(self):
        frame = ttk.Frame(self.right_notebook)
        self.right_notebook.add(frame, text="Sound Viewer")
        
        # Playback controls
        toolbar = ttk.Frame(frame)
        toolbar.pack(fill=tk.X, padx=5, pady=5)
        
        if HAS_WINSOUND:
            ttk.Button(toolbar, text="Play Selected", command=self.play_selected_sound).pack(side=tk.LEFT, padx=2)
            ttk.Button(toolbar, text="Stop", command=self.stop_sound).pack(side=tk.LEFT, padx=2)
            ttk.Button(toolbar, text="Save as WAV", command=self.save_selected_sound_wav).pack(side=tk.LEFT, padx=2)
        else:
            ttk.Label(toolbar, text="Playback requires Windows (winsound)").pack(side=tk.LEFT, padx=5)
        
        ttk.Button(toolbar, text="Export Raw", command=self.extract_selected_sound).pack(side=tk.LEFT, padx=10)
        
        self.sound_detail_text = scrolledtext.ScrolledText(frame, font=('Consolas', 9))
        self.sound_detail_text.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
    
    def play_selected_sound(self):
        selection = self.sound_listbox.curselection()
        if not selection:
            return
        text = self.sound_listbox.get(selection[0])
        
        if text.startswith("SFX "):
            if not self.vcl_file:
                return
            parts = text.split()
            sfx_idx = int(parts[1])
            if self.vcl_file.play_sound(sfx_idx, "sfx"):
                self.status_var.set(f"Playing SFX {sfx_idx}")
            else:
                self.status_var.set("Playback failed")
        elif text.startswith("Music "):
            if not self.vcl_file:
                return
            parts = text.split()
            music_idx = int(parts[1])
            if self.vcl_file.play_sound(music_idx, "music"):
                self.status_var.set(f"Playing Music {music_idx}")
            else:
                self.status_var.set("Playback failed")
        elif text.lower().endswith((".ddt", ".ark")):
            filename = text.split()[-1]
            path = self.game_path / filename
            tmp = Path(tempfile.gettempdir()) / f"jill_preview_{Path(filename).stem}.wav"
            ok, msg = render_cmf_wav(self.current_game, path, tmp, 30.0)
            if ok and play_wav_file(tmp):
                self.status_var.set(f"Playing {filename} (port OPL2 render)")
            else:
                self.status_var.set(msg if not ok else "Playback failed")
                if not ok:
                    messagebox.showwarning("Music playback", msg)
    
    def stop_sound(self):
        if HAS_WINSOUND:
            winsound.PlaySound(None, winsound.SND_PURGE)
            self.status_var.set("Playback stopped")
    
    def save_selected_sound_wav(self):
        selection = self.sound_listbox.curselection()
        if not selection:
            return
        text = self.sound_listbox.get(selection[0])
        
        if text.startswith("SFX "):
            if not self.vcl_file:
                return
            parts = text.split()
            sfx_idx = int(parts[1])
            default_name = f"sfx_{sfx_idx}.wav"
            type_ = "sfx"
        elif text.startswith("Music "):
            if not self.vcl_file:
                return
            parts = text.split()
            music_idx = int(parts[1])
            default_name = f"music_{music_idx}.wav"
            type_ = "music"
        elif text.lower().endswith((".ddt", ".ark")):
            filename = text.split()[-1]
            filepath = filedialog.asksaveasfilename(
                defaultextension=".wav",
                filetypes=[("WAV", "*.wav")],
                initialfile=f"{Path(filename).stem}.wav")
            if not filepath:
                return
            ok, msg = render_cmf_wav(self.current_game, self.game_path / filename,
                                     Path(filepath), MUSIC_EXPORT_SECONDS)
            if ok:
                self.status_var.set(f"Saved WAV to {filepath}")
            else:
                messagebox.showerror("Music export", msg)
            return
        else:
            return
        
        filepath = filedialog.asksaveasfilename(defaultextension=".wav",
                                                 filetypes=[("WAV", "*.wav")],
                                                 initialfile=default_name)
        if filepath:
            if self.vcl_file.save_wav(sfx_idx if type_ == "sfx" else music_idx, filepath, type_):
                self.status_var.set(f"Saved WAV to {filepath}")
            else:
                messagebox.showerror("Error", "Failed to save WAV")
    
    def setup_hex_viewer(self):
        frame = ttk.Frame(self.right_notebook)
        self.right_notebook.add(frame, text="Hex Viewer")
        
        toolbar = ttk.Frame(frame)
        toolbar.pack(fill=tk.X, padx=5, pady=5)
        ttk.Button(toolbar, text="Load File...", command=self.load_hex_file).pack(side=tk.LEFT, padx=2)
        ttk.Label(toolbar, text="Offset:").pack(side=tk.LEFT, padx=5)
        self.hex_offset_var = tk.StringVar(value="0")
        ttk.Entry(toolbar, textvariable=self.hex_offset_var, width=10).pack(side=tk.LEFT)
        ttk.Button(toolbar, text="Go", command=self.goto_hex_offset).pack(side=tk.LEFT, padx=2)
        
        self.hex_text = scrolledtext.ScrolledText(frame, font=('Consolas', 9))
        self.hex_text.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
    
    def load_game_files(self):
        """Load all game files for the selected game."""
        game = self.current_game
        base_path = self.game_path
        
        # Reset
        self.sha_file = None
        self.dma_file = None
        self.vcl_file = None
        self.cfg_file = None
        self.current_board = None
        self.tile_images.clear()
        self.shape_images.clear()
        if hasattr(self, '_shape_frame_cache'):
            self._shape_frame_cache.clear()
        
        # Load SHA
        sha_path = base_path / game['sha']
        if sha_path.exists():
            try:
                self.sha_file = SHAFile.load(str(sha_path))
                # Optional custom palette (e.g. Onesimus)
                pal_path = game.get('palette')
                if pal_path and Path(pal_path).is_file():
                    try:
                        pal_data = Path(pal_path).read_bytes()
                        self.sha_file.set_palette_rgb(pal_data, from_6bit=(max(pal_data) <= 63))
                    except Exception:
                        pass
                self.status_var.set(f"Loaded {game['sha']}: {sum(1 for e in self.sha_file.entries if e)} entries")
            except Exception as e:
                self.status_var.set(f"Error loading SHA: {e}")
        else:
            self.status_var.set(f"SHA not found: {sha_path}")
        
        # Load DMA
        dma_path = base_path / game['dma']
        if dma_path.exists():
            try:
                self.dma_file = DMAFile.load(str(dma_path))
            except Exception as e:
                print(f"Error loading DMA: {e}")
        
        # Load VCL
        vcl_path = base_path / game['vcl']
        if vcl_path.exists():
            try:
                self.vcl_file = VCLFile.load(str(vcl_path))
            except Exception as e:
                print(f"Error loading VCL: {e}")
        
        # Load CFG (optional)
        cfg_name = game.get('cfg')
        if cfg_name:
            cfg_path = base_path / cfg_name
            if cfg_path.exists():
                try:
                    self.cfg_file = CFGFile.load(str(cfg_path))
                except Exception as e:
                    print(f"Error loading CFG: {e}")
        
        self.episode_label.config(text=f"Game: {game['title']}")
        self.refresh_all()
    
    def select_game(self, index: int):
        if 0 <= index < len(self.games):
            self.current_game_idx = index
            self.current_game = self.games[index]
            self.current_episode = self.current_game['episode']
            self.game_path = Path(self.current_game['data_dir'])
            self.load_game_files()
    
    def rescan_games(self):
        self.games = discover_games(self.base_dir)
        if self.games:
            self.current_game_idx = min(self.current_game_idx, len(self.games) - 1)
            self.current_game = self.games[self.current_game_idx]
            self.current_episode = self.current_game['episode']
            self.game_path = Path(self.current_game['data_dir'])
            self.load_game_files()
    
    def choose_game_dir(self):
        dir_path = filedialog.askdirectory(initialdir=str(self.game_path))
        if dir_path:
            # Interpret the chosen folder as the asset data directory.
            self.game_path = Path(dir_path)
            self.current_game = dict(self.current_game)
            self.current_game['data_dir'] = self.game_path
            self.games[self.current_game_idx] = self.current_game
            self.load_game_files()
    
    def refresh_all(self):
        self.refresh_boards()
        self.refresh_shapes()
        self.refresh_sounds()
        self.refresh_dma()
        self.refresh_config()
    
    def refresh_boards(self):
        self.board_listbox.delete(0, tk.END)
        self.board_paths = []
        game = self.current_game
        base_path = self.game_path
        ext = game['board_ext']
        
        boards = []
        for board_name in game['boards']:
            board_path = base_path / f"{board_name}{ext}"
            if board_path.exists():
                boards.append(board_path)
        
        # Also check for any other board files
        try:
            for f in sorted(base_path.glob(f"*{ext}")):
                if f not in boards:
                    boards.append(f)
        except OSError:
            pass
        
        seen = set()
        for board_path in boards:
            if board_path in seen:
                continue
            seen.add(board_path)
            try:
                board = BoardFile.load(str(board_path))
                info = f"{board_path.name} - {len(board.objects)} objs, {len(board.strings)} strings"
                self.board_listbox.insert(tk.END, info)
            except Exception as e:
                info = f"{board_path.name} - ERROR: {e}"
                self.board_listbox.insert(tk.END, info)
            self.board_paths.append(board_path)
    
    def on_board_select(self, event):
        selection = self.board_listbox.curselection()
        if not selection:
            return
        idx = selection[0]
        if idx >= len(getattr(self, 'board_paths', [])):
            return
        board_path = self.board_paths[idx]
        
        if board_path.exists():
            try:
                self.current_board = BoardFile.load(str(board_path))
                self.display_board_info()
                self.draw_board()
            except Exception as e:
                self.board_info_text.delete(1.0, tk.END)
                self.board_info_text.insert(tk.END, f"Error loading board: {e}")
    
    def on_board_double_click(self, event):
        self.on_board_select(event)
        self.right_notebook.select(0)  # Switch to board viewer
    
    def display_board_info(self):
        if not self.current_board:
            return
        
        board = self.current_board
        info = []
        info.append(f"File: {board.filename}")
        info.append(f"Tile Map: 128x64 = {len(board.tile_map)} cells")
        info.append(f"Objects: {len(board.objects)} (max 192)")
        info.append(f"Strings: {len(board.strings)}")
        info.append("")
        info.append("=== Board Header ===")
        h = board.header
        info.append(f"Level ID: {h.level_id} (0x{h.level_id:04X})")
        info.append(f"Health: {h.health}")
        info.append(f"Key Count: {h.key_count}")
        info.append(f"Key IDs: {h.key_ids[:h.key_count]}")
        info.append(f"Score: {h.score}")
        info.append(f"Damage Flash: {h.damage_flash}")
        info.append("")
        info.append("=== Objects ===")
        for i, obj in enumerate(board.objects):
            flags = obj.get_flag_names()
            flag_str = f" [{', '.join(flags)}]" if flags else ""
            string_info = f" str='{obj.attached_string}'" if obj.attached_string else ""
            info.append(f"  [{i:3d}] Kind {obj.kind:2d} ({obj.get_kind_name():12s}) "
                       f"pos=({obj.x:5d},{obj.y:5d}) vel=({obj.x_delta:4d},{obj.y_delta:4d}) "
                       f"size=({obj.width:3d},{obj.height:3d}) state={obj.state} "
                       f"param={obj.primary_param} flags=0x{obj.flags:04X}{flag_str}{string_info}")
        
        if board.strings:
            info.append("")
            info.append("=== Strings ===")
            for i, s in enumerate(board.strings):
                info.append(f"  [{i}] '{s}'")
        
        self.board_info_text.delete(1.0, tk.END)
        self.board_info_text.insert(tk.END, '\n'.join(info))
    
    def draw_board(self):
        if not self.current_board or not self.sha_file or not self.dma_file:
            return
        
        board = self.current_board
        self.board_canvas.delete("all")
        
        zoom = self.zoom_var.get()
        tile_size = int(16 * zoom)
        map_width = 128 * tile_size
        map_height = 64 * tile_size
        
        self.board_canvas.config(scrollregion=(0, 0, map_width, map_height))
        
        # Draw tile map
        for x in range(128):
            for y in range(64):
                tile_id = board.get_tile_at(x, y)
                flags = board.get_tile_flags(x, y)
                
                # Get shape for this tile
                dma_rec = self.dma_file.get_record(tile_id)
                shape_id = dma_rec.shape_id if dma_rec else 0x4700
                
                # Try to get shape image
                img = self.get_tile_image(tile_id, shape_id, tile_size)
                
                px = x * tile_size
                py = y * tile_size
                
                if img:
                    self.board_canvas.create_image(px, py, anchor=tk.NW, image=img, tags="tiles")
                else:
                    # Fallback: colored rectangle based on tile ID
                    color = self.get_tile_color(tile_id, flags)
                    self.board_canvas.create_rectangle(px, py, px+tile_size, py+tile_size,
                                                      fill=color, outline='', tags="tiles")
                
                # Grid
                if self.show_grid and zoom >= 1.0:
                    self.board_canvas.create_rectangle(px, py, px+tile_size, py+tile_size,
                                                      outline='#404040', tags="grid")
        
        # Draw objects
        if self.show_objects:
            for i, obj in enumerate(board.objects):
                px = int(obj.x * zoom)
                py = int(obj.y * zoom)
                w = int(obj.width * zoom)
                h = int(obj.height * zoom)
                
                color = self.get_object_color(obj.kind)
                self.board_canvas.create_rectangle(px, py, px+w, py+h,
                                                  outline=color, width=2, tags="objects")
                self.board_canvas.create_text(px + w//2, py + h//2, 
                                             text=f"{i}", fill='white', font=('Arial', 8), tags="objects")
    
    def get_tile_image(self, tile_id: int, shape_id: int, size: int) -> Optional[ImageTk.PhotoImage]:
        # The .dma shape id encodes bank (SHA entry) in the high byte and the
        # frame index in the low byte: bank = (shape_id >> 8) & 0x3F, frame =
        # shape_id & 0xFF (see re/render_board.py).
        bank = (shape_id >> 8) & 0x3F
        frame = shape_id & 0xFF
        cache_key = (bank, frame, size)
        if cache_key in self.tile_images:
            return self.tile_images[cache_key]
        
        img = None
        try:
            img = self.sha_file.decode_frame_to_image(bank, frame, 4)
        except Exception:
            img = None
        
        if img is None:
            return None
        
        img = img.resize((size, size), Image.NEAREST)
        photo = ImageTk.PhotoImage(img)
        self.tile_images[cache_key] = photo
        return photo
    
    def get_tile_color(self, tile_id: int, flags: int) -> str:
        # Color coding based on tile ID ranges and flags
        if flags & 0xC000:
            return '#404080'  # Blocking
        if tile_id == 0:
            return '#101010'  # Empty
        elif tile_id < 0x100:
            return '#204020'  # Terrain
        elif tile_id < 0x200:
            return '#403020'  # Bricks/wood
        elif tile_id < 0x300:
            return '#303050'  # Castle/structures
        else:
            return '#202040'
    
    def get_object_color(self, kind: int) -> str:
        colors = {
            0: '#00FF00',    # Player - green
            1: '#FFD700',    # Apple - gold
            2: '#FF0000',    # Knife - red
            7: '#FF00FF',    # Demon - magenta
            12: '#00FFFF',   # Checkpoint - cyan
            14: '#FFFF00',   # Key - yellow
            33: '#00FF00',   # Gem - green
        }
        return colors.get(kind, '#FFFFFF')
    
    def on_zoom_change(self, event):
        self.draw_board()
    
    def toggle_grid(self):
        self.show_grid = not self.show_grid
        self.draw_board()
    
    def toggle_objects(self):
        self.show_objects = not self.show_objects
        self.draw_board()
    
    def on_board_canvas_click(self, event):
        if not self.current_board:
            return
        zoom = self.zoom_var.get()
        tile_size = int(16 * zoom)
        x = int(self.board_canvas.canvasx(event.x) // tile_size)
        y = int(self.board_canvas.canvasy(event.y) // tile_size)
        
        if 0 <= x < 128 and 0 <= y < 64:
            tile_id = self.current_board.get_tile_at(x, y)
            flags = self.current_board.get_tile_flags(x, y)
            dma_rec = self.dma_file.get_record(tile_id) if self.dma_file else None
            
            info = []
            info.append(f"Tile at ({x}, {y}):")
            info.append(f"  Tile ID: {tile_id} (0x{tile_id:04X})")
            info.append(f"  Flags: 0x{flags:02X} ({'Blocking' if flags & 3 else 'Open'})")
            if dma_rec:
                info.append(f"  DMA Record: shape=0x{dma_rec.shape_id:04X}, attr=0x{dma_rec.attr:04X}, name='{dma_rec.name}'")
            
            # Check objects at this position
            info.append("")
            info.append("Objects at this position:")
            for i, obj in enumerate(self.current_board.objects):
                if (obj.x <= x*16 < obj.x + obj.width and 
                    obj.y <= y*16 < obj.y + obj.height):
                    info.append(f"  [{i}] {obj.get_kind_name()} (kind {obj.kind})")
            
            self.tile_info_text.delete(1.0, tk.END)
            self.tile_info_text.insert(tk.END, '\n'.join(info))
    
    def on_board_canvas_motion(self, event):
        # Could show live coords
        pass
    
    def export_board_png(self):
        if not self.current_board:
            return
        filepath = filedialog.asksaveasfilename(defaultextension=".png",
                                                 filetypes=[("PNG", "*.png")],
                                                 initialfile=f"{self.current_board.filename}.png")
        if filepath:
            # Render board to image
            zoom = 1.0
            tile_size = 16
            img = Image.new('RGBA', (128*tile_size, 64*tile_size), (0, 0, 0, 255))
            
            for x in range(128):
                for y in range(64):
                    tile_id = self.current_board.get_tile_at(x, y)
                    color = self.get_tile_color(tile_id, self.current_board.get_tile_flags(x, y))
                    # Parse hex color
                    r = int(color[1:3], 16)
                    g = int(color[3:5], 16)
                    b = int(color[5:7], 16)
                    for tx in range(tile_size):
                        for ty in range(tile_size):
                            img.putpixel((x*tile_size+tx, y*tile_size+ty), (r, g, b, 255))
            
            img.save(filepath)
            self.status_var.set(f"Exported board to {filepath}")
    
    def extract_selected_board(self):
        selection = self.board_listbox.curselection()
        if not selection:
            return
        idx = selection[0]
        item_text = self.board_listbox.get(idx)
        board_name = item_text.split(' - ')[0]
        board_path = self.game_path / board_name
        
        if board_path.exists():
            out_dir = filedialog.askdirectory()
            if out_dir:
                try:
                    board = BoardFile.load(str(board_path))
                    out_path = Path(out_dir) / f"{board_path.stem}_extracted.json"
                    
                    # Export as JSON
                    data = {
                        'filename': board.filename,
                        'header': {
                            'level_id': board.header.level_id,
                            'health': board.header.health,
                            'key_count': board.header.key_count,
                            'key_ids': board.header.key_ids,
                            'score': board.header.score,
                            'damage_flash': board.header.damage_flash,
                            'score_copy': board.header.score_copy,
                        },
                        'tile_map': board.tile_map,
                        'objects': [],
                        'strings': board.strings
                    }
                    
                    for obj in board.objects:
                        data['objects'].append({
                            'kind': obj.kind,
                            'kind_name': obj.get_kind_name(),
                            'x': obj.x, 'y': obj.y,
                            'x_delta': obj.x_delta, 'y_delta': obj.y_delta,
                            'width': obj.width, 'height': obj.height,
                            'state': obj.state,
                            'primary_param': obj.primary_param,
                            'flags': obj.flags,
                            'flag_names': obj.get_flag_names(),
                            'string_offset': obj.string_offset,
                            'string_segment': obj.string_segment,
                            'kind_specific_2': obj.kind_specific_2,
                            'ttl': obj.ttl,
                            'attached_string': obj.attached_string
                        })
                    
                    with open(out_path, 'w') as f:
                        json.dump(data, f, indent=2)
                    
                    self.status_var.set(f"Exported board to {out_path}")
                except Exception as e:
                    messagebox.showerror("Error", f"Failed to extract board: {e}")
    
    def extract_selected_shape(self):
        selection = self.shape_listbox.curselection()
        if not selection or not self.sha_file:
            return
        idx = selection[0]
        if idx >= 128 or not self.sha_file.entries[idx]:
            return
        
        out_dir = filedialog.askdirectory()
        if not out_dir:
            return
        
        entry = self.sha_file.entries[idx]
        entry_dir = Path(out_dir) / f"shape_{idx:03d}"
        entry_dir.mkdir(exist_ok=True)
        
        count = 0
        for frame_idx in range(entry.frame_count):
            img = self.sha_file.decode_frame_to_image(idx, frame_idx, 4)
            if img:
                out_path = entry_dir / f"frame_{frame_idx:03d}.png"
                img.save(out_path)
                count += 1
        
        if entry.trailing_frame:
            img = self.sha_file.decode_frame_to_image(idx, 0, 4)
            if img:
                out_path = entry_dir / "trailing.png"
                img.save(out_path)
                count += 1
        
        self.status_var.set(f"Extracted {count} frames from shape {idx} to {out_dir}")
    
    def batch_extract_boards(self):
        out_dir = filedialog.askdirectory()
        if not out_dir:
            return
        
        base_path = self.game_path
        ext = self.current_game['board_ext']
        
        # Use discovered boards (includes manifest list + any extra on disk)
        targets = list(getattr(self, 'board_paths', []))
        if not targets:
            for board_name in self.current_game['boards']:
                p = base_path / f"{board_name}{ext}"
                if p.exists():
                    targets.append(p)
        
        count = 0
        for board_path in targets:
            if board_path.exists():
                try:
                    board = BoardFile.load(str(board_path))
                    out_path = Path(out_dir) / f"{board_path.stem}.json"
                    
                    data = {
                        'filename': board.filename,
                        'header': {
                            'level_id': board.header.level_id,
                            'health': board.header.health,
                            'key_count': board.header.key_count,
                            'key_ids': board.header.key_ids,
                            'score': board.header.score,
                        },
                        'tile_map': board.tile_map,
                        'objects': [],
                        'strings': board.strings
                    }
                    
                    for obj in board.objects:
                        data['objects'].append({
                            'kind': obj.kind,
                            'kind_name': obj.get_kind_name(),
                            'x': obj.x, 'y': obj.y,
                            'x_delta': obj.x_delta, 'y_delta': obj.y_delta,
                            'width': obj.width, 'height': obj.height,
                            'state': obj.state,
                            'primary_param': obj.primary_param,
                            'flags': obj.flags,
                            'flag_names': obj.get_flag_names(),
                            'attached_string': obj.attached_string
                        })
                    
                    with open(out_path, 'w') as f:
                        json.dump(data, f, indent=2)
                    count += 1
                except Exception as e:
                    print(f"Error extracting {board_path}: {e}")
        
        self.status_var.set(f"Extracted {count} boards to {out_dir}")
    
    def batch_extract_shapes(self):
        out_dir = filedialog.askdirectory()
        if not out_dir or not self.sha_file:
            return
        
        count = 0
        for entry_idx in range(128):
            if self.sha_file.entries[entry_idx]:
                entry = self.sha_file.entries[entry_idx]
                entry_dir = Path(out_dir) / f"shape_{entry_idx:03d}"
                entry_dir.mkdir(exist_ok=True)
                
                for frame_idx in range(entry.frame_count):
                    img = self.sha_file.decode_frame_to_image(entry_idx, frame_idx, 4)
                    if img:
                        out_path = entry_dir / f"frame_{frame_idx:03d}.png"
                        img.save(out_path)
                        count += 1
                
                if entry.trailing_frame:
                    img = self.sha_file.decode_frame_to_image(entry_idx, 0, 4)
                    if img:
                        out_path = entry_dir / "trailing.png"
                        img.save(out_path)
                        count += 1
        
        self.status_var.set(f"Extracted {count} shape frames to {out_dir}")
    
    def refresh_shapes(self):
        self.shape_listbox.delete(0, tk.END)
        if not self.sha_file:
            return
        
        for i in range(128):
            if self.sha_file.entries[i]:
                info = self.sha_file.get_entry_info(i)
                self.shape_listbox.insert(tk.END, info)
            else:
                self.shape_listbox.insert(tk.END, f"Entry {i}: EMPTY")
    
    def on_shape_select(self, event):
        selection = self.shape_listbox.curselection()
        if not selection:
            return
        idx = selection[0]
        if self.sha_file and self.sha_file.entries[idx]:
            if hasattr(self, 'right_notebook'):
                self.right_notebook.select(1)  # Shape Viewer
            entry = self.sha_file.entries[idx]
            if getattr(self, 'anim_playing', False):
                self.stop_animation()
            self.anim_entry_idx = idx
            self.anim_frame = 0
            self.frame_var.set(0)
            self.frame_spin.config(to=max(0, entry.frame_count - 1))
            self._update_frame_status()
            self.display_shape_info(idx)
            self.draw_shape(idx, 0)
    
    def display_shape_info(self, entry_idx: int):
        if not self.sha_file or not self.sha_file.entries[entry_idx]:
            return
        
        entry = self.sha_file.entries[entry_idx]
        info = []
        info.append(f"Shape Entry {entry_idx}")
        info.append(f"  Frame count: {entry.frame_count}")
        info.append(f"  Source BPP: {entry.src_bpp}")
        info.append(f"  Flags: 0x{entry.flags:04X} ({'no palette' if entry.flags & 1 else 'has palette'}, {'planar' if entry.flags & 4 else 'packed'})")
        info.append(f"  Decoded sizes: 2bpp={entry.size_2bpp}, 4bpp={entry.size_4bpp}, 8bpp={entry.size_8bpp}")
        
        if entry.palette:
            info.append(f"  Palette: {len(entry.palette)} entries")
            for i, p in enumerate(entry.palette[:8]):
                info.append(f"    [{i}] 0x{p:08X}")
            if len(entry.palette) > 8:
                info.append(f"    ... and {len(entry.palette)-8} more")
        
        info.append(f"  Frames:")
        for i, frame in enumerate(entry.frames):
            info.append(f"    [{i}] {frame['width']}x{frame['height']} opcode={frame['opcode']} "
                       f"data_len={len(frame['data'])}")
        
        if entry.trailing_frame:
            tf = entry.trailing_frame
            info.append(f"  Trailing frame: {tf['width']}x{tf['height']} opcode={tf['opcode']}")
        
        self.shape_info_text.delete(1.0, tk.END)
        self.shape_info_text.insert(tk.END, '\n'.join(info))
    
    def _selected_shape_index(self) -> Optional[int]:
        selection = self.shape_listbox.curselection()
        if not selection:
            return None
        idx = selection[0]
        if idx < 0 or idx >= 128:
            return None
        if not self.sha_file or not self.sha_file.entries[idx]:
            return None
        return idx
    
    def _clamp_frame(self, idx: int, frame: int) -> int:
        entry = self.sha_file.entries[idx]
        return max(0, min(entry.frame_count - 1, frame))
    
    def on_frame_change(self):
        """Keep the frame spinbox and the animation position connected.

        - While stopped: show the requested frame.
        - While playing: jump the animation to the requested frame.
        """
        idx = self._selected_shape_index()
        if idx is None:
            return
        raw = None
        try:
            raw = int(self.frame_spin.get())
        except (tk.TclError, ValueError):
            try:
                raw = int(self.frame_var.get())
            except (tk.TclError, ValueError):
                raw = None
        frame = self._clamp_frame(idx, raw if raw is not None else 0)
        if raw != frame:
            self.frame_var.set(frame)
        if getattr(self, 'anim_playing', False):
            if getattr(self, 'anim_entry_idx', None) != idx:
                self.stop_animation()
                self.draw_shape(idx, frame)
                return
            self.anim_frame = frame
            self._update_frame_status()
            # Restart the timer so the jump is shown immediately.
            if self.anim_timer:
                try:
                    self.root.after_cancel(self.anim_timer)
                except Exception:
                    pass
                self.anim_timer = None
            self._draw_anim_frame()
            self._schedule_animation()
        else:
            self._update_frame_status()
            self.draw_shape(idx, frame)
    
    def on_mode_change(self):
        idx = self._selected_shape_index()
        if idx is None:
            return
        frame = self._clamp_frame(idx, int(self.frame_var.get() or 0))
        self.draw_shape(idx, frame)
    
    def on_shape_canvas_resize(self, event):
        self.draw_checkerboard(force=True)
        idx = self._selected_shape_index()
        if idx is None:
            return
        try:
            frame = self._clamp_frame(idx, int(self.frame_var.get() or 0))
        except (tk.TclError, ValueError):
            frame = 0
        self.draw_shape(idx, frame)
    
    def export_shape_frame(self):
        selection = self.shape_listbox.curselection()
        if not selection:
            return
        idx = selection[0]
        frame = self.frame_var.get()
        mode = self.mode_var.get()
        
        if not self.sha_file or not self.sha_file.entries[idx]:
            return
        
        img = self.sha_file.decode_frame_to_image(idx, frame, mode)
        if img:
            filepath = filedialog.asksaveasfilename(defaultextension=".png",
                                                     filetypes=[("PNG", "*.png")],
                                                     initialfile=f"shape_{idx}_frame_{frame}.png")
            if filepath:
                img.save(filepath)
                self.status_var.set(f"Exported frame to {filepath}")
        else:
            messagebox.showwarning("Warning", "Could not decode this frame")
    
    def export_all_shape_frames(self):
        selection = self.shape_listbox.curselection()
        if not selection:
            return
        idx = selection[0]
        
        if not self.sha_file or not self.sha_file.entries[idx]:
            return
        
        out_dir = filedialog.askdirectory()
        if not out_dir:
            return
        
        entry = self.sha_file.entries[idx]
        # Use the folder layout understood by "Build .sha from PNG Folder".
        entry_dir = Path(out_dir) / f"shape_{idx:03d}"
        entry_dir.mkdir(parents=True, exist_ok=True)
        count = 0
        for frame_idx in range(entry.frame_count):
            img = self.sha_file.decode_frame_to_image(idx, frame_idx, 4)
            if img:
                img.save(entry_dir / f"frame_{frame_idx:03d}.png")
                count += 1
        
        self.status_var.set(f"Exported {count} frames to {entry_dir}")
    
    # ---- Converters (PNG -> .sha, WAV -> .vcl) -----------------------------
    def _current_sha_path(self) -> Optional[Path]:
        p = Path(self.game_path) / self.current_game['sha']
        return p if p.is_file() else None
    
    def _current_vcl_path(self) -> Optional[Path]:
        p = Path(self.game_path) / self.current_game['vcl']
        return p if p.is_file() else None
    
    def _default_out(self, src: Path, suffix: str) -> Path:
        return src.with_name(f"{src.stem}_{suffix}{src.suffix}")
    
    def import_image_to_shape(self):
        idx = self._selected_shape_index()
        if idx is None:
            messagebox.showwarning("Import Image", "Select a shape entry first.")
            return
        sha_path = self._current_sha_path()
        if not sha_path:
            messagebox.showerror("Import Image", "No shape archive for this game.")
            return
        frame = self._clamp_frame(idx, int(self.frame_var.get() or 0))
        try:
            import jill_import as ji
        except Exception as e:
            messagebox.showerror("Import Image", f"Cannot load converter: {e}")
            return
        image_path = filedialog.askopenfilename(
            title=f"Image for entry {idx}, frame {frame}",
            filetypes=[("Images", "*.png *.bmp *.gif *.tga *.jpg *.jpeg"),
                       ("All", "*.*")])
        if not image_path:
            return
        out = filedialog.asksaveasfilename(
            title="Save modified .sha", defaultextension=".sha",
            initialdir=str(sha_path.parent), initialfile=self._default_out(sha_path, "mod").name,
            filetypes=[("Shape archive", "*.sha"), ("All", "*.*")])
        if not out:
            return
        try:
            # Reload from disk so the on-screen archive is not used as source
            # if the user already imported something this session.
            ji.replace_sha_frame(sha_path, idx, frame, Path(image_path), Path(out))
            self.status_var.set(f"Imported image into entry {idx} frame {frame} -> {out}")
            if Path(out).resolve() == sha_path.resolve():
                self.load_game_files()
        except Exception as e:
            messagebox.showerror("Import Image failed", str(e))
    
    def build_sha_from_folder(self):
        sha_path = self._current_sha_path()
        folder = filedialog.askdirectory(title="Folder containing shape_NNN frame PNGs")
        if not folder:
            return
        if not sha_path:
            messagebox.showerror("Build .sha", "No reference .sha for this game.")
            return
        out = filedialog.asksaveasfilename(
            title="Save .sha", defaultextension=".sha",
            initialdir=str(sha_path.parent), initialfile=self._default_out(sha_path, "new").name,
            filetypes=[("Shape archive", "*.sha"), ("All", "*.*")])
        if not out:
            return
        try:
            import jill_import as ji
            ji.sha_from_folder(sha_path, Path(folder), Path(out))
            self.status_var.set(f"Built .sha -> {out}")
            if Path(out).resolve() == sha_path.resolve():
                self.load_game_files()
        except Exception as e:
            messagebox.showerror("Build .sha failed", str(e))
    
    def _selected_sfx_slot(self) -> Optional[int]:
        sel = self.sound_listbox.curselection()
        if not sel:
            return None
        text = self.sound_listbox.get(sel[0])
        if text.startswith("SFX "):
            try:
                return int(text.split()[1])
            except (IndexError, ValueError):
                return None
        return None
    
    def import_wav_to_vcl(self):
        vcl_path = self._current_vcl_path()
        if not vcl_path:
            messagebox.showerror("Import WAV", "No sound archive for this game.")
            return
        slot = self._selected_sfx_slot()
        if slot is None:
            messagebox.showwarning("Import WAV", "Select an SFX slot in the Sounds tab first.")
            return
        wav_path = filedialog.askopenfilename(
            title=f"WAV for SFX slot {slot}",
            filetypes=[("WAV audio", "*.wav"), ("All", "*.*")])
        if not wav_path:
            return
        out = filedialog.asksaveasfilename(
            title="Save modified .vcl", defaultextension=".vcl",
            initialdir=str(vcl_path.parent), initialfile=self._default_out(vcl_path, "mod").name,
            filetypes=[("Sound archive", "*.vcl"), ("All", "*.*")])
        if not out:
            return
        try:
            import jill_import as ji
            ji.replace_vcl_sfx(vcl_path, slot, Path(wav_path), Path(out))
            self.status_var.set(f"Imported WAV into SFX {slot} -> {out}")
            if Path(out).resolve() == vcl_path.resolve():
                self.load_game_files()
        except Exception as e:
            messagebox.showerror("Import WAV failed", str(e))
    
    def build_vcl_from_folder(self):
        vcl_path = self._current_vcl_path()
        folder = filedialog.askdirectory(title="Folder containing sfx_NN.wav / music_NN.raw")
        if not folder:
            return
        if not vcl_path:
            messagebox.showerror("Build .vcl", "No reference .vcl for this game.")
            return
        out = filedialog.asksaveasfilename(
            title="Save .vcl", defaultextension=".vcl",
            initialdir=str(vcl_path.parent), initialfile=self._default_out(vcl_path, "new").name,
            filetypes=[("Sound archive", "*.vcl"), ("All", "*.*")])
        if not out:
            return
        try:
            import jill_import as ji
            ji.vcl_from_folder(vcl_path, Path(folder), Path(out))
            self.status_var.set(f"Built .vcl -> {out}")
            if Path(out).resolve() == vcl_path.resolve():
                self.load_game_files()
        except Exception as e:
            messagebox.showerror("Build .vcl failed", str(e))
    
    def refresh_sounds(self):
        self.sound_listbox.delete(0, tk.END)
        
        # VCL sounds
        if self.vcl_file:
            self.sound_listbox.insert(tk.END, "=== VCL Sound Bank ===")
            for i in range(50):
                if self.vcl_file.sfx_sizes[i] > 0:
                    self.sound_listbox.insert(tk.END, 
                        f"SFX {i:2d}: offset=0x{self.vcl_file.sfx_offsets[i]:08X}, "
                        f"size={self.vcl_file.sfx_sizes[i]}, rate={self.vcl_file.sfx_rates[i]}Hz")
            
            self.sound_listbox.insert(tk.END, "")
            self.sound_listbox.insert(tk.END, "=== VCL Text Pages ===")
            for i in self.vcl_file.text_page_indexes():
                self.sound_listbox.insert(tk.END,
                    f"Page {i:2d}: offset=0x{self.vcl_file.music_offsets[i]:08X}, "
                    f"size={self.vcl_file.music_sizes[i]}")
        
        # DDT/ARK music files (Onesimus stores CMF tracks as SONG*.ARK)
        title_music = self.current_game.get('title_music')
        if title_music:
            track_path = self.game_path / title_music
            if track_path.exists():
                self.sound_listbox.insert(tk.END, "")
                self.sound_listbox.insert(tk.END, f"=== {title_music} ===")
                self.sound_listbox.insert(tk.END, f"Title music: {track_path.name}")
        
        # Other CMF tracks
        try:
            tracks = list(self.game_path.glob("*.ddt")) + \
                     list(self.game_path.glob("*.ark")) + \
                     list(self.game_path.glob("*.ARK"))
            seen = set()
            for ddt_file in sorted(tracks, key=lambda p: p.name.lower()):
                key = ddt_file.name.lower()
                if key in seen or not is_cmf_file(ddt_file):
                    continue
                seen.add(key)
                if title_music and ddt_file.name.lower() == title_music.lower():
                    continue
                self.sound_listbox.insert(tk.END, f"Music: {ddt_file.name}")
            # MAC files
            for mac_file in sorted(self.game_path.glob("*.mac")):
                self.sound_listbox.insert(tk.END, f"Demo: {mac_file.name}")
        except OSError:
            pass
    
    def on_sound_select(self, event):
        selection = self.sound_listbox.curselection()
        if not selection:
            return
        if hasattr(self, 'right_notebook'):
            self.right_notebook.select(2)  # Sound Viewer
        idx = selection[0]
        text = self.sound_listbox.get(idx)
        
        if text.startswith("SFX "):
            # Parse SFX index
            parts = text.split()
            sfx_idx = int(parts[1])
            self.display_sound_info(sfx_idx)
        elif text.startswith("Page "):
            parts = text.split()
            page_idx = int(parts[1])
            self.display_text_page_info(page_idx)
        elif text.startswith("Music "):
            parts = text.split()
            music_idx = int(parts[1])
            self.display_music_info(music_idx)
        elif text.lower().endswith((".ddt", ".ark")):
            self.display_ddt_info(text.split()[-1])
        elif text.endswith(".mac"):
            self.display_mac_info(text.split()[-1])
    
    def display_sound_info(self, sfx_idx: int):
        if not self.vcl_file:
            return
        
        info = []
        info.append(f"SFX {sfx_idx}")
        info.append(f"  Offset: 0x{self.vcl_file.sfx_offsets[sfx_idx]:08X}")
        info.append(f"  Size: {self.vcl_file.sfx_sizes[sfx_idx]} bytes")
        info.append(f"  Sample Rate: {self.vcl_file.sfx_rates[sfx_idx]} Hz")
        
        # Extract and show first few bytes
        data = self.vcl_file.extract_sfx(sfx_idx)
        if data:
            info.append(f"  First 64 bytes: {data[:64].hex()}")
        
        self.sound_detail_text.delete(1.0, tk.END)
        self.sound_detail_text.insert(tk.END, '\n'.join(info))
    
    def display_text_page_info(self, page_idx: int):
        if not self.vcl_file:
            return
        lines = self.vcl_file.parse_text_page(page_idx)
        info = [f"Text Page {page_idx}"]
        if not lines:
            info.append("  (empty or undecodable)")
        else:
            info.append(f"  Title: {lines[0]['text']}")
            info.append(f"  Lines: {len(lines)}")
            info.append("")
            for i, line in enumerate(lines[:24]):
                info.append(f"    [{line['colour']}] {line['text']}")
            if len(lines) > 24:
                info.append(f"    ... and {len(lines) - 24} more lines")

        self.sound_detail_text.delete(1.0, tk.END)
        self.sound_detail_text.insert(tk.END, '\n'.join(info))

    def export_text_page(self, page_idx: int):
        if not self.vcl_file:
            return
        lines = self.vcl_file.parse_text_page(page_idx)
        if not lines:
            messagebox.showwarning("Warning", "No data to extract")
            return
        filepath = filedialog.asksaveasfilename(
            defaultextension=".txt",
            filetypes=[("Text", "*.txt"), ("All", "*.*")],
            initialfile=f"page_{page_idx:02d}.txt")
        if filepath:
            with open(filepath, 'w', encoding='ascii', errors='replace') as f:
                for line in lines:
                    f.write(line['text'] + "\n")
            self.status_var.set(f"Exported text page {page_idx} to {filepath}")

    def display_music_info(self, music_idx: int):
        if not self.vcl_file:
            return
        
        info = []
        info.append(f"Music/Pattern {music_idx}")
        info.append(f"  Offset: 0x{self.vcl_file.music_offsets[music_idx]:08X}")
        info.append(f"  Size: {self.vcl_file.music_sizes[music_idx]} bytes")
        
        data = self.vcl_file.extract_music(music_idx)
        if data:
            info.append(f"  First 64 bytes: {data[:64].hex()}")
        
        self.sound_detail_text.delete(1.0, tk.END)
        self.sound_detail_text.insert(tk.END, '\n'.join(info))
    
    def display_ddt_info(self, filename: str):
        path = self.game_path / filename
        if not path.exists():
            return
        
        ddt = DDTFile.load(str(path))
        info = []
        info.append(f"Music File: {filename}")
        info.append(f"  Size: {len(ddt.data)} bytes")
        info.append(f"  Is CMF/CTMF: {ddt.is_cmf}")
        if ddt.is_cmf:
            info.append(f"  Version: 0x{ddt.version:04X}")
            info.append(f"  Instrument offset: 0x{ddt.instrument_offset:08X}")
            info.append(f"  Music data offset: 0x{ddt.music_offset:08X}")
            info.append(f"  Ticks per beat: {ddt.ticks_per_beat}")
        
        self.sound_detail_text.delete(1.0, tk.END)
        self.sound_detail_text.insert(tk.END, '\n'.join(info))
    
    def display_mac_info(self, filename: str):
        path = self.game_path / filename
        if not path.exists():
            return
        
        mac = MACFile.load(str(path))
        info = []
        info.append(f"MAC File: {filename}")
        info.append(f"  Records: {len(mac.records)}")
        
        if mac.records:
            total_time = mac.records[-1]['time']
            info.append(f"  Duration: {total_time} ticks (~{total_time/18.2:.1f} seconds)")
            info.append("")
            info.append("  First 10 records:")
            for i, rec in enumerate(mac.records[:10]):
                vals = []
                for k, v in rec['values'].items():
                    vals.append(f"{k}={v}")
                info.append(f"    [{i}] t={rec['time']} mask=0x{rec['mask']:02X} {', '.join(vals)}")
        
        self.sound_detail_text.delete(1.0, tk.END)
        self.sound_detail_text.insert(tk.END, '\n'.join(info))
    
    def extract_selected_sound(self):
        selection = self.sound_listbox.curselection()
        if not selection:
            return
        text = self.sound_listbox.get(selection[0])
        
        if text.startswith("SFX "):
            parts = text.split()
            sfx_idx = int(parts[1])
            self.export_sound(sfx_idx, "sfx")
        elif text.startswith("Page "):
            parts = text.split()
            page_idx = int(parts[1])
            self.export_text_page(page_idx)
        elif text.startswith("Music "):
            parts = text.split()
            music_idx = int(parts[1])
            self.export_sound(music_idx, "music")
        elif text.lower().endswith((".ddt", ".ark")):
            filename = text.split()[-1]
            self.export_file(self.game_path / filename)
        elif text.endswith(".mac"):
            filename = text.split()[-1]
            self.export_file(self.game_path / filename)
    
    def export_sound(self, index: int, type_: str):
        if not self.vcl_file:
            return
        
        if type_ == "sfx":
            data = self.vcl_file.extract_sfx(index)
            ext = ".raw"
            default_name = f"sfx_{index}{ext}"
        else:
            data = self.vcl_file.extract_music(index)
            ext = ".raw"
            default_name = f"music_{index}{ext}"
        
        if not data:
            messagebox.showwarning("Warning", "No data to extract")
            return
        
        filepath = filedialog.asksaveasfilename(defaultextension=ext,
                                                 filetypes=[("Raw", "*.raw"), ("All", "*.*")],
                                                 initialfile=default_name)
        if filepath:
            with open(filepath, 'wb') as f:
                f.write(data)
            self.status_var.set(f"Exported {type_} {index} to {filepath}")
    
    def export_file(self, src_path: Path):
        filepath = filedialog.asksaveasfilename(defaultextension=src_path.suffix,
                                                 filetypes=[("All", "*.*")],
                                                 initialfile=src_path.name)
        if filepath:
            import shutil
            shutil.copy2(src_path, filepath)
            self.status_var.set(f"Exported {src_path.name} to {filepath}")
    
    def batch_extract_sounds(self):
        out_dir = filedialog.askdirectory()
        if not out_dir or not self.vcl_file:
            return
        
        count = 0
        out_path = Path(out_dir)
        for i in range(50):
            data = self.vcl_file.extract_sfx(i)
            if data:
                with open(out_path / f"sfx_{i:02d}.raw", 'wb') as f:
                    f.write(data)
                wav = self.vcl_file.create_wav(i, "sfx")
                if wav:
                    with open(out_path / f"sfx_{i:02d}.wav", 'wb') as f:
                        f.write(wav)
                count += 1

        # The second VCL bank is the text-page table; export readable copies.
        page_dir = out_path / "text_pages"
        pages = 0
        for i in self.vcl_file.text_page_indexes():
            lines = self.vcl_file.parse_text_page(i)
            if lines:
                page_dir.mkdir(exist_ok=True)
                with open(page_dir / f"page_{i:02d}.txt", 'w',
                          encoding='ascii', errors='replace') as f:
                    for line in lines:
                        f.write(line['text'] + "\n")
                pages += 1

        self.status_var.set(
            f"Extracted {count} WAV/raw sounds and {pages} text pages to {out_dir}")

    # ---- Enhanced asset exporters (fonts/pages/tiles/HUD/packs) -----------
    def _pick_out(self, title: str) -> Optional[Path]:
        out_dir = filedialog.askdirectory(title=title)
        return Path(out_dir) if out_dir else None

    def batch_extract_fonts(self):
        if not self.sha_file:
            messagebox.showwarning("Fonts", "No shape archive for this game.")
            return
        out = self._pick_out("Output folder for fonts/")
        if not out:
            return
        try:
            count = export_font_assets(self.sha_file, out)
            self.status_var.set(
                f"Extracted {count} glyphs + fonts/main.png to {out / 'fonts'}")
        except Exception as e:
            messagebox.showerror("Fonts", str(e))

    def batch_extract_text_pages(self):
        if not self.vcl_file:
            messagebox.showwarning("Text pages", "No .vcl bank for this game.")
            return
        out = self._pick_out("Output folder for text_pages/")
        if not out:
            return
        try:
            count = export_text_page_assets(self.vcl_file, self.sha_file, out)
            self.status_var.set(
                f"Extracted {count} text pages to {out / 'text_pages'}")
        except Exception as e:
            messagebox.showerror("Text pages", str(e))

    def batch_extract_tiles(self):
        if not self.sha_file or not self.dma_file:
            messagebox.showwarning("Tiles", "Shape archive and jill.dma are required.")
            return
        out = self._pick_out("Output folder for tiles/")
        if not out:
            return
        try:
            count = export_tile_assets(self.sha_file, self.dma_file, out)
            self.status_var.set(f"Extracted {count} tile images to {out / 'tiles'}")
        except Exception as e:
            messagebox.showerror("Tiles", str(e))

    def batch_extract_hud(self):
        if not self.sha_file or not self.dma_file:
            messagebox.showwarning("HUD", "Shape archive and jill.dma are required.")
            return
        out = self._pick_out("Output folder for hud/ and themes/")
        if not out:
            return
        try:
            icons = export_hud_assets(self.sha_file, out)
            portrait = export_portrait_asset(self.sha_file, out)
            theme = export_theme_assets(self.current_game, self.sha_file,
                                        self.dma_file, out)
            extra = " + themes/intro.png" if theme else ""
            extra += " + portrait.png" if portrait else ""
            self.status_var.set(
                f"Extracted HUD panels and {icons} icons{extra} to {out}")
        except Exception as e:
            messagebox.showerror("HUD", str(e))

    def batch_extract_music_wavs(self):
        out = self._pick_out("Output folder for music/")
        if not out:
            return
        try:
            count = export_music_wavs(self.current_game, out)
            if count:
                self.status_var.set(
                    f"Rendered {count} music tracks to {out / 'music'}")
            else:
                self.status_var.set(
                    "No music rendered - build the port so jill.exe can render CMF")
        except Exception as e:
            messagebox.showerror("Music", str(e))

    def batch_extract_board_previews(self):
        if not self.sha_file or not self.dma_file:
            messagebox.showwarning("Board previews",
                                   "Shape archive and jill.dma are required.")
            return
        out = self._pick_out("Output folder for board_previews/ and boards/")
        if not out:
            return
        try:
            count = export_board_previews(self.current_game, self.sha_file,
                                          self.dma_file, out)
            self.status_var.set(f"Rendered {count} boards to {out}")
        except Exception as e:
            messagebox.showerror("Board previews", str(e))

    def export_palette(self):
        if not self.sha_file:
            messagebox.showwarning("Palette", "No shape archive for this game.")
            return
        out = self._pick_out("Output folder for palette.pal")
        if not out:
            return
        try:
            export_palette_assets(self.sha_file, out)
            self.status_var.set(f"Wrote palette.pal / palette.json to {out}")
        except Exception as e:
            messagebox.showerror("Palette", str(e))

    def export_reference_pack(self):
        if not self.sha_file or not self.vcl_file or not self.dma_file:
            messagebox.showwarning(
                "Reference pack",
                "Shape archive, .vcl and jill.dma are required.")
            return
        out = self._pick_out("Output folder for the reference pack")
        if not out:
            return
        if not messagebox.askyesno(
                "Reference pack",
                "This writes a full editable pack (all shape frames, boards, "
                "tiles, fonts, HUD, pages, SFX WAVs). It can take a while and "
                "use a lot of disk space. Continue?"):
            return
        try:
            stats = export_reference_pack(self.current_game, self.sha_file,
                                          self.vcl_file, self.dma_file, out)
            summary = ", ".join(f"{k}={v}" for k, v in stats.items())
            self.status_var.set(f"Reference pack -> {out} ({summary})")
        except Exception as e:
            messagebox.showerror("Reference pack", str(e))

    
    def refresh_dma(self):
        self.dma_listbox.delete(0, tk.END)
        if not self.dma_file:
            return
        
        for i, rec in enumerate(self.dma_file.records):
            self.dma_listbox.insert(tk.END, 
                f"[{i:3d}] Tile 0x{rec.tile_id:04X} -> Shape 0x{rec.shape_id:04X} "
                f"Attr 0x{rec.attr:04X} '{rec.name}'")
    
    def on_dma_select(self, event):
        selection = self.dma_listbox.curselection()
        if not selection or not self.dma_file:
            return
        idx = selection[0]
        if idx < len(self.dma_file.records):
            rec = self.dma_file.records[idx]
            info = []
            info.append(f"DMA Record {idx}")
            info.append(f"  Tile ID: {rec.tile_id} (0x{rec.tile_id:04X})")
            info.append(f"  Shape ID: {rec.shape_id} (0x{rec.shape_id:04X})")
            info.append(f"  Attribute: 0x{rec.attr:04X}")
            info.append(f"  Name: '{rec.name}'")
            info.append("")
            info.append("Attribute bits:")
            if rec.attr & 0x0001: info.append("  Bit 0: Unknown")
            if rec.attr & 0x0002: info.append("  Bit 1: Collision class bit 1")
            if rec.attr & 0x0004: info.append("  Bit 2: Unknown")
            if rec.attr & 0x0008: info.append("  Bit 3: Collision class bit 2")
            if rec.attr & 0x0010: info.append("  Bit 4: Unknown")
            if rec.attr & 0x0020: info.append("  Bit 5: Tile hook trigger")
            if rec.attr & 0x0200: info.append("  Bit 9: Unknown")
            if rec.attr & 0x2000: info.append("  Bit 13: Unknown")
            if rec.attr & 0x4000: info.append("  Bit 14: Unknown")
            if rec.attr & 0x6000: info.append("  Bits 13-14: Special")
            
            self.config_text.delete(1.0, tk.END)
            self.config_text.insert(tk.END, '\n'.join(info))
    
    def refresh_config(self):
        if not self.cfg_file:
            self.config_text.delete(1.0, tk.END)
            self.config_text.insert(tk.END, "(no config file for this game)")
            return
        
        info = []
        info.append("=== High Scores ===")
        for i in range(10):
            name = self.cfg_file.high_score_names[i]
            score = self.cfg_file.high_scores[i]
            if name or score > 0:
                info.append(f"  {i+1:2d}. {name:10s}  {score}")
        
        info.append("")
        info.append("=== Save Slots ===")
        for i in range(6):
            label = self.cfg_file.save_slot_labels[i]
            info.append(f"  Slot {i+1}: '{label}'")
        
        info.append("")
        info.append("=== Options ===")
        opt_names = ["Sound", "Music", "Joystick", "Video Mode", "Difficulty",
                     "Unk5", "Unk6", "Unk7", "Unk8", "Unk9", "Unk10"]
        for i in range(11):
            info.append(f"  {opt_names[i]:12s}: {self.cfg_file.options[i]} (0x{self.cfg_file.options[i]:04X})")
        
        self.config_text.delete(1.0, tk.END)
        self.config_text.insert(tk.END, '\n'.join(info))
    
    def load_hex_file(self):
        filepath = filedialog.askopenfilename()
        if filepath:
            self.display_hex(filepath)
    
    def display_hex(self, filepath: str):
        with open(filepath, 'rb') as f:
            data = f.read()
        
        self.hex_text.delete(1.0, tk.END)
        offset = 0
        try:
            offset = int(self.hex_offset_var.get(), 0)
        except:
            offset = 0
        
        lines = []
        for i in range(offset, min(offset + 2048, len(data)), 16):
            line_data = data[i:i+16]
            hex_part = ' '.join(f'{b:02X}' for b in line_data)
            ascii_part = ''.join(chr(b) if 32 <= b < 127 else '.' for b in line_data)
            lines.append(f"{i:08X}  {hex_part:<48}  {ascii_part}")
        
        self.hex_text.insert(tk.END, '\n'.join(lines))
    
    def goto_hex_offset(self):
        # Would need a current file context
        pass


def main():
    base_dir = None
    if len(sys.argv) > 1 and not sys.argv[1].startswith('-'):
        base_dir = Path(sys.argv[1])
    root = tk.Tk()
    app = JillExtractorApp(root, base_dir=base_dir)
    root.mainloop()


if __name__ == "__main__":
    main()