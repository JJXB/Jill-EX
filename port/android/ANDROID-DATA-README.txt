Jill-EX for Android
===================

This folder holds the configuration and game manifests.  The app installed
these files on first launch; your saves, high scores and jill.log are written
here too.

Expected layout (all under /sdcard/JillEX):

    jill.ini
    games/       *.game manifests
    profiles/    *.profile + *.pal
    external/    optional asset packs
    jillt/       Jill 1/2/3 data (jill.dma, jill1.sha, *.jn1 ...)
    ONESIM/      Onesimus data (ONESIMUS.SHA, *.ONE ...)

Game data is NOT included with the app.  Copy the folders you extracted from
your own legally obtained copy of the games:

    adb push jillt  /sdcard/JillEX/
    adb push ONESIM /sdcard/JillEX/

On devices where the Android/data folder is hidden, use a file manager that
can browse app storage, or use `adb pull` to back up saves.

Controls:

    left side      on-screen D-pad
    right side     A = jump, X = throw
    top left       BACK (menus / leave)
    top right      START (pause / confirm)

A Bluetooth or USB gamepad works too: A jumps, X throws, Start pauses, and the
D-pad or left stick moves.

Jill of the Jungle is a trademark of Epic Games.  This is an unofficial,
non-commercial fan port; no original game data is distributed with it.
