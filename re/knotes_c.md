# Object kind handler notes - batch C (kinds 46-63)

Dictionary: `docs/jill1-kind-handlers.md`. Shape ids are `(tileset<<8)|frame`.
Globals without a name: `DGROUP:xxxx`. `3C76`/`3C78` = left-right / up-down key
flags, `3C7A` = ctrl, `3C7E` = "any key" (see engine 1.3). `0xBE3A` = global frame
counter. `2CFF:2AFD(str+10,0)` is a call the API table does not cover (used by the
"Jill form" handlers).

### 46. BEESWARM (1a41:2069) [verified]
- draw: `_drawshape(0x25<<8 + p13 + 4 + (dx>=1 ? 3 : 0), x, y)` -> frames 4..9; 32-entry anim table memcpy'd from `DGROUP:0x0FB4`.
- hit: player only (`other==0`) -> `_hitplayer`; non-player hits ignored, no score.
- update: `state++`, at `state >= 0xA1` (161) -> `_killobj`. `p13 = (p13+1)%3`; `_pointvect(&dx,&dy)` toward the player, then `dx = (dx*2 + rand%2) * tbl[state&31] / 2`, `dy = (dy*2 + rand%2) * tbl[(state+16)&31] / 2`; `_justmove(x+dx, y+dy)`.
- notes: jittered homing that ignores walls (only `_justmove`/mask 1); dies purely on the 161-tick timer.

### 47. CRAB (1a41:22f3) [verified]
- draw: `_drawshape(0x26<<8 + p13, x, y)`, `p13 = (p13+1)&3` in update (4 walk frames).
- hit: player only -> `_hitplayer`.
- update: state 0 (walk): `dx = 0` -> `dx = ±4` (rand); on a 16-px x boundary, if `_cando(x,y,4)==0` (no floor/wall attr) and `rand%2==0` -> `state=1`; otherwise `_crawl(dx,0)`, on failure `dx = -dx`. State 1 (ledge): `dy = ±2` if 0, `_justmove(x,y+dy)` and flip `dy` when blocked; on every 16-px y boundary `_crawl(dx,0)` -> `state=0`, else `_crawl(-dx,0)` -> `state=0, dx=-dx`.
- notes: state 1 = deliberately walking off a ledge; re-attaches to the floor one row at a time.

### 49. EPIC (1a41:29fa) [verified]
- draw: `_drawshape(0x28<<8 + (DGROUP:0xBE3A & 7), x, y)` - 8-frame spin driven by the global frame counter.
- hit: `other==0` only: `_addscore(2, x, y)` then `state++`; if `state < 0xB` play sound (2,0x20), else `_explode1(x,y,10)` + `_killobj` + sound (3,0x30).
- update: returns 1, no motion, never expires by itself.
- notes: 11 player touches to destroy, 2 pts each (22 total); ignores all non-player hits. 35 of them in the level table.

### 50. SPINBLAD (1a41:2b1a) [verified]
- draw: `_drawshape(0x2D<<8 + p13, x, y)`; `p13 = (p13 + sign(dx)) & 3` -> spin direction follows travel.
- hit: player (`other==0`) and `p0f > 7` -> `_killobj` only (it never damages the player); other kinds with `kflags & 0x400` -> `_killobj` + `_killaward(other)` + sound (3,10).
- update: `p0f++`; `p0f >= 0x40` or offscreen -> `_killobj`. Else scan the live objects for the nearest kind with `kflags & 0x400` (not object 0) within Manhattan distance `< 0x60`, `_vectdist(it, self, &dir)`, `dx += dir`, `dy` forced to at least +1; `dx`/`dy` clamped to ±0xC. `_touchbkgnd` at the target cell (each 0xA7 tile eaten adds 10 to `p0f`), `_justmove` x with `dx` bounce, `_justmove` y with sound (1,0x23) + `dy` bounce, then `_killobj` if still stuck.
- notes: gravity-driven homing blade, ~64 ticks of life; a player touch kills it without hurting him.

### 51. SKULL (1a41:3029) [verified]
- draw: always `_drawshape(0x2F<<8 + tblA[p11/2 & 3], x, y)` (4 ints at `DGROUP:0x1014`); if `state != 0` also `tblB[p11]` at `(x, y+5)` and `tblB[(p11+4)&7]` at `(x+10, y+6)` (8 ints at `DGROUP:0x1004`).
- hit: no action-1 branch at all.
- update: `state == 0` -> return 0 (frozen); else `p11 = (p11+1)&7` and return 1.
- notes: action 3 ("trigger") is the only way in: if `state == 0` then `state = 1`, `p11 = 0`, sound (4,0x21); otherwise return 0. Pure decoration trigger - no damage, no score.

