/************************************/
/* Warcraft Tower Defense - by Noda */
/* Menu functions          12/02/08 */
/************************************/

// Includes
#include <PA9.h>       // Include for PA_Lib
#include "nds.h"

// Gfx
#include "gfx/all_gfx.h"

// Sfx
#include "ScreenPopUp.h"
#include "ScreenPopDown.h"
#include "BigButtonClick.h"

// Modules
#include "menu.h"       // Menu functions
#include "f_aux.h"      // Auxiliary functions
#include "vfont.h"      // Custom font functions
#include "map_loader.h" // Map loader
#include "strings.h"    // Text strings
#include "highscore.h"  // Highscore functions


// Options
extern bool multiple_builds;    // option: allow multiple builds
extern int build_menu_pos;      // option: build menu position
extern bool interface_switch;   // option: interface mode, switch or hold
extern bool double_clic;        // option: double-clic build mode

// Misc
extern bool continue_game;      // is there a saved game available?
bool main_menu;                    // are we still on main menu?

// Global variables for animations
static u8 moveflakes;
static bool eyes_reverse;
static u8 eyes_alpha;

// Global variables for selections
static map_desc* maps_list;
static int NUM_MAPS;
static int selected_map;
static int scroll_map;
static bool refresh_minimap;
static bool scores;
static score sc_info;
static bool found;

// move the continue-greyed sprite
void moveContinueSprite(int scrollymenu) {
    if(!continue_game && main_menu) {
        int y = 192-scrollymenu+75;
        if(y < -16 || y > 192)
            y = -16;
        PA_SetSpriteXY(0, 17, 179, y);
        PA_SetSpriteXY(0, 18, 179+16, y);
        PA_SetSpriteXY(0, 19, 179+32, y);
    }
}

// Menu pop down (screen 0, BG1)
void menuPopDown() {

    s32 scrollxmenu = 0, scrollymenu = 384;
//    PA_StopSound(6);
    moveContinueSprite(scrollymenu);
    PA_WaitForVBL();
    PA_InfLargeScrollXY(0, 1, scrollxmenu, scrollymenu);
    PA_PlaySimpleSound(/*5,*/ ScreenPopDown);
    PA_WaitForVBL();
    waitNFlakes(300);

    while(scrollymenu > 320 ) {
        scrollymenu -= 16;
        moveContinueSprite(scrollymenu);
        PA_WaitForVBL();
        PA_InfLargeScrollXY(0, 1, scrollxmenu, scrollymenu);
        moveSnowFlakes();
    }      
    while(scrollymenu > 224 ) {
        scrollymenu -= 8;
        moveContinueSprite(scrollymenu);
        PA_WaitForVBL();
        PA_InfLargeScrollXY(0, 1, scrollxmenu, scrollymenu);
        moveSnowFlakes();
    }      
    while(scrollymenu > 192 ) {
        scrollymenu -= 4;
        moveContinueSprite(scrollymenu);
        PA_WaitForVBL();
        PA_InfLargeScrollXY(0, 1, scrollxmenu, scrollymenu);
        moveSnowFlakes();
    }      
    while(scrollymenu < 204 ) {
        scrollymenu += 2;
        moveContinueSprite(scrollymenu);
        PA_WaitForVBL();
        PA_InfLargeScrollXY(0, 1, scrollxmenu, scrollymenu);
        moveSnowFlakes();
    }      
    while(scrollymenu > 192 ) {
        scrollymenu -= 1;
        moveContinueSprite(scrollymenu);
        PA_WaitForVBL();
        PA_InfLargeScrollXY(0, 1, scrollxmenu, scrollymenu);
        moveSnowFlakes();
    }      
}

