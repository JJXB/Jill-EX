Jill-EX for PS Vita - companion data
====================================

This archive contains the files that are not inside the .vpk:

  jill.ini     default settings
  games/       the four game manifests (jill1, jill2, jill3, onesimus)
  profiles/    engine profiles and palettes

Install
-------

Copy the *contents* of this archive into:

  ux0:data/JillEX/

so the final layout looks like:

  ux0:data/JillEX/jill.ini
  ux0:data/JillEX/games/jill1.game
  ux0:data/JillEX/profiles/jill1.profile
  ux0:data/JillEX/jillt/        <- your extracted Jill 1/2/3 data
  ux0:data/JillEX/ONESIM/       <- your extracted Onesimus data
  ux0:data/JillEX/external/     <- optional asset packs

The game data folders (jillt/ and ONESIM/) come from your own copy of the
original DOS games and are not included here.  The .vpk only installs the
executable.

Transferring
------------

Use VitaShell's FTP server (usually port 1337) and copy the folders, or copy
them from a USB drive.  ux0:data/JillEX must stay writable: saves, high scores
and jill.log are written there.

Running
-------

Start the bubble, then press Start/Cross on the LiveArea.  The Vita has no
keyboard, so the front-end is pad-driven; the in-game menu and the classic
message box use the same bindings as the console builds.

Debug
-----

There is no console window on the Vita.  jill.log is written to
ux0:data/JillEX/jill.log at every start; the port traces startup, board loads,
music tracks and path resolution there.
