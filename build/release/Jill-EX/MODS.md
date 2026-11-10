## Adding Your Own Mods

The port is data-driven: mods are a `games/<id>.game` manifest plus a
`profiles/<id>.profile` engine profile. You do not need to edit the C++ or
rebuild.

Python 3 is only needed for the tooling:

```bat
py -3 -m pip install -r requirements.txt
```

### 1. Asset-only mod (same engine as a shipped game)

Use this when your mod only replaces boards, art, tiles or text pages.

1. Copy `games/jill1.game` to `games/mymod.game`.
2. Edit the copy:

   ```ini
   id = mymod
   title = My Jill Mod
   profile = jill1
   data = D:/Games/MyJillMod
   shapes = jill1.sha
   tiles = jill.dma
   pages = jill1.vcl
   title_music = funky.ddt
   board_ext = .jn1
   first_board = map
   intro_board = intro
   boards = 1, 2, 3, map, intro
   ```

   `data` is the folder holding the mod assets. `shapes`, `tiles` and `pages`
   are file names inside that folder. `pages` is the `.vcl` bank, which also
   supplies the sampled sound effects. `title_music` is the CMF track played by
   the front end (omit it to stay silent in the menu). `boards` lists the
   playable boards without the extension.

3. Optionally put the folder in `jill.ini` instead:

   ```ini
   [paths]
   mymod = D:/Games/MyJillMod
   ```

   Then `data = mymod` in the manifest.

4. Run it:

   ```bat
   port\build\Release\jill.exe --game mymod
   ```

5. Optional per-mod controls in `jill.ini` (missing keys use `[controls]`):

   ```ini
   [controls.mymod]
   jump = Z|Space
   throw = C
   ```

### 2. New executable or total conversion

Use `tools/mkprofile.py` when you have a Jill-engine EXE and its asset folder.
It writes both the profile and the game manifest.

```bat
py -3 tools/mkprofile.py ^
  --exe D:/Games/MyMod/MYGAME.EXE ^
  --id mymod ^
  --title "My Jill Mod" ^
  --data D:/Games/MyMod ^
  --force
```

This creates:

```text
profiles/mymod.profile
profiles/mymod.pal        (when a palette is found)
games/mymod.game
```

Run it with:

```bat
port\build\Release\jill.exe --game mymod
```

Useful options:

```text
--base jill1          force a specific base profile
--board-ext .jn1      override the detected board extension
--boards 1,2,3,map    override the level-select list
--first-board map     board START GAME loads
--intro-board intro   board used as the menu backdrop
--no-game             write only the profile
--force               overwrite existing files
```

The tool reports anything it could not prove. It can fall back to an existing
base profile (Jill 1/2/3 or Onesimus), so check the report and edit
`profiles/<id>.profile` for any wrong values.

### 3. Editing a generated profile

A profile inherits everything from its `base`. Minimal example:

```ini
id = mymod
base = jill1
kind_count = 69

# Override only the values that differ from the base:
kind = 65, VINECLIMB, 16, 8, 0x0000, 0x3D, 0
epic_score = 25
```

Keys not listed are inherited. Unknown keys print a warning but do not stop the
game.

### 4. Regenerating the built-in data

These scripts rebuild the checked-in data for the three shipped episodes. They
are not needed for an ordinary mod.

```bat
py -3 re\gen_port_data.py
py -3 re\gen_games.py
```

* `re/gen_port_data.py` reads `docs/profiles.json` and writes
  `port/src/gamedata.inc` + `port/src/palette.inc`.
* `re/gen_games.py` reads `docs/games.json` + `docs/profiles.json` and writes
  `games/*.game` + `profiles/*.profile`.

Only use these after editing `docs/profiles.json` or `docs/games.json`.

### 5. Verify the mod

```bat
port\build\Release\jill.exe --list-games
port\build\Release\jill.exe --game mymod --board 1 --ticks 4
```

`--list-games` shows the resolved data directory and board count. A headless
run should print `done: mymod ...` with a sensible object count. Use `--dump`
for a PPM frame or `--dump-objects` for the object table.

### 6. Limits

* The mod must use the same board and asset formats as the shipped games.
* Data-only mods work without rebuilding. Changing handler *behaviour* (what a
  kind does, not its art or tables) still needs C++.
* A new EXE with rewritten kind tables may inherit base values; check the
  `mkprofile.py` report and override the affected `kind = ...` lines.
