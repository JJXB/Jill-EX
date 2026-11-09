"""Build a runtime profile (and optionally a game manifest) from a Jill-engine EXE.

This is the total-conversion entry point: point it at an EXE plus the asset
folder and it writes `profiles/<id>.profile` (engine data) and
`games/<id>.game` (assets/boards).  The port loads both at start-up, so the end
user never edits or rebuilds the source.

What it detects automatically (validated on jill1.exe, jill3.exe and
ONESIMUS.EXE):

  * MZ header size / load image
  * the kind-name block (`PLAYER`, `APPLE`, ... `ROMAN`)
  * the kind-table initialiser (the `mov word [addr], imm` run that fills it)
  * the DGROUP image base, by solving name-string offsets against the name table
  * the width/height/flags/tileset/count tables, by matching the base profile
  * the nine handler tables, by matching the whole contiguous block
  * the item-icon table
  * the 768-byte default palette at DGROUP:0x0194
  * the board list, board extension, .sha/.dma names in the asset folder

Anything it cannot prove is inherited from the base profile, and the report
says so.

Usage:
  py -3 tools/mkprofile.py --exe D:/Games/TC/GAME.EXE --id mytc --data D:/Games/TC
  py -3 tools/mkprofile.py --exe GAME.EXE --id mytc --data DATA --base jill1
"""

import argparse
import json
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)

HANDLER_FIELDS = (
    ("fly_frame", 4),
    ("fly_y", 4),
    ("phoenix_frame", 8),
    ("phoenix_mirror", 8),
    ("demon_frame", 6),
    ("fatso_frame", 4),
    ("expl2_frame", 8),
    ("bat_frame", 4),
    ("bat_y", 4),
)
KIND_FIELDS = ("width", "height", "flags", "tileset", "default_count")


class Report:
    def __init__(self, quiet):
        self.quiet = quiet
        self.lines = []

    def say(self, text):
        self.lines.append(text)
        if not self.quiet:
            print(text)

    def warn(self, text):
        self.lines.append("warning: " + text)
        print("warning: " + text, file=sys.stderr)


# ---------------------------------------------------------------- MZ / image


def load_image(path):
    data = open(path, "rb").read()
    if data[:2] != b"MZ":
        raise SystemExit("%s is not an MZ executable" % path)
    header = struct.unpack_from("<H", data, 8)[0] * 16
    if header >= len(data):
        raise SystemExit("%s: bad MZ header size" % path)
    return data, data[header:], header


# ------------------------------------------------------------ kind names


def find_kind_names(image):
    """Return [(name, image_offset)] for the contiguous kind-name block."""
    start = image.find(b"PLAYER\x00APPLE\x00KNIFE\x00KILLME")
    if start < 0:
        return []
    names = []
    p = start
    while p < len(image):
        end = image.find(b"\x00", p)
        if end < 0:
            break
        token = image[p:end]
        if not token or len(token) > 12:
            break
        if not all(65 <= c <= 90 or 48 <= c <= 57 or c == 95 for c in token):
            break
        names.append((token.decode("ascii"), p))
        p = end + 1
    return names


# --------------------------------------------------- kind-table initialiser


def parse_initializer(image):
    """Longest run of `mov word [addr], imm16` / `mov [addr], ds` records."""
    best = None  # (start, count, dict)
    pos = 0
    while True:
        j = image.find(b"\xc7\x06", pos)
        if j < 0:
            break
        q = j
        recs = {}
        count = 0
        while q < len(image) - 6:
            if image[q:q + 2] == b"\xc7\x06":
                addr, value = struct.unpack_from("<HH", image, q + 2)
                recs[addr] = value
                q += 6
                count += 1
            elif image[q:q + 2] == b"\x8c\x1e":
                recs[struct.unpack_from("<H", image, q + 2)[0]] = -1
                q += 4
                count += 1
            else:
                break
        if best is None or count > best[1]:
            best = (j, count, recs)
        pos = j + 1
    return best


