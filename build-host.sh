#!/bin/bash
# Headless Linux build of the port (verification harness)
set -e
mkdir -p build/host
CFLAGS="-O1 -g -std=gnu89 -fgnu89-inline -fcommon -Dinline= -w -I jni/shim -I jni/sounds -I jni/game"
SHIM="jni/shim/shim_video.c jni/shim/shim_misc.c jni/shim/shim_audio.c jni/shim/shim_fs.c"
GAME="jni/game/main.c jni/game/engine.c jni/game/menu.c jni/game/f_aux.c jni/game/map_loader.c jni/game/highscore.c jni/game/ai.c jni/game/binheap.c jni/game/vfont.c"
gcc $CFLAGS -o build/host/wtd $SHIM $GAME jni/sounds/sounds_data.c jni/host/host_main.c -lm
echo OK
