# Port sound: OPL2 core and CMF sequencer

The port plays the original music tracks with a register-level OPL2
synthesiser plus a CMF sequencer.  This is the same data path as the original
`_sbfm_*` driver: `.ddt` (and the Onesimus `SONG*.ARK` files, which are CMF
with a different name) are parsed into MIDI-style events and translated into
YM3812 register writes.  Sampled effects come from the `.vcl` bank and are
mixed as unsigned 8-bit PCM.

Files:

* `port/src/nuked_opl.h`, `port/src/nuked_opl.cpp` - default Nuked OPL3 core
* `port/src/fmopl_opl.h`, `port/src/fmopl_opl.cpp` - optional fmopl core
* `port/src/cmf.h`, `port/src/cmf.cpp` - CMF parser
* `port/src/music.h`, `port/src/music.cpp` - sequencer (MIDI to OPL)
* `port/src/midi.h`, `port/src/midi.cpp` - system MIDI output (WinMM)
* `port/src/sfx.h`, `port/src/sfx.cpp` - `.vcl` bank and effect voice
* `port/src/audio.h`, `port/src/audio.cpp` - SDL device, volume, WAV render

## MIDI output

The same parsed CMF events can instead be sent to a system MIDI device through
WinMM (`midiOutOpen` / `midiOutShortMsg`).  `OPTIONS > SOUND > MIDI OUT` lists
`OFF (OPL2)` plus every installed output; `--list-midi` prints the indices and
`--midi N` selects one at start-up.  CMF rhythm channels 11..15 map to GM
percussion on channel 10.  The CMF program index is not sent directly (it is
an AdLib patch index): a pre-pass scans the track's note-on events, measures
the register each program plays in, and combines that with the OPL patch
envelope/brightness to choose a GM program (bass for low parts, strings/pads
for slow sustained parts, brass/reed for bright sustained parts, piano/mallet
for plucked parts, and so on).  The OPL2 core is bypassed while MIDI output is
selected, but sampled SFX still use the SDL audio device.
**[verified: device enumeration; GM mapping is a heuristic]**


## Music data **[verified]**

All tracks start with `CTMF` (Creative Music File, version 1.1):

| offset | field |
|--|--|
| `0x00` | `CTMF` |
| `0x04` | version minor, major (both `1`) |
| `0x06` | instrument block offset (`u16`) |
| `0x08` | music block offset (`u16`) |
| `0x0A` | ticks per quarter note |
| `0x0C` | ticks per second (96 in every shipped track) |
| `0x0E` | title / composer / remarks string offsets (usually 0) |
| `0x10` | |
| `0x12` | |
| `0x14` | 16-byte channel-in-use table |
| `0x24` | instrument count (`u16`) |
| `0x26` | tempo (`u16`, unused for timing) |

Each instrument is 16 bytes: the eleven OPL2 register values for
`0x20/0x23`, `0x40/0x43`, `0x60/0x63`, `0x80/0x83`, `0xE0/0xE3` and `0xC0`,
followed by five padding bytes.  The music block is a standard MIDI track
(delta times, running status).

The shipped tracks use only instruments defined in their own block, so the
sixteen Creative default instruments are not needed. **[verified: all 22
`.ddt` and 16 `SONG*.ARK` program numbers are below the instrument count]**

## Sequencer

Supported events:

* note on / note off (velocity 0 counts as note off)
* program change (`program % instrument_count`, then loads the instrument)
* controller 0x63 (AM/VIB depth: bit 0 vibrato, bit 1 tremolo; extension)
* controller 0x67 (`0` melody mode, `1` rhythm mode; performs the driver reset)
* controllers 0x68 / 0x69 (fine transpose, **1/256 semitone** per step)

Ignored like the original: channel volume (0x07), pitch bend, aftertouch,
sysex, tempo meta events.  Track end loops the song.

The sequencer is aligned with the Creative **SBFMDRV** driver (Creative Music
File driver, as reimplemented by `viiri/fmdrv` and AdPlug):

* Nine melodic OPL voices in melody mode, six in rhythm mode.  Note-on picks a
  voice with the driver's four-tier search (same channel released, never used,
  any released, oldest sounding); note-off releases every voice on that
  channel/note.  This matters for the tracks' legato/retrigger patterns.
* A repeated note-on for a still-sounding note is treated as a retie: the old
  voice is released and the note is retriggered at the following note-off, so
  same-tick `on`/`off` pairs are not cut short.
* Program changes invalidate parked voices (so the next note reloads the new
  patch) and reprogram sounding voices live.
* Pitch uses the driver's note table quirks: notes 0..11 fold up an octave,
  notes 108..119 repeat 96..107, and 120..127 sit on B7.
* Velocity: `TL = 63 - (((velocity | 0x80) * (63 - patchTL)) >> 8)`, with the
  instrument's KSL bits OR'd back in.  This is the driver's logarithmic
  velocity response; it keeps low-velocity notes audible (the old
  `(127 - velocity) / 2` form silenced many title/level notes).
