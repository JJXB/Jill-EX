# Licensing assessment [verified]

This note looks at what licence the project can use if the goal is
**non-commercial use only**, and what the existing code and tools allow.
It is an engineering assessment, not legal advice.

## 1. What can actually be licensed

| Part | Status | Can the project license it? |
| --- | --- | --- |
| `port/src/*.cpp`, `*.h` (original port code) | authored for this project | yes |
| `jill_extractor.py`, `jill_import.py`, `extract_all.py`, `re/`, `tools/` | authored for this project | yes |
| `docs/`, `*.md` guides | authored for this project | yes |
| `profiles/`, `games/` manifests, generated tables | authored/generated here | yes, if the tables are not verbatim copies of Epic data [open] |
| `port/src/third_party/*` | third-party, permissive | no; keep their notices |
| Extracted `.sha`/`.dma`/`.vcl`/board data, rendered screenshots, `intro_full.png`, `intro_panel.png` | derived from Epic's game data | **no** - these remain Epic's |
| Game titles and character names ("Jill of the Jungle", "Jill", "Onesimus") | Epic trademarks | no trademark licence is granted |

The README states that no game data or artwork is shipped and that users must
supply their own copy; the only extracted byte sequence in the repository is
`profiles/onesimus.pal`, which the README calls out.  Any licence must repeat
that and must not pretend to cover extracted or rendered game content.

## 2. Dependency audit

| Dependency | Licence | Compatible with a non-commercial project licence? |
| --- | --- | --- |
| SDL2 | zlib [verified] | yes; permissive, requires notice |
| `stb_image` | public domain / MIT [verified] | yes |
| `stb_vorbis` | public domain / MIT [verified] | yes |
| `dr_mp3` | public domain / MIT-0 [verified] | yes |
| `miniz` | MIT + Unlicense [verified] | yes |
| Media Foundation (`mfplat`, `mfreadwrite`, `mfuuid`, `ole32`) | Windows OS API [verified] | yes; no redistribution of the API |
| Pillow | MIT-CMU [verified] | yes |
| Capstone | BSD [verified] | yes |
| numpy | BSD-3-Clause and permissive components [verified] | yes |
| Python / Tk (tool UI) | PSF [verified] | yes |

No GPL, LGPL or AGPL code is present [verified]. That matters because
copyleft licences forbid adding a non-commercial restriction on top. The
current dependency set therefore puts **no obstacle** in the way of a
non-commercial licence.

The optional PS4 build adds a build-time dependency on the **OpenOrbis PS4
Toolchain**, which is GPL-3.0.  It is used the same way as a compiler: no
toolchain source is copied into this repository and the linked runtime is the
toolchain's freestanding CRT/libc plus NID stubs for Sony system libraries.
Published homebrew built with it is not treated as a derivative of the
toolchain.  The packaging step also copies Sony's `libc.prx`,
`libSceFios2.prx` and `right.sprx` from the installed toolkit into the `.pkg`;
those files are console system components, are not stored in this repository,
and are not covered by the project licence.  Distributing a built `.pkg`
therefore also distributes Sony code - users who are not comfortable with that
should build their own or use the loose `eboot.bin` with a loader.

If a GPL/AGPL library is added later (a codec, a GUI toolkit, a sound
library), a non-commercial licence on the combined work is not possible for
that part. Keep new dependencies permissive.

## 3. Candidate non-commercial licences

| Licence | Nature | Fit for this project |
| --- | --- | --- |
| **PolyForm Noncommercial 1.0.0** | source-available, software-specific, non-commercial | **best fit** |
| PolyForm Strict 1.0.0 | source-available, personal/non-commercial use only, very limited redistribution | too restrictive for a modding/extractor project |
| Prosperity Public License 3.0.0 | non-commercial with a 30-day commercial trial | workable, but more complex than needed |
| CC BY-NC-SA 4.0 | content licence, non-commercial, share-alike | good for `docs/` and art guides, **not** for code |
| CC BY-NC 4.0 | content licence, non-commercial, no share-alike | good for documentation only |
| Business Source License 1.1 | eventually converts to open source | not truly non-commercial; delays the question |
| Elastic License 2.0 | source-available; allows internal commercial use | not non-commercial |
| MIT / Apache-2.0 + Commons Clause | forbids "selling", allows most other commercial use | not non-commercial |
| GPL-3.0 / AGPL-3.0 + non-commercial clause | copyleft | **invalid**: the GPL forbids extra restrictions |

