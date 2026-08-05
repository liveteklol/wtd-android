/*
 * shim_video.c — software emulation of the DS 2D engines as used by PAlib:
 * 2 screens x (4 backgrounds + 128 sprites), 8-bit paletted graphics,
 * rot/zoom sets, alpha blending, brightness, sprite VRAM arena.
 */
#include <PA9.h>
#undef fopen
#undef main

/* ------------------------------------------------------------------ */
/* State                                                                */
/* ------------------------------------------------------------------ */

#define VRAM_U16_PER_SCREEN 0x200000u   /* matches game's screen offset */
#define TILE_U16            32          /* 64 bytes per gfx block unit  */

typedef struct {
    int  type;              /* 0 off, 1 tiled, 2 large, 3 bmp8          */
    int  prio;
    int  size_code;         /* tiled                                    */
    int  tw, th;            /* large: tile dimensions                   */
    s32  scx, scy;
    int  wrap;              /* large bg: wrap scrolling                 */
    u16  pal[256];          /* extended palette (per-bg, like PAlib)    */
    u8   tiles[65536];
    u16  map[65536];        /* tiled: screen-block layout; large: row-major */
} bglayer;

typedef struct {
    u8  used;
    s16 x, y;
    u16 w, h;
    u32 gfx;                /* block index in the vram arena (tiles)    */
    u16 ntiles;             /* allocated tiles                          */
    u8  pal, prio, hflip, vflip, mode;
    u8  dspace;             /* positioned in modal-dialog space         */
    s8  rotset;             /* -1: none                                 */
} oamobj;

typedef struct { u16 zoomx, zoomy; } rotset_t;

typedef struct {
    bglayer  bg[4];
    oamobj   obj[PA_NMAXSPRITES];
    rotset_t rot[32];
    u16      bgpal[256];
    u16      objpal[16][256];
    u16      draw[128 * 256];   /* 8-bit bitmap layer, 2 px per u16      */
    s8       bright;
    u8       eva, evb;
    u32      vram_alloc;        /* bump pointer (tiles); freed on reset  */
} screen_t;

static screen_t S[2];
static int screens_switched = 0;

/* ------------------------------------------------------------------ */
/* Extended map viewport (bottom screen only).                          */
/* Engine screen 0 composes at shim_view_w x shim_view_h; the top       */
/* screen (engine 1) always stays 256x192.                              */
/* ------------------------------------------------------------------ */

#define VIEW_MAX_W 1024
#define VIEW_MAX_H 1024

int shim_view_w = 256, shim_view_h = 192;

static int mapview_active;
static int modal_depth;
static int map_lim_w = 4096, map_lim_h = 4096;
static int want_w = 256, want_h = 192;

static void update_view(void)
{
    if (mapview_active) {
        int w = want_w, h = want_h;
        if (w > map_lim_w) w = map_lim_w;
        if (h > map_lim_h) h = map_lim_h;
        if (w > VIEW_MAX_W) w = VIEW_MAX_W;
        if (h > VIEW_MAX_H) h = VIEW_MAX_H;
        /* zoomed-in views may go below 256x192... */
        if (w < 144) w = 144;
        if (h < 108) h = 108;
        /* ...but modal dialogs need the full 256x192 dialog space */
        if (modal_depth) {
            if (w < 256) w = 256;
            if (h < 192) h = 192;
        }
        shim_view_w = w;
        shim_view_h = h;
    } else {
        shim_view_w = 256;
        shim_view_h = 192;
    }
}

/* While a modal dialog is open over the extended map view, its 256x192
   content (8-bit bitmap layer + dialog sprites) is rendered centred.    */
void shim_GetDialogOffset(int *ox, int *oy)
{
    if (modal_depth && mapview_active) {
        *ox = (shim_view_w - 256) / 2;
        *oy = (shim_view_h - 192) / 2;
    } else {
        *ox = 0;
        *oy = 0;
    }
}

void shim_SetMapView(int active, int map_w, int map_h)
{
    mapview_active = active;
    if (map_w > 0) map_lim_w = map_w; else if (!active) map_lim_w = 4096;
    if (map_h > 0) map_lim_h = map_h; else if (!active) map_lim_h = 4096;
    update_view();
}

void shim_ModalBegin(void) { modal_depth++; update_view(); }
void shim_ModalEnd(void)   { if (modal_depth) modal_depth--; update_view(); }
int  shim_InModal(void)    { return modal_depth > 0; }

