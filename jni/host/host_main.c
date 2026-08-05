/*
 * host_main.c — headless Linux harness for verifying the port without a
 * device: runs the game loop, feeds scripted input from WTD_SCRIPT, and
 * dumps both screens as PPM images on selected frames.
 *
 * Script line format (one action per line):
 *   <frame> touch <x> <y> <holdframes>
 *   <frame> pad <name> <holdframes>      (a b x y l r start select up down left right)
 *   <frame> view <w> <h>                 (mid-run viewport change, i.e. pinch zoom)
 *   <frame> dump <label>
 *   <frame> exit
 */
#include <PA9.h>
#undef fopen
#undef main
#include <unistd.h>

void shim_MixAudio(s16 *out, u32 nframes);

static u32 frame_no;
static char outdir[512] = "frames";

typedef struct { u32 frame; int type; int x, y, hold; char label[64]; } act_t;
static act_t acts[512];
static int nacts;

static int touch_until = -1, touch_x, touch_y;
static int pad_until = -1; static u32 pad_bit;

static const char *padnames[12] = {
    "a","b","x","y","l","r","start","select","up","down","left","right"
};

static void load_script(void)
{
    const char *path = getenv("WTD_SCRIPT");
    if (!path) return;
    FILE *f = fopen(path, "r");
    if (!f) return;
    char line[256];
    while (fgets(line, sizeof(line), f) && nacts < 512) {
        act_t *a = &acts[nacts];
        char cmd[32], arg[64];
        int x, y, h;
        if (sscanf(line, "%u touch %d %d %d", &a->frame, &x, &y, &h) == 4) {
            a->type = 1; a->x = x; a->y = y; a->hold = h; nacts++;
        } else if (sscanf(line, "%u pad %31s %d", &a->frame, cmd, &h) == 3) {
            a->type = 2; a->hold = h;
            for (x = 0; x < 12; x++)
                if (!strcmp(cmd, padnames[x])) a->x = x;
            nacts++;
        } else if (sscanf(line, "%u view %d %d", &a->frame, &x, &y) == 3) {
            a->type = 5; a->x = x; a->y = y; nacts++;
        } else if (sscanf(line, "%u probe %63s", &a->frame, arg) == 2) {
            a->type = 6; snprintf(a->label, sizeof(a->label), "%s", arg); nacts++;
        } else if (sscanf(line, "%u dump %63s", &a->frame, arg) == 2) {
            a->type = 3; snprintf(a->label, sizeof(a->label), "%s", arg); nacts++;
        } else if (sscanf(line, "%u %31s", &a->frame, cmd) == 2 && !strcmp(cmd, "exit")) {
            a->type = 4; nacts++;
        }
    }
    fclose(f);
}

static void dump_screens(const char *label)
{
    static u32 buf[1024 * 1024];
    int s;
    for (s = 0; s < 2; s++) {
        char path[1024];
        int w = s ? 256 : shim_view_w, h = s ? 192 : shim_view_h;
        snprintf(path, sizeof(path), "%s/f%06u_%s_s%d.ppm", outdir, frame_no, label, s);
        shim_ComposeScreen(s, buf);
        FILE *f = fopen(path, "wb");
        if (!f) continue;
        fprintf(f, "P6\n%d %d\n255\n", w, h);
        int i;
        for (i = 0; i < w * h; i++) {
            u8 px[3] = { buf[i] & 0xff, (buf[i] >> 8) & 0xff, (buf[i] >> 16) & 0xff };
            fwrite(px, 1, 3, f);
        }
        fclose(f);
    }
    fprintf(stderr, "[frame %u] dumped %s (%dx%d)\n", frame_no, label,
            shim_view_w, shim_view_h);
}

void platform_vblank(void)
{
    int i;
    /* consume audio so channels advance/free */
    static s16 abuf[48000 / 60 * 2];
    shim_MixAudio(abuf, 48000 / 60);

    for (i = 0; i < nacts; i++) {
        if (acts[i].frame != frame_no) continue;
        switch (acts[i].type) {
        case 1: touch_x = acts[i].x; touch_y = acts[i].y;
                touch_until = frame_no + acts[i].hold; break;
        case 2: pad_bit = 1u << acts[i].x;
                pad_until = frame_no + acts[i].hold; break;
        case 3: dump_screens(acts[i].label); break;
        case 4: fprintf(stderr, "[frame %u] script exit\n", frame_no); exit(0);
        case 5: shim_SetWantedViewSize(acts[i].x, acts[i].y);
                fprintf(stderr, "[frame %u] view -> %dx%d (now %dx%d)\n",
                        frame_no, acts[i].x, acts[i].y,
                        shim_view_w, shim_view_h);
                break;
        case 6: {   /* build-menu sprite visibility probe (BUILD_MENU_ID=124) */
                int bx = PA_GetSpriteX(0, 124), by = PA_GetSpriteY(0, 124);
                int cx = PA_GetSpriteX(0, 125), cy = PA_GetSpriteY(0, 125);
                int okb = bx > -64 && bx < shim_view_w && by > -32 && by < shim_view_h;
                int okc = cx > -64 && cx < shim_view_w && cy > -32 && cy < shim_view_h;
                fprintf(stderr,
                    "[frame %u] PROBE %s view=%dx%d menu=(%d,%d)%s cancel=(%d,%d)%s\n",
                    frame_no, acts[i].label, shim_view_w, shim_view_h,
                    bx, by, okb ? "" : " <-- OFFSCREEN",
                    cx, cy, okc ? "" : " <-- OFFSCREEN");
                break; }
        }
    }

    shim_SetTouchState((int)frame_no < touch_until, touch_x, touch_y);
    shim_SetPadState((int)frame_no < pad_until ? pad_bit : 0);

    frame_no++;
    if (frame_no > 20000) { fprintf(stderr, "frame limit reached\n"); exit(0); }
}

int main(int argc, char **argv)
{
    const char *root = getenv("WTD_ROOT");
    if (root) shim_SetFsRoot(root);
    const char *od = getenv("WTD_OUT");
    if (od) snprintf(outdir, sizeof(outdir), "%s", od);
    const char *vs = getenv("WTD_VIEW");     /* e.g. WTD_VIEW=256x348 */
    if (vs) {
        int w, h;
        if (sscanf(vs, "%dx%d", &w, &h) == 2)
            shim_SetWantedViewSize(w, h);
    }
    (void)argc; (void)argv;
    load_script();
    return wtd_game_main(0, NULL);
}