// Menu pop up (screen 0, BG1)
void menuPopUp() {

    s32 scrollxmenu = 0, scrollymenu = 192;
//    PA_StopSound(7);
    PA_WaitForVBL();
    PA_PlaySimpleSound(/*7,*/ ScreenPopUp);
    PA_WaitForVBL();
        
    while(scrollymenu < 204 ) {
        scrollymenu += 1;
        moveContinueSprite(scrollymenu);
        PA_WaitForVBL();
        PA_InfLargeScrollXY(0, 1, scrollxmenu, scrollymenu);
        moveSnowFlakes();
    }      
    while(scrollymenu > 192 ) {
        scrollymenu -= 2;
        moveContinueSprite(scrollymenu);
        PA_WaitForVBL();
        PA_InfLargeScrollXY(0, 1, scrollxmenu, scrollymenu);
        moveSnowFlakes();
    }      
    while(scrollymenu < 224 ) {
        scrollymenu += 4;
        moveContinueSprite(scrollymenu);
        PA_WaitForVBL();
        PA_InfLargeScrollXY(0, 1, scrollxmenu, scrollymenu);
        moveSnowFlakes();
    }      
    while(scrollymenu < 320 ) {
        scrollymenu += 8;
        moveContinueSprite(scrollymenu);
        PA_WaitForVBL();
        PA_InfLargeScrollXY(0, 1, scrollxmenu, scrollymenu);
        moveSnowFlakes();
    }      
    while(scrollymenu < 384 ) {
        scrollymenu += 16;
        moveContinueSprite(scrollymenu);
        PA_WaitForVBL();
        PA_InfLargeScrollXY(0, 1, scrollxmenu, scrollymenu);
        moveSnowFlakes();
    }      
}

// Init snow flakes
void initSnowFlakes() {
    int i;
    for(i=20; i<80; i++) {
        PA_LoadSpritePal(0, 8, (void*)flake_Pal);
        PA_CreateSprite(0, i, (void*)flake_Sprite, OBJ_SIZE_8X8, 1, 8, rand(0,255), rand(0, 255));
        PA_SetSpritePrio(0, i, 2);
    }
    moveflakes = 0;
}

// Moves snow flakes & flash the eyes
void moveSnowFlakes() {
    if(moveflakes == 0) {
        moveflakes = 6;
        s16 i, x, y;
        for(i=20; i<80; i++) {
            x = PA_GetSpriteX(0, i) + rand(-1, 1);
            y = PA_GetSpriteY(0, i) + rand(0, 2);
            if((x<=29 && x>=0 && y>=170) || (x<=255 && x>=229 && y>=174)) {
                x = rand(0,255);
                y = rand(192, 256);
            }
            PA_SetSpriteXY(0, i, x, y);
        }
    }
    moveflakes--;
    FlashEyes();
    checkLid();
}

// Waits & moves snow flakes & flash the eyes
void waitNFlakes(int time) {
    int i, nb;
    nb = ((time*1000)/16666);
    for(i = 0; i < nb; i++){
        PA_WaitForVBL();
        moveSnowFlakes();
        FlashEyes();
    }
}

// Init the flashing eyes
void initFlashEyes() {
    PA_LoadSpritePal(1, 9, (void*)flashing_eyes_Pal);
    PA_CreateSprite(1, 10, (void*)flashing_eyes_Sprite, OBJ_SIZE_64X64, 1, 9, 128, 5);
    PA_CreateSprite(1, 11, (void*)flashing_eyes_Sprite, OBJ_SIZE_64X64, 1, 9, 128+64, 5);
    PA_SetSpriteMode(1, 10, 1);
    PA_SetSpriteMode(1, 11, 1);
    PA_SetSpriteAnim(1, 11, 1);
    PA_EnableSpecialFx(1, SFX_ALPHA, 0, SFX_BG0 | SFX_BG1 | SFX_BG2 | SFX_BG3 | SFX_BD); 
    PA_SetSFXAlpha(1, 0, 15);
    eyes_alpha = 0;
    eyes_reverse = false;
}

// Flash the eyes
void FlashEyes() {
    if(eyes_reverse) {
        if(eyes_alpha == 0)
            eyes_reverse = false;
        else
            eyes_alpha--;
    } else {
        if(eyes_alpha == 6*16)
            eyes_reverse = true;
        else
            eyes_alpha++;
    }
    PA_SetSFXAlpha(1, eyes_alpha/6, 16-eyes_alpha/6);
}

