# Music catalog [verified]

Every shipped board carries its level music as CHECKPT strings beginning with
`*`.  The first music string in board order is the level's start track; later
ones are section changes and fire when the player touches that checkpoint.
The port now starts the first track when the board loads and keeps handling
later triggers.

Track names are the original `.ddt` / `.ark` stems.  An external pack can
replace a track with `music/<stem>.<ext>` (for example `music/zeppelin.ogg`).

Title/menu tracks: `funky.ddt` (Jill 1), `peaks.ddt` (Jill 2), `wild.ddt`
(Jill 3) and `SONG5.ARK` (Onesimus).

## Jill 1 - Jill of the Jungle

| Board | Start track | Later music triggers |
| --- | --- | --- |
| `1` | `zeppelin.ddt` | `stealth.ddt` |
| `2` | `spiders.ddt` | `zeppelin.ddt` |
| `3` | `ominous.ddt` | `jupiter.ddt` |
| `4` | `dan.ddt` | `dan.ddt` |
| `6` | `funky.ddt` | `it.ddt`, `ominous.ddt` |
| `9` | `jupiter.ddt` | `stealth.ddt` |
| `50` | `seven.ddt` | - |
| `map` | `funky.ddt` | - |
| `intro` | `funky.ddt` | `funky.ddt` |

## Jill 2 - Jill goes Underground

| Board | Start track | Later music triggers |
| --- | --- | --- |
| `0` | `peaks.ddt` | - |
| `1` | `cubist.ddt` | `action.ddt` |
| `3` | `dungo.ddt` | - |
| `9` | `bonzo.ddt` | - |
| `10` | `action.ddt` | `cubist.ddt` |
| `11` | `mobydick.ddt` | - |
| `13` | `undrgrnd.ddt` | - |
| `15` | `peaks.ddt` | - |
| `17` | `cubist.ddt` | `peaks.ddt` |
| `intro` | `funky.ddt` | `funky.ddt` |

## Jill 3 - Jill Saves the Prince

| Board | Start track | Later music triggers |
| --- | --- | --- |
| `1` | `seven.ddt` | - |
| `2` | `dan.ddt` | - |
| `3` | `stealth.ddt` | - |
| `4` | `kuntamon.ddt` | - |
| `5` | `wild.ddt` | - |
| `6` | `desert.ddt` | - |
| `7` | `alien.ddt` | - |
| `8` | `dan.ddt` | - |
| `9` | `seven.ddt` | - |
| `10` | `deepevil.ddt` | - |
| `11` | `kuntamon.ddt` | - |
| `12` | `cubist.ddt` | - |
| `13` | `kuntamon.ddt` | - |
| `14` | `wild.ddt` | `neoclass.ddt` |
| `map` | `neoclass.ddt` | - |
| `intro` | `funky.ddt` | - |

## Onesimus - A Quest for Freedom

Onesimus stores its CMF tracks as `SONG*.ARK`.

| Board | Start track | Later music triggers |
| --- | --- | --- |
| `1` | `song2.ark` | `song3.ark` |
| `3` | `song9.ark` | - |
| `6` | `song4.ark` | `song7.ark` |
| `8` | `song1.ark` | `song1.ark` |
| `10` | `song14.ark` | - |
| `11` | `song13.ark` | `song16.ark`, `song13.ark`, `song11.ark` |
| `17` | `song8.ark` | - |
| `18` | `song10.ark` | - |
| `19` | `song12.ark` | `song4.ark` |
| `21` | `song6.ark` | - |
| `22` | `song1.ark` | `song1.ark`, `song4.ark` |
| `26` | `song3.ark` | `song3.ark` |
| `28` | `song9.ark` | - |
| `29` | `song2.ark` | `song15.ark` |
| `MAP` | `song5.ark` | - |

## Implementation notes [verified]

* `Engine::load_board` sets the music request from the first valid
  `*...ddt` / `*...ark` CHECKPT string.
* A music CHECKPT with `p13` equal to the current level id still fires; only
  level-entrance strings are suppressed to avoid immediately re-entering the
  board Jill just came from.
* The app resolves the name against the game's data directory and starts the
  CMF track.  A mounted pack's `music/<stem>.<ext>` replaces it.
