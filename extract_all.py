#!/usr/bin/env python3
"""
Command-line batch extraction tool for Jill of the Jungle game files.

Locates installed games via the port's jill.ini + games/*.game manifests
(falling back to a ./jillt directory) and extracts board/shape/sound assets
without using the GUI.

Examples:
    python extract_all.py                    # extract all games into ./extracted
    python extract_all.py out_dir --all      # same, explicit output dir
    python extract_all.py out_dir --game jill1 --shapes --boards
    python extract_all.py out_dir --game jill3 --sounds --music
"""
import sys
import os
import json
import shutil
import argparse
from pathlib import Path

sys.path.insert(0, os.path.dirname(__file__))
from jill_extractor import (
    BoardFile, DMAFile, SHAFile, VCLFile, DDTFile, CFGFile, MACFile,
    discover_games, export_font_assets, export_hud_assets,
    export_palette_assets, export_reference_pack, export_text_page_assets,
    export_theme_assets, export_tile_assets, export_board_previews,
    export_music_wavs,
)

BASE = Path(__file__).resolve().parent


def board_to_dict(board, game):
    data = {
        'filename': board.filename,
        'game': game['id'],
        'profile': game['profile'],
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
        'strings': board.strings,
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
            'attached_string': obj.attached_string,
        })
    return data


def extract_boards(game, out_dir: Path):
    data_dir = Path(game['data_dir'])
    ext = game['board_ext']
    out_dir.mkdir(parents=True, exist_ok=True)

    targets = [data_dir / f"{n}{ext}" for n in game['boards']]
    try:
        for f in sorted(data_dir.glob(f"*{ext}")):
            if f not in targets:
                targets.append(f)
    except OSError:
        pass

    count = 0
    for p in targets:
        if not p.exists():
            continue
        try:
            board = BoardFile.load(str(p))
            out_path = out_dir / f"{p.stem}.json"
            with open(out_path, 'w', encoding='utf-8') as f:
                json.dump(board_to_dict(board, game), f, indent=2)
            count += 1
            print(f"  Extracted: {p.name} -> {out_path.name}")
        except Exception as e:
            print(f"  ERROR extracting {p.name}: {e}")
    print(f"Extracted {count} boards to {out_dir}")


def extract_shapes(game, out_dir: Path):
    data_dir = Path(game['data_dir'])
    sha_path = data_dir / game['sha']
    out_dir.mkdir(parents=True, exist_ok=True)
    if not sha_path.exists():
        print(f"SHA file not found: {sha_path}")
        return

    sha = SHAFile.load(str(sha_path))
    if game.get('palette') and Path(game['palette']).is_file():
        pal = Path(game['palette']).read_bytes()
        sha.set_palette_rgb(pal, from_6bit=(max(pal) <= 63))

    count = 0
    for entry_idx in range(128):
        entry = sha.entries[entry_idx]
        if not entry:
            continue
        entry_dir = out_dir / f"shape_{entry_idx:03d}"
        entry_dir.mkdir(exist_ok=True)
        for frame_idx in range(entry.frame_count):
            img = sha.decode_frame_to_image(entry_idx, frame_idx, 4)
            if img:
                img.save(entry_dir / f"frame_{frame_idx:03d}.png")
                count += 1
    print(f"Extracted {count} shape frames to {out_dir}")


def extract_sounds(game, out_dir: Path):
    data_dir = Path(game['data_dir'])
    vcl_path = data_dir / game['vcl']
    out_dir.mkdir(parents=True, exist_ok=True)
    if not vcl_path.exists():
        print(f"VCL file not found: {vcl_path}")
        return

    vcl = VCLFile.load(str(vcl_path))
    count = 0
    sfx_dir = out_dir / "sfx"
    for i in range(50):
        data = vcl.extract_sfx(i)
        if data:
            sfx_dir.mkdir(exist_ok=True)
            (sfx_dir / f"sfx_{i:02d}.raw").write_bytes(data)
            wav = vcl.create_wav(i, "sfx")
            if wav:
                (sfx_dir / f"sfx_{i:02d}.wav").write_bytes(wav)
            count += 1
    print(f"Extracted {count} sound files to {out_dir}")


