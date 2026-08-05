/************************************/
/* Warcraft Tower Defense - by Noda */
/* Auxiliary functions     12/02/07 */
/************************************/

// Includes
#include <PA9.h>       // Include for PA_Lib
#include <nds.h>

// Play a sound using the sfx mixer
//#define playSound(sound)   soundMix((void*)sound, (u32)sound##_size, 0, 127, 64)
#define playSound(sound)    AS_SoundQuickPlay(sound)

// Simple fade in from white
void fadeFromWhite();

// Fade in top screen
void fadeInTop();

// Fade in bottom screen
void fadeInBottom();

// Simple fade in
void fadeIn();

// Simple fade out
void fadeOut();

// Wait some time, in milliseconds (time must be > 16 to be effective)
void wait(int time);

// translate a number in millisecond to a number of vbls
inline int time_to_vbl(int num);

// Returns an int rounded to the nearest 4 multiple
inline int round4(int i);

// Returns an int rounded to the nearest 8 multiple
inline int round8(int i);

// Returns an int rounded to the nearest 16 multiple
inline int round16(int i);

// Returns an int rounded to the nearest 32 multiple
inline int round32(int i);

// similar to siprintf but WORKS
void sformat(char* buf, char* text, ...);

// Wait for a key or screen press
void waitForAnyKey();

// Returns a random number between a & b
//inline int rand(int a, int b);
#define rand(a,b)   PA_RandMinMax(a, b)

// Init special sound system
void initSoundSystem();

// Queue a sound
//void queueSound(void* data, u32 length, u8 type, u8 volume, u8 pan);

// empty sound queue
//void playQueue();

// Simple sound effect mixer, return true if sound is playing
bool soundMix(void* data, u32 length, u8 type, u8 volume, u8 pan);

// Play sound & pause if lid is closed
bool checkLid();

// load options
void loadOptions();

// save options
void saveOptions();

// check if a saved game exists
bool checkSavedGame();

// calculate map size & magic sum
void calcMapSizeAndSum(char *map_filename, int *size, int *sum);

// simple strcpy function...
void strcopy(char* dest, char* src);

// Simple strlen function...
int strlength(register char *s);

// Simple tolower function...
char ctolower(char c);

// Simple strncasecmp function...
int strncomp(const char *s1, const char *s2, int n);

// Simple strstr function...
char *strstring(const char *in, const char *str);

// Simple strcat function...
char *strccat(register char *s, register const char *append);
