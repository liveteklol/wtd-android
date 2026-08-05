/*
 * PA9.h — PAlib compatibility layer for the Android port of
 * "Warcraft : Tower Defense" v0.5 (homebrew by Noda, zlib-style license).
 *
 * This header reproduces the exact subset of the PAlib / ASlib / libnds API
 * used by the game sources, backed by a software 2D compositor that emulates
 * the two 256x192 DS screens (OAM sprites, tiled/large/8-bit bitmap
 * backgrounds, palettes, alpha blending, brightness, rot/zoom sets).
 */
#ifndef _PA9_SHIM_H
#define _PA9_SHIM_H

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
/* Basic libnds types                                                   */
/* ------------------------------------------------------------------ */
typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;
typedef int8_t    s8;
typedef int16_t   s16;
typedef int32_t   s32;
typedef int64_t   s64;

/* ------------------------------------------------------------------ */
/* Directions (PAlib values, verified against the WTD sprite sheets)    */
/* ------------------------------------------------------------------ */
#define UP      0
#define DOWN    1
#define LEFT    2
#define RIGHT   3
#define NONE    4

/* ------------------------------------------------------------------ */
/* Screen geometry                                                      */
/* ------------------------------------------------------------------ */
#define PA_SCREENW 256
#define PA_SCREENH 192

/* ------------------------------------------------------------------ */
/* OBJ sizes: encoded as (width << 8) | height                          */
/* ------------------------------------------------------------------ */
#define OBJ_SIZE_8X8     ((8 << 8) | 8)
#define OBJ_SIZE_16X16   ((16 << 8) | 16)
#define OBJ_SIZE_32X32   ((32 << 8) | 32)
#define OBJ_SIZE_64X64   ((64 << 8) | 64)
#define OBJ_SIZE_32X16   ((32 << 8) | 16)
#define OBJ_SIZE_64X32   ((64 << 8) | 32)
#define OBJ_SIZE_16X8    ((16 << 8) | 8)
#define OBJ_SIZE_8X16    ((8 << 8) | 16)
#define OBJ_SIZE_16X32   ((16 << 8) | 32)
#define OBJ_SIZE_32X64   ((32 << 8) | 64)

/* ------------------------------------------------------------------ */
/* Colors                                                               */
/* ------------------------------------------------------------------ */
#define PA_RGB(r, g, b) ((u16)(((b) << 10) | ((g) << 5) | (r) | 0x8000))

/* ------------------------------------------------------------------ */
/* Input                                                                */
/* ------------------------------------------------------------------ */
typedef struct {
    u8 A, B, X, Y, L, R, Start, Select, Up, Down, Left, Right;
} padkeys;

typedef struct {
    padkeys Newpress;
    padkeys Held;
    padkeys Released;
} padtype;

typedef struct {
    u8  Newpress;   /* first frame of a touch            */
    u8  Held;       /* touch currently down              */
    u8  Released;   /* first frame after release         */
    u8  DblClick;   /* double tap detected this frame    */
    s16 X, Y;       /* stylus position (bottom screen)   */
    s16 Vx, Vy;
    u8  Downtime;
} stylustype;

extern padtype    Pad;
extern stylustype Stylus;

/* variable-width font letter renderer (used by vfont.c) */
typedef void (*letterfp)(u8 size, u8 screen, u16 x, u16 y, char letter, u8 color);

#define PA_StylusInZone(x1, y1, x2, y2) \
    ((Stylus.X >= (x1)) && (Stylus.X <= (x2)) && (Stylus.Y >= (y1)) && (Stylus.Y <= (y2)))

int PA_SpriteTouched(int sprite);   /* touch-screen (0) sprites only */

/* ------------------------------------------------------------------ */
/* Misc system                                                          */
/* ------------------------------------------------------------------ */
typedef struct { char Name[32]; } pa_userinfo_t;
typedef struct { u8 Hour, Minutes, Seconds, Day, Month; u16 Year; } pa_rtc_t;

extern pa_userinfo_t PA_UserInfo;
extern pa_rtc_t      PA_RTC;