def extract_fonts(game, out_dir: Path):
    data_dir = Path(game['data_dir'])
    sha_path = data_dir / game['sha']
    if not sha_path.exists():
        print(f"SHA file not found: {sha_path}")
        return
    sha = _load_sha(game, sha_path)
    count = export_font_assets(sha, out_dir)
    print(f"Extracted {count} font glyphs to {out_dir / 'fonts'}")


def extract_pages(game, out_dir: Path):
    data_dir = Path(game['data_dir'])
    vcl_path = data_dir / game['vcl']
    if not vcl_path.exists():
        print(f"VCL file not found: {vcl_path}")
        return
    vcl = VCLFile.load(str(vcl_path))
    sha = None
    sha_path = data_dir / game['sha']
    if sha_path.exists():
        sha = _load_sha(game, sha_path)
    count = export_text_page_assets(vcl, sha, out_dir)
    print(f"Extracted {count} text pages to {out_dir / 'text_pages'}")


def extract_tiles(game, out_dir: Path):
    data_dir = Path(game['data_dir'])
    sha_path = data_dir / game['sha']
    dma_path = data_dir / game['dma']
    if not sha_path.exists() or not dma_path.exists():
        print("Shape archive or jill.dma missing")
        return
    sha = _load_sha(game, sha_path)
    dma = DMAFile.load(str(dma_path))
    count = export_tile_assets(sha, dma, out_dir)
    print(f"Extracted {count} tile images to {out_dir / 'tiles'}")


def extract_hud(game, out_dir: Path):
    data_dir = Path(game['data_dir'])
    sha_path = data_dir / game['sha']
    dma_path = data_dir / game['dma']
    if not sha_path.exists() or not dma_path.exists():
        print("Shape archive or jill.dma missing")
        return
    sha = _load_sha(game, sha_path)
    dma = DMAFile.load(str(dma_path))
    count = export_hud_assets(sha, out_dir)
    portrait = export_portrait_asset(sha, out_dir)
    theme = export_theme_assets(game, sha, dma, out_dir)
    print(f"Extracted HUD panels + {count} icons"
          + (" + portrait.png" if portrait else "")
          + (" + themes/intro.png" if theme else ""))


def extract_palette(game, out_dir: Path):
    data_dir = Path(game['data_dir'])
    sha_path = data_dir / game['sha']
    if not sha_path.exists():
        print(f"SHA file not found: {sha_path}")
        return
    sha = _load_sha(game, sha_path)
    export_palette_assets(sha, out_dir)
    print(f"Wrote palette.pal / palette.json to {out_dir}")


def extract_previews(game, out_dir: Path):
    data_dir = Path(game['data_dir'])
    sha_path = data_dir / game['sha']
    dma_path = data_dir / game['dma']
    if not sha_path.exists() or not dma_path.exists():
        print("Shape archive or jill.dma missing")
        return
    sha = _load_sha(game, sha_path)
    dma = DMAFile.load(str(dma_path))
    count = export_board_previews(game, sha, dma, out_dir)
    print(f"Rendered {count} board previews to {out_dir}")


def extract_pack(game, out_dir: Path, seconds: float = 60.0):
    data_dir = Path(game['data_dir'])
    sha_path = data_dir / game['sha']
    vcl_path = data_dir / game['vcl']
    dma_path = data_dir / game['dma']
    if not sha_path.exists() or not vcl_path.exists() or not dma_path.exists():
        print("Shape archive, .vcl or jill.dma missing")
        return
    sha = _load_sha(game, sha_path)
    vcl = VCLFile.load(str(vcl_path))
    dma = DMAFile.load(str(dma_path))
    stats = export_reference_pack(game, sha, vcl, dma, out_dir, seconds)
    print("Reference pack: " + ", ".join(f"{k}={v}" for k, v in stats.items()))


