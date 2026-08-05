/*
 * shim_android.c — Android NativeActivity backend for the WTD port.
 *
 * Display: the top DS screen keeps its native 256x192 format (scaled to
 * the device width); the bottom screen is an extended viewport that
 * fills 100% of the remaining display at native DS pixel scale.
 *
 * Touch (100% touchscreen, no on-screen buttons):
 *  - top screen: direct touch (stylus + virtual L, see on_input)
 *  - bottom screen, in game: 1-finger drag pans the map, a short tap is
 *    the stylus, pinch with 2 fingers zooms the viewport
 *  - bottom screen, in menus/dialogs: classic direct stylus
 */
#include <PA9.h>
#undef fopen
#undef main

#include <android/log.h>
#include <android/native_activity.h>
#include <android/asset_manager.h>
#include <android_native_app_glue.h>
#include <android/window.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <aaudio/AAudio.h>
#include <math.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  "WTD", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "WTD", __VA_ARGS__)

void shim_MixAudio(s16 *out, u32 nframes);
void shim_SetAudioOutRate(u32 rate);
int  wtd_game_main(int argc, char **argv);

/* engine camera (game global) — panned directly by the drag gesture */
extern s32 window_x, window_y;

static struct android_app *g_app;
static EGLDisplay egl_dpy = EGL_NO_DISPLAY;
static EGLSurface egl_surf = EGL_NO_SURFACE;
static EGLContext egl_ctx = EGL_NO_CONTEXT;
static int surf_w, surf_h;
static int has_focus;

static GLuint prog, tex_top, tex_bot, vbo;
static GLint  a_pos, a_uv, u_tex;

#define VIEW_TEX 1024
static u32 fb_top[256 * 192];
static u32 fb_bot[VIEW_TEX * VIEW_TEX];

/* ------------------------------------------------------------------ */
/* Layout                                                               */
/* ------------------------------------------------------------------ */

static float scr_top_h;     /* top screen height in device px           */
static float bot_area_y;    /* bottom area origin                       */
static float bot_area_h;    /* bottom area height                       */

/* current display rect of the bottom viewport (updated per frame)      */
static float bot_rx, bot_ry, bot_scale = 4.0f;

static float zoom = 1.0f;   /* 1.0 = native DS pixel = width/256        */

static void compute_layout(void)
{
    scr_top_h = (float)surf_w * 192.0f / 256.0f;
    bot_area_y = scr_top_h;
    bot_area_h = (float)surf_h - scr_top_h;
    if (bot_area_h < 64) bot_area_h = 64;
}

/* ------------------------------------------------------------------ */
/* GL                                                                   */
/* ------------------------------------------------------------------ */

static const char *vs_src =
    "attribute vec2 a_pos; attribute vec2 a_uv; varying vec2 v_uv;"
    "void main(){ gl_Position = vec4(a_pos, 0.0, 1.0); v_uv = a_uv; }";
static const char *fs_src =
    "precision mediump float; varying vec2 v_uv; uniform sampler2D u_tex;"
    "void main(){ gl_FragColor = texture2D(u_tex, v_uv); }";

static GLuint mkshader(GLenum type, const char *src)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);
    return s;
}

static void gl_init(void)
{
    prog = glCreateProgram();
    glAttachShader(prog, mkshader(GL_VERTEX_SHADER, vs_src));
    glAttachShader(prog, mkshader(GL_FRAGMENT_SHADER, fs_src));
    glLinkProgram(prog);
    a_pos = glGetAttribLocation(prog, "a_pos");
    a_uv  = glGetAttribLocation(prog, "a_uv");
    u_tex = glGetUniformLocation(prog, "u_tex");

    glGenTextures(1, &tex_top);
    glBindTexture(GL_TEXTURE_2D, tex_top);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 192, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &tex_bot);
    glBindTexture(GL_TEXTURE_2D, tex_bot);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, VIEW_TEX, VIEW_TEX, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenBuffers(1, &vbo);
}

