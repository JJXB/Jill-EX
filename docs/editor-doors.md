# Doors in JILL MAKER

This is a practical guide to making a working **key door** or **gem door** from
scratch, and to the properties that link doors to their triggers.

The short version: a door is a normal **DOOR object** (kind 24) standing on a
specific pair of **map tiles**, plus a **trigger sender** (usually a PAD) on the
same **P13 trigger channel**.  The object is invisible in a level; the map tiles
are the door you see and collide with.  The object only handles the opening.

For the full per-kind property reference see `docs/editor-object-properties.md`.

---

## 1. The two door types

The tile under the DOOR object decides which type it is:

| Type | Tile at the door cell | Extra tiles | Consumes |
| --- | --- | --- | --- |
| **Key door** | `0x0A1` (`DOORT`) | `0x0A2` (`DOORB`) directly below it | item 1 = key |
| **Gem door** | `0x0BE` (`MAPDOOR`) | none required below | item 3 = gem |

The DOOR object itself is 16 x 24 pixels, so it covers the anchor cell and the
upper half of the cell below.  For a key door that lower half is the `DOORB`
tile.  For a gem door the cell below is normally ordinary floor.

You do **not** need to paint the object itself; the editor's presets paint the
map tiles and place the DOOR object together, which is why they are the safe
route.

---

## 2. What the DOOR object needs

Place the object at the top-left of the door cell:

| Property | Value for a door |
| --- | --- |
| `KIND` | `24` (DOOR) |
| `X` | `column * 16` |
| `Y` | `row * 16` |
| `W` / `H` | `16` / `24` |
| `STATE` | `0` = closed |
| `P13` | trigger channel, shared with the sender |
| `DX` / `DY` | gem doors only: destination cell offset |
| `PARAM` (`P0F`) | unused |
| `P11` | unused |
| `P1B` | unused |
| `TTL` | unused |
| `STRING` | unused |
| `FLAGS` | leave `0`; these are runtime collision/redraw bits |

Notes:

* `X` and `Y` must be 16-pixel aligned.  The DOOR handler reads only
  `x >> 4` and `y >> 4`; a door placed between cells may look wrong or trigger
  from the wrong cell.
* `STATE` must be `0`.  A copied door with `STATE > 0` immediately plays its
  opening animation and vanishes when the playtest starts.
* `P13` is the **trigger channel**, not an item or score value.
* `FLAGS` in the object panel is the object's runtime board field.  The
  `0x0100` "receives triggers" flag belongs to the kind table, not this field.

### Key door target tile

When a key door opens, the original does not invent an opening; it copies the
tiles from the column immediately to its **left** into the two door cells.  In
the shipped boards that left column is open/empty (`0`), so the doorway becomes
walkable.

Practical rule: put a key door with an **open or empty tile directly to its
left** at both rows.  If the left tile is solid, the "open" door will become
solid.  A key door at column `0` has no left column and will not clear.

### Gem door target tile

A gem door copies one tile into its cell: the tile at

```
(column + DX, row + DY)
```

The default used by the editor preset is `DX = 0, DY = 1` (the cell below).
Point that at a passable neighbour.  If it points at a wall or off the map,
the door spends the gem but the doorway stays blocked.

You can adjust `DX` / `DY` in the object property panel (`O`).

---

## 3. Trigger channels: how a door is opened

A door is **not** opened by the player touching it.  It opens when something
sends trigger code **3** on its `P13` channel.

All objects whose kind has the `0x0100` flag are trigger receivers.  In Jill 1
that includes:

* DOOR (24)
* FALLDOOR (25)
* BRIDGER (26)
* STALAG (38)
* MEGA (42)
* KNIGHT (44)
* SKULL (51)
* ELEV (61)

The sender and receiver must have the **same `P13` value**.  A sender's `P13`
is the channel it transmits on; a receiver's `P13` is the channel it listens
to.

### Common senders

| Kind | Object | What it sends |
| --- | --- | --- |
| `15` | **PAD** | Code `3` when touched while `STATE == 0`.  `STATE == -1` sends `4`; `STATE == 1` sends `5`. |
| `32` | **SWITCH** | Sends code `3` only when `DX == 1`; otherwise it sends `4`/`5`.  Requires the player to hold Up/Down while touching it. |
| `52` | **BUTTON** | Sends `4` when pressed and `5` when released.  Never sends `3`. |

Because DOOR only reacts to code `3`:

* **Use a PAD for a normal door.**  Leave the PAD `STATE` at `0`.
* A SWITCH can control a door if you set the switch `DX = 1`.
* A BUTTON is for receivers that use pressed/released codes (for example a
  BRIDGER or KNIGHT); it will not open a DOOR.

In a level the PAD is normally an invisible trigger.  Place it on the floor
where the player will walk, in front of the door.  The shipped boards put it
immediately beside the door on the approach side.

### Trigger codes

Only three trigger codes exist:

| Code | Meaning |
| --- | --- |
| `3` | neutral / "use" — what a PAD or a `DX == 1` SWITCH sends |
| `4` | on / pressed |
| `5` | off / released |

DOOR checks `code == 3` and `state == 0`.

---

## 4. What happens when a door opens

### Key door

1. The trigger sends code 3.
2. The door consumes one inventory item `1` (key).
3. `STATE` becomes `1`.
4. Both door rows copy the tile from `column - 1`.
5. `STATE` counts up to `0x10`, then the object is removed.

If the player has no key, it does nothing.  Place a **KEY** pickup (kind 14)
somewhere reachable; picking it up adds inventory item 1.

### Gem door