def _load_sha(game, sha_path: Path) -> SHAFile:
    sha = SHAFile.load(str(sha_path))
    pal = game.get('palette')
    if pal and Path(pal).is_file():
        data = Path(pal).read_bytes()
        sha.set_palette_rgb(data, from_6bit=(max(data) <= 63))
    return sha


def extract_config(game, out_dir: Path):
    data_dir = Path(game['data_dir'])
    cfg_name = game.get('cfg')
    if not cfg_name:
        return
    cfg_path = data_dir / cfg_name
    out_dir.mkdir(parents=True, exist_ok=True)
    if not cfg_path.exists():
        print(f"Config file not found: {cfg_path}")
        return
    cfg = CFGFile.load(str(cfg_path))
    data = {
        'game': game['id'],
        'high_scores': [
            {'rank': i + 1, 'name': cfg.high_score_names[i], 'score': cfg.high_scores[i]}
            for i in range(10)
        ],
        'save_slots': [
            {'slot': i + 1, 'label': cfg.save_slot_labels[i]} for i in range(6)
        ],
        'options': [{'index': i, 'value': cfg.options[i]} for i in range(11)],
    }
    out_path = out_dir / f"{cfg_path.name}.json"
    with open(out_path, 'w', encoding='utf-8') as f:
        json.dump(data, f, indent=2)
    print(f"Extracted config to {out_path}")


def extract_dma(game, out_dir: Path):
    data_dir = Path(game['data_dir'])
    dma_path = data_dir / game['dma']
    out_dir.mkdir(parents=True, exist_ok=True)
    if not dma_path.exists():
        print(f"DMA file not found: {dma_path}")
        return
    dma = DMAFile.load(str(dma_path))
    data = {
        'game': game['id'],
        'records': [
            {'index': i, 'tile_id': r.tile_id, 'shape_id': r.shape_id,
             'attr': r.attr, 'name': r.name}
            for i, r in enumerate(dma.records)
        ],
    }
    out_path = out_dir / f"{dma_path.name}.json"
    with open(out_path, 'w', encoding='utf-8') as f:
        json.dump(data, f, indent=2)
    print(f"Extracted DMA ({len(dma.records)} records) to {out_path}")


def extract_ddt(game, out_dir: Path):
    data_dir = Path(game['data_dir'])
    out_dir.mkdir(parents=True, exist_ok=True)
    count = 0
    try:
        files = sorted(data_dir.glob("*.ddt"))
    except OSError:
        files = []
    for f in files:
        shutil.copy2(f, out_dir / f.name)
        print(f"  Copied: {f.name}")
        count += 1
    print(f"Copied {count} DDT files to {out_dir}")


def extract_music_wavs(game, out_dir: Path, seconds: float = 60.0):
    count = export_music_wavs(game, out_dir, seconds)
    print(f"Rendered {count} music tracks to {out_dir / 'music'}")


def extract_macs(game, out_dir: Path):
    data_dir = Path(game['data_dir'])
    out_dir.mkdir(parents=True, exist_ok=True)
    count = 0
    try:
        files = sorted(data_dir.glob("*.mac"))
    except OSError:
        files = []
    for f in files:
        shutil.copy2(f, out_dir / f.name)
        print(f"  Copied: {f.name}")
        count += 1
    print(f"Copied {count} MAC files to {out_dir}")