void shim_SetWantedViewSize(int w, int h)
{
    want_w = w;
    want_h = h;
    update_view();
}

int shim_MapViewInteractive(void) { return mapview_active && !modal_depth; }

void shim_GetMapLimits(int *w, int *h)
{
    if (w) *w = map_lim_w;
    if (h) *h = map_lim_h;
}

/* exported PAlib-internals used by the game's own macros */
u16 *PA_DrawBg[2];
u16 *SPRITE_GFX1;
u16  used_mem[2][PA_VRAM_TILES];
const u8 *PA_SpriteAnimP[2][PA_VRAM_TILES];
pa_bginfo_t PA_BgInfo[2][4];
const u32 Blank[16384];

pa_userinfo_t PA_UserInfo = { "Player" };
pa_rtc_t      PA_RTC;
pa_soundoption_t PA_SoundOption = { 22050, 2 };

static ipc_sound_t ipc_sound_storage;
ipc_sound_t *IPC_Sound = &ipc_sound_storage;

static u16 *vram_base;          /* 2 * VRAM_U16_PER_SCREEN u16          */

/* ------------------------------------------------------------------ */
/* Init / reset                                                         */
/* ------------------------------------------------------------------ */

void PA_Init(void)
{
    if (!vram_base) {
        vram_base = (u16 *)calloc(2 * VRAM_U16_PER_SCREEN, sizeof(u16));
        SPRITE_GFX1 = vram_base;
    }
    PA_DrawBg[0] = S[0].draw;
    PA_DrawBg[1] = S[1].draw;
    PA_ResetBgSys();
    PA_ResetSpriteSys();
    screens_switched = 0;
}

void PA_InitVBL(void) {}

void PA_ResetBgSys(void)
{
    int s, b;
    for (s = 0; s < 2; s++) {
        for (b = 0; b < 4; b++) {
            S[s].bg[b].type = 0;
            S[s].bg[b].scx = S[s].bg[b].scy = 0;
            PA_BgInfo[s][b].BgMode = 0;
            PA_BgInfo[s][b].NTiles = 0;
            PA_BgInfo[s][b].Tiles = NULL;
        }
        memset(S[s].draw, 0, sizeof(S[s].draw));
        memset(S[s].bgpal, 0, sizeof(S[s].bgpal));
    }
}

static void vram_reset_alloc(int screen);

void PA_ResetSpriteSys(void)
{
    int s, i;
    for (s = 0; s < 2; s++) {
        memset(S[s].obj, 0, sizeof(S[s].obj));
        for (i = 0; i < PA_NMAXSPRITES; i++)
            S[s].obj[i].rotset = -1;
        for (i = 0; i < 32; i++)
            S[s].rot[i].zoomx = S[s].rot[i].zoomy = 256;
        vram_reset_alloc(s);
        memset(used_mem[s], 0, sizeof(used_mem[s]));
        S[s].eva = 16; S[s].evb = 0;
    }
}

/* ------------------------------------------------------------------ */
/* Screens / fx                                                         */
/* ------------------------------------------------------------------ */

void PA_SetBrightness(u8 screen, s8 b) { S[screen & 1].bright = b; }
void PA_SwitchScreens(void)            { screens_switched = !screens_switched; }
int  shim_ScreensSwitched(void)        { return screens_switched; }
void PA_EnableSpecialFx(u8 screen, u8 fx, u8 o, u8 t) { (void)screen; (void)fx; (void)o; (void)t; }
void PA_SetSFXAlpha(u8 screen, u8 a1, u8 a2)
{
    S[screen & 1].eva = (a1 > 16) ? 16 : a1;
    S[screen & 1].evb = (a2 > 16) ? 16 : a2;
}

/* ------------------------------------------------------------------ */
/* Palettes                                                             */
/* ------------------------------------------------------------------ */

void PA_LoadBgPal(u8 screen, u8 bg, const void *pal)
{
    /* extended palette: private to this tiled/large background */
    memcpy(S[screen & 1].bg[bg & 3].pal, pal, 512);
}
void PA_Load8bitBgPal(u8 screen, const void *pal)
{
    /* the 8-bit bitmap layer uses the standard bg palette */
    memcpy(S[screen & 1].bgpal, pal, 512);
}
void PA_SetBgPalCol(u8 screen, u8 index, u16 color) { S[screen & 1].bgpal[index] = color; }
void PA_LoadSpritePal(u8 screen, u8 palette, const void *pal)
{
    memcpy(S[screen & 1].objpal[palette & 15], pal, 512);
}