def find_name_table(recs, names):
    """Solve the DGROUP image base from the name offset/segment table."""
    n = len(names)
    for addr in recs:
        if addr < 0:
            continue
        if any((addr + 4 * k) not in recs for k in range(n)):
            continue
        deltas = set(names[k][1] - recs[addr + 4 * k] for k in range(n))
        if len(deltas) != 1:
            continue
        base = deltas.pop()
        if base > 0 and base % 16 == 0:
            return addr, base
    return None, None


def read_field_tables(recs, base_values, count):
    """Find each kind field table by matching the base profile's values."""
    found = {}
    for field in KIND_FIELDS:
        want = base_values.get(field)
        if not want:
            continue
        for addr in recs:
            if addr < 0:
                continue
            if any((addr + 2 * k) not in recs for k in range(count)):
                continue
            if all(recs[addr + 2 * k] == want[k] for k in range(count)):
                found[field] = addr
                break
    return found


# ------------------------------------------------------------ static blocks


def find_handler_tables(image, base_offsets, base_values):
    """Locate the handler-table block; return its DGROUP origin or None."""
    anchor = "phoenix_frame"
    if anchor not in base_values or anchor not in base_offsets:
        return None
    rel = {f: base_offsets[f] - base_offsets[anchor] for f, _ in HANDLER_FIELDS
           if f in base_offsets}
    pattern = struct.pack("<%dh" % len(base_values[anchor]),
                          *base_values[anchor])
    pos = 0
    while True:
        hit = image.find(pattern, pos)
        if hit < 0:
            return None
        origin = hit - rel[anchor]
        if origin >= 0:
            ok = True
            for field, size in HANDLER_FIELDS:
                if field not in rel or field not in base_values:
                    continue
                at = origin + rel[field]
                if at < 0 or at + 2 * size > len(image):
                    ok = False
                    break
                if image[at:at + 2 * size] != struct.pack(
                        "<%dh" % size, *base_values[field]):
                    ok = False
                    break
            if ok:
                return origin
        pos = hit + 1


def find_u16_block(image, values):
    pat = struct.pack("<%dH" % len(values), *values)
    hits = []
    pos = 0
    while True:
        hit = image.find(pat, pos)
        if hit < 0:
            return hits
        hits.append(hit)
        pos = hit + 1


def find_palette(image, dgroup_base):
    if dgroup_base is None:
        return None
    off = dgroup_base + 0x194
    pal = image[off:off + 768]
    if len(pal) != 768 or max(pal) > 63 or not any(pal):
        return None
    return pal


def find_level_titles(image, dgroup_base):
    """36-entry (offset, segment) table whose entry 3 is the shoes caption."""
    seg = dgroup_base // 16
    for q in range(0, len(image) - 4 * 36, 2):
        if struct.unpack_from("<H", image, q + 2)[0] != seg:
            continue
        titles = []
        for k in range(36):
            off, s = struct.unpack_from("<HH", image, q + 4 * k)
            if s != seg:
                break
            at = dgroup_base + off
            end = image.find(b"\x00", at, at + 300)
            if end < 0:
                break
            text = image[at:end]
            if any(c < 0x20 and c not in (0x0D,) for c in text):
                break
            titles.append(text.decode("latin1"))
        if len(titles) == 36 and titles[3].startswith("Hey,"):
            return titles
    return None


# --------------------------------------------------------------- base profile