def main():
    parser = argparse.ArgumentParser(
        description="Batch extract Jill of the Jungle / Onesimus game files")
    parser.add_argument("output_dir", nargs="?", default="extracted",
                        help="Output directory (default: ./extracted)")
    parser.add_argument("--game", action="append", default=None,
                        help="Game id to extract (repeatable). Default: all")
    parser.add_argument("--list", action="store_true", help="List known games and exit")
    parser.add_argument("--boards", action="store_true")
    parser.add_argument("--shapes", action="store_true")
    parser.add_argument("--sounds", action="store_true")
    parser.add_argument("--music", action="store_true")
    parser.add_argument("--music-wavs", action="store_true",
                        help="Render .ddt music to WAV via the built port")
    parser.add_argument("--music-seconds", type=float, default=60.0,
                        help="Seconds to render per music track (default 60)")
    parser.add_argument("--demos", action="store_true")
    parser.add_argument("--config", action="store_true")
    parser.add_argument("--dma", action="store_true")
    parser.add_argument("--fonts", action="store_true",
                        help="Export fonts/main.png and per-glyph PNGs")
    parser.add_argument("--pages", action="store_true",
                        help="Export VCL text pages (.txt + previews)")
    parser.add_argument("--tiles", action="store_true",
                        help="Export tiles/<id>.png and tiles.json")
    parser.add_argument("--hud", action="store_true",
                        help="Export HUD panels, icons and intro menu theme")
    parser.add_argument("--palette", action="store_true",
                        help="Export palette.pal / palette.json")
    parser.add_argument("--previews", action="store_true",
                        help="Render full board previews (large)")
    parser.add_argument("--pack", action="store_true",
                        help="Export a ready-to-edit external asset pack")
    parser.add_argument("--all", action="store_true", help="Extract everything")
    args = parser.parse_args()

    games = discover_games(BASE)
    if args.list:
        for g in games:
            print(f"{g['id']:10s} {g['title']:32s} {g['data_dir']}")
        return 0

    if args.game:
        wanted = {x.lower() for x in args.game}
        games = [g for g in games if g['id'].lower() in wanted]
        if not games:
            print("No matching games (use --list to see ids)")
            return 1

    out_base = Path(args.output_dir)
    do_all = args.all or not (args.boards or args.shapes or args.sounds or
                              args.music or args.demos or args.config or
                              args.dma or args.fonts or args.pages or
                              args.tiles or args.hud or args.palette or
                              args.music_wavs or args.previews or args.pack)

    for g in games:
        print(f"\n=== {g['id']} ({g['title']}) ===")
        data_dir = Path(g['data_dir'])
        if not data_dir.is_dir():
            print(f"  Skipping: data dir missing ({data_dir})")
            continue
        ep_out = out_base / g['id']
        if do_all or args.boards:
            print("Boards:")
            extract_boards(g, ep_out / "boards")
        if do_all or args.shapes:
            print("Shapes:")
            extract_shapes(g, ep_out / "shapes")
        if do_all or args.sounds:
            print("Sounds:")
            extract_sounds(g, ep_out / "sounds")
        if do_all or args.config:
            print("Config:")
            extract_config(g, ep_out / "config")
        if do_all or args.dma:
            print("DMA:")
            extract_dma(g, ep_out / "dma")
        if do_all or args.music:
            print("Music:")
            extract_ddt(g, ep_out / "music")
        if do_all or args.music_wavs:
            print("Music WAVs:")
            extract_music_wavs(g, ep_out, args.music_seconds)
        if do_all or args.demos:
            print("Demos:")
            extract_macs(g, ep_out / "demos")
        if do_all or args.fonts:
            print("Fonts:")
            extract_fonts(g, ep_out)
        if do_all or args.pages:
            print("Text pages:")
            extract_pages(g, ep_out)
        if do_all or args.tiles:
            print("Tiles:")
            extract_tiles(g, ep_out)
        if do_all or args.hud:
            print("HUD + theme:")
            extract_hud(g, ep_out)
        if do_all or args.palette:
            print("Palette:")
            extract_palette(g, ep_out)
        if args.previews:
            print("Board previews:")
            extract_previews(g, ep_out)
        if args.pack:
            print("Reference pack:")
            extract_pack(g, ep_out / "reference_pack", args.music_seconds)

    print("\nDone!")
    return 0


if __name__ == "__main__":
    sys.exit(main())