* The driver reset (song start and controller 0x67) enables waveform select
  (`0x01 = 0x20`), clears CSM/SEL, programs the default instrument on every
  channel, enables AM+VIB depth (`0xBD = 0xC0/0xE0`), and clears voice state.

MIDI channel `n` maps to an OPL voice dynamically in melody mode.  In rhythm
mode the last five MIDI channels become percussion, matching the CMF
specification:

| MIDI channel | voice | OPL operators |
|--|--|--|
| 11 | bass drum | channel 6 modulator + carrier |
| 12 | snare | channel 7 carrier |
| 13 | tom | channel 8 modulator |
| 14 | top cymbal | channel 8 carrier |
| 15 | hi-hat | channel 7 modulator |

Percussion uses the instrument's operator-0 bytes for the dedicated rhythm
operator, its carrier scaling byte (`instrument[3]`) for velocity, and the
`0xBD` key bits, exactly like the chip; the connection register is forced
additive (`instrument[10] | 1`).

F-numbers are derived from the chip's native 49,716 Hz, so both cores play at
the same pitch.  The original loader divides 1,193,180 by the CMF header's
ticks/sec (`+0x0C`) and passes the result to `_sbfm_song_speed`, so the header
tempo is authoritative.

## OPL2 register model

Both cores accept the normal chip register map (`0x20` am/vib/eg/ksr/mult,
`0x40` ksl/tl, `0x60` attack/decay, `0x80` sustain/release, `0xA0`/`0xB0`
frequency and key, `0xC0` feedback/connection, `0xE0` waveform, `0xBD` rhythm
and depth) and render 9 two-operator channels:

* 4 waveforms (sine, half sine, absolute sine, pulse sine)
* ADSR envelopes with key scaling, sustain level and the sustaining-tone bit
* feedback and FM / additive connection
* vibrato and tremolo
* rhythm mode with bass drum, snare, tom, top cymbal and hi-hat

Nuked OPL3 is the default because it is cycle-accurate and cheaper than the
old in-tree model.  fmopl remains selectable for comparison. **[inferred]**

## Selectable synthesis cores

`Music` can drive two register-compatible cores; the CMF sequencer and the
MIDI-to-OPL translation are identical for both:

| `opl_core` | Core | Source | Licence |
| --- | --- | --- | --- |
| 0 (default) | `NUKED OPL3` | `src/third_party/opl3.c` (OPL2 mode) | LGPL-2.1 |
| 1 | `FMOPL` | `src/third_party/fmopl.c` | LGPL-2.1 |

Select with **OPTIONS > SOUND > OPL CORE** (cycles through the two),
`[audio] opl_core = 0|1` in `jill.ini`, or `--opl-core 0|1`.
`--dump-music` uses the same switch for offline A/B comparison.  Both are
gain-matched, so switching does not jump in volume.  Changing the core
restarts the current track so the new core rebuilds its register state.  The
selection survives both exits and restarts: the audio device rebuilds its
sequencer at the negotiated rate (`MusicPlayer::rebuild_at()`), which keeps
the chosen core.  See `THIRD_PARTY_NOTICES.md` for the LGPL obligations.

## Sampled effects

`SfxBank` reads the same `.vcl` file as the text pages (the manifest `pages`
key): 50 offsets, sizes and rates followed by the text-page table.  Each entry
is raw **unsigned 8-bit mono PCM** (silence = 128), normally 6000 Hz.

`Engine::sound(priority, logical_id)` appends a request to a small queue; the
app drains it after each tick and calls `MusicPlayer::play_sfx()`:

* the logical id is folded through the shared 50-byte remap table
  (`SfxBank::remap`, identical in all episodes);
* zero-size entries (`0x11` and `0x25` in the shipped banks) are silent no-ops;
* one voice plays at a time, like the original single DSP DMA channel; a new
  effect replaces the current one unless its priority is strictly lower;
* playback resamples 6000 Hz to the device rate with linear interpolation and
  mixes into the same buffer as the OPL music.

Settings: `OPTIONS > SOUND` has `SFX: ON/OFF` and `SFX VOLUME: N%`; the same
values are `[audio] sfx` and `[audio] sfx_volume` in `jill.ini`, with
`--no-sfx` / `--sfx-volume N` overrides.  The music switch and volume only
gate the OPL mix, so turning music off leaves effects audible (and
`--audio-check` still reports an effect peak in that mode).  The headless
runner reports the number of effect requests as `sfx requests: N`.

The original also had a PC-speaker synthesis path for machines without a
Sound Blaster.  The port only implements the DSP/PCM behaviour, which is what
the sampled data was authored for.

### SOUND TEST

`OPTIONS > SOUND > SOUND TEST` auditions the raw sample slots instead of the
logical effect ids.