### Why PolyForm Noncommercial 1.0.0

* Written for software, not for prose or art.
* Clear definition of commercial vs non-commercial purpose.
* Includes a patent grant, which CC licences do not.
* Allows personal use, non-commercial organisations, education and research.
* No share-alike, so example packs and mods can choose their own terms.
* The author keeps the right to dual-license commercially later.
* It is not an OSI open-source licence; the project is "source-available".

Downsides to accept:

* Linux distributions and other packagers will not ship it.
* Monetised videos, paid mod packs and commercial bundles are not permitted
  without a separate licence [inferred].
* It is not compatible with GPL/AGPL code [verified by licence text].
* It does not cover the game data, so users still need their own copy.

## 4. Recommended layout

1. `LICENSE.md` - PolyForm Noncommercial 1.0.0 for the code.
2. `LICENSE-DOCS` or a section in `LICENSE.md` - CC BY-NC-SA 4.0 for the
   documentation, if a content licence is wanted.
3. `NOTICE` / `THIRD_PARTY_NOTICES.md` - SDL2 zlib notice, MIT notices for
   miniz, public-domain statements for `stb_image`, `stb_vorbis`, `dr_mp3`,
   and the licence lines for Pillow/Capstone used by the tools.
4. A short `ASSETS.md`/README paragraph:
   * no game data or rendered game art is licensed;
   * users must supply their own copy of the original games;
   * game names and characters are Epic trademarks;
   * extracted or edited assets remain the property of their owners.
5. Keep `intro_full.png` and `intro_panel.png` out of the repository. They are
   test renders of Epic's art and are currently untracked [verified]. The same
   applies to any generated `reference_pack` containing game art.

## 5. Current state [verified]

* `LICENSE.md` - PolyForm Noncommercial 1.0.0, with a scope note excluding game
  data, third-party code and trademarks.  The `Required Notice` line reads
  `Copyright 2026 JJXB`.
* `THIRD_PARTY_NOTICES.md` - SDL2, stb_image, stb_vorbis, dr_mp3, miniz, the
  Python tool dependencies, the PS4/OpenOrbis notes and the game-content
  disclaimer.
* `README.md` - has a Legal/trademarks section with the Epic disclaimer and the
  "supply your own game data" statement.  `DISCLAIMER.txt` repeats it for
  binary/data distributions and is copied beside the `.pkg` and into
  `JillEX-data.zip`.
* The PS4 application icon (`port/ps4/sce_sys/icon0.png`) is rendered from the
  original game's artwork.  It is excluded from the project licence, called out
  in the README/notices, and the port also prints a two-line notice on every
  splash page.
* Reverse-engineering scratch output is no longer tracked:
  `re/img*.bin` (extracted executable images), `re/proj/` (Ghidra project),
  `re/export/` (decompiled C, symbol/data dumps), `re/*.txt` (disassembly and
  string listings), `re/*.asm`, `re/*.png` (rendered game art) and
  `hiscores_panel.png` (screenshot).  They remain on disk and are excluded by
  `.gitignore`.  Deleting them from history requires a `git filter-repo` pass;
  until then they are still reachable through old commits.
* `profiles/` and `games/` contain reconstructed engine configuration.  The one
  extracted byte sequence is `profiles/onesimus.pal` (768-byte palette from
  `ONESIMUS.EXE`), called out in the README and not covered by the licence.
  `tools/mkprofile.py --data` regenerates it from the user's own executable.

## 6. Open points (reduced)

* Whether the generated profile tables are "facts" or a substantial extraction
  of the original data.  They are reconstructed from addresses and behaviour,
  but this is a judgement call in some jurisdictions.
* Verbatim strings and disassembly quoted inside the analysis `docs/*.md`
  files.  The scratch listings are gone, but prose still cites addresses and
  short strings.
* Whether including Sony system modules in a fake-signed `.pkg` is acceptable
  for redistribution; documented above, not solved here.

## 7. Bottom line [inferred]

A non-commercial licence is compatible with every current dependency and can
be applied to the port, the tools and the documentation. PolyForm
Noncommercial 1.0.0 is the cleanest choice for the code. The only real
blocker is not the licence but the content: Epic's game data, rendered art and
trademarks cannot be covered by it, so they must stay out of the repository
and the licence text must say so.