/* ------------------------------------------------------------------ */
/* Backgrounds                                                          */
/* ------------------------------------------------------------------ */

int PA_GetPAGfxBgSize(int width, int height)
{
    if (width == 512 && height == 512) return BG_512X512;
    if (width == 512)                  return BG_512X256;
    if (height == 512)                 return BG_256X512;
    return BG_256X256;
}

void PA_DeleteBg(u8 screen, u8 bg) { S[screen & 1].bg[bg & 3].type = 0; }

void PA_LoadBgTilesEx(u8 screen, u8 bg, const void *tiles, u32 size)
{
    bglayer *l = &S[screen & 1].bg[bg & 3];
    if (size > sizeof(l->tiles)) size = sizeof(l->tiles);
    memcpy(l->tiles, tiles, size);
}

void PA_LoadBgMap(u8 screen, u8 bg, const void *map, int bg_size)
{
    bglayer *l = &S[screen & 1].bg[bg & 3];
    int entries = 1024;
    if (bg_size == BG_512X256 || bg_size == BG_256X512) entries = 2048;
    if (bg_size == BG_512X512) entries = 4096;
    memcpy(l->map, map, entries * 2);
}

void PA_InitBg(u8 screen, u8 bg, int bg_size, u8 wraparound, u8 color_mode)
{
    (void)color_mode;
    bglayer *l = &S[screen & 1].bg[bg & 3];
    l->type = 1;
    l->size_code = bg_size;
    l->wrap = wraparound;
    l->prio = bg & 3;
    l->scx = l->scy = 0;
}

void PA_InitLargeBg(u8 screen, u8 bg, int tw, int th, const void *map)
{
    bglayer *l = &S[screen & 1].bg[bg & 3];
    l->type = 2;
    l->tw = tw;
    l->th = th;
    l->prio = bg & 3;
    u32 n = (u32)tw * (u32)th;
    if (n > 65536) n = 65536;
    memcpy(l->map, map, n * 2);
}

void shim_LoadTiledBg(u8 screen, u8 bg, const void *tiles, u32 tiles_size,
                      const void *map, const void *pal, const int *info)
{
    int sc = PA_GetPAGfxBgSize(info[1], info[2]);
    PA_LoadBgPal(screen, bg, pal);
    PA_LoadBgTilesEx(screen, bg, tiles, tiles_size);
    PA_LoadBgMap(screen, bg, map, sc);
    PA_InitBg(screen, bg, sc, 0, 1);
}

void shim_LoadLargeBgEx(u8 screen, u8 bg, const void *tiles, u32 tiles_size,
                        const void *map, int tw, int th)
{
    PA_LoadBgTilesEx(screen, bg, tiles, tiles_size);
    PA_InitLargeBg(screen, bg, tw, th, map);
}

void shim_LoadLargeBg(u8 screen, u8 bg, const void *tiles, u32 tiles_size,
                      const void *map, const void *pal, const int *info)
{
    PA_LoadBgPal(screen, bg, pal);
    shim_LoadLargeBgEx(screen, bg, tiles, tiles_size, map, info[1] >> 3, info[2] >> 3);
}

void PA_BGScrollXY(u8 screen, u8 bg, s32 x, s32 y)
{
    S[screen & 1].bg[bg & 3].scx = x;
    S[screen & 1].bg[bg & 3].scy = y;
}
void PA_LargeScrollXY(u8 screen, u8 bg, s32 x, s32 y)   { PA_BGScrollXY(screen, bg, x, y); }
void PA_InfLargeScrollXY(u8 screen, u8 bg, s32 x, s32 y){ PA_BGScrollXY(screen, bg, x, y); }

/* 8-bit bitmap layer ------------------------------------------------ */

void PA_Init8bitBg(u8 screen, u8 priority)
{
    bglayer *l = &S[screen & 1].bg[3];
    l->type = 3;
    l->prio = priority;
    memset(S[screen & 1].draw, 0, sizeof(S[screen & 1].draw));
}

void PA_Clear8bitBg(u8 screen)
{
    memset(S[screen & 1].draw, 0, sizeof(S[screen & 1].draw));
}