// Display the selected button
void dispSelected(s16 x, s16 y) {
    PA_LoadSpritePal(0, 0, (void*)selected_Pal);
    PA_CreateSprite(0, 0, (void*)selected_Sprite, OBJ_SIZE_32X16, 1, 0, x, y);
    PA_CreateSprite(0, 1, (void*)selected_Sprite, OBJ_SIZE_32X16, 1, 0, x+32, y);
    PA_CreateSprite(0, 2, (void*)selected_Sprite, OBJ_SIZE_32X16, 1, 0, x+64, y);
    PA_SetSpriteMode(0, 0, 1);
    PA_SetSpriteMode(0, 1, 1);
    PA_SetSpriteMode(0, 2, 1);
    PA_SetSpriteAnim(0, 1, 1);
    PA_SetSpriteAnim(0, 2, 2);
    PA_EnableSpecialFx(0, SFX_ALPHA, 0, SFX_BG0 | SFX_BG1 | SFX_BG2 | SFX_BG3 | SFX_BD); 
    PA_SetSFXAlpha(0, 7, 15);

    while(Stylus.Held) {
        PA_WaitForVBL();
        moveSnowFlakes();
    }
    
    waitNFlakes(100);
    PA_DeleteSprite(0, 0);
    PA_DeleteSprite(0, 1);
    PA_DeleteSprite(0, 2);
    PA_PlaySimpleSound(/*3,*/ BigButtonClick);
    PA_WaitForVBL();
}

// Display the selected button for options
void dispSelectedOption(s16 x, s16 y) {
    PA_LoadSpritePal(0, 0, (void*)selected_op_Pal);
    PA_CreateSprite(0, 0, (void*)selected_op_Sprite, OBJ_SIZE_32X16, 1, 0, x, y);
    PA_CreateSprite(0, 1, (void*)selected_op_Sprite, OBJ_SIZE_32X16, 1, 0, x+32, y);
    PA_SetSpriteMode(0, 0, 1);
    PA_SetSpriteMode(0, 1, 1);
    PA_SetSpriteAnim(0, 1, 1);
    PA_EnableSpecialFx(0, SFX_ALPHA, 0, SFX_BG0 | SFX_BG1 | SFX_BG2 | SFX_BG3 | SFX_BD); 
    PA_SetSFXAlpha(0, 7, 15);

    while(Stylus.Held) {
        PA_WaitForVBL();
        moveSnowFlakes();
    }
    
    waitNFlakes(100);
    PA_DeleteSprite(0, 0);
    PA_DeleteSprite(0, 1);
    PA_PlaySimpleSound(/*3,*/ BigButtonClick);
    PA_WaitForVBL();
}

// Main menu
u8 mainMenu() {

    u8 select = 0;                     // menu selection
    main_menu = true;
    continue_game = checkSavedGame();  // check for saved game
    
    PA_LoadPAGfxLargeBg(0, 1, menu);
    
    if(!continue_game) {
        PA_LoadSpritePal(0, 4, (void*)menu_continue_Pal);
        PA_CreateSprite(0, 17, (void*)menu_continue_Sprite, OBJ_SIZE_16X16, 1, 4, 179, -16);
        PA_CreateSprite(0, 18, (void*)menu_continue_Sprite, OBJ_SIZE_16X16, 1, 4, 179+16, -16);
        PA_CreateSprite(0, 19, (void*)menu_continue_Sprite, OBJ_SIZE_16X16, 1, 4, 179+32, -16);
        PA_SetSpriteAnim(0, 18, 1);
        PA_SetSpriteAnim(0, 19, 2);
    }
    
    PA_WaitForVBL();
    menuPopDown();

    while(1) {
        PA_WaitForVBL();
        moveSnowFlakes();
        
        // solo
        if (Pad.Newpress.Start || Pad.Newpress.A) {
            dispSelected(158, 45);
            break;
        }

        // new game
        if (Stylus.Newpress && PA_StylusInZone(157, 45, 248, 60)) {
            dispSelected(158, 45);
            break;
        }
        
        // continue game
        if (continue_game && (Stylus.Newpress && PA_StylusInZone(157, 75, 248, 90))) {
            dispSelected(158, 75);
            select = 1;
            break;            
        }

        // options
        if (Stylus.Newpress && PA_StylusInZone(157, 105, 248, 120)) {
            dispSelected(158, 105);
            select = 2;
            break;
        }

        // credits
        if (Stylus.Newpress && PA_StylusInZone(157, 135, 248, 150)) {
            dispSelected(158, 135);
            select = 3;
            break;
        }
    }   

    menuPopUp();

    if(!continue_game) {
        PA_DeleteSprite(0, 17);
        PA_DeleteSprite(0, 18);
        PA_DeleteSprite(0, 19);
    }
    
    main_menu = false;
    
    return select;
}
    
