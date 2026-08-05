/************************************/
/* Warcraft Tower Defense - by Noda */
/* Auxiliary functions     12/02/07 */
/************************************/

// Includes
#include <PA9.h>       // Include for PA_Lib
#include "types.h"     // Types definitions
#include "defines.h"   // Defines
#include "efs_lib.h"
#include "f_aux.h"

// Sfx
#include "PeonPissed2.h"
#include "PeonReady.h"

// Options
extern bool multiple_builds;    // option: allow multiple builds
extern int build_menu_pos;      // option: build menu position
extern bool interface_switch;   // option: interface mode, switch or hold
extern bool double_clic;        // option: double-clic build mode

// Sound variables
//static u8 soundchannel;     // current sound channel
/*static u8 need_queue;       // need to queue sound
static u8 death_snds;       // monster deaths sounds counter
static u8 attack_snds;      // attacks sounds counter
static u8 voice_snds;       // voices deaths sounds counter
static u8 queue_size;       // size of the sound queue
static sfx snd_queue[SND_QUEUE_SIZE];   // the sound queue
*/
// Simple fade in from white
void fadeFromWhite() {
    s16 i;
    for (i = 31; i > 0; i--) {
        PA_SetBrightness(0, i);
        PA_SetBrightness(1, i);
        PA_WaitForVBL();
        PA_WaitForVBL();
    }
}

// Fade in top screen
void fadeInTop() {
    s16 i;
    for (i = -31; i <= 0; i++) {
        PA_SetBrightness(1, i);
        PA_WaitForVBL();
        PA_WaitForVBL();
    }
}

// Fade in bottom screen
void fadeInBottom() {
    s16 i;
    for (i = -31; i <= 0; i++) {
        PA_SetBrightness(0, i);
        PA_WaitForVBL();
        PA_WaitForVBL();
    }
}

// Simple fade in
void fadeIn() {
    s16 i;
    for(i = -31; i <= 0; i++) {
        PA_SetBrightness(0, i);
        PA_SetBrightness(1, i);
        PA_WaitForVBL();
        PA_WaitForVBL();
    }
    initSoundSystem();
}

// Simple fade out
void fadeOut() {
    s16 i;
    for(i = 0; i > -31; i--) {
        PA_SetBrightness(0, i);
        PA_SetBrightness(1, i);
//        PA_SetSoundVol((31+i) * 4);     // fade out global sound volume
        PA_WaitForVBL();
        PA_WaitForVBL();
    }
    // stop sounds
    for(i = 2; i < 16; i++)
        PA_StopSound(i);
}

// Wait some time, in milliseconds (time must be > 16 to be effective)
void wait(int time) {
    int i, nb;
    nb = time_to_vbl(time);
    for(i = 0; i < nb; i++){
        PA_WaitForVBL();
    }
}

// translate a number in millisecond to a number of vbls
int time_to_vbl(int num) { return ((num*1000)/16666); }

// Returns an int rounded to the nearest 4 multiple
int round4(int i) { return i - (i & 3); }

// Returns an int rounded to the nearest 8 multiple
int round8(int i) { return i - (i & 7); }

// Returns an int rounded to the nearest 16 multiple
int round16(int i) { return i - (i & 15); }

// Returns an int rounded to the nearest 32 multiple
int round32(int i) { return i - (i & 31); }

// load options
void loadOptions() {
    EFS_FILE* file = EFS_fopen(FILE_SETTINGS);
    EFS_fread(&multiple_builds, sizeof(bool), 1, file);
    EFS_fread(&build_menu_pos, sizeof(int), 1, file);
    EFS_fread(&interface_switch, sizeof(bool), 1, file);
    EFS_fread(&double_clic, sizeof(bool), 1, file);
    EFS_fclose(file);
}

// save options
void saveOptions() {
    EFS_FILE* file = EFS_fopen(FILE_SETTINGS);
    EFS_fwrite(&multiple_builds, sizeof(bool), 1, file);
    EFS_fwrite(&build_menu_pos, sizeof(int), 1, file);
    EFS_fwrite(&interface_switch, sizeof(bool), 1, file);
    EFS_fwrite(&double_clic, sizeof(bool), 1, file);
    EFS_fclose(file);
}

// check if a saved game exists
bool checkSavedGame() {
    bool save = false;
    EFS_FILE* file = EFS_fopen(FILE_GAMESAVE);
    EFS_fread(&save, sizeof(bool), 1, file);
    EFS_fclose(file);
    return save;
}

// calculate map size & magic sum
void calcMapSizeAndSum(char *map_filename, int *size, int *sum) {

    int i, sz, szd16, t, sm = 0;
//    char header[MAP_HEADER_SIZE];

    FILE* file = fopen(map_filename, "rb");
/*    
    // check for new map format
    fread(header, sizeof(char), MAP_HEADER_SIZE, file);

    if(strncomp(header, MAP_HEADER_V3, MAP_HEADER_SIZE) == 0) {
    
        fread(&sz, sizeof(int), 1, file);
        fread(&sm, sizeof(int), 1, file);    
    
    } else {
*/
        // get file size
        fseek(file, 0, SEEK_END);
        sz = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        // calculate magic sum
        szd16 = sz / 16;
        for(i=0; i < szd16; i+=1) {
            fread(&t, 4, 1, file);
            sm += t;
            fread(&t, 4, 1, file);
            sm += t;
            fread(&t, 4, 1, file);
            sm += t;
            fread(&t, 4, 1, file);
            sm += t;
        }    
        for(i=0; i < sz - szd16; i+=1)
            if(fread(&t, 4, 1, file));        
                sm += t;        
//    }
    fclose(file);

    *size = sz;
    *sum = sm;    
}

