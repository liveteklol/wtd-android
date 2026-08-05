/************************************/
/* Warcraft Tower Defense - by Noda */
/* v0.5                    12/02/08 */
/************************************/

// Includes
#include <PA9.h>        // Include for PA_Lib
#include <stdio.h>

// Gfx
#include "gfx/all_gfx.c"
#include "gfx/all_gfx.h"
//#include "neo_splash.h"
//#include "sm_neo.h"

// Sfx
#include "WarlockAppears.h"

// Modules
#include "f_aux.h"      // Auxiliary functions
#include "menu.h"       // Menu functions
#include "engine.h"     // Engine functions
#include "map_loader.h" // Map loader
#include "vfont.h"      // Custom font functions
#include "efs_lib.h"

// Global variables
bool first_launch = true;
bool multiple_builds = false;    // option: allow multiple builds
int build_menu_pos = 0;          // option: build menu position
bool interface_switch = false;   // option: interface mode, switch or hold
bool double_clic = true;         // option: double-clic build mode

bool continue_game = false;      // is there a saved game available?
bool load_game = false;          // load saved game
char curr_map_path[256];         // current map path
int curr_map_size;               // current map file size
int curr_map_sum;                // current map magic sum


// Title screens
void title() {
    int i, j;
    
    PA_ResetSpriteSys();
    PA_ResetBgSys();
    PA_Init8bitBg(0, 0);
    loadTextPalette(0, 0);

    for(i = 2; i < 16; i++)
        PA_StopSound(i);

    initSoundSystem();

//    PA_PlaySimpleSound(0, MainScreenMusic);
    if(first_launch) {
        PA_PlaySimpleSound(/*0,*/ WarlockAppears);
        first_launch = false;
    }

//    PA_SetSoundChannelPan(0, 32);
    PA_SetBrightness(0, -32);
    PA_SetBrightness(1, -32);
    PA_LoadTiledBg(1, 2, title1); 

    // use sprites to draw title 2 to free some bg vram
    PA_LoadSpritePal(0, 12, (void*)title2_Pal);
    for(j=0; j<3; j++)
        for(i=0; i<4; i++) {
            PA_CreateSprite(0, 90+i+j*4, (void*)title2_Sprite, OBJ_SIZE_64X64, 1, 12, (i*64)%256, j*64);
            PA_SetSpritePrio(0, 90+i+j*4, 3);
            PA_SetSpriteAnim(0, 90+i+j*4, i+j*4);
        }

    initSnowFlakes();
    initFlashEyes();
    fadeInTop();

    // [Android port] the DS held the title screen a full second here to let
    // the intro sound play off slow flashcard I/O; nothing to wait for now.

//    waitNFlakes(200);

//    while(PA_SoundChannelIsBusy(0))
//        moveSnowFlakes();
    
    fadeInBottom();
}

// Main program
int main(int argc, char ** argv) 
{
    PA_Init();
    PA_InitVBL();
//    PA_InitSound();

//    PA_SetDefaultSound(127, 22050, 2);
    PA_SetLedBlink(0, 0);   // Stop led blinking on DS-X
    PA_SetAutoCheckLid(false);

    // white screens
    PA_SetBrightness(0, 32);
    PA_SetBrightness(1, 32);

    // Variables
    u8 select;

    // [Android port] the DS waited some VBL here for flashcard compatibility;
    // there is no flashcard to settle on Android.

    PA_InitASLibForMP3(AS_MODE_MP3 | AS_MODE_16CH);
    AS_SetDefaultSettings(AS_ADPCM, 22050, AS_NO_DELAY);

    // Splash for the compo
//    PA_Init16bitBg(0, 0);
//    PA_Init8bitBg(1, 0);
//    PA_LoadGif(1, (void*)neo_splash);
    
    // Splash screens
    fadeFromWhite();
//    PA_LoadGifXY(0, 108, 76, (void*)sm_neo);
//    wait(1000);

//    fadeOut();
        
    // init libfat EFS
    PA_Init8bitBg(0, 0);
    PA_SetBgPalCol(0, 1, PA_RGB(31, 31, 31));  // white

    if(!fatInitDefault()) {

        PA_Clear8bitBg(0);
        centerAlignSmartText(0, 0, 84, 255, 0, "FAT init failed!\n\nMake sure the binary is DLDI-patched.", 1, 1, 1);
        while(1);

    } else if(!EFS_Init()) {

        PA_Clear8bitBg(0);
        centerAlignSmartText(0, 0, 92, 255, 0, "EFS init failed!", 1, 1, 1);
        while(1);
    
    }

    loadOptions();    
    fadeOut();
    
//    AS_MP3StreamPlay("/test.mp3");
//    AS_SetMP3Loop(true);
    
    // Title screens
    title();

    while(1) {
    
        // Main menu
        select = mainMenu();
   
        switch(select) {
            case 0:        // new game
                if(selectMap())
                    title();
                break;

            case 1:        // continue game
                loadSavedGame();                
                title();
                break;
            
            case 2:        // options
                options();
                break;
    
            case 3:        // credits
                credits();
                break;
        }

    }
    
    return 0;
}