### 52. BUTTON (1a41:3261) [verified]
- draw: `_drawshape((0x31<<8 | 1) - state + (dx>0)*2, x, y)` -> 4 frames: 0x3100/0x3101 (dx<=0) and 0x3102/0x3103 (dx>0), so `dx` selects the side and `state` up/down.
- hit: if `p0f == 0` (3-tick debounce): `state = 1-state`; `state==1` -> `_sendtrig(p13,4,self)`, else `_sendtrig(p13,5,self)`; sound (3,0x2C); `p0f = 3`.
- update: decrement `p0f` if > 0, return 0.
- notes: toggles on every touch - trigger 4 = pressed, 5 = released, both to objects whose `p13` matches the button's `p13`.

### 53. PAC (1a41:3402) [verified]
- draw: `_drawshape(0x32<<8 + f, x, y)` with f = 1 if `dx<0`, 2 if `dy<0`, 3 if `dy>=1`, else 0.
- hit: player -> `_hitplayer`; non-player whose kind byte == `0x3E` (62 FIREBULLET) -> `_killobj`, return 1.
- update: nothing unless `(x&0xF)==0 && (y&0xF)==0` - then read the current cell id `map[(x>>4)*0x40 + (y>>4)] & 0x3FFF`, and probe neighbours along `sign(dx),sign(dy)` (a random axis if both are 0) for a cell whose id differs; try the two sign flips, then the reverse direction; if every candidate matches, `dx=dy=0`. Then `dx = p13*sgnX`, `dy = p13*sgnY` (`p13` = speed, from board data) and `_trymove`.
- notes: Pac-Man style corridor follower that only ever moves on tile centres; tile map is [col*0x40 + row] with col=`x>>4`, row=`y>>4`.