void PA_Init(void);
void PA_InitVBL(void);
void PA_WaitForVBL(void);

#define PA_SetLedBlink(a, b)    ((void)0)
#define PA_SetAutoCheckLid(a)   ((void)0)
#define PA_CheckLid()           ((void)0)
#define PA_LidClosed()          (0)

int  PA_RandMinMax(int min, int max);
#define PA_Distance(x1, y1, x2, y2) \
    ((((s32)(x2) - (s32)(x1)) * ((s32)(x2) - (s32)(x1))) + \
     (((s32)(y2) - (s32)(y1)) * ((s32)(y2) - (s32)(y1))))

/* fake IPC sound status (only read after PA_LidClosed(), never true) */
typedef struct { struct { u8 busy; } chan[16]; } ipc_sound_t;
extern ipc_sound_t *IPC_Sound;

/* libfat init — filesystem is always available on Android */
#define fatInitDefault() (1)

/* fopen path remap: "/maps/..." lives in the app data directory        */
FILE *shim_fopen(const char *path, const char *mode);
#define fopen(p, m) shim_fopen((p), (m))

/* ------------------------------------------------------------------ */
/* Screens / effects                                                    */
/* ------------------------------------------------------------------ */
void PA_SetBrightness(u8 screen, s8 bright);
void PA_SwitchScreens(void);

#define SFX_NONE  0
#define SFX_ALPHA 1
#define SFX_BG0   (1 << 0)
#define SFX_BG1   (1 << 1)
#define SFX_BG2   (1 << 2)
#define SFX_BG3   (1 << 3)
#define SFX_OBJ   (1 << 4)
#define SFX_BD    (1 << 5)

void PA_EnableSpecialFx(u8 screen, u8 fx, u8 fx_other, u8 targets);
void PA_SetSFXAlpha(u8 screen, u8 alpha1, u8 alpha2);

/* ------------------------------------------------------------------ */
/* Backgrounds                                                          */
/* ------------------------------------------------------------------ */
#define BG_TILEDBG   0
#define BG_LARGEBG   3
#define MAX_TILES    1024

/* size codes (subset used) */
#define BG_256X256   0
#define BG_512X256   1
#define BG_256X512   2
#define BG_512X512   3

typedef struct {
    int       BgMode;
    int       NTiles;
    const u8 *Tiles;
} pa_bginfo_t;

extern pa_bginfo_t PA_BgInfo[2][4];

/* 8-bit bitmap layer (bg3), 2 pixels per u16, 128 u16 per line */
extern u16 *PA_DrawBg[2];

/* big zero buffer (used as blank source for DMA copies / tiles) */
extern const u32 Blank[16384];

void PA_ResetBgSys(void);
void PA_DeleteBg(u8 screen, u8 bg);

int  PA_GetPAGfxBgSize(int width, int height);
void PA_LoadBgPal(u8 screen, u8 bg, const void *pal);
void PA_Load8bitBgPal(u8 screen, const void *pal);
void PA_SetBgPalCol(u8 screen, u8 index, u16 color);

void PA_LoadBgTilesEx(u8 screen, u8 bg, const void *tiles, u32 size);
void PA_LoadBgMap(u8 screen, u8 bg, const void *map, int bg_size);
void PA_InitBg(u8 screen, u8 bg, int bg_size, u8 wraparound, u8 color_mode);
void PA_InitLargeBg(u8 screen, u8 bg, int width_tiles, int height_tiles, const void *map);
void PA_BGScrollXY(u8 screen, u8 bg, s32 x, s32 y);
void PA_LargeScrollXY(u8 screen, u8 bg, s32 x, s32 y);
void PA_InfLargeScrollXY(u8 screen, u8 bg, s32 x, s32 y);

void shim_LoadTiledBg(u8 screen, u8 bg, const void *tiles, u32 tiles_size,
                      const void *map, const void *pal, const int *info);