def parse_profile(path, profiles_dir, depth=0):
    """Read a profiles/*.profile (with inheritance) into a dict."""
    seen = set()
    if depth > 8:
        raise SystemExit("profile base chain too deep at %s" % path)
    data = {}
    base_id = None
    keys = []
    for line in open(path, encoding="ascii", errors="replace"):
        line = line.strip()
        if not line or line[0] in "#;[":
            continue
        if "=" not in line:
            continue
        key, value = (s.strip() for s in line.split("=", 1))
        if key.lower() == "base":
            base_id = value
        else:
            keys.append((key.lower(), value))
    if base_id:
        base_path = os.path.join(profiles_dir, base_id + ".profile")
        if not os.path.exists(base_path):
            raise SystemExit("base profile %s not found" % base_path)
        if base_path in seen:
            raise SystemExit("cyclic base chain at %s" % base_path)
        seen.add(base_path)
        data = parse_profile(base_path, profiles_dir, depth + 1)
    data.setdefault("kinds", [])
    data.setdefault("tables", {})
    for key, value in keys:
        if key == "kind_count":
            data["kind_count"] = int(value)
        elif key == "epic_score":
            data["epic_score"] = int(value)
        elif key == "object_max":
            data["object_max"] = int(value)
        elif key == "object_stride":
            data["object_stride"] = int(value)
        elif key == "item_icons":
            data["item_icons"] = [int(v, 0) for v in value.replace(",", " ").split()]
        elif key == "kind":
            parts = [p.strip() for p in value.split(",")]
            if len(parts) >= 7:
                k = int(parts[0], 0)
                while len(data["kinds"]) <= k:
                    data["kinds"].append(None)
                data["kinds"][k] = {
                    "name": parts[1],
                    "width": int(parts[2], 0),
                    "height": int(parts[3], 0),
                    "flags": int(parts[4], 0),
                    "tileset": int(parts[5], 0),
                    "default_count": int(parts[6], 0),
                }
        elif key in dict(HANDLER_FIELDS):
            data["tables"][key] = [int(v, 0) for v in value.replace(",", " ").split()]
        elif key == "handler_offsets":
            data["handler_offsets"] = [
                int(v, 0) for v in value.replace(",", " ").split()]
        elif key == "dgroup_base":
            data["dgroup_base"] = int(value, 0)
        elif key in ("id", "episode", "data", "shapes", "tiles", "board_ext",
                     "first_board", "intro_board", "palette", "suit",
                     "hud_icon_bank", "boards", "pages", "text_pages"):
            data[key] = value
    data["id"] = data.get("id")
    return data


def base_field_values(base):
    out = {f: [] for f in KIND_FIELDS}
    for kind in base.get("kinds", []):
        if kind is None:
            raise SystemExit("base profile has a kind gap")
        for field in KIND_FIELDS:
            out[field].append(kind[field])
    return out


def pick_base(image, profiles_dir, names, dgroup, forced=None):
    if forced:
        return forced
    best = None
    for entry in sorted(os.listdir(profiles_dir)):
        if not entry.endswith(".profile"):
            continue
        path = os.path.join(profiles_dir, entry)
        try:
            prof = parse_profile(path, profiles_dir)
        except SystemExit:
            continue
        kinds = prof.get("kinds") or []
        same = sum(1 for i, (name, _) in enumerate(names)
                   if i < len(kinds) and kinds[i] and kinds[i]["name"] == name)
        if same < 8:
            continue
        # A derivative keeps the DGROUP layout of the build it was made from,
        # so the base whose recorded image base is nearest is the best guess;
        # the handler block confirms it.
        base_dgroup = prof.get("dgroup_base")
        dist = abs(base_dgroup - dgroup) if base_dgroup else 1 << 30
        block = 0
        offsets = prof.get("handler_offsets")
        if offsets and prof.get("tables"):
            base_offsets = {field: offsets[i]
                            for i, (field, _) in enumerate(HANDLER_FIELDS)}
            if find_handler_tables(image, base_offsets, prof["tables"]) is not None:
                block = 1
        score = (-dist, same, block)
        if best is None or score > best[0]:
            best = (score, prof["id"] or entry[:-8])
    return best[1] if best else None


# ------------------------------------------------------------------- boards


def board_count(path):
    try:
        size = os.path.getsize(path)
        with open(path, "rb") as f:
            f.seek(0x4000)
            head = f.read(2)
    except OSError:
        return None
    if len(head) < 2 or size < 0x4000 + 2 + 0x46:
        return None
    count = struct.unpack("<H", head)[0]
    if 0x4002 + count * 31 + 0x46 > size:
        return None
    return count


