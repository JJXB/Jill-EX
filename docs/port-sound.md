# Port sound: OPL2 core and CMF sequencer

The port plays the original music tracks with a register-level OPL2
synthesiser plus a CMF sequencer.  This is the same data path as the original
`_sbfm_*` driver: `.ddt` (and the Onesimus `SONG*.ARK` files, which are CMF
with a different name) are parsed into MIDI-style events and translated into
YM3812 register writes.  Sampled effects come from the `.vcl` bank and are
mixed as unsigned 8-bit PCM.

Files:

* `port/src/opl2.h`, `port/src/opl2.cpp` - YM3812 core
* `port/src/cmf.h`, `port/src/cmf.cpp` - CMF parser
* `port/src/music.h`, `port/src/music.cpp` - sequencer (MIDI to OPL)
* `port/src/sfx.h`, `port/src/sfx.cpp` - `.vcl` bank and effect voice
* `port/src/audio.h`, `port/src/audio.cpp` - SDL device, volume, WAV render

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
* program change (loads the instrument into the OPL channel)
* controller 0x07 (channel volume)
* controller 0x63 (AM/VIB depth: bit 0 vibrato, bit 1 tremolo)
* controller 0x67 (`0` melody mode, `1` rhythm mode)
* controllers 0x68 / 0x69 (fine transpose, 1/128 semitone)

Ignored like the original: pitch bend, aftertouch, sysex, tempo meta events.
Track end loops the song (level music in the original does not stop).

MIDI channel `n` maps to OPL channel `n` in melody mode.  In rhythm mode the
last five MIDI channels become percussion, matching the CMF specification:

| MIDI channel | voice | OPL operators |
|--|--|--|
| 11 | bass drum | channel 6 modulator + carrier |
| 12 | snare | channel 7 carrier |
| 13 | tom | channel 8 modulator |
| 14 | top cymbal | channel 8 carrier |
| 15 | hi-hat | channel 7 modulator |

Percussion uses instrument register writes plus the `0xBD` key bits, exactly
like the chip.  Velocity and controller 7 are added to the carrier total level
(`(127 - value) / 2` TL steps), which reproduces the logarithmic CMF velocity
response.

## OPL2 core

The core accepts the normal chip register map (`0x20` am/vib/eg/ksr/mult,
`0x40` ksl/tl, `0x60` attack/decay, `0x80` sustain/release, `0xA0`/`0xB0`
frequency and key, `0xC0` feedback/connection, `0xE0` waveform, `0xBD` rhythm
and depth) and renders 9 two-operator channels:

* 4 waveforms (sine, half sine, absolute sine, pulse sine)
* ADSR envelopes with key scaling, sustain level and the sustaining-tone bit
* feedback and FM / additive connection
* vibrato (7 or 14 cents) and tremolo (1 or 4.8 dB)
* rhythm mode with bass drum, snare, tom, top cymbal and hi-hat

The numeric model is tuned by ear against the shipped tracks; it is not
cycle-exact and does not emulate the chip's log-sine quantisation.  Envelope
times follow the OPL rate halving rule (rate 15 about 0.2 ms, rate 0 about
6 s), which is close enough for the original instrument banks. **[inferred]**

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

### NOISEMAKER (sound test)

`OPTIONS > SOUND > NOISEMAKER` is the port's version of the front end's `N`
page: it auditions the raw sample slots instead of the logical effect ids.

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

The original's page 99 `_noisemaker` is the same idea, but it drives the
PC-speaker driver; the port plays the sampled bank because that is the data
every shipped release actually contains.

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
* Settings: `[audio] music = 0/1`, `[audio] volume = 0..100` and
  `[audio] pc_speaker = 0/1` in `jill.ini`, the **OPTIONS > SOUND** screen
  (MUSIC, MUSIC VOLUME, SFX, SFX VOLUME, PC SPEAKER), and
  `--no-music` / `--music-volume N` / `--pc-speaker`.
* PC SPEAKER replaces each sampled effect with a 1-bit square-wave rendition
  of the same sample (the original's `_noisemaker` PIT output is not
  data-driven in the shipped banks).  Music stays on the OPL2 path.
* Headless verification: `--dump-music FILE` renders the title track (or
  `--music-file NAME`) to a 16-bit mono WAV for `--music-seconds N` seconds,
  so playback can be checked without an audio device.

## Limits **[open]**

* The PC-speaker mode is a 1-bit rendering of the sampled effects, not a
  cycle-accurate PIT channel-2 emulation.
* The port does not emulate OPL3 or the chip's timers/status port.
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
