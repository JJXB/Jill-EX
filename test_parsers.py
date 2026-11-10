#!/usr/bin/env python3
"""
Test script to verify all parsers work with the installed game files.

Uses the port's jill.ini + games/*.game manifests to locate the asset
directories, falling back to a ./jillt directory.
"""
import sys
import os
from pathlib import Path

sys.path.insert(0, os.path.dirname(__file__))

from jill_extractor import (
    BoardFile, DMAFile, SHAFile, VCLFile, DDTFile, CFGFile, MACFile,
    discover_games,
)

BASE = Path(__file__).resolve().parent


def test_games():
    games = discover_games(BASE)
    print("=" * 60)
    print("Discovered games")
    print("=" * 60)
    for g in games:
        print(f"  {g['id']:10s} {g['title']:32s} data={g['data_dir']}")

    for g in games:
        data = Path(g['data_dir'])
        print("\n" + "=" * 60)
        print(f"{g['id']} - {g['title']}")
        print("=" * 60)
        if not data.is_dir():
            print(f"  DATA DIR MISSING: {data}")
            continue

        # Boards
        print("  Boards:")
        ext = g['board_ext']
        for name in g['boards']:
            p = data / f"{name}{ext}"
            if p.exists():
                try:
                    b = BoardFile.load(str(p))
                    print(f"    {p.name}: OK - {len(b.objects)} objects, "
                          f"{len(b.strings)} strings, level={b.header.level_id}")
                except Exception as e:
                    print(f"    {p.name}: ERROR - {e}")
            else:
                print(f"    {p.name}: NOT FOUND")

        # SHA
        sha_entries = 0
        sha_path = data / g['sha']
        if sha_path.exists():
            try:
                sha = SHAFile.load(str(sha_path))
                if g.get('palette') and Path(g['palette']).is_file():
                    sha.set_palette_rgb(Path(g['palette']).read_bytes(),
                                        from_6bit=False)
                sha_entries = sum(1 for e in sha.entries if e)
                print(f"  {g['sha']}: OK - {sha_entries}/128 entries")
            except Exception as e:
                print(f"  {g['sha']}: ERROR - {e}")
        else:
            print(f"  {g['sha']}: NOT FOUND")

        # DMA + verify tile mapping
        dma_path = data / g['dma']
        if dma_path.exists() and sha_path.exists():
            try:
                dma = DMAFile.load(str(dma_path))
                ok = bad = 0
                for r in dma.records:
                    bank = (r.shape_id >> 8) & 0x3F
                    fr = r.shape_id & 0xFF
                    e = sha.entries[bank]
                    if e and fr < e.frame_count:
                        ok += 1
                    else:
                        bad += 1
                print(f"  {g['dma']}: OK - {len(dma.records)} records "
                      f"(tile->shape ok={ok} bad={bad})")
            except Exception as e:
                print(f"  {g['dma']}: ERROR - {e}")
        else:
            print(f"  {g['dma']}: NOT FOUND")

        # VCL
        vcl_path = data / g['vcl']
        if vcl_path.exists():
            try:
                vcl = VCLFile.load(str(vcl_path))
                sfx = sum(1 for s in vcl.sfx_sizes if s > 0)
                pages = len(vcl.text_page_indexes())
                print(f"  {g['vcl']}: OK - {sfx} SFX, {pages} text pages")
            except Exception as e:
                print(f"  {g['vcl']}: ERROR - {e}")
        else:
            print(f"  {g['vcl']}: NOT FOUND")

        # CFG (optional)
        if g.get('cfg'):
            cfg_path = data / g['cfg']
            if cfg_path.exists():
                try:
                    cfg = CFGFile.load(str(cfg_path))
                    print(f"  {g['cfg']}: OK - options={cfg.options[:3]}...")
                except Exception as e:
                    print(f"  {g['cfg']}: ERROR - {e}")


def test_shared_assets():
    games = discover_games(BASE)
    # Use the first existing data dir for shared DDT/MAC files.
    data = next((Path(g['data_dir']) for g in games
                 if Path(g['data_dir']).is_dir()), None)
    if data is None:
        return

    print("\n" + "=" * 60)
    print(f"Shared assets in {data}")
    print("=" * 60)

    ddts = sorted(data.glob("*.ddt"))
    for f in ddts:
        try:
            d = DDTFile.load(str(f))
            print(f"  {f.name}: {len(d.data)} bytes, CMF={d.is_cmf}")
        except Exception as e:
            print(f"  {f.name}: ERROR - {e}")

    for f in sorted(data.glob("*.mac")):
        try:
            m = MACFile.load(str(f))
            total = m.records[-1]['time'] if m.records else 0
            print(f"  {f.name}: {len(m.records)} records, {total} ticks")
        except Exception as e:
            print(f"  {f.name}: ERROR - {e}")


if __name__ == "__main__":
    test_games()
    test_shared_assets()
    print("\n" + "=" * 60)
    print("All tests completed!")
    print("=" * 60)