def board_sort_key(name):
    return (0, int(name)) if name.isdigit() else (1, name.lower())


def scan_data_dir(data_dir, prefer_ext=None):
    exts = {}
    shapes = []
    tiles = []
    vcl = []
    for entry in sorted(os.listdir(data_dir)):
        path = os.path.join(data_dir, entry)
        if not os.path.isfile(path):
            continue
        low = entry.lower()
        if low.endswith(".sha"):
            shapes.append(entry)
        elif low.endswith(".dma"):
            tiles.append(entry)
        elif low.endswith(".vcl"):
            vcl.append(entry)
        else:
            if board_count(path) is not None:
                ext = os.path.splitext(entry)[1]
                exts.setdefault(ext, []).append(os.path.splitext(entry)[0])
    if not exts:
        return None
    if prefer_ext and prefer_ext in exts:
        ext = prefer_ext
    else:
        ext = sorted(exts, key=lambda e: (-len(exts[e]), e))[0]
    boards = sorted(exts[ext], key=board_sort_key)
    return {
        "board_ext": ext,
        "boards": boards,
        "shapes": shapes[0] if len(shapes) == 1 else (shapes or [None])[0],
        "tiles": tiles[0] if len(tiles) == 1 else (tiles or [None])[0],
        "pages": vcl[0] if len(vcl) == 1 else (vcl or [None])[0],
    }


