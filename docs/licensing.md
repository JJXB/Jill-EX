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

The README already states that no copyright assets are shipped and users must
provide their own data [verified]. Any licence must repeat that and must not
pretend to cover extracted or rendered game content.

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

## 5. Current files [verified]

* `LICENSE.md` - PolyForm Noncommercial 1.0.0, with a scope note excluding game
  data, third-party code and trademarks.  The `Required Notice` line reads
  `Copyright 2026 Jill-EX contributors`; replace the holder if a different
  name should appear.
* `THIRD_PARTY_NOTICES.md` - SDL2, stb_image, stb_vorbis, dr_mp3, miniz and
  the Python tool dependencies, plus the game-content disclaimer.

## 6. Open points

* Whether the generated data tables in `profiles/` and `re/` are verbatim
  copies of the original executable's tables or independently reconstructed
  facts [open]. Verbatim tables weaken the claim that the whole repository is
  the author's work.
* Whether disassembly listings quoted in `docs/` count as copied code in the
  relevant jurisdiction [open].
* The project name and game names need a trademark disclaimer regardless of
  the code licence [verified that no such disclaimer exists today].

## 7. Bottom line [inferred]

A non-commercial licence is compatible with every current dependency and can
be applied to the port, the tools and the documentation. PolyForm
Noncommercial 1.0.0 is the cleanest choice for the code. The only real
blocker is not the licence but the content: Epic's game data, rendered art and
trademarks cannot be covered by it, so they must stay out of the repository
and the licence text must say so.
