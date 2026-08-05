/*
 * shim_misc.c — input snapshot (Pad/Stylus), frame pacing entry point,
 * random numbers, RTC.
 */
#include <PA9.h>
#undef fopen
#undef main
#include <time.h>

padtype    Pad;
stylustype Stylus;

int shim_game_speed = 1;    /* 1x / 2x / 4x, cycled by the in-game button */

/* raw state fed by the platform backend */
static int raw_touch_down, raw_touch_x, raw_touch_y;
static u32 raw_pad_mask;

/* previous frame state */
static int prev_touch_down;
static u32 prev_pad_mask;
static int frames_since_release = 1000;
static int last_press_x, last_press_y;

void shim_SetTouchState(int down, int x, int y)
{
    /* clamp to the current bottom-screen viewport (>= 256x192; the top
       screen backend always sends coordinates within 0..255/0..191) */
    int mx = shim_view_w > 256 ? shim_view_w : 256;
    int my = shim_view_h > 192 ? shim_view_h : 192;
    raw_touch_down = down;
    if (down) {
        if (x < 0) x = 0; if (x > mx - 1) x = mx - 1;
        if (y < 0) y = 0; if (y > my - 1) y = my - 1;
        raw_touch_x = x;
        raw_touch_y = y;
    }
}

void shim_SetPadState(u32 mask) { raw_pad_mask = mask; }

/* bit order: A B X Y L R Start Select Up Down Left Right */
enum { BA, BB, BX, BY, BL, BR, BSTART, BSELECT, BUP, BDOWN, BLEFT, BRIGHT };

static void setkeys(padkeys *k, u32 m)
{
    k->A = (m >> BA) & 1;  k->B = (m >> BB) & 1;
    k->X = (m >> BX) & 1;  k->Y = (m >> BY) & 1;
    k->L = (m >> BL) & 1;  k->R = (m >> BR) & 1;
    k->Start = (m >> BSTART) & 1;  k->Select = (m >> BSELECT) & 1;
    k->Up = (m >> BUP) & 1;  k->Down = (m >> BDOWN) & 1;
    k->Left = (m >> BLEFT) & 1;  k->Right = (m >> BRIGHT) & 1;
}

static void frame_update_input(void)
{
    u32 held = raw_pad_mask;
    setkeys(&Pad.Held, held);
    setkeys(&Pad.Newpress, held & ~prev_pad_mask);
    setkeys(&Pad.Released, ~held & prev_pad_mask);
    prev_pad_mask = held;

    int down = raw_touch_down;
    Stylus.Newpress = down && !prev_touch_down;
    Stylus.Held     = down;
    Stylus.Released = !down && prev_touch_down;
    Stylus.DblClick = 0;
    if (down) {
        /* modal dialogs live in a 256x192 space centred in the viewport */
        int ox, oy;
        shim_GetDialogOffset(&ox, &oy);
        int x = raw_touch_x - ox, y = raw_touch_y - oy;
        if (x < 0) x = 0; if (x > 255 && ox) x = 255;
        if (y < 0) y = 0; if (y > 191 && oy) y = 191;
        Stylus.X = x;
        Stylus.Y = y;
    }
    if (Stylus.Newpress) {
        int dx = Stylus.X - last_press_x, dy = Stylus.Y - last_press_y;
        if (frames_since_release < 20 && dx * dx + dy * dy < 24 * 24)
            Stylus.DblClick = 1;
        last_press_x = Stylus.X;
        last_press_y = Stylus.Y;
    }
    if (!down)
        frames_since_release++;
    else if (Stylus.Newpress)
        frames_since_release = 0;
    prev_touch_down = down;
}

void PA_WaitForVBL(void)
{
    /* platform: render both screens, pace to 60 Hz, pump events */
    platform_vblank();
    frame_update_input();

    time_t t = time(NULL);
    struct tm tmv;
    localtime_r(&t, &tmv);
    PA_RTC.Hour    = (u8)tmv.tm_hour;
    PA_RTC.Minutes = (u8)tmv.tm_min;
    PA_RTC.Seconds = (u8)tmv.tm_sec;
}

int PA_RandMinMax(int min, int max)
{
    if (max <= min) return min;
    return min + (int)(random() % (unsigned)(max - min + 1));
}
