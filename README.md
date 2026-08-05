> **This is an unofficial native Android port** of the Nintendo DS homebrew
> *Warcraft: Tower Defense* (WTD) v0.5 by Noda, released under a zlib-style
> license (see [`LICENSE-original.txt`](LICENSE-original.txt)). Per the
> license, this is a **modified version** of the original software: the
> game logic (`jni/game/`) is unchanged from the original release below,
> aside from the Android/touch quality-of-life adaptations documented in
> [Android Port](#android-port) at the end of this file.

```
                         ***************************************
                         * WARCRAFT : TOWER DEFENSE // by Noda *
                         * for nds/nds lite                    *
                         *-------------------------------------*
                         *     http://ndswtd.wordpress.com     *
                         ***************************************
```

## About

This project was started in august 2006.
It's an NDS adaptation of the famous Warcraft III mod, tower defense.
This game is entirely coded in C, with the help of the PAlib library.
The map editor is coded in C# and use the .NET 2.0 runtime.

Some facts about the engine & the editor:

- real time strategic game
- can load custom-made maps very easily
- advanced dynamic pathfinding
- max. 40 towers and their attacks gfx along with 40 monsters on screen
  simultaneously
- 10 special powers for towers (slow, poison, critic damage, pierce, fire magic,
  water magic, lightning magic, wind magic, splash damage, reveal invisible monsters)
- same for monsters resistance & immunes
- flying & walking monsters types (and ability for towers to attack both or only
  one type)
- dynamic sound engine & stereo sound placement
- max 256 differents types of monsters
- max 256 differents types of towers
- max 256 rounds
- max 11 possible upgrades per tower
- max 8 base evolution levels
- max 8 different clans to choose from at the start
- 128 icons gfx for towers, evolutions ...
- 64 towers gfx with 6 color variations for each
- 36 towers gfx with 4 color variations for each (more to come)
- fully customizable engine through map definitions
- advanced graphical map editor
- and more...

## How to use

Put the maps (*.tdm) in the /Maps subfolder, then read the "how to install" notes.

## Changelog

**14/01/2007 : v0.1 beta**
- first public version

**20/01/2007 : v0.2 beta**
- corrected the bug with the animation of some monsters
- corrected a bug which may occurs when selling a tower
- adjusted the pathfinder's memory allocation: big maps should now works
- adjusted detection zone for build/cancel buttons
- increased values range some parameters (life of the dragon in Spiral TD should now
  be correct)
- spawn and end points are now displayed on the minimap
- added preliminary libfat support
- new map: Save The Snowman !
- added some 3rd party maps to the package, thanks to their authors !
- now the PAFS version can run on the DeSmuME emulator ! it's now easier to test your
  maps created with the editor

**28/01/2007 : v0.3 beta**
- now use latest libfat with DLDI support
- corrected a bug in the file filter, now FAT version works without having to rename
  the maps
- fixed the missing border of some icons
- temporary workaround for solving the screwed up sprites problem
- added a new bunch of great 3rd party maps :)
- corrected some bugs in the "Save The Snowman" map
- corrected tower collision map in the "Long Walk TD" map
- added a circle around the current selection
- new map: Orange Frenzy

**04/02/2007 : v0.31 beta**
- now use latest devkitpro (r20) with latest libnds, libfat & palib
- stylus precision & jumping problems fixed (thanks to latest libnds)
- stylus problems with R4/M3 simply should be fixed (thanks to latest palib)
- corrected the bug that make the map selection screen froze when there was more than
  10 maps (FAT version)
- stat screen flickering/disappear bug in the upper part fixed (thanks to latest palib?)

**04/02/2007 : v0.4 beta**
- PAFS version was broken in the last release, now it's working again
- corrected the bug that prevented max towers bonus for first evolution to be applied at
  start
- corrected colors of selection circle, it looks better now
- fixed the animation bug when a monster is killed at spawn
- fixed sprite scrolling, now sprites are in phase with background while scrolling
- updated the map editor, see its changelog for details
- added a bunch of new features for maps, like small & transparent monsters, W3 damage
  style, choice for the colors used in the minimap for painting entities...
- complete rewrite of the sprite engine, now sprites corruptions problems should be fixed
- added a nice zoom effect when building/upgrading towers
- added current window display on minimap
- you can now scroll the map directly by dragging the window on the minimap
- monsters are now hidden and no more over a tower when you build a tower over a dead one
- added a flashing message when a new evolution is available and wasn't before
- you can now scroll maps in the map selection screen directly by touching the scroll bar
- added the option to build tower by double-tapping
- added the option to choose L/R to work as a switch and don't need to be held
- added the option to allow to build multiple towers at once
- added the option to choose the position of the in-game build menu
- added an option menu
- added a Linux script for PAFS version
- added multiple paths management (up to 4)
- removed the 40 monsters spawn limit per rounds, now up to 256 monsters per path can be
  spawned in a round (but remember that only 40 can be active at the same time)
- added stereo sound placement
- updated & added some new maps (try the map Hellgate Keeper's to the new features in
  action!)
- added a new pathfinder, quite faster than the old one but less accurate
- little optimizations here and there :)