// Credits
void credits() {

    PA_LoadPAGfxLargeBg(0, 1, credits);
    PA_WaitForVBL();
    menuPopDown();
    
    while(1) {
        PA_WaitForVBL();
        moveSnowFlakes();
        
        if (Pad.Newpress.Start || Pad.Newpress.A || (Stylus.Newpress && PA_StylusInZone(82, 146, 173, 161))) {
            dispSelected(83, 146);
            break;
        }
    }   

    menuPopUp();
}
    

// display map list
void dispMapList() {
    int i, j;

    // clear bg
    PA_Clear8bitBg(0);

    // draw selected area
    if(selected_map != -1) {
        if((selected_map-scroll_map) >= 0 && (selected_map-scroll_map) < 12) {
            for(j=38+11*(selected_map-scroll_map); j<49+11*(selected_map-scroll_map); j++)
                for(i=11; i<=104; i+=2)
                    PA_PutDouble8bitPixels(0, i, j, 16, 16);
        }
    }

    // scroll position indicator
    int ymin = 48, ymax = 154-ymin, pos;
    pos = ((scroll_map*100)/(NUM_MAPS-12)*ymax/100)+ymin;
    PA_SetSpriteXY(0, 121, 108, pos);

    // show or hide minimap
    if(scores)
        PA_SetSpritePrio(0, 120, 3);
    else
        PA_SetSpritePrio(0, 120, 0);      

    // refresh scores for current map
    if(scores && selected_map != -1) {
    
        char *diff_texts[5] = { S_DIFF_VERYEASY, S_DIFF_EASY, S_DIFF_NORMAL, S_DIFF_HARD, S_DIFF_VERYHARD };    
        int size, sum;
        char str[256];
            
        if(refresh_minimap) {
            // display texts (simple & dirty way to avoid names to disappear while loading scores)
            for(i=scroll_map; (i<NUM_MAPS) && (i<scroll_map+12); i++)
                SmartText(0, 11, 39+11*(i-scroll_map), 104, 49+11*(i-scroll_map), maps_list[i].name, (i == selected_map ? 7 : 1), 1, 1, MAP_NAME_MAX);
                
            found = false;
            centerAlignSmartText(0, 141, 52, 251, 63, S_WAIT, 7, 1, 1);
            calcMapSizeAndSum(maps_list[selected_map].filename, &size, &sum);
            found = searchAndGetScore(size, sum, &sc_info);
            centerAlignSmartText(0, 141, 52, 251, 63, S_EMPTY, 7, 1, 0);
            refresh_minimap = false;
        }

        // display high score
        centerAlignSmartText(0, 141, 28, 251, 39, S_MAP_COMPLETED, 7, 1, 1);
        centerAlignSmartText(0, 141, 61, 251, 72, S_HIGHSCORE, 7, 1, 1);

        if(found) {

            if(sc_info.finished)
                sformat(str, "%s (%s)", S_YES, diff_texts[sc_info.finish_diff]);
            else
                sformat(str, S_NO);
            centerAlignSmartText(0, 141, 40, 251, 51, str, 1, 1, 1);

            sformat(str, "%d (%s)", ~sc_info.score, diff_texts[sc_info.score_diff]);
            centerAlignSmartText(0, 141, 73, 251, 84, str, 1, 1, 1);
        } else {        
            centerAlignSmartText(0, 141, 40, 251, 51, S_NO, 1, 1, 1);
            centerAlignSmartText(0, 141, 73, 251, 84, S_NO_SCORE, 1, 1, 1);
        }
    
//        sformat(str, "size: %dKo\nSum: %d", size/1024, sum);
//        SmartText(0, 155, 32, 250, 64, str, 7, 1, 1, 256);
    }

    // display texts
    for(i=scroll_map; (i<NUM_MAPS) && (i<scroll_map+12); i++)
        SmartText(0, 11, 39+11*(i-scroll_map), 104, 49+11*(i-scroll_map), maps_list[i].name, (i == selected_map ? 7 : 1), 1, 1, MAP_NAME_MAX);

    // draw minimap
    if(!scores && refresh_minimap && selected_map != -1) {
        if((selected_map-scroll_map) >= 0 && (selected_map-scroll_map) < 12) {
            // minimap
            if(maps_list[selected_map].file) {
                PA_DeleteSprite(0, 120);
                PA_LoadSpritePal(0, 10, (u16*)getMapPreviewPal(selected_map));
                PA_CreateSprite(0, 120, (u8*)getMapPreviewSprite(selected_map), OBJ_SIZE_64X64, 1, 10, 168, 27);
                refresh_minimap = false;
            }
        }
    }
}