// similar to siprintf but WORKS
void sformat(char* buf, char* text, ...) {

    s16 i, j;
    u16 textcount = 0;
    u8 nbtext[32];
    u8 nbtextcount = 0;
    u8 *extext;
    s32 nbtextnumber;
     
    va_list varg;
    va_start(varg, text);
    nbtextcount = 0;
   
    for (j = 0; text[j]; j++) {
        if (text[j] == '%') {
            if (text[j+1] == 's') {  // if %s, it's a string
                extext = (u8*)va_arg(varg, const u8 *);  // string pointer
                for (i = 0; extext[i]; i++) {
                    buf[textcount] = extext[i];
                    textcount++;
                }
                j++;
            } else if (text[j+1] == 'd') {
                nbtextnumber = va_arg(varg, s32);

                s16 neg = 0;
                if (nbtextnumber < 0) {
                    nbtextnumber = -nbtextnumber;
                    neg = 1;
                }
                for (i = 0; nbtextnumber || (!nbtextnumber && !i); i++) {
                    nbtext[i] = '0' + (nbtextnumber%10);
                    nbtextnumber = nbtextnumber / 10;
                }
                if (neg) {
                    nbtext[i] = '-';
                    i++;
                }
                for (i--; i > -1; i--) {
                    buf[textcount] = nbtext[i];
                    textcount++;
                }
                j++;
            
            } else if ((text[j+3] == 'd') && (text[j+1] != ' ') && (text[j+2] != ' ')) {
                nbtextnumber = va_arg(varg, s32);
                s16 neg = 0;
                u8 amettre;

                if((text[j+1] == '_'))
                    amettre = ' ';
                else
                    amettre = text[j+1];
                
                s16 nfois = text[j+2] - '0';
    
                if (nbtextnumber < 0) {
                    nbtextnumber = -nbtextnumber;
                    neg = 1;
                }
                for (i = 0; nbtextnumber || (!nbtextnumber && !i); i++) {
                    nbtext[i] = 48 + (nbtextnumber%10);
                    nbtextnumber = nbtextnumber / 10;
                    nfois--;
                }
                while(nfois > 0) {
                    buf[textcount] = amettre;
                    textcount++;
                    nfois--;
                }
                if (neg) {
                    nbtext[i] = '-';
                    i++;
                }
                for (i--; i > -1; i--) {
                    buf[textcount] = nbtext[i];
                    textcount++;
                }
                j += 3;
          
            } else {
                buf[textcount] = text[j];
                textcount++;
            }
        } else {
            buf[textcount] = text[j];
            textcount++;
        }
    }

    buf[textcount] = '\0';
    va_end(varg);
}

// Wait for a key or screen press
void waitForAnyKey() {
    while(1) {
        PA_WaitForVBL();
        if(Pad.Newpress.A ||
           Pad.Newpress.B ||
           Pad.Newpress.L ||
           Pad.Newpress.R ||
           Pad.Newpress.Start ||
           Pad.Newpress.Select ||
           Pad.Newpress.Up ||
           Pad.Newpress.Down ||
           Pad.Newpress.Left ||
           Pad.Newpress.Right ||
           Stylus.Newpress) 
            break;
    }
}

// Returns a random number between a & b
//int rand(int a, int b) {
//    return PA_RandMinMax(a, b);
//}

