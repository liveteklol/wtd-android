# Warcraft : Tower Defense — portage Android natif

Portage du homebrew Nintendo DS **WTD v0.5** de Noda (licence type zlib,
voir `LICENSE-original.txt`) vers Android natif. Conformément à la licence,
cette version est une **version modifiée** du logiciel original : la logique
du jeu (`jni/game/`) reste celle d'origine, hormis les adaptations de confort
(QoL) documentées ci-dessous.

## Améliorations de confort (QoL, écran du bas)

- **Écran du bas plein écran, natif (non étiré)** : au lieu d'un cadre DS
  256×192, le moteur compose la vue carte dans un **viewport dynamique**
  (`shim_view_w × shim_view_h`) qui remplit 100 % de la zone basse en pixels
  DS natifs — on voit donc *plus de carte*, pas une image agrandie. L'écran
  du haut (interface) garde son format 256×192 d'origine.
- **Pinch-to-zoom** : deux doigts sur la carte agrandissent/rétrécissent le
  viewport (le point sous les doigts reste fixe), borné à la taille de la
  carte.
- **Déplacement à un doigt** : glisser un doigt sur la carte fait défiler la
  vue ; un tap court reste un clic de stylet (construction, sélection).
- **Bouton de vitesse de jeu** (en haut à gauche de la carte, aligné avec la
  rangée Build/Cancel) : cycle 1x → 2x → 4x. À 2x/4x le backend n'affiche
  qu'une frame sur N, donc la logique du jeu (inchangée) tourne N fois plus
  vite. Repasse à 1x dans les menus et dialogues.
- **Bornes de zoom** : le zoom avant est plafonné pour ne jamais faire
  apparaître de bandes noires ; le zoom arrière s'arrête quand toute la carte
  est visible.
- **Dialogues** (difficulté, pause, menu…) : rendus au format 256×192 centré
  d'origine, leurs coordonnées internes étant figées dans les bitmaps.

Côté moteur (`jni/game/engine.c`), les constantes d'écran codées en dur
(256/192 pour le culling des sprites, les clamps de scroll, la minimap, le
menu de construction, le placement stéréo des sons) ont été remplacées par
`VIEW_W/VIEW_H`, recalculés à chaque frame depuis le viewport courant.

## Architecture

```
jni/game/     sources du jeu d'origine, non modifiées (engine, ai, menu, …)
jni/shim/     couche de compatibilité PAlib/ASlib/EFS écrite pour ce portage
  PA9.h           API PAlib reproduite (types, macros, prototypes)
  shim_video.c    émulation logicielle des 2 écrans DS : sprites OAM +
                  VRAM, fonds tuilés / larges / bitmap 8-bit, palettes
                  étendues par fond, rot/zoom sets, alpha, luminosité
  shim_audio.c    16 canaux IMA-ADPCM 22 kHz, volume + panoramique stéréo
  shim_misc.c     Pad/Stylus (snapshot par frame, double-tap), VBL, RNG, RTC
  shim_fs.c       EFS + « /maps » remappés sur le stockage interne, dirent
  shim_android.c  NativeActivity : EGL/GLES2, AAudio, tactile multi-point,
                  boutons DS à l'écran, extraction des assets, cadence 60 Hz
jni/sounds/   les 61 sons .raw convertis en tableaux C (mêmes symboles que bin2o)
jni/host/     harnais headless Linux (script d'input → dumps PPM des 2 écrans)
assets/       cartes .tdm (16) + fichiers EFS initiaux (settings, highscores…)
```

## Adaptations (seules choses changées, comme prévu)

- **Écrans** : les deux écrans 256×192 sont empilés en portrait (haut = écran
  supérieur DS, bas = écran tactile), mis à l'échelle à la largeur du device.
- **Entrées** : 100 % tactile, sans boutons à l'écran (voir la section QoL
  ci-dessus pour le pan/zoom sur la carte). L'écran du haut (interface) est
  tactile : le toucher maintient un « L virtuel », ce qui fait entrer le
  moteur dans son mode interface avec les coordonnées du stylet sur cet écran ;
  l'affichage n'échange jamais les écrans (`PA_SwitchScreens` est virtualisé).
  Les manettes physiques restent supportées (D-pad, A/B/X/Y, L/R, Start,
  Select).
- **Filesystem** : l'EFS embarqué et `/maps` deviennent des fichiers dans le
  répertoire interne de l'app, initialisés depuis les assets de l'APK.
- **Audio** : le mixeur ARM7/ASlib est remplacé par un décodeur IMA-ADPCM et
  un mixeur 16 canaux poussé vers AAudio.

## Compiler

```bash
./build-android.sh    # → build/wtd.apk (arm64-v8a + x86_64, signé debug)
./build-host.sh       # → build/host/wtd (harnais de vérification headless)
```

Prérequis : Android SDK (build-tools 34, platform 34, NDK 26.3), JDK 17, zip.

Vérification headless :
```bash
WTD_ROOT=build/fsroot WTD_OUT=build/frames WTD_SCRIPT=script.txt ./build/host/wtd
# script.txt : "<frame> touch x y hold" | "<frame> pad start hold" |
#              "<frame> dump label" | "<frame> exit"
```

## Installer

```bash
adb install -r build/wtd.apk
```