1. The trigger sends code 3.
2. The door consumes one inventory item `3` (gem).
3. The tile at `(column + DX, row + DY)` is copied into the door cell.
4. The door object is removed immediately.

Gems are special: a gem picked up anywhere stays in the inventory until a gem
door spends it.  Place a **GEM** pickup (kind 33) somewhere reachable.

If several doors share one channel, the trigger fires all of them, and each
door tries to spend one key/gem.  Use separate channels when you want one
trigger per door.

---

## 5. Quick recipe: key door with a PAD

1. Enter Object mode (`2`).
2. Press `R`, choose **KEY DOOR**, and click the door cell.
   * This paints `DOORT` / `DOORB` and places the DOOR object.
   * Leave the tile immediately to the left open/empty.
3. Choose the object palette (`E` / `Tab`) and place a **PAD** (kind 15) on
   the floor in front of the door.
4. Give both objects the same channel:
   * Select the DOOR, press `O`, move to `P13`, set it to a small unique value
     such as `5`.
   * Select the PAD, press `O`, move to `P13`, press `Enter`/pad `A`.  The
     link helper sets the PAD's `P13` to the nearest receiver (the door).
   * Alternatively set both `P13` values manually to `5`.
5. Check the PAD: `STATE` must be `0`.
6. Place a **KEY** pickup (kind 14) somewhere the player can reach.
7. Playtest and walk over the PAD.  With a key, the door opens.

## 6. Quick recipe: gem door with a PAD

1. Enter Object mode (`2`).
2. Press `R`, choose **GEM DOOR**, and click the door cell.
   * This paints `MAPDOOR` and places the DOOR object.
3. Place a **PAD** (kind 15) on the approach floor, `STATE = 0`.
4. Link the same `P13` channel on both objects (as above).
5. Place a **GEM** pickup (kind 33) somewhere reachable.
6. Select the DOOR, open `O`, and check `DX` / `DY`.
   * Default `0, 1` opens into the cell below.
   * Change it if that cell is solid or off the map.
7. Playtest and walk over the PAD.

## 7. Manual setup (without presets)

If you prefer to build it by hand:

1. Tile mode (`1`), Tile palette (`T`).
2. For a key door, paint `0x0A1` (`DOORT`) at the anchor cell and `0x0A2`
   (`DOORB`) directly below it.
3. For a gem door, paint `0x0BE` (`MAPDOOR`) at the anchor cell.
4. Object mode (`2`), choose kind `24` (DOOR), and place it at the same cell.
5. Open `O` and confirm `X = column * 16`, `Y = row * 16`, `W = 16`,
   `H = 24`, `STATE = 0`.
6. Add the PAD and link the `P13` channel as described above.

The map tiles are the visible door and the collision surface.  The DOOR object
is the invisible logic on top of them.

---

## 8. The `P13` link helper

In the object property panel the `P13` row shows the channel and the number of
other trigger objects sharing it, for example:

```
P13  5 L2
```

`L2` means two other trigger senders/receivers are on channel 5.

Pressing `Enter` / pad `A` on the `P13` row links the object to the nearest
trigger counterpart:

* on a sender (PAD/SWITCH/BUTTON), it finds the nearest receiver (DOOR etc.)
  and copies that receiver's channel;
* on a receiver (DOOR etc.), it finds the nearest sender and copies its
  channel.

The helper only changes the object you are editing.  Use it on one side after
setting the channel on the other side, then check that the `L` count is at
least `1`.

Channel `0` is valid and is what presets use by default.  For anything beyond a
single door/pad pair, give each pair a small unique number so channels do not
cross-link.

---

## 9. Troubleshooting

| Symptom | Likely cause |
| --- | --- |
| Door is visible but walking into it does nothing | Doors are not touch-activated.  Add a PAD or SWITCH and link `P13`. |
| PAD is stepped on but the door stays shut | `P13` mismatch, PAD `STATE` not `0`, DOOR `STATE` not `0`, or the player has no key/gem. |
| Door disappears as soon as playtest starts | The DOOR object's `STATE` is greater than `0`.  Set it back to `0`. |
| Gem door opens but the doorway is still blocked | `DX`/`DY` points at a solid tile or outside the map.  Point it at a passable neighbour. |
| Key door opens but leaves solid tiles | The tile to the left of the door is solid; use an open/empty left column. |
| Key door at the far-left edge does nothing | Key doors need a `column - 1` donor column.  Move the door right one tile. |
| Closed door is walk-through | The map tiles under the DOOR object were cleared or painted over.  The DOOR object itself is not collision; repaint the door tiles. |
| One pad opens several doors at once | All those doors share the same `P13`.  Give each door its own channel if that is not intended. |
| Door's map art looks wrong | Key doors need `DOORT` + `DOORB`; gem doors need `MAPDOOR`.  Using the `R` presets avoids mixing them up. |
| Button will not open a door | BUTTON sends codes 4/5; DOOR only accepts code 3.  Use a PAD or a SWITCH with `DX = 1`. |

---

## 10. Related objects

* **FALLDOOR** (25) is a different object: it falls and turns into a tile.  It
  is triggered by code 3 but does not consume keys or gems, and its `P13` is
  overwritten with the tile it will become when armed.
* **BRIDGER** (26) uses all three codes: code 3 toggles, 4 erases, 5 builds.
* **KNIGHT** (44) uses 3 = swing, 4 = idle, 5 = freeze mid-swing.
* **PAD**, **SWITCH** and **BUTTON** are the normal trigger senders; the PAD is
  the simplest and the one shipped doors almost always use.