// Map selection
bool selectMap() {
    
    bool played = false;
    int new_selection = 0;
    selected_map = 0;
    scores = false;
    scroll_map = 0;
    refresh_minimap = true;

    PA_LoadPAGfxLargeBg(0, 1, select_map);
    PA_WaitForVBL();
    menuPopDown();

    // scroll position indicator
    PA_LoadSpritePal(0, 11, (void*)scroll_position_Pal);
    PA_CreateSprite(0, 121, (void*)scroll_position_Sprite, OBJ_SIZE_8X8, 1, 11, 108, 48);

    // dummy minimap
    PA_CreateSprite(0, 120, NULL, OBJ_SIZE_64X64, 1, 10, -64, -64);
    
    // init map list
    listMaps(&maps_list, &NUM_MAPS);

    // draw map choice
    dispMapList();
    
    while(1) {
        PA_WaitForVBL();
        moveSnowFlakes();
                
        // select map
        if(Stylus.Held && PA_StylusInZone(11, 39, 104, 170)) {
            if((((Stylus.Y-39)/11)+scroll_map < NUM_MAPS) ) { 
            
                new_selection = ((Stylus.Y-39)/11)+scroll_map;
                
                if(selected_map != new_selection) {
                    selected_map = new_selection;
                    refresh_minimap = true;
                    // redraw list
                    dispMapList();
                }
            }
        }
        
        // switch to highscore view
        if(Pad.Newpress.R || Pad.Newpress.L) {
            if(scores) {
                PA_InitLargeBg(0, 1, (select_map_Info[1]) >> 3, (select_map_Info[2]) >> 3, (void*)select_map_Map);
//                PA_LoadLargeBg(0, 1, select_map_Tiles, select_map_Map, 1, (select_map_Info[1]) >> 3, (select_map_Info[2]) >> 3);
                PA_InfLargeScrollXY(0, 1, 0, 192);  
                scores = false;
            } else {
                PA_InitLargeBg(0, 1, (select_map_Info[1]) >> 3, (select_map_Info[2]) >> 3, (void*)select_map_highscore_Map);
//                PA_LoadLargeBg(0, 1, select_map_Tiles, select_map_highscore_Map, 1, (select_map_highscore_Info[1]) >> 3, (select_map_highscore_Info[2]) >> 3);
                PA_InfLargeScrollXY(0, 1, 0, 192);    
                scores = true;
            }
            refresh_minimap = true;
            dispMapList();
        }        
        
        // move up
        if(Pad.Newpress.Up) {
            if(selected_map > 0)
                selected_map--;
        
            while(scroll_map > selected_map && scroll_map > 0)
                scroll_map--;
                
            // redraw list
            refresh_minimap = true;
            dispMapList();
        }
        // move down
        else if(Pad.Newpress.Down) {
            if(selected_map < NUM_MAPS-1)
                selected_map++;

            while(scroll_map+11 < selected_map && scroll_map < (NUM_MAPS-12))
                scroll_map++;
                
            // redraw list
            refresh_minimap = true;
            dispMapList();
        }
        
        // scroll up
        if(Stylus.Newpress && PA_StylusInZone(106, 36, 117, 46)) {
            if(scroll_map > 0)
                scroll_map--;                
            // redraw list
            dispMapList();
        }
        // scroll down
        else if(Stylus.Newpress && PA_StylusInZone(106, 163, 117, 173)) {
            if(scroll_map < (NUM_MAPS-12))
                scroll_map++;                
            // redraw list
            dispMapList();
        }
        // scroll by moving the scroll bar
        else if(Stylus.Held && PA_StylusInZone(106, 46, 117, 162)) {
            int scroll_step = 116/(NUM_MAPS-12);
            scroll_map = ((Stylus.Y-46)+scroll_step/2)/scroll_step;
            if(scroll_map > (NUM_MAPS-12))
                scroll_map = NUM_MAPS-12;
            // redraw list
            dispMapList();
        }
        // play
        else if(Pad.Newpress.Start || Pad.Newpress.A || (Stylus.Newpress && PA_StylusInZone(157, 123, 246, 138))) {
            dispSelected(158, 123);
            played = true;
            
            // clear items
            PA_DeleteSprite(0, 120);
            PA_DeleteSprite(0, 121);
            PA_Clear8bitBg(0);
            menuPopUp();

            // launch map
            loadMap(maps_list[selected_map].filename);
            break;
        }
        // return
        else if(Stylus.Newpress && PA_StylusInZone(157, 151, 246, 166)) {
            dispSelected(158, 151);
            PA_DeleteSprite(0, 120);
            PA_DeleteSprite(0, 121);
            PA_Clear8bitBg(0);
            menuPopUp();
            break;
        }
    }   

    return played;
}

