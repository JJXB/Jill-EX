Jill-EX for PS4 - companion data
================================

This archive contains the files that are not inside the .pkg:

  jill.ini     default settings
  games/       the four game manifests (jill1, jill2, jill3, onesimus)
  profiles/    engine profiles and palettes

Install
-------

Copy the *contents* of this archive into:

  /data/JillEX/

so the final layout looks like:

  /data/JillEX/jill.ini
  /data/JillEX/games/jill1.game
  /data/JillEX/profiles/jill1.profile
  /data/JillEX/jillt/        <- your extracted Jill 1/2/3 data
  /data/JillEX/ONESIM/       <- your extracted Onesimus data
  /data/JillEX/external/     <- optional asset packs

The game data folders (jillt/ and ONESIM/) come from your own copy of the
original DOS games and are not included here.  The .pkg only installs the
executable.

Transferring
------------

Use FTP on a jailbroken console (usually port 2121), or copy the folder from
a USB drive.  /data/JillEX must be writable by the game: saves, high scores
and jill.log are written there.

Debug
-----

Edit /data/JillEX/jill.ini and set:

  [debug]
  overlay = 1

to show the FPS/profiler overlay.