static void draw_quad(float x, float y, float w, float h, GLuint tex,
                      float u1, float v1)
{
    float x0 = x / surf_w * 2 - 1, x1 = (x + w) / surf_w * 2 - 1;
    float y0 = 1 - y / surf_h * 2, y1 = 1 - (y + h) / surf_h * 2;
    float v[16] = {
        x0, y0, 0,  0,   x1, y0, u1, 0,
        x0, y1, 0,  v1,  x1, y1, u1, v1
    };
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STREAM_DRAW);
    glVertexAttribPointer(a_pos, 2, GL_FLOAT, GL_FALSE, 16, (void *)0);
    glVertexAttribPointer(a_uv,  2, GL_FLOAT, GL_FALSE, 16, (void *)8);
    glEnableVertexAttribArray(a_pos);
    glEnableVertexAttribArray(a_uv);
    glBindTexture(GL_TEXTURE_2D, tex);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static void egl_open(void)
{
    egl_dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(egl_dpy, NULL, NULL);
    const EGLint cfg_attr[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8,
        EGL_NONE
    };
    EGLConfig cfg; EGLint n;
    eglChooseConfig(egl_dpy, cfg_attr, &cfg, 1, &n);
    egl_surf = eglCreateWindowSurface(egl_dpy, cfg, g_app->window, NULL);
    const EGLint ctx_attr[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    egl_ctx = eglCreateContext(egl_dpy, cfg, EGL_NO_CONTEXT, ctx_attr);
    eglMakeCurrent(egl_dpy, egl_surf, egl_surf, egl_ctx);
    eglQuerySurface(egl_dpy, egl_surf, EGL_WIDTH, &surf_w);
    eglQuerySurface(egl_dpy, egl_surf, EGL_HEIGHT, &surf_h);
    eglSwapInterval(egl_dpy, 1);
    compute_layout();
    gl_init();
    LOGI("EGL surface %dx%d", surf_w, surf_h);
}

static void egl_close(void)
{
    if (egl_dpy != EGL_NO_DISPLAY) {
        eglMakeCurrent(egl_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (egl_ctx != EGL_NO_CONTEXT) eglDestroyContext(egl_dpy, egl_ctx);
        if (egl_surf != EGL_NO_SURFACE) eglDestroySurface(egl_dpy, egl_surf);
        eglTerminate(egl_dpy);
    }
    egl_dpy = EGL_NO_DISPLAY; egl_surf = EGL_NO_SURFACE; egl_ctx = EGL_NO_CONTEXT;
}

/* ------------------------------------------------------------------ */
/* Input: gestures                                                      */
/* ------------------------------------------------------------------ */

static u32 key_mask;            /* physical gamepad/keyboard            */
static int virtual_l;           /* held while touching the top screen   */

/* bottom-screen gesture state machine */
enum { G_IDLE, G_PENDING, G_PAN, G_PINCH, G_HOLD };
static int   gmode = G_IDLE;
static float gx, gy;            /* last single-finger position (px)     */
static float gsx, gsy;          /* press origin (px)                    */
static int   gframes;           /* frames since press                   */
static float pinch_dist;
static float pinch_cx, pinch_cy;
static float pan_acc_x, pan_acc_y;   /* sub-pixel pan remainders        */
static int   tap_frames;        /* synthetic tap countdown              */
static int   tap_x, tap_y;

#define TAP_MOVE_PX   (surf_w * 0.03f)
#define HOLD_FRAMES   18

static void bottom_to_view(float px, float py, int *vx, int *vy)
{
    *vx = (int)((px - bot_rx) / bot_scale);
    *vy = (int)((py - bot_ry) / bot_scale);
}

static void pan_by(float dx_px, float dy_px)
{
    int lw, lh;
    shim_GetMapLimits(&lw, &lh);
    pan_acc_x += dx_px / bot_scale;
    pan_acc_y += dy_px / bot_scale;
    int dx = (int)pan_acc_x, dy = (int)pan_acc_y;
    pan_acc_x -= dx; pan_acc_y -= dy;
    window_x -= dx;
    window_y -= dy;
    int maxx = lw - shim_view_w, maxy = lh - shim_view_h;
    if (window_x > maxx) window_x = maxx;
    if (window_y > maxy) window_y = maxy;
    if (window_x < 0) window_x = 0;
    if (window_y < 0) window_y = 0;
}

static void zoom_by(float ratio, float cx_px, float cy_px)
{
    float base = (float)surf_w / 256.0f;
    int lw, lh;
    shim_GetMapLimits(&lw, &lh);

    /* zoom-in cap: never let the requested viewport shrink below the
       compositor floor (144x108), which would letterbox the display   */
    float zmax = 256.0f / 150.0f;
    float t = bot_area_h / (base * 114.0f);
    if (t < zmax) zmax = t;
    if (zmax < 1.0f) zmax = 1.0f;

    /* zoom-out floor: stop when the whole map is visible               */
    float zmin = 0.30f;
    if (lw > 0 && 256.0f / lw > zmin) zmin = 256.0f / lw;
    if (lh > 0 && bot_area_h / (base * lh) > zmin)
        zmin = bot_area_h / (base * lh);
    if (zmin > zmax) zmin = zmax;

    float old = zoom;
    zoom *= ratio;
    if (zoom < zmin) zoom = zmin;
    if (zoom > zmax) zoom = zmax;
    if (zoom != old) {
        /* keep the map point under the pinch centre stationary */
        int vx, vy;
        bottom_to_view(cx_px, cy_px, &vx, &vy);
        float k = old / zoom;   /* view grows by k when zooming out */
        window_x += (int)(vx - vx * k);
        window_y += (int)(vy - vy * k);
    }
}

static int32_t on_input(struct android_app *app, AInputEvent *ev)
{
    (void)app;
    if (AInputEvent_getType(ev) == AINPUT_EVENT_TYPE_MOTION) {
        int action = AMotionEvent_getAction(ev) & AMOTION_EVENT_ACTION_MASK;
        size_t count = AMotionEvent_getPointerCount(ev);
        size_t skip = (size_t)-1;
        if (action == AMOTION_EVENT_ACTION_UP ||
            action == AMOTION_EVENT_ACTION_CANCEL)
            count = 0;
        if (action == AMOTION_EVENT_ACTION_POINTER_UP)
            skip = (size_t)((AMotionEvent_getAction(ev) &
                    AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >>
                    AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT);

        /* collect pointers per zone */
        float bx[8], by[8];
        int   nbot = 0;
        float topx = 0, topy = 0;
        int   ntop = 0;
        size_t i;
        for (i = 0; i < count && nbot < 8; i++) {
            if (i == skip) continue;
            float x = AMotionEvent_getX(ev, i);
            float y = AMotionEvent_getY(ev, i);
            if (y >= bot_area_y) {
                bx[nbot] = x; by[nbot] = y; nbot++;
            } else if (ntop == 0) {
                topx = x; topy = y; ntop = 1;
            }
        }

        int interactive = shim_MapViewInteractive();
        int stylus_down = 0, sx = 0, sy = 0;
        virtual_l = 0;

        if (nbot >= 2 && interactive) {
            /* ---- pinch: zoom (+ two-finger pan via centroid) ---- */
            float d = hypotf(bx[0] - bx[1], by[0] - by[1]);
            float cx = (bx[0] + bx[1]) * 0.5f, cy = (by[0] + by[1]) * 0.5f;
            if (gmode == G_PINCH) {
                if (pinch_dist > 8.0f && d > 8.0f)
                    zoom_by(d / pinch_dist, cx, cy);
                pan_by(cx - pinch_cx, cy - pinch_cy);
            }
            gmode = G_PINCH;
            pinch_dist = d;
            pinch_cx = cx; pinch_cy = cy;
        } else if (nbot == 1) {
            float x = bx[0], y = by[0];
            if (!interactive) {
                /* menus & dialogs: classic direct stylus */
                gmode = G_HOLD;
                bottom_to_view(x, y, &sx, &sy);
                stylus_down = 1;
            } else switch (gmode) {
            case G_IDLE:
                gmode = G_PENDING;
                gsx = gx = x; gsy = gy = y;
                gframes = 0;
                pan_acc_x = pan_acc_y = 0;
                break;
            case G_PENDING:
                if (fabsf(x - gsx) > TAP_MOVE_PX ||
                    fabsf(y - gsy) > TAP_MOVE_PX) {
                    gmode = G_PAN;
                    pan_by(x - gx, y - gy);
                }
                gx = x; gy = y;
                break;
            case G_PAN:
                pan_by(x - gx, y - gy);
                gx = x; gy = y;
                break;
            case G_PINCH:
                /* one finger lifted: continue as pan */
                gmode = G_PAN;
                gx = x; gy = y;
                break;
            case G_HOLD:
                bottom_to_view(x, y, &sx, &sy);
                stylus_down = 1;
                gx = x; gy = y;
                break;
            }
        } else if (nbot == 0) {
            if (gmode == G_PENDING) {
                /* released without moving: synthetic short tap */
                bottom_to_view(gsx, gsy, &tap_x, &tap_y);
                tap_frames = 3;
            }
            gmode = G_IDLE;
        }

        /* top screen: direct stylus + virtual L (interface mode) */
        if (!stylus_down && !tap_frames && ntop && gmode != G_PAN &&
            gmode != G_PINCH) {
            stylus_down = 1;
            virtual_l = 1;
            sx = (int)(topx * 256.0f / surf_w);
            sy = (int)(topy * 192.0f / scr_top_h);
            if (sx > 255) sx = 255;
            if (sy > 191) sy = 191;
        }

        if (!tap_frames)
            shim_SetTouchState(stylus_down, sx, sy);
        shim_SetPadState(key_mask | (virtual_l ? (1u << 4) : 0));
        return 1;
    }
    if (AInputEvent_getType(ev) == AINPUT_EVENT_TYPE_KEY) {
        int32_t kc = AKeyEvent_getKeyCode(ev);
        int down = AKeyEvent_getAction(ev) == AKEY_EVENT_ACTION_DOWN;
        int bit = -1;
        switch (kc) {
        case AKEYCODE_DPAD_UP:    bit = 8;  break;
        case AKEYCODE_DPAD_DOWN:  bit = 9;  break;
        case AKEYCODE_DPAD_LEFT:  bit = 10; break;
        case AKEYCODE_DPAD_RIGHT: bit = 11; break;
        case AKEYCODE_BUTTON_A:   bit = 0;  break;
        case AKEYCODE_BUTTON_B:   bit = 1;  break;
        case AKEYCODE_BUTTON_X:   bit = 2;  break;
        case AKEYCODE_BUTTON_Y:   bit = 3;  break;
        case AKEYCODE_BUTTON_L1:  bit = 4;  break;
        case AKEYCODE_BUTTON_R1:  bit = 5;  break;
        case AKEYCODE_BUTTON_START:  bit = 6; break;
        case AKEYCODE_BUTTON_SELECT: bit = 7; break;
        default: return 0;
        }
        if (down) key_mask |= 1u << bit; else key_mask &= ~(1u << bit);
        shim_SetPadState(key_mask | (virtual_l ? (1u << 4) : 0));
        return 1;
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* App lifecycle                                                        */
/* ------------------------------------------------------------------ */

static void on_cmd(struct android_app *app, int32_t cmd)
{
    (void)app;
    switch (cmd) {
    case APP_CMD_INIT_WINDOW:
        if (g_app->window) egl_open();
        break;
    case APP_CMD_TERM_WINDOW:
        egl_close();
        break;
    case APP_CMD_GAINED_FOCUS: has_focus = 1; break;
    case APP_CMD_LOST_FOCUS:   has_focus = 0; break;
    case APP_CMD_DESTROY:
        exit(0);
    }
}

/* ------------------------------------------------------------------ */
/* Audio                                                                */
/* ------------------------------------------------------------------ */

static aaudio_data_callback_result_t audio_cb(AAudioStream *stream, void *ud,
                                              void *audio_data, int32_t nframes)
{
    (void)stream; (void)ud;
    shim_MixAudio((s16 *)audio_data, (u32)nframes);
    return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

static void audio_init(void)
{
    AAudioStreamBuilder *b;
    if (AAudio_createStreamBuilder(&b) != AAUDIO_OK) return;
    AAudioStreamBuilder_setFormat(b, AAUDIO_FORMAT_PCM_I16);
    AAudioStreamBuilder_setChannelCount(b, 2);
    AAudioStreamBuilder_setDataCallback(b, audio_cb, NULL);
    AAudioStream *stream;
    if (AAudioStreamBuilder_openStream(b, &stream) == AAUDIO_OK) {
        shim_SetAudioOutRate((u32)AAudioStream_getSampleRate(stream));
        AAudioStream_requestStart(stream);
        LOGI("AAudio started at %d Hz", AAudioStream_getSampleRate(stream));
    }
    AAudioStreamBuilder_delete(b);
}

/* ------------------------------------------------------------------ */
/* Asset extraction                                                     */
/* ------------------------------------------------------------------ */

/* mode 0: never overwrite an existing file (user data: settings, saves)
   mode 1: refresh only when the size differs (maps: follow APK upgrades
           without paying the copy on every single launch)                */
static void copy_asset(AAssetManager *am, const char *src, const char *dst,
                       int overwrite)
{
    struct stat st;
    int exists = (stat(dst, &st) == 0);
    if (exists && !overwrite) return;

    AAsset *a = AAssetManager_open(am, src, AASSET_MODE_STREAMING);
    if (!a) { LOGE("asset missing: %s", src); return; }

    if (exists && st.st_size == AAsset_getLength(a)) {
        AAsset_close(a);
        return;
    }

    FILE *f = fopen(dst, "wb");
    if (f) {
        char buf[16384];
        int n;
        while ((n = AAsset_read(a, buf, sizeof(buf))) > 0)
            fwrite(buf, 1, (size_t)n, f);
        fclose(f);
    }
    AAsset_close(a);
}

/* Custom maps: the game lists a single /maps directory, so user maps dropped
   into the app's external files dir (visible over USB / in any file manager,
   no permission and no root needed) are mirrored into it on every launch.
   They are prefixed so they stay distinguishable from the bundled ones and
   can be dropped again when the user deletes them outside.                 */
#define USER_MAP_PREFIX "user_"

static int has_tdm_ext(const char *name)
{
    size_t n = strlen(name);
    return n > 4 && strcasecmp(name + n - 4, ".tdm") == 0;
}

static void copy_file(const char *src, const char *dst)
{
    struct stat ss, ds;
    if (stat(src, &ss) != 0) return;
    /* skip when an identical copy is already there */
    if (stat(dst, &ds) == 0 && ds.st_size == ss.st_size &&
        ds.st_mtime >= ss.st_mtime)
        return;

    FILE *in = fopen(src, "rb");
    if (!in) return;
    FILE *out = fopen(dst, "wb");
    if (out) {
        char buf[16384];
        size_t n;
        while ((n = fread(buf, 1, sizeof(buf), in)) > 0)
            fwrite(buf, 1, n, out);
        fclose(out);
    }
    fclose(in);
}

static void sync_user_maps(const char *root)
{
    const char *ext = g_app->activity->externalDataPath;
    char udir[1024], ipath[1024];

    if (!ext || !*ext) return;                 /* external storage unavailable */

    /* create <external>/maps so the user has an obvious place to drop files */
    snprintf(udir, sizeof(udir), "%s", ext);
    mkdir(udir, 0755);
    snprintf(udir, sizeof(udir), "%s/maps", ext);
    if (mkdir(udir, 0755) != 0 && errno != EEXIST) return;

    /* drop previously imported maps that are gone from the external dir */
    snprintf(ipath, sizeof(ipath), "%s/maps", root);
    DIR *id = opendir(ipath);
    if (id) {
        struct dirent *e;
        while ((e = readdir(id))) {
            if (strncmp(e->d_name, USER_MAP_PREFIX, strlen(USER_MAP_PREFIX)) != 0)
                continue;
            char probe[1024], victim[1024];
            snprintf(probe, sizeof(probe), "%s/%s", udir,
                     e->d_name + strlen(USER_MAP_PREFIX));
            struct stat st;
            if (stat(probe, &st) != 0) {
                snprintf(victim, sizeof(victim), "%s/%s", ipath, e->d_name);
                remove(victim);
                LOGI("removed user map %s", e->d_name);
            }
        }
        closedir(id);
    }

    /* import everything that looks like a map */
    DIR *ud = opendir(udir);
    if (!ud) return;
    struct dirent *e;
    int n = 0;
    while ((e = readdir(ud))) {
        if (!has_tdm_ext(e->d_name)) continue;
        char s[1024], d[1024];
        snprintf(s, sizeof(s), "%s/%s", udir, e->d_name);
        snprintf(d, sizeof(d), "%s/" USER_MAP_PREFIX "%s", ipath, e->d_name);
        copy_file(s, d);
        n++;
    }
    closedir(ud);
    if (n) LOGI("%d user map(s) in %s", n, udir);
}

static void extract_assets(void)
{
    AAssetManager *am = g_app->activity->assetManager;
    const char *root = g_app->activity->internalDataPath;
    char path[1024];

    shim_SetFsRoot(root);
    snprintf(path, sizeof(path), "%s/maps", root);
    mkdir(path, 0755);

    static const char *efs[] = { "settings", "highscores", "last_game" };
    unsigned i;
    for (i = 0; i < 3; i++) {
        char s[64], d[1024];
        snprintf(s, sizeof(s), "efsroot/%s", efs[i]);
        snprintf(d, sizeof(d), "%s/%s", root, efs[i]);
        copy_asset(am, s, d, 0);
    }

    AAssetDir *dir = AAssetManager_openDir(am, "maps");
    if (dir) {
        const char *name;
        while ((name = AAssetDir_getNextFileName(dir))) {
            char s[512], d[1024];
            snprintf(s, sizeof(s), "maps/%s", name);
            snprintf(d, sizeof(d), "%s/maps/%s", root, name);
            copy_asset(am, s, d, 1);
        }
        AAssetDir_close(dir);
    }

    /* after the bundled maps, so user maps are never clobbered by them */
    sync_user_maps(root);
}

/* ------------------------------------------------------------------ */
/* Frame pump (called from PA_WaitForVBL)                               */
/* ------------------------------------------------------------------ */

static u64 next_frame_ns;

static u64 now_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (u64)ts.tv_sec * 1000000000ull + (u64)ts.tv_nsec;
}

void platform_vblank(void)
{
    int events;
    struct android_poll_source *source;
    while (ALooper_pollAll(0, NULL, &events, (void **)&source) >= 0) {
        if (source) source->process(g_app, source);
        if (g_app->destroyRequested) exit(0);
    }

    while (egl_surf == EGL_NO_SURFACE || !has_focus) {
        if (ALooper_pollAll(100, NULL, &events, (void **)&source) >= 0) {
            if (source) source->process(g_app, source);
            if (g_app->destroyRequested) exit(0);
        } else if (egl_surf == EGL_NO_SURFACE) {
            continue;
        }
        if (egl_surf != EGL_NO_SURFACE && has_focus) break;
    }

    /* synthetic tap injection (short stylus press) */
    if (tap_frames > 0) {
        tap_frames--;
        shim_SetTouchState(tap_frames > 0, tap_x, tap_y);
    }

    /* game speed 2x/4x: only render (and pace) every Nth engine tick,
       so the unmodified game logic simply runs N times faster.  Modal
       dialogs and menus drop MapViewInteractive and run at 1x.        */
    {
        static u32 tick;
        int speed = shim_MapViewInteractive() ? shim_game_speed : 1;
        if (speed > 1 && (++tick % (u32)speed) != 0)
            return;
    }

    /* requested viewport from the device size and current zoom */
    float base = (float)surf_w / 256.0f;      /* device px per DS px    */
    float ppd  = base * zoom;
    shim_SetWantedViewSize((int)lroundf(surf_w / ppd),
                           (int)lroundf(bot_area_h / ppd));

    /* compose the two DS engines (top always native 256x192) */
    shim_ComposeScreen(1, fb_top);
    shim_ComposeScreen(0, fb_bot);
    int vw = shim_view_w, vh = shim_view_h;

    /* bottom display rect: fit & centre (fills exactly when unclamped) */
    float fit = (float)surf_w / vw;
    if (bot_area_h / vh < fit) fit = bot_area_h / vh;
    bot_scale = fit;
    bot_rx = ((float)surf_w - vw * fit) / 2.0f;
    bot_ry = bot_area_y + (bot_area_h - vh * fit) / 2.0f;

    glViewport(0, 0, surf_w, surf_h);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(prog);
    glUniform1i(u_tex, 0);
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_BLEND);

    glBindTexture(GL_TEXTURE_2D, tex_top);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 192, GL_RGBA,
                    GL_UNSIGNED_BYTE, fb_top);
    draw_quad(0, 0, (float)surf_w, scr_top_h, tex_top, 1.0f, 1.0f);

    glBindTexture(GL_TEXTURE_2D, tex_bot);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, vw, vh, GL_RGBA,
                    GL_UNSIGNED_BYTE, fb_bot);
    draw_quad(bot_rx, bot_ry, vw * fit, vh * fit, tex_bot,
              (float)vw / VIEW_TEX, (float)vh / VIEW_TEX);

    eglSwapBuffers(egl_dpy, egl_surf);

    /* pace to 60 Hz regardless of display refresh */
    u64 t = now_ns();
    const u64 step = 1000000000ull / 60ull;
    if (!next_frame_ns || t > next_frame_ns + 4 * step)
        next_frame_ns = t;
    next_frame_ns += step;
    if (next_frame_ns > t) {
        struct timespec ts = {
            (time_t)((next_frame_ns - t) / 1000000000ull),
            (long)((next_frame_ns - t) % 1000000000ull)
        };
        nanosleep(&ts, NULL);
    }
}

/* ------------------------------------------------------------------ */

void android_main(struct android_app *app)
{
    g_app = app;
    app->onAppCmd = on_cmd;
    app->onInputEvent = on_input;

    ANativeActivity_setWindowFlags(app->activity,
        AWINDOW_FLAG_KEEP_SCREEN_ON | AWINDOW_FLAG_FULLSCREEN, 0);

    srandom((unsigned)time(NULL));
    extract_assets();
    audio_init();

    while (egl_surf == EGL_NO_SURFACE) {
        int events;
        struct android_poll_source *source;
        if (ALooper_pollAll(100, NULL, &events, (void **)&source) >= 0) {
            if (source) source->process(app, source);
            if (app->destroyRequested) return;
        }
    }
    has_focus = 1;

    LOGI("starting game main");
    wtd_game_main(0, NULL);
}
