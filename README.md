# Jill-EX

A standalone attempt at reverse engineering and recreating the Jill of the Jungle engine.

## AI
This did use Deepseek 4.1 Flash to construct and make. Disclaimer Over.

## Supported Games

* Jill of the Jungle
* Jill Goes Underground
* Jill Saves The Prince
* Onesimus: A Quest for Freedom

No copyright assets shipped, provide your own.

## Features

### Modernization
* Modern HUD and Menus with Customization
* High framerate support - 60/90/120/144 Hz - Interpolation as a separate option - Logic still at 18.2hz
* Expanded Limits (Board Obj Cap = 384, KIND Limit = 138, Visible/Active Obj Cap = 384)
* Windowed, exclusive fullscreen and borderless modes; resolution presets and 1x-6x window scaling.
* Extra Features (All Optional)
  * Minimap
  * Centred fish shot
  * Fire while crouched
  * 5 knife limit maximum
  * Retain items between levels
  * faster dialog boxes
  * Air jump buffer
  * Jill Select: play as the Jill 1/2/3 or Custom Colour in any episode. Onesimus Support WIP
* Keyboard and gamepad input with rebindable movement, jump and throw.
* SDL Sound Output
* Cheat menu:
  * Invincible
  * Debug Fly
  * Super Jump
  * Add Items
  * Refill Health 
  * BECOME JILL/FROG/BIRD/SPIDER/FISH (Spider Unused but logic present in files, generated temporary art).
  * Level/Board Select

## TODO

### Remaining
* Splash screens to be fixed.
* Noisemaker Modes
* Work on Display Filters to be continued 
* Verify music playback is right.
* Character portraits.
* External enhanced asset loading
* Mac/Linux/console builds.

## Engine and gameplay Info
[See ENGINE](ENGINE.md)

## Tools and data
[See TOOLS_DATA](TOOLS_DATA.md)

## Adding Mods
[See MODS](MODS.md)

## Building
[See BUILDING](BUILDING.md)