// Init special sound system
void initSoundSystem() {

    AS_SetMasterVolume(127);

    // init the sound channel & queue
/*    PA_SetSoundVol(127);
    need_queue = 0;
    death_snds = 0;
    attack_snds = 0;
    voice_snds = 0;
    queue_size = 0;*/
}
/*
// Queue a sound
void queueSound(void* data, u32 length, u8 type, u8 volume, u8 pan) {

    // skip sound
    bool skip = true;
    
    if(queue_size < SND_QUEUE_SIZE) {
        // check for special type allocations
        if(type == SND_STANDARD) {
            skip = false;
        } else if((type == SND_DEATH) && (death_snds < SND_DEATH_MAX)) {
            skip = false;
            death_snds++;
        } else if((type == SND_ATTACK) && (attack_snds < SND_ATTACK_MAX)) {
            skip = false;
            attack_snds++;
        } else if((type == SND_VOICE) && (voice_snds < SND_VOICE_MAX)) {
            skip = false;
            voice_snds++;
        }
    }
    // queue sound
    if(!skip) {
        snd_queue[queue_size].data = data;
        snd_queue[queue_size].size = length;
        snd_queue[queue_size].type = type;
        snd_queue[queue_size].volume = volume;
        snd_queue[queue_size].pan = pan;
        queue_size++;
    }
}

// empty sound queue
void playQueue() {

    int i;
    for(i=0; i<8; i++) {    // just a quick patch to allow multiple sounds in a single vbl

    // latency to allow IPC working
//    if(need_queue > 0)
//        need_queue--;

//    else 
        if(queue_size > 0) {
    
        // play top priority sounds first
        u8 i, play_snd = 0;
        for(i=0; i<queue_size; i++)
            if((snd_queue[i].type == SND_STANDARD) || (snd_queue[i].type == SND_VOICE))
                play_snd = i;
    
        u8 type = snd_queue[play_snd].type;

        // free slots for special sound types
        if(type == SND_DEATH) {
            death_snds--;
        } else if(type == SND_ATTACK) {
            attack_snds--;
        } else if(type == SND_VOICE) {
            voice_snds--;
        }
    
        // try to play the sound
        if(soundMix(snd_queue[play_snd].data, snd_queue[play_snd].size, snd_queue[play_snd].type, snd_queue[play_snd].volume, snd_queue[play_snd].pan)) {
            // reorganize queue
            queue_size--;
            for(i=play_snd+1; i<queue_size; i++) {
                snd_queue[i-1] = snd_queue[i];
            }
        }

    }
    
    }
}*/

// Simple sound effect mixer
bool soundMix(void* data, u32 length, u8 type, u8 volume, u8 pan) {


    AS_SoundDefaultPlay((u8*)data, length, volume, pan, false, type);
    return true;

//    if(need_queue == 0) {
/*
        s8 soundchannel = PA_GetFreeSoundChannel();
        
        if(soundchannel != -1) {
            PA_PlaySoundEx(soundchannel, data, length, volume, PA_SoundOption.freq, PA_SoundOption.format);
            PA_SetSoundChannelPan(soundchannel, pan);
            // 2 vbl needed between 2 IPC for playing sound
//            need_queue = 2;
            return true;
        } else {
//            queueSound(data, length, type, volume, pan);
            return false;
        }*/
/*
        if(!PA_SoundChannelIsBusy(soundchannel)) {
            PA_PlaySoundEx(soundchannel, data, length, volume, PA_SoundOption.freq, PA_SoundOption.format);
            // 2 vbl needed between 2 IPC for playing sound
            need_queue = 2;
        } else {
            int channel = PA_GetFreeSoundChannel();
            if(channel != -1) {
            soundchannel = (soundchannel+1)%16;  // limit to 0-15
            if(!PA_SoundChannelIsBusy(soundchannel)) {
                PA_PlaySoundEx(soundchannel, data, length, volume, PA_SoundOption.freq, PA_SoundOption.format);
                // 2 vbl needed between 2 IPC for playing sound
                need_queue = 2;
            } else {
                queueSound(data, length, type, volume, pan);
            }
        }*/

//    } else {
//        queueSound(data, length, type, volume, pan);
//    }
}

// Play sound & pause if lid is closed
bool checkLid() {
    if(PA_LidClosed()) {
        PA_SetLedBlink(1, 0);
        PA_WaitForVBL();
//        PA_StopSound(15);
        u8 chan = PA_PlaySimpleSound(/*15,*/ PeonPissed2);       
        PA_WaitForVBL();
        while(IPC_Sound->chan[chan].busy)
            PA_WaitForVBL();        
//        PA_WaitFor(!PA_SoundChannelIsBusy(15));
        PA_CheckLid();
        PA_SetLedBlink(0, 0);
        PA_PlaySimpleSound(/*15,*/ PeonReady);
        return true;
    }
    return false;
}

// Simple strcpy function...
void strcopy(char* dest, char* src) {
    int i = 0;
    while(src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

// Simple strlen function...
int strlength(register char *s)
{
    register char *startpos;
    startpos = s;
    while (*s++);
    return ((int) (s-startpos-1));
}

// Simple tolower function...
char ctolower(char c) {                                                                                              
    return (c >= 'A' && c <= 'Z') ? (c+32) : (c);
}

// Simple strncmp function...
int strncomp(const char *s1, const char *s2, int n)
{
    if (n == 0)
        return 0;
    do {
        if (ctolower(*s1) != ctolower(*s2++))
            return (ctolower(*(const unsigned char *)s1) - ctolower(*(const unsigned char *)(s2 - 1)));
        if (*s1++ == 0)
            break;
    } while (--n != 0);
    return 0;
}

// Simple strstr function...
char *strstring(const char *in, const char *str)
{
    char c;
    int len;
    c = *str++;
    if (!c)
        return (char *) in;
        len = strlength((char *)str);
    do {
        char sc;
        do {
            sc = *in++;
            if (!sc)
                return (char *) 0;
        } while (sc != c);
    } while (strncomp(in, str, len) != 0);
    return (char *) (in - 1);
 }

// Simple strcat function...
char *strccat(register char *s, register const char *append)
{
    char *save = s;
    for (; *s; ++s);
    while ((*s++ = *append++) != 0);
    return(save);
}