// Options
void options() {

    PA_LoadPAGfxLargeBg(0, 1, options);
    PA_WaitForVBL();
    menuPopDown();
    
    while(1) {
        PA_WaitForVBL();
        moveSnowFlakes();
        
        // clear bg
        PA_Clear8bitBg(0);
        
        // draw options
        if(interface_switch)
            centerAlignSmartText(0, 153, 61, 215, 72, S_ON, 1, 1, 1);
        else
            centerAlignSmartText(0, 153, 61, 215, 72, S_OFF, 1, 1, 1);
        
        switch(build_menu_pos) {
            case 0:
                centerAlignSmartText(0, 153, 78, 215, 89, S_UPRIGHT_CORNER, 1, 1, 1);
                break;
            case 1:
                centerAlignSmartText(0, 153, 78, 215, 89, S_UPLEFT_CORNER, 1, 1, 1);
                break;
            case 2:
                centerAlignSmartText(0, 153, 78, 215, 89, S_DOWNRIGHT_CORNER, 1, 1, 1);
                break;
            case 3:
                centerAlignSmartText(0, 153, 78, 215, 89, S_DOWNLEFT_CORNER, 1, 1, 1);
                break;
        }
        
        if(multiple_builds)
            centerAlignSmartText(0, 153, 95, 215, 106, S_ON, 1, 1, 1);
        else
            centerAlignSmartText(0, 153, 95, 215, 106, S_OFF, 1, 1, 1);
        
        if(double_clic)
            centerAlignSmartText(0, 153, 112, 215, 123, S_ON, 1, 1, 1);
        else
            centerAlignSmartText(0, 153, 112, 215, 123, S_OFF, 1, 1, 1);
        
        
        // L&R switch
        if (Stylus.Newpress && PA_StylusInZone(153, 58, 215, 71)) {
            interface_switch = !interface_switch;
            dispSelectedOption(152, 57);
        }
        
        // build menu position
        if (Stylus.Newpress && PA_StylusInZone(153, 75, 215, 88)) {
            build_menu_pos = (build_menu_pos+1) % 4;
            dispSelectedOption(152, 74);
        }

        // allow chain building
        if (Stylus.Newpress && PA_StylusInZone(153, 92, 215, 105)) {
            multiple_builds = !multiple_builds;
            dispSelectedOption(152, 91);
        }

        // double tap building
        if (Stylus.Newpress && PA_StylusInZone(153, 109, 215, 122)) {
            double_clic = !double_clic;
            dispSelectedOption(152, 108);
        }

        // return to main menu
        if (Pad.Newpress.Start || Pad.Newpress.A || (Stylus.Newpress && PA_StylusInZone(82, 146, 173, 161))) {
            dispSelected(83, 146);
            PA_Clear8bitBg(0);
            break;
        }
    }   

    saveOptions();
    menuPopUp();
}