#define PA_LoadTiledBg(screen, bg, name) \
    shim_LoadTiledBg((screen), (bg), name##_Tiles, sizeof(name##_Tiles), \
                     name##_Map, name##_Pal, name##_Info)

void shim_LoadLargeBg(u8 screen, u8 bg, const void *tiles, u32 tiles_size,
                      const void *map, const void *pal, const int *info);
#define PA_LoadPAGfxLargeBg(screen, bg, name) \
    shim_LoadLargeBg((screen), (bg), name##_Tiles, sizeof(name##_Tiles), \
                     name##_Map, name##_Pal, name##_Info)
#define PA_LoadLargeBg(screen, bg, tiles, map, cm, tw, th) \
    shim_LoadLargeBgEx((screen), (bg), (tiles), 65536, (map), (tw), (th))
void shim_LoadLargeBgEx(u8 screen, u8 bg, const void *tiles, u32 tiles_size,
                        const void *map, int tw, int th);

/* 8-bit bitmap layer ops */
void PA_Init8bitBg(u8 screen, u8 priority);
void PA_Clear8bitBg(u8 screen);
void PA_Load8bitBitmap(u8 screen, const void *bitmap);
void PA_Put8bitPixel(u8 screen, s32 x, s32 y, u8 color);
void PA_PutDouble8bitPixels(u8 screen, s32 x, s32 y, u8 color1, u8 color2);
void PA_Draw8bitLine(u8 screen, s32 x1, s32 y1, s32 x2, s32 y2, u8 color);

/* stubs kept for completeness (unused paths are commented out in game) */
#define PA_Init16bitBg(s, b)      ((void)0)
#define PA_Init16cBg(s, b)        ((void)0)
#define PA_InitText(s, b)         ((void)0)
#define PA_FSInit()               (0)
#define PA_PAFSFile(i)            ((u8 *)0)

/* ------------------------------------------------------------------ */
/* Sprites / OAM / sprite VRAM emulation                                */
/* ------------------------------------------------------------------ */
#define PA_NMAXSPRITES  128
#define PA_VRAM_TILES   65536           /* per screen, 64 bytes each    */
#define NUMBER_DECAL    5               /* gfx block -> u16 offset      */
#define MEM_DECAL       4               /* used_mem -> halfword count   */

extern u16 *SPRITE_GFX1;                        /* base of sprite vram   */
extern u16  used_mem[2][PA_VRAM_TILES];         /* block size in tiles   */
extern const u8 *PA_SpriteAnimP[2][PA_VRAM_TILES]; /* anim source ptr    */

#define DMA_16NOW 1
#define DMA_32NOW 2
void DMA_Copy(const void *src, void *dst, u32 count, int mode);

void PA_ResetSpriteSys(void);
void PA_CreateSprite(u8 screen, u8 obj, const void *gfx, int obj_size,
                     u8 color_mode, u8 palette, s16 x, s16 y);
void PA_DeleteSprite(u8 screen, u8 obj);
void PA_SetSpriteXY(u8 screen, u8 obj, s16 x, s16 y);
s16  PA_GetSpriteX(u8 screen, u8 obj);
s16  PA_GetSpriteY(u8 screen, u8 obj);
void PA_SetSpriteAnim(u8 screen, u8 obj, int frame);
void PA_SetSpriteAnimEx(u8 screen, u8 obj, int w, int h, u8 color_mode, int frame);
void PA_SetSpritePrio(u8 screen, u8 obj, u8 prio);
void PA_SetSpritePal(u8 screen, u8 obj, u8 palette);
void PA_SetSpriteMode(u8 screen, u8 obj, u8 mode);
void PA_SetSpriteHflip(u8 screen, u8 obj, u8 hflip);
u8   PA_GetSpriteHflip(u8 screen, u8 obj);
void PA_SetSpriteRotEnable(u8 screen, u8 obj, u8 rotset);
void PA_SetSpriteRotDisable(u8 screen, u8 obj);
void PA_SetRotsetNoAngle(u8 screen, u8 rotset, u16 zoomx, u16 zoomy);
void PA_LoadSpritePal(u8 screen, u8 palette, const void *pal);
int  PA_GetSpriteGfx(u8 screen, u8 obj);
void PA_UpdateGfx(u8 screen, int gfx_num, const void *data);
/* PA_UpdateSpriteGfxAndMem is redefined by the game (defines.h) on top
 * of the primitives above. */

/* ------------------------------------------------------------------ */
/* Sound: PAlib entry points + ASlib (IMA-ADPCM, 16 channels)           */
/* ------------------------------------------------------------------ */
#define AS_MODE_MP3   1
#define AS_MODE_SURROUND 2
#define AS_MODE_16CH  4
#define AS_ADPCM      2
#define AS_PCM_8BIT   0
#define AS_NO_DELAY   0
#define AS_SURROUND   1

typedef struct { int freq; int format; } pa_soundoption_t;
extern pa_soundoption_t PA_SoundOption;

void PA_InitSound(void);
#define PA_InitASLibForMP3(mode) ((void)0)
void AS_SetDefaultSettings(u8 format, u16 rate, u8 delay);
void AS_SetMasterVolume(u8 volume);
int  AS_SoundDefaultPlay(const u8 *data, u32 size, u8 volume, u8 pan,
                         u8 loop, u8 prio);
#define AS_SoundQuickPlay(name) \
    AS_SoundDefaultPlay((const u8 *)(name), (u32)(name##_size), 127, 64, 0, 0)

int  shim_PlaySimpleSound(const void *data, u32 size);
#define PA_PlaySimpleSound(name) \
    shim_PlaySimpleSound((const void *)(name), (u32)(name##_size))

void PA_StopSound(int channel);
int  PA_SoundChannelIsBusy(int channel);
void PA_SetSoundChannelPan(int channel, u8 pan);
void PA_SetSoundVol(u8 vol);
int  PA_GetFreeSoundChannel(void);
void PA_PlaySoundEx(int channel, const void *data, u32 size, u8 volume,
                    int freq, int format);
#define PA_SetDefaultSound(vol, freq, fmt) ((void)0)

/* ------------------------------------------------------------------ */
/* Compositor interface (used by the platform backends)                 */
/* ------------------------------------------------------------------ */
/* Renders engine screen (0/1) into a 256x192 RGBA8888 buffer.          */
void shim_ComposeScreen(int screen, u32 *out);
/* 1 if PA_SwitchScreens is active (engine 0 displayed on top).         */
int  shim_ScreensSwitched(void);
/* Per-frame input snapshot, fed by the platform backend.               */
void shim_SetTouchState(int down, int x, int y);
void shim_SetPadState(u32 held_mask); /* bit order: A B X Y L R Start Select Up Down Left Right */
/* Called by PA_WaitForVBL: platform hook (render + pace + pump input). */
void platform_vblank(void);
/* Root directory for the virtual filesystem ("/maps", "/settings"...). */
void shim_SetFsRoot(const char *root);
const char *shim_GetFsRoot(void);

/* ---- extended map viewport (Android QoL: fill the screen + zoom) ---- */
/* Effective size of engine screen 0 (256x192 in menus/dialogs; larger   */
/* while the in-game map view is active).  The engine reads these.       */
extern int shim_view_w, shim_view_h;
/* Game hooks: map view on/off with the map pixel size for clamping.     */
void shim_SetMapView(int active, int map_w, int map_h);
/* Modal dialogs render at classic 256x192 while open.                   */
void shim_ModalBegin(void);
void shim_ModalEnd(void);
/* Platform backend: requested viewport size (from device size / zoom).  */
void shim_SetWantedViewSize(int w, int h);
/* 1 while the map view is active and no modal dialog is open.           */
int  shim_MapViewInteractive(void);
void shim_GetMapLimits(int *w, int *h);
/* modal-dialog rendering: 256x192 dialog space centred in the viewport  */
int  shim_InModal(void);
void shim_GetDialogOffset(int *ox, int *oy);
/* in-game speed multiplier (1 / 2 / 4), cycled by the on-screen button; */
/* the platform backend runs that many engine ticks per displayed frame  */
extern int shim_game_speed;

/* rename the game entry point; the platform backend calls it */
#define main wtd_game_main
int wtd_game_main(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif /* _PA9_SHIM_H */
