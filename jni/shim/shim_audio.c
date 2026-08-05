/*
 * shim_audio.c — ASlib/PAlib sound emulation: 16 channels of IMA-ADPCM
 * (22050 Hz) with volume + stereo panning.  Decoded clips are cached.
 * The platform backend pulls mixed stereo s16 frames via shim_MixAudio().
 */
#include <PA9.h>
#undef fopen
#undef main

#define NCH        16
#define SRC_RATE   22050

typedef struct {
    const u8 *src;
    s16      *pcm;
    u32       frames;
} clip_t;

static clip_t clips[256];
static int    nclips;

typedef struct {
    volatile int active;
    const s16 *pcm;
    u32  frames;
    u64  pos;          /* 32.16 fixed point in source frames (u32 would
                          wrap after ~3 s at 22 kHz and loop the clip)  */
    u32  step;         /* set from output rate                          */
    u8   vol;          /* 0..127                              */
    u8   pan;          /* 0..127, 64 = centre                 */
    u8   loop;
} chan_t;

static chan_t ch[NCH];
static u8  master_vol = 127;
static u32 out_rate = 48000;

/* ---------------- IMA ADPCM decoder ---------------- */

static const int ima_index_table[16] = {
    -1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8
};
static const int ima_step_table[89] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34, 37,
    41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130, 143, 157, 173,
    190, 209, 230, 253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658,
    724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358, 5894,
    6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899, 15289,
    16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

/* DS hardware IMA-ADPCM: 4-byte header = initial predictor (s16 LE) +
   initial step index (u8) + padding; then nibbles, low first. */
static void ima_decode(const u8 *in, u32 inbytes, s16 *out)
{
    int predictor = 0, index = 0;
    if (inbytes >= 4) {
        predictor = (s16)(in[0] | (in[1] << 8));
        index = in[2];
        if (index > 88) index = 88;
        in += 4;
        inbytes -= 4;
    }
    u32 i;
    for (i = 0; i < inbytes * 2; i++) {
        int nibble = (i & 1) ? (in[i >> 1] >> 4) : (in[i >> 1] & 0x0f);
        int step = ima_step_table[index];
        int diff = step >> 3;
        if (nibble & 1) diff += step >> 2;
        if (nibble & 2) diff += step >> 1;
        if (nibble & 4) diff += step;
        if (nibble & 8) predictor -= diff; else predictor += diff;
        if (predictor > 32767) predictor = 32767;
        if (predictor < -32768) predictor = -32768;
        index += ima_index_table[nibble];
        if (index < 0) index = 0;
        if (index > 88) index = 88;
        out[i] = (s16)predictor;
    }
}

static clip_t *get_clip(const u8 *data, u32 size)
{
    int i;
    for (i = 0; i < nclips; i++)
        if (clips[i].src == data) return &clips[i];
    if (nclips >= (int)(sizeof(clips) / sizeof(clips[0]))) return NULL;
    clip_t *c = &clips[nclips++];
    c->src = data;
    c->frames = (size > 4 ? size - 4 : size) * 2;
    c->pcm = (s16 *)malloc(c->frames * sizeof(s16));
    ima_decode(data, size, c->pcm);
    return c;
}

/* ---------------- channel control ---------------- */

void shim_SetAudioOutRate(u32 rate) { if (rate) out_rate = rate; }

static void start_channel(int n, const u8 *data, u32 size, u8 vol, u8 pan, u8 loop)
{
    clip_t *c = get_clip(data, size);
    if (!c) return;
    ch[n].active = 0;
    ch[n].pcm = c->pcm;
    ch[n].frames = c->frames;
    ch[n].pos = 0;
    ch[n].step = (u32)(((u64)SRC_RATE << 16) / out_rate);
    ch[n].vol = vol > 127 ? 127 : vol;
    ch[n].pan = pan > 127 ? 127 : pan;
    ch[n].loop = loop;
    ch[n].active = 1;
}

static int find_free_channel(void)
{
    int i;
    for (i = 1; i < NCH; i++)
        if (!ch[i].active) return i;
    return -1;
}

void PA_InitSound(void) {}
void AS_SetDefaultSettings(u8 f, u16 r, u8 d) { (void)f; (void)r; (void)d; }
void AS_SetMasterVolume(u8 v) { master_vol = v > 127 ? 127 : v; }

int AS_SoundDefaultPlay(const u8 *data, u32 size, u8 volume, u8 pan,
                        u8 loop, u8 prio)
{
    (void)prio;
    int n = find_free_channel();
    if (n < 0) return -1;
    start_channel(n, data, size, volume, pan, loop);
    return n;
}

int shim_PlaySimpleSound(const void *data, u32 size)
{
    int n = AS_SoundDefaultPlay((const u8 *)data, size, 127, 64, 0, 0);
    return n < 0 ? 0 : n;
}

void PA_PlaySoundEx(int channel, const void *data, u32 size, u8 volume,
                    int freq, int format)
{
    (void)freq; (void)format;
    if (channel < 0 || channel >= NCH) return;
    start_channel(channel, (const u8 *)data, size, volume, 64, 0);
}

void PA_StopSound(int channel)
{
    if (channel >= 0 && channel < NCH) ch[channel].active = 0;
}

int PA_SoundChannelIsBusy(int channel)
{
    return (channel >= 0 && channel < NCH) ? ch[channel].active : 0;
}

void PA_SetSoundChannelPan(int channel, u8 pan)
{
    if (channel >= 0 && channel < NCH) ch[channel].pan = pan > 127 ? 127 : pan;
}

void PA_SetSoundVol(u8 vol) { master_vol = vol > 127 ? 127 : vol; }

int PA_GetFreeSoundChannel(void) { return find_free_channel(); }

/* ---------------- mixer (called from the audio thread) ---------------- */

void shim_MixAudio(s16 *out, u32 nframes)
{
    u32 f;
    int n;
    memset(out, 0, nframes * 2 * sizeof(s16));
    for (n = 0; n < NCH; n++) {
        chan_t *c = &ch[n];
        if (!c->active) continue;
        /* gains normalised to 0..256 to keep the per-sample maths in s32 */
        int gl = ((127 - c->pan) * c->vol * master_vol) / 8064; /* 256 = full */
        int gr = (c->pan * c->vol * master_vol) / 8064;
        if (gl > 256) gl = 256;
        if (gr > 256) gr = 256;
        for (f = 0; f < nframes; f++) {
            u32 ip = (u32)(c->pos >> 16);
            if (ip >= c->frames) {
                if (c->loop) { c->pos = 0; ip = 0; }
                else { c->active = 0; break; }
            }
            int s = c->pcm[ip];
            int l = out[f * 2]     + ((s * gl) >> 8);
            int r = out[f * 2 + 1] + ((s * gr) >> 8);
            if (l > 32767) l = 32767; if (l < -32768) l = -32768;
            if (r > 32767) r = 32767; if (r < -32768) r = -32768;
            out[f * 2]     = (s16)l;
            out[f * 2 + 1] = (s16)r;
            c->pos += c->step;
        }
    }
}