# --------------------------------------------------------------------- main


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--exe", required=True, help="the game's MZ executable")
    ap.add_argument("--id", required=True, help="profile/game id, e.g. mytc")
    ap.add_argument("--title", help="title shown in the game list")
    ap.add_argument("--base", help="profile to inherit from (auto-detected)")
    ap.add_argument("--save-prefix", help="save file stem, e.g. onesave")
    ap.add_argument("--save-slots", type=int, default=6,
                    help="number of save slots (default 6)")
    ap.add_argument("--data", help="asset folder; also emits a game manifest")
    ap.add_argument("--profiles-dir", default=os.path.join(ROOT, "profiles"))
    ap.add_argument("--games-dir", default=os.path.join(ROOT, "games"))
    ap.add_argument("--board-ext", help="override the detected board extension")
    ap.add_argument("--boards", help="override the board list (comma separated)")
    ap.add_argument("--first-board", help="override the start board")
    ap.add_argument("--intro-board", help="override the menu backdrop board")
    ap.add_argument("--no-game", action="store_true",
                    help="write only the profile, no game manifest")
    ap.add_argument("--force", action="store_true", help="overwrite existing files")
    ap.add_argument("--quiet", action="store_true")
    args = ap.parse_args()

    rep = Report(args.quiet)
    exe_data, image, header = load_image(args.exe)
    rep.say("exe      %s" % args.exe)
    rep.say("size     %d bytes, MZ header %#x, image %d bytes"
            % (len(exe_data), header, len(image)))

    names = find_kind_names(image)
    if not names:
        raise SystemExit("no kind-name block found: is this a Jill-engine build?")
    rep.say("kinds    %d names, first at image %#x (%s)"
            % (len(names), names[0][1], names[0][0]))

    init = parse_initializer(image)
    if not init:
        raise SystemExit("no kind-table initialiser found")
    start, count, recs = init
    rep.say("init     %d stores at image %#x" % (count, start))

    name_table, dgroup = find_name_table(recs, names)
    if dgroup is None:
        raise SystemExit("could not solve the DGROUP base from the name table")
    stored_seg = dgroup // 16
    rep.say("dgroup   image %#x, stored segment %#04x, runtime segment %#04x"
            % (dgroup, stored_seg, stored_seg + 0x1000))

    base_id = pick_base(image, args.profiles_dir, names, dgroup, args.base)
    base = None
    if base_id:
        base_path = os.path.join(args.profiles_dir, base_id + ".profile")
        if not os.path.exists(base_path):
            raise SystemExit("base profile %s not found" % base_path)
        base = parse_profile(base_path, args.profiles_dir)
        rep.say("base     %s" % base_id)
    else:
        rep.warn("no matching base profile; writing a profile with no inheritance")

    base_values = base_field_values(base) if base else {}
    base_count = len(base.get("kinds") or []) if base else 0
    kind_count = max(len(names), base_count)
    if base_count and len(names) < base_count:
        kind_count = base_count
    rep.say("count    %d kinds" % kind_count)

    # Read the numeric kind tables (only possible when the base values match).
    detected = {f: [] for f in KIND_FIELDS}
    base_len = min((len(v) for v in base_values.values()), default=0)
    match_count = min(len(names), kind_count, base_len) if base_len else 0
    field_addrs = read_field_tables(recs, base_values, match_count)
    for field, addr in field_addrs.items():
        detected[field] = [recs[addr + 2 * k] for k in range(len(names))]
        base_at = field_addrs.get(field)
        rep.say("table    %-13s at %#x (%d values)" % (field, base_at, len(names)))
    missing = [f for f in KIND_FIELDS if f not in field_addrs]
    if missing:
        rep.warn("could not locate %s; those values are inherited"
                 % ", ".join(missing))

    # Handler tables / icons / palette.
    if base and base.get("tables") and base.get("handler_offsets"):
        offsets = base["handler_offsets"]
        base_offsets = {field: offsets[i]
                        for i, (field, _) in enumerate(HANDLER_FIELDS)}
        origin = find_handler_tables(image, base_offsets, base["tables"])
        if origin is not None:
            rep.say("tables   handler block at DGROUP:%#x" % origin)
        else:
            rep.warn("handler tables differ or were not found; inherited")
    else:
        rep.warn("base has no handler offsets to verify; tables inherited")

    icons = None
    if base and base.get("item_icons"):
        hits = find_u16_block(image, base["item_icons"])
        if hits:
            icons = base["item_icons"]
            rep.say("icons    item table at image %#x" % hits[0])
        else:
            rep.warn("item-icon table differs; inherited")

    palette = find_palette(image, dgroup)
    if palette:
        rep.say("palette  768 bytes at DGROUP:0x0194")
    else:
        rep.warn("no palette at DGROUP:0x0194")

    titles = find_level_titles(image, dgroup)
    if titles:
        rep.say("titles   36 level captions (index = level id + 4)")
    else:
        rep.warn("level-title table not found; captions inherited")

    # ------------------------------------------------------------ write profile
    os.makedirs(args.profiles_dir, exist_ok=True)
    out_profile = os.path.join(args.profiles_dir, args.id + ".profile")
    if os.path.exists(out_profile) and not args.force:
        raise SystemExit("%s exists (use --force)" % out_profile)

    lines = [
        "# Generated by tools/mkprofile.py - review before shipping.",
        "# exe: %s (%d bytes, MZ header %#x)" % (args.exe, len(exe_data), header),
        "# DGROUP image base %#x (stored segment %#04x, runtime %#04x)."
        % (dgroup, stored_seg, stored_seg + 0x1000),
        "",
        "id = %s" % args.id,
    ]
    if base_id:
        lines.append("base = %s" % base_id)
    lines.append("kind_count = %d" % kind_count)
    if args.save_prefix:
        lines.append("save_prefix = %s" % args.save_prefix)
        lines.append("save_slots = %d" % args.save_slots)

    name_rows = []
    for k in range(kind_count):
        name = names[k][0] if k < len(names) else None
        old = base["kinds"][k] if base and k < base_count else None
        if name is None:
            continue
        if old and old["name"] == name and all(
                detected[f] and detected[f][k] == old[f] for f in KIND_FIELDS):
            continue
        vals = {}
        for field in KIND_FIELDS:
            if detected[field] and k < len(detected[field]):
                vals[field] = detected[field][k]
            elif old:
                vals[field] = old[field]
            else:
                vals[field] = 0
        name_rows.append("kind = %d, %s, %d, %d, 0x%04X, 0x%02X, %d"
                         % (k, name, vals["width"], vals["height"],
                            vals["flags"], vals["tileset"],
                            vals["default_count"]))
    lines.extend(name_rows)
    if titles:
        for i, raw in enumerate(titles):
            text = raw.replace("\r", "\n").rstrip("\n")
            if not text:
                continue
            esc = text.replace("\\", "\\\\").replace("\n", "\\n")
            lines.append("level_title = %d, %s" % (i, esc))
    lines.append("")
    text = "\n".join(lines)
    open(out_profile, "w", encoding="ascii", newline="\n").write(text)
    rep.say("wrote    %s (%d bytes, %d kind override(s))"
            % (out_profile, len(text), len(name_rows)))

    # ------------------------------------------------------------ write game
    if args.data and not args.no_game:
        scan = scan_data_dir(args.data, base.get("board_ext") if base else None)
        if not scan:
            raise SystemExit("%s: no board files found" % args.data)
        board_ext = args.board_ext or scan["board_ext"]
        boards = ([b.strip() for b in args.boards.split(",")]
                  if args.boards else scan["boards"])
        by_lower = {b.lower(): b for b in boards}

        def pick(*names):
            for name in names:
                if name in by_lower:
                    return by_lower[name]
            return None

        first = (args.first_board or pick("map", "0", "1") or boards[0])
        intro = (args.intro_board or pick("intro") or first)
        if args.first_board and args.first_board not in boards:
            rep.warn("first_board %s is not in the board list"
                     % args.first_board)
        rep.say("boards   %d with %s (first %s, intro %s)"
                % (len(boards), board_ext, first, intro))

        shape = scan["shapes"] or ""
        tiles = scan["tiles"] or ""
        if not shape:
            rep.warn("no .sha in %s; the manifest inherits the base shape bank"
                     % args.data)
        if not tiles:
            rep.warn("no .dma in %s; the manifest inherits the base tile table"
                     % args.data)

        gpath = os.path.join(args.games_dir, args.id + ".game")
        if os.path.exists(gpath) and not args.force:
            raise SystemExit("%s exists (use --force)" % gpath)
        pal_ref = ""
        if palette:
            pal_path = os.path.join(args.profiles_dir, args.id + ".pal")
            os.makedirs(args.profiles_dir, exist_ok=True)
            # The extracted palette is 6-bit (VGA DAC values); the port's
            # --palette path is raw 8-bit RGB, so expand it here.
            open(pal_path, "wb").write(
                bytes(min(255, v * 255 // 63) for v in palette))
            pal_ref = pal_path.replace("\\", "/")
            rep.say("wrote    %s" % pal_path)

        lines = [
            "# Generated by tools/mkprofile.py - review before shipping.",
            "id = %s" % args.id,
            "title = %s" % (args.title or args.id.upper()),
            "profile = %s" % args.id,
            "data = %s" % args.data.replace("\\", "/"),
        ]
        if shape:
            lines.append("shapes = %s" % shape)
        if tiles:
            lines.append("tiles = %s" % tiles)
        if scan["pages"]:
            lines.append("pages = %s" % scan["pages"])
        lines.append("board_ext = %s" % board_ext)
        lines.append("first_board = %s" % first)
        lines.append("intro_board = %s" % intro)
        if pal_ref:
            lines.append("palette = %s" % pal_ref)
        lines.append("boards = " + ", ".join(boards))
        text = "\n".join(lines) + "\n"
        os.makedirs(args.games_dir, exist_ok=True)
        open(gpath, "w", encoding="ascii", newline="\n").write(text)
        rep.say("wrote    %s" % gpath)
        rep.say("play     jill --game %s" % args.id)

    return 0


if __name__ == "__main__":
    sys.exit(main())