**25/02/2008 : v0.5**
- PAFS version is gone, now only the fat/DLDI version is available
- you can now use the arrow keys to select maps in the menu
- added 5 automatic levels of difficulty for each map: it affects the life of the monsters, and the final score
- options are now saved within the rom (require DLDI driver with write support)
- in-game menu added
- added possibility to restart the map in-game
- added quicksave & exit in-game option so you can save your current game and finish it later (require DLDI driver with write support)
- high-scores are now saved (require DLDI driver with write support)
- high-scores can be viewed on the map selection screen (press L or R to switch minimap/high-scores)
- small speedups here and there
- added some new maps
- new game icon
- fixed some bugs
- fixed Long Walk TD skeleton's round

## Many thanks to

- Mollusk for his great PALib, and for his hard work adding in every single feature
  he requested :-D
- Gutter Talk for the great documentation he made for the Map Editor
- YnotnA, Zippi and Balu, Deshi, Mat88, TheRock and all other map makers, for their great
  maps, thanks a lot for your contribution!
- Arialia for her tutorial on how to use the new libfat
- The DS-Xtreme team for sending me a free sample, without it FAT version would
  probably not exist!
- Vodevil, Bobby Sixkilla, OMG & M@T for their help with the beta test

---

## Android Port

Native Android port of NDS homebrew **WTD v0.5** by Noda (zlib-style license,
see [`LICENSE-original.txt`](LICENSE-original.txt)).

### Quality-of-life improvements (bottom screen)

- **Full-screen, native bottom screen (not stretched)**: instead of a fixed
  256x192 DS frame, the engine composes the map view in a **dynamic
  viewport** (`shim_view_w x shim_view_h`) that fills 100% of the bottom area
  in native DS pixels — so you see *more map*, not a scaled-up image. The top
  screen (interface) keeps its original 256x192 layout.
- **Pinch-to-zoom**: two fingers on the map zoom the viewport in/out (the
  point under the fingers stays fixed), clamped to the map size.
- **One-finger pan**: dragging one finger on the map scrolls the view; a
  short tap remains a stylus click (building, selection).
- **Game speed button** (top-left of the map, aligned with the Build/Cancel
  row): cycles 1x -> 2x -> 4x. At 2x/4x the backend only renders one frame
  out of N, so the (unchanged) game logic runs N times faster. Resets to 1x
  in menus and dialogs.
- **Zoom bounds**: zooming in is capped so black bars never appear; zooming
  out stops once the whole map is visible.
- **Dialogs** (difficulty, pause, menu...) are rendered at the original,
  centered 256x192 layout, since their internal coordinates are baked into
  the bitmaps.

In the engine (`jni/game/engine.c`), hardcoded screen constants (256/192 for
sprite culling, scroll clamps, the minimap, the build menu, stereo sound
placement) were replaced with `VIEW_W`/`VIEW_H`, recomputed every frame from
the current viewport.

### Architecture

```
jni/game/     original game sources, unmodified (engine, ai, menu, ...)
jni/shim/     PAlib/ASlib/EFS compatibility layer written for this port
  PA9.h           reproduced PAlib API (types, macros, prototypes)
  shim_video.c    software emulation of the 2 DS screens: OAM sprites +
                  VRAM, tiled/wide/8-bit bitmap backgrounds, per-background
                  extended palettes, rotation/zoom sets, alpha, brightness
  shim_audio.c    16-channel IMA-ADPCM 22 kHz, volume + stereo panning
  shim_misc.c     Pad/Stylus (per-frame snapshot, double-tap), VBL, RNG, RTC
  shim_fs.c       EFS + "/maps" remapped onto internal storage, dirent
  shim_android.c  NativeActivity: EGL/GLES2, AAudio, multi-touch, on-screen
                  DS buttons, asset extraction, 60 Hz pacing
jni/sounds/   the 61 .raw sounds converted to C arrays (same symbols as bin2o)
jni/host/     headless Linux harness (input script -> PPM dumps of both screens)
assets/       .tdm maps (16) + initial EFS files (settings, highscores...)
```

### Adaptations (the only things changed, as intended by the license)

- **Screens**: the two 256x192 screens are stacked in portrait (top = DS top
  screen, bottom = touch screen), scaled to the device width.
- **Input**: fully touch-based, no on-screen buttons (see the QoL section
  above for map pan/zoom). The top screen (interface) is touch-enabled:
  touching it holds a "virtual L", which puts the engine into interface mode
  using the stylus coordinates on that screen; the display never swaps
  screens (`PA_SwitchScreens` is virtualized). Physical gamepads remain
  supported (D-pad, A/B/X/Y, L/R, Start, Select).
- **Filesystem**: the embedded EFS and `/maps` become files in the app's
  internal storage, seeded from the APK assets.
- **Audio**: the ARM7/ASlib mixer is replaced by an IMA-ADPCM decoder and a
  16-channel mixer feeding AAudio.

### Build

```bash
./build-android.sh    # -> build/wtd.apk (arm64-v8a + x86_64, debug-signed)
./build-host.sh       # -> build/host/wtd (headless verification harness)
```

Requirements: Android SDK (build-tools 34, platform 34, NDK 26.3), JDK 17, zip.

Headless verification:
```bash
WTD_ROOT=build/fsroot WTD_OUT=build/frames WTD_SCRIPT=script.txt ./build/host/wtd
# script.txt: "<frame> touch x y hold" | "<frame> pad start hold" |
#             "<frame> dump label" | "<frame> exit"
```

### Install

```bash
adb install -r build/wtd.apk
```