void PA_Load8bitBitmap(u8 screen, const void *bitmap)
{
    memcpy(S[screen & 1].draw, bitmap, 256 * 192);
}

void PA_Put8bitPixel(u8 screen, s32 x, s32 y, u8 color)
{
    if ((u32)x >= 256 || (u32)y >= 256) return;
    ((u8 *)S[screen & 1].draw)[x + (y << 8)] = color;
}

void PA_PutDouble8bitPixels(u8 screen, s32 x, s32 y, u8 c1, u8 c2)
{
    if ((u32)x >= 256 || (u32)y >= 256) return;
    S[screen & 1].draw[(x >> 1) + (y << 7)] = (u16)c1 | ((u16)c2 << 8);
}

void PA_Draw8bitLine(u8 screen, s32 x1, s32 y1, s32 x2, s32 y2, u8 color)
{
    s32 dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    s32 dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    s32 err = dx + dy;
    for (;;) {
        PA_Put8bitPixel(screen, x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        s32 e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}

/* ------------------------------------------------------------------ */
/* Sprite VRAM arena                                                    */
/* ------------------------------------------------------------------ */

void DMA_Copy(const void *src, void *dst, u32 count, int mode)
{
    memcpy(dst, src, mode == DMA_32NOW ? count * 4 : count * 2);
}

/* free lists per block size (in tiles, max 64 = a 64x64 sprite):
   sprites are deleted/re-created every frame by the game's UI code, so
   freed blocks must be reused or the arena eventually wraps onto the
   gfx of long-lived sprites (this corrupted the minimap after ~20 s) */
#define VRAM_MAX_NTILES 64
static s32 vram_free_head[2][VRAM_MAX_NTILES + 1];
static s32 vram_free_next[2][PA_VRAM_TILES];

static void vram_reset_alloc(int screen)
{
    int i;
    for (i = 0; i <= VRAM_MAX_NTILES; i++)
        vram_free_head[screen][i] = -1;
    S[screen].vram_alloc = 0;
}

static void vram_free_tiles(int screen, u32 block, u32 ntiles)
{
    if (ntiles == 0 || ntiles > VRAM_MAX_NTILES) return;
    vram_free_next[screen][block] = vram_free_head[screen][ntiles];
    vram_free_head[screen][ntiles] = (s32)block;
}

static u32 vram_alloc_tiles(int screen, u32 ntiles)
{
    screen_t *sc = &S[screen];
    if (ntiles <= VRAM_MAX_NTILES && vram_free_head[screen][ntiles] >= 0) {
        u32 block = (u32)vram_free_head[screen][ntiles];
        vram_free_head[screen][ntiles] = vram_free_next[screen][block];
        return block;
    }
    if (sc->vram_alloc + ntiles > PA_VRAM_TILES)
        sc->vram_alloc = 0;                 /* should not happen anymore */
    u32 block = sc->vram_alloc;
    sc->vram_alloc += ntiles;
    return block;
}

static u16 *vram_ptr(int screen, u32 block)
{
    return SPRITE_GFX1 + (u32)screen * VRAM_U16_PER_SCREEN + (block << NUMBER_DECAL);
}

/* ------------------------------------------------------------------ */
/* Sprites                                                              */
/* ------------------------------------------------------------------ */

void PA_CreateSprite(u8 screen, u8 obj, const void *gfx, int obj_size,
                     u8 color_mode, u8 palette, s16 x, s16 y)
{
    (void)color_mode;
    screen &= 1;
    oamobj *o = &S[screen].obj[obj];
    int w = (obj_size >> 8) & 0xff, h = obj_size & 0xff;
    u32 ntiles = (u32)(w * h) / 64;

    if (o->used)                       /* re-created without a delete */
        vram_free_tiles(screen, o->gfx, o->ntiles);

    o->used   = 1;
    /* dialog-space only for visible positions: sprites parked offscreen
       (-32,-32...) must stay offscreen, not be shifted into view */
    o->dspace = shim_InModal() && x > -16 && y > -16;
    o->x = x; o->y = y;
    o->w = w; o->h = h;
    o->pal = palette & 15;
    o->prio = 0;               /* PAlib default: OBJ priority 0 */
    o->hflip = o->vflip = 0;
    o->mode = 0;
    o->rotset = -1;
    o->gfx = vram_alloc_tiles(screen, ntiles);
    o->ntiles = ntiles;
    used_mem[screen][o->gfx] = ntiles;
    PA_SpriteAnimP[screen][o->gfx] = (const u8 *)gfx;

    if (gfx)
        memcpy(vram_ptr(screen, o->gfx), gfx, (size_t)w * h);
    else
        memset(vram_ptr(screen, o->gfx), 0, (size_t)w * h);
}

void PA_DeleteSprite(u8 screen, u8 obj)
{
    oamobj *o = &S[screen & 1].obj[obj];
    if (o->used)
        vram_free_tiles(screen & 1, o->gfx, o->ntiles);
    o->used = 0;
}

void PA_SetSpriteXY(u8 screen, u8 obj, s16 x, s16 y)
{
    S[screen & 1].obj[obj].x = x;
    S[screen & 1].obj[obj].y = y;
    /* sprites (re)positioned while a dialog is open belong to it;
       parked positions (negative) always leave dialog space */
    S[screen & 1].obj[obj].dspace = shim_InModal() && x > -16 && y > -16;
}
s16 PA_GetSpriteX(u8 screen, u8 obj) { return S[screen & 1].obj[obj].x; }
s16 PA_GetSpriteY(u8 screen, u8 obj) { return S[screen & 1].obj[obj].y; }

void PA_SetSpriteAnimEx(u8 screen, u8 obj, int w, int h, u8 cm, int frame)
{
    (void)cm;
    screen &= 1;
    oamobj *o = &S[screen].obj[obj];
    const u8 *src = PA_SpriteAnimP[screen][o->gfx];
    if (!src) return;
    memcpy(vram_ptr(screen, o->gfx), src + (size_t)frame * w * h, (size_t)w * h);
}

void PA_SetSpriteAnim(u8 screen, u8 obj, int frame)
{
    oamobj *o = &S[screen & 1].obj[obj];
    PA_SetSpriteAnimEx(screen, obj, o->w, o->h, 1, frame);
}

void PA_SetSpritePrio(u8 screen, u8 obj, u8 prio) { S[screen & 1].obj[obj].prio = prio & 3; }
void PA_SetSpritePal(u8 screen, u8 obj, u8 pal)   { S[screen & 1].obj[obj].pal = pal & 15; }
void PA_SetSpriteMode(u8 screen, u8 obj, u8 mode) { S[screen & 1].obj[obj].mode = mode; }
void PA_SetSpriteHflip(u8 screen, u8 obj, u8 f)   { S[screen & 1].obj[obj].hflip = f ? 1 : 0; }
u8   PA_GetSpriteHflip(u8 screen, u8 obj)         { return S[screen & 1].obj[obj].hflip; }
void PA_SetSpriteRotEnable(u8 screen, u8 obj, u8 rotset) { S[screen & 1].obj[obj].rotset = rotset & 31; }
void PA_SetSpriteRotDisable(u8 screen, u8 obj)    { S[screen & 1].obj[obj].rotset = -1; }
void PA_SetRotsetNoAngle(u8 screen, u8 rotset, u16 zx, u16 zy)
{
    S[screen & 1].rot[rotset & 31].zoomx = zx ? zx : 256;
    S[screen & 1].rot[rotset & 31].zoomy = zy ? zy : 256;
}

int PA_GetSpriteGfx(u8 screen, u8 obj) { return (int)S[screen & 1].obj[obj].gfx; }

void PA_UpdateGfx(u8 screen, int gfx_num, const void *data)
{
    screen &= 1;
    memcpy(vram_ptr(screen, gfx_num), data,
           (size_t)used_mem[screen][gfx_num] * 64);
    PA_SpriteAnimP[screen][gfx_num] = (const u8 *)data;
}

/* touch detection: stylus held inside the sprite's bounding box on
   the touch engine (screen 0) */
int PA_SpriteTouched(int sprite)
{
    extern stylustype Stylus;
    oamobj *o = &S[0].obj[sprite & (PA_NMAXSPRITES - 1)];
    if (!o->used || !Stylus.Held) return 0;
    return Stylus.X >= o->x && Stylus.X < o->x + o->w &&
           Stylus.Y >= o->y && Stylus.Y < o->y + o->h;
}

/* ------------------------------------------------------------------ */
/* Composition                                                          */
/* ------------------------------------------------------------------ */

static inline u32 col555_to_rgba(u16 c)
{
    u32 r = (c & 31),  g = ((c >> 5) & 31), b = ((c >> 10) & 31);
    r = (r << 3) | (r >> 2); g = (g << 3) | (g >> 2); b = (b << 3) | (b >> 2);
    return 0xff000000u | (b << 16) | (g << 8) | r;
}

/* tiled bg pixel lookup (DS screen-block layout), returns palette idx  */
static inline u8 tiled_pix(const bglayer *l, u32 x, u32 y)
{
    u32 W = (l->size_code == BG_512X256 || l->size_code == BG_512X512) ? 512 : 256;
    u32 H = (l->size_code == BG_256X512 || l->size_code == BG_512X512) ? 512 : 256;
    x &= (W - 1); y &= (H - 1);
    u32 block = 0;
    if (x >= 256) { block += 1; x -= 256; }
    if (y >= 256) { block += (W == 512) ? 2 : 1; y -= 256; }
    u16 e = l->map[block * 1024 + ((y >> 3) << 5) + (x >> 3)];
    u32 tile = e & 0x3ff;
    u32 tx = x & 7, ty = y & 7;
    if (e & 0x400) tx = 7 - tx;
    if (e & 0x800) ty = 7 - ty;
    return l->tiles[tile * 64 + ty * 8 + tx];
}

static inline u8 large_pix(const bglayer *l, s32 x, s32 y)
{
    u32 W = (u32)l->tw * 8, H = (u32)l->th * 8;
    u32 ux = ((u32)((x % (s32)W) + W)) % W;
    u32 uy = ((u32)((y % (s32)H) + H)) % H;
    u16 e = l->map[(uy >> 3) * l->tw + (ux >> 3)];
    u32 tile = e & 0x3ff;
    u32 tx = ux & 7, ty = uy & 7;
    if (e & 0x400) tx = 7 - tx;
    if (e & 0x800) ty = 7 - ty;
    return l->tiles[tile * 64 + ty * 8 + tx];
}

static void draw_bg_line(screen_t *sc, const bglayer *l, u32 *line, int y, int vw)
{
    int x;
    switch (l->type) {
    case 1:
        for (x = 0; x < vw; x++) {
            u8 p = tiled_pix(l, (u32)(x + l->scx), (u32)(y + l->scy));
            if (p) line[x] = col555_to_rgba(l->pal[p]);
        }
        break;
    case 2:
        for (x = 0; x < vw; x++) {
            u8 p = large_pix(l, x + l->scx, y + l->scy);
            if (p) line[x] = col555_to_rgba(l->pal[p]);
        }
        break;
    case 3: {
        /* the 8-bit bitmap layer is a fixed 256x192 surface; while a
           modal dialog is open over the extended view it is centred */
        int ox, oy;
        shim_GetDialogOffset(&ox, &oy);
        int by = y - oy;
        if (by < 0 || by >= 192) break;
        const u8 *row = (const u8 *)sc->draw + (by << 8);
        int x0 = ox > 0 ? ox : 0;
        int xm = ox + 256 < vw ? ox + 256 : vw;
        for (x = x0; x < xm; x++)
            if (row[x - ox]) line[x] = col555_to_rgba(sc->bgpal[row[x - ox]]);
        break;
    }
    }
}

static inline u8 sprite_texel(const u16 *gfx8, int w, int sx, int sy)
{
    /* 1D tiled layout: 8x8 tiles, row-major tiles, row-major in tile */
    const u8 *g = (const u8 *)gfx8;
    int tpr = w >> 3;
    int t = (sy >> 3) * tpr + (sx >> 3);
    return g[t * 64 + (sy & 7) * 8 + (sx & 7)];
}

static inline u32 blend(u32 src, u32 dst, int eva, int evb)
{
    u32 r = (((src) & 0xff) * eva + ((dst) & 0xff) * evb) >> 4;
    u32 g = ((((src) >> 8) & 0xff) * eva + (((dst) >> 8) & 0xff) * evb) >> 4;
    u32 b = ((((src) >> 16) & 0xff) * eva + (((dst) >> 16) & 0xff) * evb) >> 4;
    if (r > 255) r = 255; if (g > 255) g = 255; if (b > 255) b = 255;
    return 0xff000000u | (b << 16) | (g << 8) | r;
}

static void draw_sprite(screen_t *sc, int screen, const oamobj *o, u32 *fb,
                        int vw, int vh)
{
    const u16 *gfx = vram_ptr(screen, o->gfx);
    const u16 *pal = sc->objpal[o->pal];
    int eva = sc->eva, evb = sc->evb;
    int obx = o->x, oby = o->y;

    if ((screen & 1) == 0 && o->dspace) {
        int ox, oy;
        shim_GetDialogOffset(&ox, &oy);
        obx += ox;
        oby += oy;
    }

    if (o->rotset < 0) {
        int sy, sx;
        for (sy = 0; sy < o->h; sy++) {
            int py = oby + sy;
            if ((u32)py >= (u32)vh) continue;
            u32 *line = fb + py * vw;
            for (sx = 0; sx < o->w; sx++) {
                int px = obx + sx;
                if ((u32)px >= (u32)vw) continue;
                u8 p = sprite_texel(gfx, o->w,
                                    o->hflip ? (o->w - 1 - sx) : sx,
                                    o->vflip ? (o->h - 1 - sy) : sy);
                if (!p) continue;
                u32 c = col555_to_rgba(pal[p]);
                line[px] = o->mode == 1 ? blend(c, line[px], eva, evb) : c;
            }
        }
    } else {
        /* zoom around sprite centre, double-size coverage */
        const rotset_t *r = &sc->rot[o->rotset];
        s32 cx = obx + o->w / 2, cy = oby + o->h / 2;
        s32 zx = r->zoomx, zy = r->zoomy;
        int oy, ox;
        for (oy = -o->h; oy < o->h; oy++) {
            int py = cy + oy;
            if ((u32)py >= (u32)vh) continue;
            u32 *line = fb + py * vw;
            s32 sy = ((oy * zy) >> 8) + o->h / 2;
            if (sy < 0 || sy >= o->h) continue;
            for (ox = -o->w; ox < o->w; ox++) {
                int px = cx + ox;
                if ((u32)px >= (u32)vw) continue;
                s32 sx = ((ox * zx) >> 8) + o->w / 2;
                if (sx < 0 || sx >= o->w) continue;
                u8 p = sprite_texel(gfx, o->w,
                                    o->hflip ? (o->w - 1 - (int)sx) : (int)sx,
                                    o->vflip ? (o->h - 1 - (int)sy) : (int)sy);
                if (!p) continue;
                u32 c = col555_to_rgba(pal[p]);
                line[px] = o->mode == 1 ? blend(c, line[px], eva, evb) : c;
            }
        }
    }
}

void shim_ComposeScreen(int screen, u32 *out)
{
    screen_t *sc = &S[screen & 1];
    int y, prio, b, i;
    int vw = (screen & 1) ? 256 : shim_view_w;
    int vh = (screen & 1) ? 192 : shim_view_h;
    int npix = vw * vh;

    u32 backdrop = col555_to_rgba(sc->bgpal[0]);
    for (i = 0; i < npix; i++) out[i] = backdrop;

    for (prio = 3; prio >= 0; prio--) {
        for (b = 3; b >= 0; b--) {
            bglayer *l = &sc->bg[b];
            if (l->type && l->prio == prio)
                for (y = 0; y < vh; y++)
                    draw_bg_line(sc, l, out + y * vw, y, vw);
        }
        for (i = PA_NMAXSPRITES - 1; i >= 0; i--) {
            oamobj *o = &sc->obj[i];
            if (o->used && o->prio == prio)
                draw_sprite(sc, screen & 1, o, out, vw, vh);
        }
    }

    /* brightness: -32 (black) .. 0 .. +32 (white) */
    if (sc->bright) {
        int br = sc->bright;
        if (br > 32) br = 32;
        if (br < -32) br = -32;
        if (br > 0) {
            for (i = 0; i < npix; i++) {
                u32 c = out[i];
                u32 r = c & 0xff, g = (c >> 8) & 0xff, bl = (c >> 16) & 0xff;
                r += ((255 - r) * br) >> 5;
                g += ((255 - g) * br) >> 5;
                bl += ((255 - bl) * br) >> 5;
                out[i] = 0xff000000u | (bl << 16) | (g << 8) | r;
            }
        } else {
            int f = 32 + br;   /* 0..32 */
            for (i = 0; i < npix; i++) {
                u32 c = out[i];
                u32 r = ((c & 0xff) * f) >> 5;
                u32 g = (((c >> 8) & 0xff) * f) >> 5;
                u32 bl = (((c >> 16) & 0xff) * f) >> 5;
                out[i] = 0xff000000u | (bl << 16) | (g << 8) | r;
            }
        }
    }
}