### 54. JILLFISH (1fae:202a) [verified]
- draw: `_drawshape(0x33<<8 + tbl[p13] + (p0f>0 ? 3 : 0), x, y)`, 4-entry table at `DGROUP:0x11F3` (= two facing sets of 4 frames).
- hit: no action-1 branch (this is the player's fish form, never "hit").
- update: `p13 = (p13+1)&3`; `_fishdo(x,y)` tests water; `p0f = dx` (remember facing) and `dx = DGROUP:3C76 << 3` (left/right key x8). If the water test failed -> `dy += 2` clamped to ±0x10 (sinking out of water); else `dy += 3C78*2 + (dy<2)` clamped to ±8. `rand%4==0` -> `_addobj(0x3A BUBBLE, x+6, y-2)`. `3C7A` held and in water -> `DGROUP:3D9E = 1`, sound (1,0x25), `dy = -2`. `3C7E` -> `DGROUP:3DA0 = 1`, sound (1,0x26), `_addobj(0x3F FISHBULLET, x + (p0f<0 ? 0x20 : 0) - 8, y)` and sets that bullet's `dx = sign(p0f)*8 + sign(dx)*4`. Then `_justmove` on y (snapping to the next tile row when blocked), `_fishdo(x+dx, y)`, calls `2CFF:2AFD(str+10,0)`.
- notes: swim physics for Jill-as-fish; also the source of BUBBLEs and FISHBULLETs. Always returns 1.

### 55. JILLSPIDER (1fae:262f) [verified]
- draw: none - falls through and returns 0 without calling `_drawshape`.
- hit: returns 1 (no code).
- update: returns 0.
- notes: empty 12-line stub; whatever draws/animates the spider form lives elsewhere (kind 56 JILLBIRD has no decomp either). [open]

### 57. JILLFROG (1fae:2648) [verified]
- draw: base `0x3F<<8 + (p1b < 1 ? 3 : 0)`; if `state == 1` add 1 when `dy < 1`, else 2 (hop frames).
- hit: none.
- update: `p1b = dx` (facing memory); `dx = DGROUP:3C76 << 3` when `(x&7)==0 && dx!=0`, else `3C76 << 2`. `_cando(x, y+1, 3)`: match -> `dy += 1` (cap 0xC); no match -> `dy = -8` + sound (1,0x12), or `dy = -0xC` + sound (1,0x2B) while 3C7A/3C7E are held. `_trymove(x+dx, y+dy)`; if fully blocked: `dy<0` -> `dy = 0`, else snap onto `((y&~0xF)+0x10 - h)`, `p13 = 0`, `dy = 0`. Calls `2CFF:2AFD(str+10,0)` every tick.
- notes: hop/swim controller for Jill-as-frog; the sound on each landing is the only feedback.

### 58. BUBBLE (1a41:3834) [verified]
- draw: `_drawshape(0x33<<8 + p13 + 6, x, y)` -> frames 6..8.
- hit: none.
- update: `p13++` with 1-in-15 probability; if `p13 < 3` and onscreen -> `_fishdo(x + rand%3 - 1, y)` and `_killobj` when that fails; otherwise `_killobj`. Returns 1.
- notes: `p13` = growth step; it only wobbles ±1 px horizontally, never rises. Popped after 3 steps or when off-screen/out of water.

### 59. JELLYFISH (1a41:3957) [verified]
- draw: `_drawshape(0x33<<8 + tbl[p13], x, y)`; 10-entry table memcpy'd from `DGROUP:0x101C`.
- hit: player -> `_hitplayer`; other kind with `kflags & 0x4000` -> `_explode1(x,y,0x14)` (20 particles) + `_killobj`.
- update: `p13 = (p13+1)` wrapping at 9; every 10th tick a random-walk kick: `dx += (rand%3 - 1) * (|dx|<3 ? 2 : 1)` clamped ±6, `dy += (rand%3 - 1) * ((dy<2)+1)` clamped ±6; then `_fishdo(x+dx)` (bounce `dx` on failure) and `_fishdo(y+dy)` (bounce `dy`).
- notes: drifts in water at up to 6 px/tick, no gravity, no score of its own.

### 60. BADFISH (1a41:3d03) [verified]
- draw: `_drawshape(0x33<<8 + tbl[p13&3] + (dx>0 ? 3 : 0) + 0xF, x, y)`; 4-entry table at `DGROUP:0x1030` (two facing sets).
- hit: player -> `_hitplayer`; other kind with `kflags & 0x4000` -> `_killaward(self)` (score popup + poof, no explosion).
- update: `p13++`; 1-in-20 chance to re-roll `dx` in {-4,0,4} and `dy` in {-2,0,2} (if `dx==0` then `dy` in {-2,2}); `_fishdo(x+dx)` bounce `dx`, `_fishdo(y+dy)` bounce `dy`; 1-in-4 chance to `_addobj(0x3A BUBBLE, x+6, y-2)`.
- notes: water-only enemy, 7 in the level table; the only fish that hurts the player.

### 61. ELEV (1a41:3fe8) [verified]
- draw: fixed `_drawshape(0xE2C, x, y)`.
- hit: `kind(other)==0` (player) only: `state = 6`; the first touch of the level (`DGROUP:0x0ECE != 0`) clears that flag and does `_putbotmsg("Press UP/DOWN to use elevator",2)`. `3C78 < 0`: require `_justmove(0, player_x, (y>>4 - 2) << 4)`, then `_setorigin(x, y-0x10)` and `map[col*0x40+row] = 0xC08A`. `3C78 > 0`: require `map[col*0x40+row+1] & 0x3FFF == 0x8A`, then write `map[col*0x40+row] & 0x3FFF | 0xC000` into `[row+1]`, `_setorigin(x, y+0x10)`, `_justmove(0, player_x, row<<4)`. Sound (2,0x1D)/(2,0x1E) only when `3C78` changed (`p0f` caches it).
- update: `state--`; when it hits 0 and the cell below is still tile 0x8A and `p13 != -1` -> same tile copy-down + `_setorigin(x, y+0x10)`, returns 1; else 0.
- notes: the platform carries floor tile 0x8A with it, cell-major map (`[col*0x40+row]`, col=`x>>4`). `p13 == -1` marks an elevator that must not auto-descend.

### 62. FIREBULLET (1a41:436d) [verified]
- draw: `_drawshape(0x1A<<8 + p13, x, y)` (4 frames).
- hit: non-player kind with `kflags & 0x80` -> `_killobj(self)` + `_killaward(other)` + sound (3,10); player hits do nothing; always returns 1.
- update: offscreen -> `_killobj`. `p13 = (p13+1)&3`; `state = (state+1) % 0x14`; `_touchbkgnd` at the pre-move position; `_justmove(x, y + dy + arc[state])` with the 20-entry arc table at `DGROUP:0x1038`; if the horizontal move is blocked -> `_killobj`; `p0f++`, `p0f > 0x4F` (79) -> `_killobj`.
- notes: arcing projectile (not gravity - a baked per-frame y offset); eats 0xA7 tiles, inert against the player.

### 63. FISHBULLET (1a41:45ED) [verified]
- draw: `_drawshape(0x33<<8 + 0x15 + (dx>0), x, y)` - frames 0x15/0x16 for the two directions.
- hit: none - no action-1 branch at all, so it has no damage or score logic of its own.
- update: offscreen -> `_killobj` (returns 0); else `_justmove(x+dx, y)` and `_killobj` if blocked; `y` never changes.
- notes: flat, gravity-free shot fired by JILLFISH (`p13` unused); killed by any solid tile.
