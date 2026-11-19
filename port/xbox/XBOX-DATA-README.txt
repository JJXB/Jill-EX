Jill-EX for the original Xbox - install bundle
===============================================

This bundle contains the game executable and its configuration:

  default.xbe  the game
  jill.ini     default settings (relative paths)
  games/       the four game manifests (jill1, jill2, jill3, onesimus)
  profiles/    engine profiles and palettes

Install
-------

Extract everything into one folder on the console, for example:

  E:\Apps\Jill-EX\default.xbe
  E:\Apps\Jill-EX\jill.ini
  E:\Apps\Jill-EX\games\jill1.game
  E:\Apps\Jill-EX\profiles\jill1.profile
  E:\Apps\Jill-EX\jillt\        <- your extracted Jill 1/2/3 data
  E:\Apps\Jill-EX\ONESIM\       <- your extracted Onesimus data
  E:\Apps\Jill-EX\external\     <- optional asset packs

or burn the same layout as an XISO (the disc root is D:\).

The game uses the folder default.xbe was launched from as its data root.  If
that folder has no jill.ini, it falls back to the first of D:\, E:\JillEX,
E:\Apps\Jill-EX, E:\Apps\JillEX, E:\Games\Jill-EX, E:\Games\JillEX that has
one, so do not mix two installs.

The game data folders (jillt/ and ONESIM/) come from your own copy of the
original DOS games and are not included here.

Notes
-----

* The data root must be writable for saves, high scores and jill.log.  A
  burned disc (D:\) is read-only, so use the hard disk if you want to save.
* jill.log is written next to default.xbe on the first run; the same lines
  are printed to the debug channel (visible in xemu/Cxbx or with an XBDM
  debug setup).
* jill.ini uses relative paths on purpose: the Windows jill.ini points at
  D:/Emu/... which does not exist on the console.
