#!/bin/bash
# Build the signed APK without gradle: NDK clang -> aapt -> zipalign -> apksigner
set -e
cd "$(dirname "$0")"

SDK=${SDK:-$HOME/android-sdk}
NDK=$SDK/ndk/26.3.11579264
BT=$SDK/build-tools/34.0.0
PLATFORM=$SDK/platforms/android-34/android.jar
TC=$NDK/toolchains/llvm/prebuilt/linux-x86_64
GLUE=$NDK/sources/android/native_app_glue
API=26

CFLAGS="-O2 -std=gnu89 -fgnu89-inline -fcommon -Dinline= -w -fPIC \
  -I jni/shim -I jni/sounds -I jni/game -I $GLUE"
SHIM="jni/shim/shim_video.c jni/shim/shim_misc.c jni/shim/shim_audio.c \
  jni/shim/shim_fs.c jni/shim/shim_android.c"
GAME="jni/game/main.c jni/game/engine.c jni/game/menu.c jni/game/f_aux.c \
  jni/game/map_loader.c jni/game/highscore.c jni/game/ai.c jni/game/binheap.c \
  jni/game/vfont.c"
LIBS="-landroid -llog -lEGL -lGLESv2 -laaudio -lm"

mkdir -p build/apk/lib

for ABI in aarch64 x86_64; do
  case $ABI in
    aarch64) OUT=arm64-v8a ;;
    x86_64)  OUT=x86_64 ;;
  esac
  echo "== $OUT =="
  mkdir -p build/apk/lib/$OUT
  $TC/bin/clang --target=$ABI-linux-android$API $CFLAGS \
    -shared -o build/apk/lib/$OUT/libwtd.so \
    $GLUE/android_native_app_glue.c \
    $SHIM $GAME jni/sounds/sounds_data.c \
    -u ANativeActivity_onCreate $LIBS
done

# debug keystore
if [ ! -f build/debug.keystore ]; then
  keytool -genkeypair -keystore build/debug.keystore -storepass android \
    -keypass android -alias androiddebugkey -dname "CN=WTD Debug" \
    -keyalg RSA -keysize 2048 -validity 10000
fi

rm -f build/wtd-unsigned.apk build/wtd-aligned.apk build/wtd.apk
$BT/aapt package -f -M AndroidManifest.xml -A assets -I "$PLATFORM" \
  -F build/wtd-unsigned.apk
(cd build/apk && zip -q -r ../wtd-unsigned.apk lib)
$BT/zipalign -f 4 build/wtd-unsigned.apk build/wtd-aligned.apk
$BT/apksigner sign --ks build/debug.keystore --ks-pass pass:android \
  --key-pass pass:android --out build/wtd.apk build/wtd-aligned.apk
rm -f build/wtd-aligned.apk
echo "APK: $(ls -la build/wtd.apk | awk '{print $5}') bytes -> build/wtd.apk"