* `SET` cycles every installed game and loads that game's `.vcl` bank, so all
  episodes and packs can be compared without restarting.
* `SOUND n` selects any of the 50 raw sample slots; the label comes from
  `docs/profiles.json` `sound_names` (emitted as `kSfxNames` by
  `re/gen_port_data.py`).  Empty slots report `SILENT`.
* `PLAY` (or Enter on the sound row) calls `MusicPlayer::play_sfx_sample()`,
  which bypasses the logical-id remap so the selected slot plays verbatim.
* Music is muted while the screen is open; the volume follows the SFX slider
  even when gameplay SFX are switched off.  Leaving the screen restores the
  selected game's bank and the music switch.

### NOISEMAKER (original board screen)

`NOISEMAKER` on the main menu opens the original front-end page 99 instead of
a port-drawn list.  It asks which episode to use, then loads that episode's
`intro` board with its own private `Engine` and shows the board around the
page-99 CHECKPT (`x - 32`, `y - 8`), including the large `NOISE` title, Jill
portrait and `NOISE BLASTER ... PRESS THE QWERTY KEYS` text.  The running game
is untouched.

The QWERTY scan order comes from the string at `2cff:1246` in the original:

```
1234567890-=QWERTYUIOP[]ASDFGHJKL;'ZXCVBNM,./
```

Pressing those keys plays raw `.vcl` slots 1..45 through the selected
episode's bank.  Esc, pad B/Back or Start leaves the screen and restores the
running game's sound bank.

## Integration

* `MusicPlayer` opens SDL audio at 48 kHz (stereo, 16-bit) and runs the
  sequencer in the audio callback.  Track changes are posted under the device
  lock, so the game thread never touches OPL state.  If the endpoint is 44.1
  kHz (or another rate/format) SDL is allowed to return its own spec and the
  sequencer is rebuilt at that rate, so a strict 48 kHz request cannot leave
  the game silent.
* `--audio-check` opens the device, plays the title track for 1.5 s and prints
  the device format, mixed frame count and peak level; use it when no music is
  audible.
* A game manifest can name a title theme:

  ```ini
  title_music = funky.ddt
  ```

  It is played when the front end starts and when the episode changes.
* Board CHECKPT strings beginning with `*` request a level track
  (`*stealth.ddt`, `*song5.ark`).  The first such string in a board is started
  when the board loads, so board 1 begins on `zeppelin.ddt`; later strings are
  section changes.  The engine reports each change once through
  `Engine::take_music_request()`; the app resolves it against the game's data
  directory.  The full per-board mapping is in `docs/music-catalog.md`.
* Loading a save and returning to the hub through `!` both bypass the normal
  board loader, so they refresh the track explicitly: a restored save starts
  its board's first track (the save can be made far from the music checkpoint),
  and a completed level hands the hub its own track back.
* Settings: `[audio] music = 0/1`, `[audio] volume = 0..100`,
  `[audio] opl_core = 0|1|2` and `[audio] pc_speaker = 0/1` in `jill.ini`,
  the **OPTIONS > SOUND** screen (MUSIC, MUSIC VOLUME, SFX, SFX VOLUME,
  PC SPEAKER, OPL CORE), and `--no-music` / `--music-volume N` /
  `--pc-speaker` / `--opl-core N`.
* PC SPEAKER replaces each sampled effect with a 1-bit square-wave rendition
  of the same sample (the original's `_noisemaker` PIT output is not
  data-driven in the shipped banks).  Music stays on the OPL2 path.
* Headless verification: `--dump-music FILE` renders the title track (or
  `--music-file NAME`) to a 16-bit mono WAV for `--music-seconds N` seconds,
  so playback can be checked without an audio device.

## Limits **[open]**

* The PC-speaker mode is a 1-bit rendering of the sampled effects, not a
  cycle-accurate PIT channel-2 emulation.
* The built-in core does not emulate OPL3 or the chip's timers/status port;
  the Nuked core runs in OPL2 mode and ignores the second register bank.
* Rhythm-mode noise is gated on the rhythm flag; ordinary melodic channels
  7-9 were incorrectly using the LFSR and produced a hiss **[verified]**.
* The five rhythm voices ignore their instrument sustain level and decay to
  silence, as on the chip.  Before this, hi-hat/snare/cymbal notes settled at
  a few dB and left a continuous static-like wash **[verified]**.
* External pack music is resampled with linear interpolation, so 44.1 kHz
  tracks no longer alias into a gritty hiss at 48 kHz **[verified]**.
* Some songs may send no note-off for a melodic note; that voice's envelope
  then sustains until the next retrigger.  The shipped Jill/Onesimus tracks do
  send them.
* Envelope and modulation depths are approximations, so A/B comparisons with
  DOSBox will differ in timbre even though notes, timing and rhythm match.
