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
    discover_games,
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
    music_dir = out_dir / "music"
    for i in range(50):
        data = vcl.extract_sfx(i)
        if data:
            sfx_dir.mkdir(exist_ok=True)
            (sfx_dir / f"sfx_{i:02d}.raw").write_bytes(data)
            count += 1
    for i in range(40):
        data = vcl.extract_music(i)
        if data:
            music_dir.mkdir(exist_ok=True)
            (music_dir / f"music_{i:02d}.raw").write_bytes(data)
            count += 1
    print(f"Extracted {count} sound files to {out_dir}")


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
    parser.add_argument("--demos", action="store_true")
    parser.add_argument("--config", action="store_true")
    parser.add_argument("--dma", action="store_true")
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
                              args.music or args.demos or args.config or args.dma)

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
        if do_all or args.demos:
            print("Demos:")
            extract_macs(g, ep_out / "demos")

    print("\nDone!")
    return 0


if __name__ == "__main__":
    sys.exit(main())
