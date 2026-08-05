/************************************/
/* Warcraft Tower Defense - by Noda */
/* Engine functions        12/02/08 */
/************************************/

// Includes
#include <PA9.h>       // Include for PA_Lib

// Gfx
#include "gfx/all_gfx.h"

// Sfx
#include "Hint.h"
#include "MapPing.h"
#include "BigButtonClick.h"
#include "PeonCannotBuildThere.h"
#include "KnightNoGold.h"
#include "KnightNoFood.h"
#include "CreepAggroWhat.h"
#include "BuildingPlacement.h"
#include "RallyPointPlace.h"
#include "MouseClick.h"

// Modules
#include "types.h"      // Types definitions
#include "defines.h"    // Defines
#include "strings.h"    // Text strings
#include "engine.h"     // For the prototypes
#include "f_aux.h"      // Auxiliary functions
#include "menu.h"       // Menu functions
#include "vfont.h"      // Custom font functions
#include "ai.h"         // AI functions
#include "map_loader.h" // Map loader
#include "highscore.h"  // High score functions
#include "efs_lib.h"

// Defines
#define MSG_TIME            time_to_vbl(2000) // display time of a message
#define NEW_EVO_MSG_TIME    time_to_vbl(7000) // display time of the new evolution available message
#define BUILD_ZOOM_TIME     10                // duration of the zoom effect when building towers

// [Android port] dynamic map viewport (bottom screen fills the device
// screen and can be pinch-zoomed); clamped to the map dimensions.
#define VIEW_W ((shim_view_w < td->map_Width) ? shim_view_w : td->map_Width)
#define VIEW_H ((shim_view_h < td->map_Height) ? shim_view_h : td->map_Height)

// Constants
const int base_color = 182;     // base color for screen 0 text palette
/*u32 air_shadow[8*6] = {
    0, 1, 1, 1, 1, 1, 1, 0, 
    1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 
    0, 1, 1, 1, 1, 1, 1, 0
};*/


// Options
extern bool multiple_builds;    // option: allow multiple builds
extern int build_menu_pos;      // option: build menu position
extern bool interface_switch;   // option: interface mode, switch or hold
extern bool double_clic;        // option: double-clic build mode

// Externals
extern bool load_game;          // load saved game
extern char curr_map_path[256]; // current map path
extern int curr_map_size;       // current map file size
extern int curr_map_sum;        // current map magic sum

// Global variables
char strtmp[255];               // temp string
char msg[255];                  // message to display
map* td;                        // td map descriptor
int difficulty;                 // difficulty factor (%)
u8 difficulty_value;            // difficulty value (0-5)
bool interface;                 // interface activation
bool menu;                      // menu activation
bool desc;                      // display tower description
bool dark;                      // dark palette switch
bool blocked;                   // if monsters are blocked
bool evolution_available;       // an evolution is available
s16 build_pal;                  // tower to build palette
s32 menu_x, menu_y;             // base position of the menu
s32 window_x, window_y;         // base position of the window
s32 build_x, build_y;           // position of new tower when building
u16 menutimer;                  // timer for the menu
s16 spawntimer;                 // spawn timer
u16 msg_timer;                  // message timer
u16 evo_timer;                  // new evolution available timer
int total_time;                 // total time
int stylus_old_x, stylus_old_y; // previous stylus coordinate, used for double tap detection

evolution* evolutions;          // current evolution set
s16 current_clan;               // current clan
s16 current_round;              // current round
s16 current_evo;                // current evolution
u16 gold;                       // gold amount
u8 max_towers;                  // max towers
s16 lifes;                      // remaining lifes
u16 timer;                      // timer
u8 vblcount;                    // num of current vbl
u8 deadmonsters;                // num of dead monsters
u16 kills;                      // num of kills
s16 m_vanish;                   // vanish monsters cycle
s16 selected_evo;               // selected evolution
s16 selected_monster;           // selected monster
s16 selected_tower;             // selected tower
s16 selected_clan;              // selected clan
u8 towerToBuild;                // tower to build
s16 upgrade_to;                 // tower to upgrade to
s16 upgrade_base;               // upgrade base selection
s16 to_sell;                    // tower to sell
u8 num_icons;                   // number of icons displayed
u8 num_sp_icons;                // number of special effects icons displayed
u8 blocked_monsters;            // number of monsters that are unable to move
u8 build_menu_sel_id;           // build menu selected sprite id
bool boxhidden;                 // tips box is hidden
bool new_round;                 // new round start
bool exit_engine;               // exit the engine
bool retry;                     // replay map
bool stats;                     // show stats
bool confirm;                   // confirmation icons
u8 zoom_timer;                  // zoom timer for building sprites
s8 zoom_tower;                  // current zoomed tower
u8 current_path;                // current spawn path
u8 num_spawned[MAX_PATHS];      // number of monsters spawned per path

u16 minimap_x_adjust, minimap_y_adjust;     // value to adjust minimap display
u8 minimap_x_base, minimap_y_base;  // precalculated values for minimap display
int minimap_x_windowsize, minimap_y_windowsize;  // [port] int: recomputed per frame from the viewport
int minimap_x_factor, minimap_y_factor;        // precalculated values for minimap display

u8 num_towers;                                  // number of towers
tower_instance towers[TOWERS_INST_MAX+1];       // the built towers
attack_instance attacks[ATTACKS_INST_MAX+1];    // the attacks
u16 num_monsters;                               // number of monsters
monster_instance monsters[MONSTERS_INST_MAX+1]; // spawned monsters

u8 num_paths;                   // number of paths
u16* waypoints[MAX_PATHS];      // the waypoints
u8 num_waypoints[MAX_PATHS];    // number of waypoints

u8* m_path;                     // monsters space
u8* t_path;                     // towers space
u16 path_width, path_height;    // spaces size

int build_sel_x1, build_sel_x2, build_sel_y1, build_sel_y2;     // build button selection rectangles ([port] int: viewport > 255)
int cancel_sel_x1, cancel_sel_x2;           // cancel button selection rectangles
int dbl_click_menu_x1, dbl_click_menu_x2;   // build menu zone inhibition for double click build mode

// [Android port] recompute the build-menu anchoring from the current
// viewport size (the viewport can change every frame with pinch zoom)
static void computeBuildMenuLayout(void) {
    switch(build_menu_pos) {
        case 1:     // top-left
            menu_x = 0;
            build_sel_x1 = 64;
            build_sel_x2 = 129;
            build_sel_y1 = 0;
            build_sel_y2 = 30;
            cancel_sel_x1 = 0;
            cancel_sel_x2 = 63;
            dbl_click_menu_x1 = 0;
            dbl_click_menu_x2 = 127;
            break;
        case 2:     // bottom-right
            menu_x = VIEW_W - 128;
            build_sel_x1 = VIEW_W - 127;
            build_sel_x2 = VIEW_W - 64;
            build_sel_y1 = VIEW_H - 30;
            build_sel_y2 = VIEW_H;
            cancel_sel_x1 = VIEW_W - 63;
            cancel_sel_x2 = VIEW_W - 1;
            dbl_click_menu_x1 = VIEW_W - 127;
            dbl_click_menu_x2 = VIEW_W - 1;
            break;
        case 3:     // bottom-left
            menu_x = 0;
            build_sel_x1 = 64;
            build_sel_x2 = 129;
            build_sel_y1 = VIEW_H - 30;
            build_sel_y2 = VIEW_H;
            cancel_sel_x1 = 0;
            cancel_sel_x2 = 63;
            dbl_click_menu_x1 = 0;
            dbl_click_menu_x2 = 127;
            break;
        default:    // top-right
            menu_x = VIEW_W - 128;
            build_sel_x1 = VIEW_W - 127;
            build_sel_x2 = VIEW_W - 64;
            build_sel_y1 = 0;
            build_sel_y2 = 30;
            cancel_sel_x1 = VIEW_W - 63;
            cancel_sel_x2 = VIEW_W - 1;
            dbl_click_menu_x1 = VIEW_W - 127;
            dbl_click_menu_x2 = VIEW_W - 1;
    }
}

//int nbs=0, ms=-1;

// Attacks 
u8* attack_Sprites[ATTACKS_NUM] = { // the sprites
    (u8*)s_arrow_Sprite,
    (u8*)s_bullet_Sprite,
    (u8*)s_cannon_Sprite,
    (u8*)s_fire_lightning_Sprite,
    (u8*)s_firerock_Sprite,
    (u8*)s_icerock_Sprite,
    (u8*)s_lightning_Sprite,
    (u8*)s_mudrock_Sprite,
    (u8*)s_poison_lightning_Sprite,
    (u8*)s_poisonrock_Sprite,
    (u8*)s_blaster_shot_Sprite,
    (u8*)s_blue_beam_Sprite,
    (u8*)s_blue_blast_Sprite,
    (u8*)s_blue_grenade_Sprite,
    (u8*)s_blue_psy_Sprite,
    (u8*)s_fire_blaster_shot_Sprite,
    (u8*)s_fire_breath_Sprite,
    (u8*)s_fire_spore_Sprite,
    (u8*)s_fire_virus_Sprite,
    (u8*)s_greenflame_cannon_Sprite,
    (u8*)s_green_beam_Sprite,
    (u8*)s_green_blast_Sprite,
    (u8*)s_green_psy_Sprite,
    (u8*)s_grenade_Sprite,
    (u8*)s_iceflame_cannon_Sprite,
    (u8*)s_ice_blaster_shot_Sprite,
    (u8*)s_ice_breath_Sprite,
    (u8*)s_ice_spore_Sprite,
    (u8*)s_ice_virus_Sprite,
    (u8*)s_magenta_blast_Sprite,
    (u8*)s_poison_blaster_shot_Sprite,
    (u8*)s_poison_breath_Sprite,
    (u8*)s_poison_spore_Sprite,
    (u8*)s_poison_virus_Sprite,
    (u8*)s_redflame_cannon_Sprite,
    (u8*)s_red_beam_Sprite,
    (u8*)s_red_blast_Sprite,
    (u8*)s_red_psy_Sprite,
    (u8*)s_rocket_Sprite,
    (u8*)s_static_blue_Sprite,
    (u8*)s_static_green_Sprite,
    (u8*)s_static_red_Sprite,
    (u8*)s_tornado_Sprite,
    (u8*)s_yellow_beam_Sprite,
    (u8*)s_yellow_grenade_Sprite,
    (u8*)s_yellow_psy_Sprite,    
};

// Palettes
u8* monster_pals[MONSTER_PALS_NUM] = {
    (u8*)monsters_Pal,
    (u8*)monsters_blue_Pal,
    (u8*)monsters_green_Pal,
    (u8*)monsters_dark_Pal,
};

// Minimap colors
u8 minimap_colorsets[MINIMAP_COLORSETS_NUM*5] = {
    // set 1: normal
    12, 3, 1, 2, 1,
    // set 2: alternative
    12, 4, 1, 7, 1,
    // set 3: dark
    6, 3, 5, 2, 5,
    // set 4: dark alternative
    6, 4, 5, 7, 5,
};

// Presentation of the map
void showPresentation(map* map) {

    // Reset everything
    fadeOut();
//    PA_StopSound(0);    // stop the music
    PA_ResetSpriteSys();
    PA_ResetBgSys();

    // Show presentation screen
    PA_Init8bitBg(0, 1);
    loadTextPalette(0, 0);
    PA_LoadTiledBg(1, 3, title1); 
    initFlashEyes();
    
    if(map->simple_presentation) {
        PA_LoadTiledBg(0, 2, map_presentation_simple); 
        centerAlignSmartText(0, 106, 92, 253, 102, map->name, 7, 1, 1);
        SmartText(0, 106, 104, 253, 156, map->presentation, 1, 1, 1, PRESENTATION_MAX);
        centerAlignSmartText(0, 64, 169, 192, 191, S_PRESS_KEY, 7, 0, 1);
    } else {
        PA_LoadTiledBg(0, 2, map_presentation); 
        PA_LoadSpritePal(0, MINIMAP_PAL_ID, (void*)map->minimap_Pal);
        PA_CreateSprite(0, MINIMAP_ID, (void*)map->minimap_Sprite, OBJ_SIZE_64X64, 1, MINIMAP_PAL_ID, 18, 46);
        centerAlignSmartText(0, 81, 19, 253, 29, map->name, 7, 1, 1);
        SmartText(0, 83, 32, 250, 144, map->presentation, 1, 1, 1, PRESENTATION_MAX);
        centerAlignSmartText(0, 64, 169, 192, 191, S_PRESS_KEY, 7, 0, 1);
    }
    
    fadeIn();
    
    while(1) {
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
        PA_WaitForVBL();
        checkLid();
        FlashEyes();
    }
}

// Init the engine
void initEngine(map* map) {

    int i;

    // Reset everything
    if(!load_game)
        fadeOut();
    PA_ResetSpriteSys();
    PA_ResetBgSys();
    
    // Init variables
    exit_engine = false;
    td = map;
    interface = false;
    retry = false;
    stats = false;
    menu = false;
    confirm = false;
    desc = false;
    dark = false;
    blocked = false;
    build_pal = -1;
    stylus_old_x = 0;
    stylus_old_y = 0;
    
    // [Android port] map viewport active for this map (enables the
    // extended bottom screen + pinch zoom in the platform backend)
    shim_SetMapView(1, td->map_Width, td->map_Height);

    // Init build menu position & behavior (zones recomputed every frame)
    computeBuildMenuLayout();
    menu_y = (build_menu_pos > 1) ? VIEW_H : -32;
    
    window_x = map->start_x*16;
    window_y = map->start_y*16;
    build_x = 0;
    build_y = 0;
    menutimer = 0;
    msg_timer = 0;
    total_time = 0;
    zoom_timer = 0;
    zoom_tower = -1;
    evo_timer = 0;

    gold = td->gold_start;
    max_towers = td->max_towers_start;
    if(max_towers > TOWERS_INST_MAX)
        max_towers = TOWERS_INST_MAX;
    
    timer = td->init_delay;
    num_towers = 0;
    num_monsters = 0;
    
    if(td->numClans == 1) {
        current_clan = 0;
        evolutions = td->clans[0].evolutions;
        max_towers += evolutions[0].max_towers;
        if(max_towers > TOWERS_INST_MAX)
            max_towers = TOWERS_INST_MAX;
        evolution_available = (td->clans[0].num_evolutions > 1 && evolutions[1].minRound <= 0);
    } else {
        current_clan = -1;
        evolutions = NULL;
        evolution_available = false;
    }
   
    current_round = -1;
    current_evo = 0;
    vblcount = 0;
    deadmonsters = 0;
    m_vanish = -1;
    selected_evo = -1;
    selected_monster = -1;
    selected_tower = -1;
    selected_clan = -1;
    new_round = true;
    kills = 0;
    lifes = td->lifes;
    towerToBuild = 0;
    upgrade_to = -1;
    upgrade_base = -1;
    to_sell = -1;
    num_icons = 0;
    num_sp_icons = 0;
    minimap_x_adjust = td->minimap_x_adjust;
    minimap_y_adjust = td->minimap_y_adjust;
    build_menu_sel_id = 0;

    for(i=0; i < num_paths; i++)
        num_spawned[i] = 0;
            
    current_path = 0;
    
    // Interface
    PA_Init8bitBg(1, 1);
    PA_Init8bitBg(0, 0);
    PA_LoadTiledBg(1, 2, interface); 

    // Load text palette
    loadTextPalette(1, 0);
    loadTextPalette(0, 0);

    // Map
    initMapGfx();
    PA_LargeScrollXY(0, 2, window_x, window_y);

    // Minimap
    PA_LoadSpritePal(1, MINIMAP_PAL_ID, (void*)td->minimap_Pal);
    PA_CreateSprite(1, MINIMAP_ID, (void*)td->minimap_Sprite, OBJ_SIZE_64X64, 1, MINIMAP_PAL_ID, MINIMAP_X, MINIMAP_Y);
    PA_SetSpritePrio(1, MINIMAP_ID, 2);

    // Sprites palettes
//    PA_LoadSpritePal(0, M_RED_PAL_ID, (void*)monsters_Pal);
    PA_LoadSpritePal(0, M_FROZ_PAL_ID, (void*)monsters_froz_Pal);
    PA_LoadSpritePal(0, M_POIS_PAL_ID, (void*)monsters_pois_Pal);
//    PA_LoadSpritePal(0, M_GREEN_PAL_ID, (void*)monsters_green_Pal);
//    PA_LoadSpritePal(0, M_DARK_PAL_ID, (void*)monsters_dark_Pal);
//    PA_LoadSpritePal(0, M_BLUE_PAL_ID, (void*)monsters_blue_Pal);
    PA_LoadSpritePal(0, T_NORM_PAL_ID, (void*)t_all_normal_Pal);
    PA_LoadSpritePal(0, T_FROZ_PAL_ID, (void*)t_all_frozen_Pal);
    PA_LoadSpritePal(0, T_POIS_PAL_ID, (void*)t_all_poison_Pal);
    PA_LoadSpritePal(0, T_DARK_PAL_ID, (void*)t_all_dark_Pal);
    PA_LoadSpritePal(0, BUILD_SELEC_PAL_ID, (void*)t_select24x24_Pal);
    PA_LoadSpritePal(0, ATTACKS_PAL_ID, (void*)attacks_Pal);
    PA_LoadSpritePal(0, BUILD_MENU_PAL_ID, (void*)build_menu_Pal);
    PA_LoadSpritePal(1, ICONS_PAL_ID, (void*)icons_Pal);
    PA_LoadSpritePal(1, ICONS_DARK_PAL_ID, (void*)icons_dark_Pal);
    PA_LoadSpritePal(1, HIDE_BOX_PAL_ID, (void*)hide_box_Pal);

    // Hide tips box
    PA_CreateSprite(1, HIDE_BOX_ID, (void*)hide_box_Sprite, OBJ_SIZE_32X32, 1, HIDE_BOX_PAL_ID, 77, 107);
    PA_CreateSprite(1, HIDE_BOX_ID+1, (void*)hide_box_Sprite, OBJ_SIZE_32X32, 1, HIDE_BOX_PAL_ID, 77+32, 107);
    PA_CreateSprite(1, HIDE_BOX_ID+2, (void*)hide_box_Sprite, OBJ_SIZE_32X32, 1, HIDE_BOX_PAL_ID, 77+64, 107);
    PA_SetSpriteAnim(1, HIDE_BOX_ID+1, 1);
    PA_SetSpriteAnim(1, HIDE_BOX_ID+2, 2);
    boxhidden = true;

    // Shadows bg
//    PA_Init16cBg(0, 0);
//    PA_InitText(1, 0);

    // Init map variables
    m_path = td->m_path;
    t_path = td->t_path;
    path_width = td->map_Width / 16;
    path_height = td->map_Height / 16;
    num_paths = td->num_paths;
    
    for(i=0; i<num_paths; i++) {
        waypoints[i] = td->waypoints[i];
        num_waypoints[i] = td->num_waypoints[i];
    }
    
    // Precalculate some values for minimap display
    minimap_x_base = MINIMAP_X+minimap_x_adjust;
    minimap_y_base = MINIMAP_Y+minimap_y_adjust;
    minimap_x_factor = ((MINIMAP_WIDTH-(minimap_x_adjust*2)) << FIXED_POINT_PRECISION)/td->map_Width;
    minimap_y_factor = ((MINIMAP_HEIGHT-(minimap_y_adjust*2)) << FIXED_POINT_PRECISION)/td->map_Height;
    minimap_x_windowsize = (((VIEW_W)*minimap_x_factor) >> FIXED_POINT_PRECISION);
    minimap_y_windowsize = (((VIEW_H)*minimap_y_factor) >> FIXED_POINT_PRECISION);
    if(window_x < 0) window_x = 0;
    if(window_y < 0) window_y = 0;
    if(window_x > td->map_Width-VIEW_W) window_x = td->map_Width-VIEW_W;
    if(window_y > td->map_Height-VIEW_H) window_y = td->map_Height-VIEW_H;
    
    // Enable transparency effects
    PA_EnableSpecialFx(0, SFX_ALPHA, 0, SFX_BG0 | SFX_BG1 | SFX_BG2 | SFX_BG3 | SFX_BD); 
    PA_EnableSpecialFx(1, SFX_ALPHA, 0, SFX_BG0 | SFX_BG1 | SFX_BG2 | SFX_BG3 | SFX_BD); 

    // Selection circle
    PA_CreateSprite(0, SELECTION_ID, (void*)selection_Sprite, OBJ_SIZE_64X64, 1, BUILD_SELEC_PAL_ID, -64, -64);
    PA_SetSpritePrio(0, SELECTION_ID, 2);

    // Allocate all sprites for the bottom screen
    for(i=0; i<MONSTERS_INST_MAX; i++) {
        PA_CreateSprite(0, BASE_MONSTERS_ID - i, td->monsters[0].gfx, OBJ_SIZE_32X32, 1, MONSTERS_PAL_ID, -32, -32);
        PA_SetSpritePrio(0, BASE_MONSTERS_ID - i, 2);
        PA_SetSpriteRotEnable(0, BASE_MONSTERS_ID - i, ZOOMSET_MONSTERS);
//        PA_UpdateSpriteGfxAndMem(0, BASE_MONSTERS_ID - i, td->monsters[1].gfx);
    }
    
    for(i=0; i<ATTACKS_INST_MAX; i++) {
        PA_CreateSprite(0, BASE_ATTACK_ID - i, attack_Sprites[0], OBJ_SIZE_16X16, 1, ATTACKS_PAL_ID, -32, -32);
        PA_SetSpritePrio(0, BASE_ATTACK_ID - i, 2);
    }

    for(i=0; i<TOWERS_INST_MAX; i++) {
        PA_CreateSprite(0, BASE_TOWERS_ID - i, (void*)t_all_normal_Sprite, OBJ_SIZE_32X32, 1, 0, -32, -32);
        PA_SetSpritePrio(0, BASE_TOWERS_ID - i, 2);
    }
    
    // Build menu
    PA_CreateSprite(0, BUILD_MENU_ID, (void*)build_menu_Sprite, OBJ_SIZE_64X32, 1, BUILD_MENU_PAL_ID, 128, -32);
    PA_CreateSprite(0, BUILD_MENU_ID+1, (void*)build_menu_Sprite, OBJ_SIZE_64X32, 1, BUILD_MENU_PAL_ID, 128+64, -32);
    PA_SetSpriteAnim(0, BUILD_MENU_ID+1, 1);

    // Create dialog button sprites
    PA_LoadSpritePal(0, SELECTED_PAL_ID, (void*)ingame_menu_selected_Pal);
    PA_CreateSprite(0, SELECTED_ID, (void*)ingame_menu_selected_Sprite, OBJ_SIZE_64X32, 1, SELECTED_PAL_ID, -32, -32);
    PA_CreateSprite(0, SELECTED_ID+1, (void*)ingame_menu_selected_Sprite, OBJ_SIZE_64X32, 1, SELECTED_PAL_ID, -32, -32);
    PA_SetSpriteMode(0, SELECTED_ID, 1);
    PA_SetSpriteMode(0, SELECTED_ID+1, 1);
    PA_SetSpriteAnim(0, SELECTED_ID+1, 1);


    // Load saved game
    if(load_game) {
        loadGame();
        engineActions();
    }

    fadeIn();
    

    // Choose game difficulty
    if(!load_game)
        chooseDifficulty();    
        
    if(load_game)
        load_game = false;

    // Create build tower & grid sprites
    PA_CreateSprite(0, BUILD_OK_ID, (void*)t_select24x24_Sprite, OBJ_SIZE_32X32, 1, BUILD_SELEC_PAL_ID, -32, -32);
    PA_SetSpritePrio(0, BUILD_OK_ID, 2);
    PA_CreateSprite(0, BUILD_TOWER_ID, (void*)t_all_normal_Sprite, OBJ_SIZE_32X32, 1, 0, -32, -32);
    PA_SetSpritePrio(0, BUILD_TOWER_ID, 2);

    // Let's go!
    playSound(MapPing);
    playSound(MapPing);
}


// Main loop
bool startEngine() {

    while(!engineActions());

    shim_SetMapView(0, 0, 0);   // [port] leave extended map view

    if(!retry)
        fadeOut();
    if(interface)
        PA_SwitchScreens();
    if(stats)
        statScreen();
    
    return retry;
}

// Almost everything of the game engine...
// [Android port] on-screen game-speed button (top-left of the map view,
// aligned with the Build/Cancel row): cycles 1x -> 2x -> 4x
#define MENUBTN_X1 4
#define MENUBTN_Y1 3
#define MENUBTN_X2 68
#define MENUBTN_Y2 21
static void drawMenuButton(void) {
    int i, j;
    PA_SetBgPalCol(0, 240, PA_RGB(2, 2, 8));      // dark blue fill
    PA_SetBgPalCol(0, 241, PA_RGB(14, 14, 18));   // border
    PA_SetBgPalCol(0, 242, PA_RGB(31, 26, 8));    // gold text
    for(j=MENUBTN_Y1; j<=MENUBTN_Y2; j++)
        for(i=MENUBTN_X1; i<MENUBTN_X2; i+=2)
            PA_PutDouble8bitPixels(0, i, j, 240, 240);
    for(i=MENUBTN_X1; i<=MENUBTN_X2; i++) {
        PA_Put8bitPixel(0, i, MENUBTN_Y1, 241);
        PA_Put8bitPixel(0, i, MENUBTN_Y2, 241);
    }
    for(j=MENUBTN_Y1; j<=MENUBTN_Y2; j++) {
        PA_Put8bitPixel(0, MENUBTN_X1, j, 241);
        PA_Put8bitPixel(0, MENUBTN_X2, j, 241);
    }
    char spd[8];
    sformat(spd, "%dx", shim_game_speed);
    centerAlignSmartText(0, MENUBTN_X1, 8, MENUBTN_X2, 20, spd, 242, 1, 1);
}

bool engineActions() {

    // clear interface texts
    DMA_Copy(Blank, (void*)PA_DrawBg[1], 256*48, DMA_32NOW); 

    // [Android port] viewport may change every frame (pinch zoom):
    // refresh everything derived from it
    computeBuildMenuLayout();
    minimap_x_windowsize = (((VIEW_W)*minimap_x_factor) >> FIXED_POINT_PRECISION);
    minimap_y_windowsize = (((VIEW_H)*minimap_y_factor) >> FIXED_POINT_PRECISION);
    drawMenuButton();
    
    // empty sound queue
//    playQueue();

    // clear air shadows bg
//    DMA_Copy(Blank, (void*)PA_DrawBg[0], 256*48, DMA_32NOW);
//    PA_16cErase(0);

    // interface interactions
    interfaceInteract();
    checkForSelection();

    // scroll window position
    window_x += (Pad.Held.Right - Pad.Held.Left)*4 + (Pad.Held.A - Pad.Held.Y)*4;
    window_y += (Pad.Held.Down - Pad.Held.Up)*4 + (Pad.Held.B - Pad.Held.X)*4;
    if(window_x < 0) window_x = 0;
    if(window_y < 0) window_y = 0;
    if(window_x > td->map_Width-VIEW_W) window_x = td->map_Width-VIEW_W;
    if(window_y > td->map_Height-VIEW_H) window_y = td->map_Height-VIEW_H;

    // build menu management
    manageBuildMenu();

    // put build sprites
    if(menu) {
        if(build_x >= window_x-32 && build_y >= window_y-32 && build_x < window_x+VIEW_W && build_y < window_y+VIEW_H) {
            PA_SetSpriteXY(0, BUILD_OK_ID, build_x-window_x+8, build_y-window_y+4);
            PA_SetSpriteXY(0, BUILD_TOWER_ID, build_x-window_x+8, build_y-window_y+4);
        } else {
            PA_SetSpriteXY(0, BUILD_OK_ID, -32, -32);
            PA_SetSpriteXY(0, BUILD_TOWER_ID, -32, -32);
        }
    }

    // switch dark palettes
    dark = !dark;

    // zoom effect when building towers
    if(zoom_tower != -1) {
        if(zoom_timer > 0) {
            u16 zoom_size = 256+(128*zoom_timer/15);
            PA_SetRotsetNoAngle(0, ZOOMSET_BUILD_TOWER, zoom_size, zoom_size);
            zoom_timer--;
        } else {
            PA_SetSpriteRotDisable(0, zoom_tower);
            zoom_tower = -1;
        }
    }

    // switch palette if the tower is set to dark variation
    if(menu && build_pal != -1) {
        if(dark)
            PA_SetSpritePal(0, BUILD_TOWER_ID, T_DARK_PAL_ID);
        else
            PA_SetSpritePal(0, BUILD_TOWER_ID, build_pal);
    }
    
    // put the selection round at the right position
    if(selected_tower == -1 && selected_monster == -1) {
        // nothing selected
        PA_SetSpriteXY(0, SELECTION_ID, -64, -64);
    } else if(selected_tower != -1) {
        // a tower is selected
        int pos_x = towers[BASE_TOWERS_ID-selected_tower].pos_x, pos_y = towers[BASE_TOWERS_ID-selected_tower].pos_y;
        if(pos_x >= window_x-42 && pos_y >= window_y-44 && pos_x < window_x+VIEW_W && pos_y < window_y+VIEW_H)
            PA_SetSpriteXY(0, SELECTION_ID, pos_x-window_x-10, pos_y-window_y-12);
        else
            PA_SetSpriteXY(0, SELECTION_ID, -64, -64);
    } else {
        // a monster is selected
        int pos_x = monsters[BASE_MONSTERS_ID-selected_monster].pos_x, pos_y = monsters[BASE_MONSTERS_ID-selected_monster].pos_y;
        if(td->monsters[monsters[BASE_MONSTERS_ID-selected_monster].type].air)
            if(pos_x >= window_x-56 && pos_y >= window_y-56 && pos_x < window_x+VIEW_W && pos_y < window_y+VIEW_H)
                PA_SetSpriteXY(0, SELECTION_ID, pos_x-window_x-24, pos_y-window_y-40);
            else
                PA_SetSpriteXY(0, SELECTION_ID, -64, -64);
        else
            if(pos_x >= window_x-56 && pos_y >= window_y-56 && pos_x < window_x+VIEW_W && pos_y < window_y+VIEW_H)
                PA_SetSpriteXY(0, SELECTION_ID, pos_x-window_x-24, pos_y-window_y-24);
            else
                PA_SetSpriteXY(0, SELECTION_ID, -64, -64);
    }

    // the engine...
    doDarkTowers();
    updateMinimap();
    refreshUI();
    refreshIcons();
    checkForNewRound();
    spawnMonsters();
    monstersEngine();
/*
    sformat(strtmp, "nbs:%d" , nbs);
    SmartText(1, 3, 79, 256, 192, strtmp, 6, 1, 1, 100);
    if(nbs>ms) ms = nbs; nbs=0; 
    sformat(strtmp, "nbs:%d" , ms);
    SmartText(1, 3, 90, 256, 192, strtmp, 6, 1, 1, 100);
*/
    
    towerReveal();
    showMonsters();
    vanishMonsters();
    towersEngine();

    // check if monsters are blocked
    if(blocked) {
        simpleDialog(td->name, 7, S_BLOCKED_MONSTERS, 1, true, S_OK, 1);
        selected_tower = towers[num_towers-1].id;
        sellTower();
        blocked = false;
    }

    // check if it is possible to build tower
    if(menu) {
    
        u8 i, j;
        bool cantbuild = false;
    
        for(j=0; j<3; j++)
            for(i=0; i<3; i++)
                if((t_path[((build_x >> 4)+i) + ((build_y >> 4)+j)*path_width] > 0))
                    cantbuild = true;
    
//        if((t_path[((build_x >> 4)+1) + ((build_y >> 4)+1)*path_width] > 0))
        if(cantbuild)
            PA_SetSpriteAnim(0, BUILD_OK_ID, 1);
        else
            PA_SetSpriteAnim(0, BUILD_OK_ID, 0);
    }

    // check for game over
    if(lifes <= 0)
        gameLose();

    // timers
    vblcount++;
    if(vblcount >= 60) {
        vblcount = 0;
        if(timer > 0)
            timer--;
        total_time++;
        
        // if timed_gold is on
        if(td->timed_gold) {
            if(current_round > 0)
                gold += td->rounds[current_round-1].gold_bonus;
            else
                gold += td->initial_bonus;
        }
    }
    
    // update old stylus position for double tap detection
    stylus_old_x = Stylus.X;
    stylus_old_y = Stylus.Y;
    
    // check if lid was closed
    if(checkLid())
        gamePause();
        
    PA_WaitForVBL();

    // scroll window (after vbl to compensate the delay of sprite scrolling)
    PA_LargeScrollXY(0, 2, window_x, window_y);

    return exit_engine;
}

/**********************************************************************************************************************/

// Display statistics of the game
void statScreen() {

    PA_ResetSpriteSys();
    PA_ResetBgSys();
    
    // calculate score
    int scr = (kills*10 + lifes*100 + gold) * difficulty / 100;
    
    // Show stat screen
    PA_Init8bitBg(0, 1);
    loadTextPalette(0, 0);
    PA_LoadTiledBg(1, 3, title1); 
    initFlashEyes();
    PA_LoadTiledBg(0, 2, stat_screen); 
    
    sformat(strtmp, "%s", PA_UserInfo.Name);
    SmartText(0, 2, 41, 64, 51, strtmp, 1, 1, 1, 20);
    sformat(strtmp, "%d", kills);
    centerAlignSmartText(0, 88, 41, 156, 51, strtmp, 1, 1, 1);
    sformat(strtmp, "%d", scr);
    centerAlignSmartText(0, 162, 41, 230, 51, strtmp, 1, 1, 1);
    SmartText(0, 3, 128, 80, 138, S_TOTAL_TIME, 7, 1, 1, 20);
    sformat(strtmp, "%02d:%02d:%02d", total_time/(60*60), total_time/60, total_time%60);
    SmartText(0, 77, 128, 255, 138, strtmp, 1, 1, 1, 20);

    // get previous high score if exists
    score sc_info;
    
    if(searchAndGetScore(curr_map_size, curr_map_sum, &sc_info)) {
    
        if(~sc_info.score < scr) {
            SmartText(0, 3, 147, 255, 158, S_NEW_HIGHSCORE, 14, 1, 1, 40);
        } else {
            SmartText(0, 3, 147, 255, 158, S_CURR_HIGHSCORE, 14, 1, 1, 40);
            sformat(strtmp, "%d", ~sc_info.score);
            SmartText(0, 130, 147, 255, 158, strtmp, 1, 1, 1, 20);
        }    
    }
    
    // set new score
    sc_info.valid = true;
    sc_info.finished = (current_round+1 == td->numRounds);

    if(sc_info.finished)
        sc_info.finish_diff = difficulty_value;
    else
        sc_info.finish_diff = 0;
        
    sc_info.score = ~scr;
    sc_info.score_diff = difficulty_value;    
    sc_info.map_size = curr_map_size;
    sc_info.map_sum = curr_map_sum;
    setScore(sc_info);
    
    fadeIn();
    
    while(1) {
    
        PA_WaitForVBL();
        FlashEyes();
        
        if(Stylus.Newpress && PA_StylusInZone(80, 165, 155, 187)) {
            PA_LoadSpritePal(0, 1, (void*)stat_screen_selected_Pal);
            PA_CreateSprite(0, 0, (void*)stat_screen_selected_Sprite, OBJ_SIZE_32X16, 1, 1, 87, 170);
            PA_CreateSprite(0, 1, (void*)stat_screen_selected_Sprite, OBJ_SIZE_32X16, 1, 1, 87+32, 170);
            PA_CreateSprite(0, 2, (void*)stat_screen_selected_Sprite, OBJ_SIZE_32X16, 1, 1, 87+32, 170);
            PA_SetSpriteMode(0, 0, 1);
            PA_SetSpriteMode(0, 1, 1);
            PA_SetSpriteMode(0, 2, 1);
            PA_SetSpriteAnim(0, 1, 1);
            PA_SetSpriteAnim(0, 2, 2);
//            PA_EnableSpecialFx(0, SFX_ALPHA, 0, SFX_BG0 | SFX_BG1 | SFX_BG2 | SFX_BG3 | SFX_BD); 
            PA_SetSFXAlpha(0, 5, 15);
            
            while(Stylus.Held) {
                PA_WaitForVBL();
                FlashEyes();
            }
            
            PA_DeleteSprite(0, 0);
            PA_DeleteSprite(0, 1);
            PA_DeleteSprite(0, 2);
            playSound(BigButtonClick);
            
            retry = true;
            break;
            
        } else if(Stylus.Newpress && PA_StylusInZone(158, 165, 234, 187)) {
            PA_LoadSpritePal(0, 1, (void*)stat_screen_selected_Pal);
            PA_CreateSprite(0, 0, (void*)stat_screen_selected_Sprite, OBJ_SIZE_32X16, 1, 1, 162, 170);
            PA_CreateSprite(0, 1, (void*)stat_screen_selected_Sprite, OBJ_SIZE_32X16, 1, 1, 162+32, 170);
            PA_CreateSprite(0, 2, (void*)stat_screen_selected_Sprite, OBJ_SIZE_32X16, 1, 1, 162+32, 170);
            PA_SetSpriteMode(0, 0, 1);
            PA_SetSpriteMode(0, 1, 1);
            PA_SetSpriteMode(0, 2, 1);
            PA_SetSpriteAnim(0, 1, 1);
            PA_SetSpriteAnim(0, 2, 2);
//            PA_EnableSpecialFx(0, SFX_ALPHA, 0, SFX_BG0 | SFX_BG1 | SFX_BG2 | SFX_BG3 | SFX_BD); 
            PA_SetSFXAlpha(0, 5, 15);
            
            while(Stylus.Held) {
                PA_WaitForVBL();
                FlashEyes();
            }
            
            PA_DeleteSprite(0, 0);
            PA_DeleteSprite(0, 1);
            PA_DeleteSprite(0, 2);
            playSound(BigButtonClick);
            break;
        }
    }
    
    if(retry == false)
        fadeOut();
}

/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/

// Animate a monster
void monsterAnim(u8 num) {

    if(monsters[num].wait != 0) {
        if(!monsters[num].dead) {
            if(monsters[num].dir == NONE) {
                monsters[num].cycle = 0;
                monsters[num].reverse = false;
                monsters[num].step = false;
                if(monsters[num].last_dir != NONE) 
                    PA_SetSpriteAnimEx(0, monsters[num].id, 32, 32, 1, monsters[num].last_dir + 4*monsters[num].cycle);
            } else {
                PA_SetSpriteAnimEx(0, monsters[num].id, 32, 32, 1, monsters[num].dir + 4*monsters[num].cycle);
            }
        }
        monsters[num].wait--;

    } else {
    
        monsters[num].wait = td->monsters[monsters[num].type].anim_speed;

        // test if the monster is alive
        if(monsters[num].life > 0) {
    
            if(monsters[num].dir == NONE) {
                monsters[num].cycle = 0;
                monsters[num].reverse = false;
                monsters[num].step = false;
                PA_SetSpriteAnimEx(0, monsters[num].id, 32, 32, 1, monsters[num].last_dir + 4*monsters[num].cycle);
            
            } else {

                if(td->monsters[monsters[num].type].no_anim) {
                    monsters[num].cycle = 0;
                    monsters[num].reverse = false;
                    monsters[num].step = false;
                    PA_SetSpriteAnimEx(0, monsters[num].id, 32, 32, 1, monsters[num].dir + 4*monsters[num].cycle);

                } else  {
                
                    if(monsters[num].cycle == 0) {
                        monsters[num].reverse = false;
                        monsters[num].step = !monsters[num].step;
                    }

                    if(monsters[num].reverse) {
                        PA_SetSpriteAnimEx(0, monsters[num].id, 32, 32, 1, monsters[num].dir + 4*monsters[num].cycle);
                        monsters[num].cycle--;
                    } else {
                        PA_SetSpriteAnimEx(0, monsters[num].id, 32, 32, 1, monsters[num].dir+ 4*monsters[num].cycle);
                        monsters[num].cycle++;
                    }
        
                    if(monsters[num].cycle == 5 && monsters[num].step)
                        monsters[num].cycle = 0;
                    else if(monsters[num].cycle == 3 && !monsters[num].step)
                        monsters[num].cycle = 0;
                    else if(monsters[num].cycle == 1 && monsters[num].step)
                        monsters[num].cycle = 3;
                        
                }
            }
            
        } else {
        
            // death anim base frame
            u8 base_dead_cycle = 5;
        
            if(td->monsters[monsters[num].type].no_anim)
                base_dead_cycle = 1;
        
            // set to play dead anim once
            if(!monsters[num].dead) {
                monsters[num].dead = true;
                monsters[num].cycle = base_dead_cycle;
                deadmonsters++;
                // bonus for killing the monster if it's not a loose life case
                if(monsters[num].wp != num_waypoints[0]) {
                    gold += td->monsters[monsters[num].type].gold_bonus;
                    kills++;
                }
                
                if(selected_monster == monsters[num].id)
                    selected_monster = -1;

                // death sound
                if(monsters[num].pos_x >= window_x-32 && monsters[num].pos_y >= window_y-32 && monsters[num].pos_x < window_x+VIEW_W && monsters[num].pos_y < window_y+VIEW_H+32) {
                    // stereo placement
                    int x = (monsters[num].pos_x-window_x)*128/VIEW_W, y = ((monsters[num].pos_y-window_y)*32/VIEW_H)-16;
                    if(x>127)
                        x = 127;
                    if(x<0)
                        x = 0;
                    if(y<0)
                        y = -y;
                    soundMix(td->monsters[monsters[num].type].sound, td->monsters[monsters[num].type].sound_size, SND_DEATH,70-y, x);
                }
                
                // place is now buildable
                t_path[monsters[num].last_x + monsters[num].last_y*path_width]--;
            }
        
            if(monsters[num].cycle < base_dead_cycle+td->monsters[monsters[num].type].death_anims) {
                if(monsters[num].last_dir != NONE) 
                    PA_SetSpriteAnimEx(0, monsters[num].id, 32, 32, 1, monsters[num].last_dir + 4*monsters[num].cycle);
                else
                    PA_SetSpriteAnimEx(0, monsters[num].id, 32, 32, 1, UP + 4*monsters[num].cycle);
                monsters[num].cycle++;
            }
        }
    }
}

/**********************************************************************************************************************/

// AI, animation & movement for a monster
void monsterEngine(u8 m) {

    u8 path = monsters[m].path;

    if(monsters[m].life > 0) {

        // hide monster if invisible
        if(td->monsters[monsters[m].type].invisible)
            monsters[m].visible = false;

        // save last good direction
        if(monsters[m].dir != NONE)
            monsters[m].last_dir = monsters[m].dir;

        // reinit steps if needed
        if(monsters[m].steps == 16)
            monsters[m].steps = 0;

        if(monsters[m].steps == 0) {

            // test if a waypoint has been reached
            if((monsters[m].pos_x / 16) == waypoints[path][monsters[m].wp] && (monsters[m].pos_y / 16) == waypoints[path][monsters[m].wp+1])
                monsters[m].wp += 2;
            
                // if the goal has been reached
                if(monsters[m].wp == num_waypoints[path]) {
                    monsters[m].dir = NONE;
                    monsters[m].life = 0;
                    lifes--;
                }
            
            else {
                // find the new direction to take
                if(td->fast_pathfinder)
                    monsters[m].dir = pathfinder(path_width, path_height, m_path, monsters[m].pos_x >> 4, monsters[m].pos_y >> 4, waypoints[path][monsters[m].wp], waypoints[path][monsters[m].wp+1], td->monsters[monsters[m].type].air, monsters[m].last_dir, monsters[m].ai_count);
                else
                    monsters[m].dir = adv_pathfinder(path_width, path_height, m_path, monsters[m].pos_x >> 4, monsters[m].pos_y >> 4, waypoints[path][monsters[m].wp], waypoints[path][monsters[m].wp+1], td->monsters[monsters[m].type].air, monsters[m].last_dir, monsters[m].ai_count);
                monsters[m].ai_count = monsters[m].ai_count+1;
                
                if(monsters[m].dir == NONE) {
                    blocked = true;
                }
                
//                nbs++;
            }
        }
        
    } else {
        monsters[m].dir = NONE;
    }

    // special effect palettes
    monsters[m].transp = !monsters[m].transp;
    if(monsters[m].transp) {
        if(monsters[m].slow_time > 0)
            PA_SetSpritePal(0, monsters[m].id, M_FROZ_PAL_ID);
        else if(monsters[m].poison_time > 0)
            PA_SetSpritePal(0, monsters[m].id, M_POIS_PAL_ID);
    } else {
        PA_SetSpritePal(0, monsters[m].id, MONSTERS_PAL_ID);
    }

    // cancel special effects if the monster is dead
    if(monsters[m].dead) {
        monsters[m].slow_time = 0;
        monsters[m].poison_time = 0;
    }

    if(monsters[m].slow_time > 0)
        monsters[m].slow_time--;

    if(monsters[m].poison_time > 0) {
        monsters[m].poison_time--;
        if(monsters[m].poison == 0) {
            monsters[m].poison = 60;
            monsters[m].life -= rand(monsters[m].poison_param1, monsters[m].poison_param2);
        } else {
            monsters[m].poison--;
        }
    
    }
        
    // move the monster
    if(monsters[m].slow_time > 0 && monsters[m].slow > 0 && monsters[m].slow2 == 0) {
        monsters[m].slow--;
    } else {
    
        if(monsters[m].slow_time > 0) {
            if(monsters[m].slow2 > 0) {
                monsters[m].slow2--;
                monsters[m].slow2 = monsters[m].slow_param2;
            }
            monsters[m].slow = monsters[m].slow_param1;
        }
    
        // animate the monster
        monsterAnim(m);
        
        int i;
        for(i=0; i < td->monsters[monsters[m].type].speed; i++) {
            monsters[m].steps++;
            switch(monsters[m].dir) {
                case LEFT:
                    monsters[m].pos_x--;
                    break;
                case RIGHT:
                    monsters[m].pos_x++;
                    break;
                case UP:
                    monsters[m].pos_y--;
                    break;
                case DOWN:
                    monsters[m].pos_y++;
                    break;
                case NONE:
                    break;
            }
        }
    }
    
    int curr_x = (monsters[m].pos_x+8) / 16;
    int curr_y = (monsters[m].pos_y+8) / 16;
    
    // Forbid to build a tower on monster's current position
    if((curr_x != monsters[m].last_x) || (curr_y != monsters[m].last_y)) {
        
        t_path[curr_x + curr_y*path_width]++;
        
        if((monsters[m].last_x != -1) && (monsters[m].last_y != -1))
            t_path[monsters[m].last_x + monsters[m].last_y*path_width]--;
    
        monsters[m].last_x = curr_x;
        monsters[m].last_y = curr_y;
    }
    
/*    
    // display or not the sprite
    if(monsters[m].visible && monsters[m].pos_x >= window_x-32 && monsters[m].pos_y >= window_y-32 && monsters[m].pos_x < window_x+VIEW_W && monsters[m].pos_y < window_y+VIEW_H) {
        if(td->monsters[monsters[m].type].air) {
            PA_SetSpriteXY(0, monsters[m].id, monsters[m].pos_x-window_x-8, monsters[m].pos_y-window_y-16);
        } else {
            PA_SetSpriteXY(0, monsters[m].id, monsters[m].pos_x-window_x-8, monsters[m].pos_y-window_y-8);
        }
    } else {
        PA_SetSpriteXY(0, monsters[m].id, -32, -32);
    }
*/
}

/**********************************************************************************************************************/

// display monsters
void showMonsters() {
    int m;
    for(m=0; m < num_monsters; m++) {
        // display or not the sprite
        if(!(monsters[m].dead && monsters[m].cycle == 5+td->monsters[monsters[m].type].death_anims && td->monsters[monsters[m].type].hide_corpse) && monsters[m].visible && monsters[m].pos_x >= window_x-32 && monsters[m].pos_y >= window_y-32 && monsters[m].pos_x < window_x+VIEW_W && monsters[m].pos_y < window_y+VIEW_H+32) {
            if(td->monsters[monsters[m].type].air) {
                PA_SetSpriteXY(0, monsters[m].id, monsters[m].pos_x-window_x-8, monsters[m].pos_y-window_y-24);
//                if(monsters[m].life > 0)
//                    drawAirShadow(monsters[m].pos_x-window_x+4, monsters[m].pos_y-window_y+4);
            } else {
                PA_SetSpriteXY(0, monsters[m].id, monsters[m].pos_x-window_x-8, monsters[m].pos_y-window_y-8);
            }
        } else {
            PA_SetSpriteXY(0, monsters[m].id, -32, -32);
        }
    }
}

/**********************************************************************************************************************/

// AI, animation & movement for all monsters
void monstersEngine() {
    int i;
    for(i=0; i < num_monsters; i++)
        monsterEngine(i);
}

/**********************************************************************************************************************/

// Update the minimap display
void updateMinimap() {

    int i, x, y, x2, y2, x_base, y_base, x_factor, y_factor;
    u8 c1 = minimap_colorsets[td->minimap_colors*5];
    u8 c2 = minimap_colorsets[td->minimap_colors*5+1];
    u8 c3 = minimap_colorsets[td->minimap_colors*5+2];
    u8 c4 = minimap_colorsets[td->minimap_colors*5+3];
    u8 c5 = minimap_colorsets[td->minimap_colors*5+4];
    
    // localize the precalculated values to speed things up
    x_base = minimap_x_base;
    y_base = minimap_y_base;
    x_factor = minimap_x_factor;
    y_factor = minimap_y_factor;

/*    // spawn point
    x = MINIMAP_X+minimap_x_adjust+((td->spawn_x)*(MINIMAP_WIDTH-minimap_x_adjust*2)/td->map_Width);
    y = MINIMAP_Y+minimap_y_adjust+((td->spawn_y)*(MINIMAP_HEIGHT-minimap_y_adjust*2)/td->map_Height);
    PA_PutDouble8bitPixels (1, x, y, 1, 1);
    PA_PutDouble8bitPixels (1, x, y+1, 1, 1);

    // finish point
    x = MINIMAP_X+minimap_x_adjust+((waypoints[num_waypoints-2]*16)*(MINIMAP_WIDTH-minimap_x_adjust*2)/td->map_Width);
    y = MINIMAP_Y+minimap_y_adjust+((waypoints[num_waypoints-1]*16)*(MINIMAP_HEIGHT-minimap_y_adjust*2)/td->map_Height);
    PA_PutDouble8bitPixels (1, x, y, 3, 3);
    PA_PutDouble8bitPixels (1, x, y+1, 3, 3);    

    // draw monsters on minimap
    for(i=0; i < num_monsters; i++) {
        if(monsters[i].life > 0 && monsters[i].visible) {
            PA_Put8bitPixel(1, MINIMAP_X+minimap_x_adjust+((monsters[i].pos_x+8)*(MINIMAP_WIDTH-minimap_x_adjust*2)/td->map_Width), MINIMAP_Y+minimap_y_adjust+((monsters[i].pos_y+8)*(MINIMAP_HEIGHT-minimap_y_adjust*2)/td->map_Height), 1);
        }
    }
    
    // draw towers on minimap
    for(i=0; i < num_towers; i++) {
        x = MINIMAP_X+minimap_x_adjust+((towers[i].pos_x+24)*(MINIMAP_WIDTH-minimap_x_adjust*2)/td->map_Width);
        y = MINIMAP_Y+minimap_y_adjust+((towers[i].pos_y+24)*(MINIMAP_HEIGHT-minimap_y_adjust*2)/td->map_Height);
        PA_PutDouble8bitPixels (1, x, y, 2, 2);
        PA_PutDouble8bitPixels (1, x, y+1, 2, 2);
    }
*/

    // spawn point
    for(i=0; i < num_paths; i++) {
        x = x_base+(((waypoints[i][0]*16)*x_factor) >> FIXED_POINT_PRECISION);
        y = y_base+(((waypoints[i][1]*16)*y_factor) >> FIXED_POINT_PRECISION);
        PA_PutDouble8bitPixels (1, x, y, c1, c1);
        PA_PutDouble8bitPixels (1, x, y+1, c1, c1);
    }

    // finish point
    x = x_base+(((waypoints[0][num_waypoints[0]-2]*16)*x_factor) >> FIXED_POINT_PRECISION);
    y = y_base+(((waypoints[0][num_waypoints[0]-1]*16)*y_factor) >> FIXED_POINT_PRECISION);
    PA_PutDouble8bitPixels (1, x, y, c2, c2);
    PA_PutDouble8bitPixels (1, x, y+1, c2, c2);    

    // draw monsters on minimap
    for(i=0; i < num_monsters; i++) {
        if(monsters[i].life > 0 && monsters[i].visible) {
            PA_Put8bitPixel(1, x_base+(((monsters[i].pos_x+8)*x_factor) >> FIXED_POINT_PRECISION), y_base+(((monsters[i].pos_y+8)*y_factor) >> FIXED_POINT_PRECISION), c3);
        }
    }
    
    // draw towers on minimap
    for(i=0; i < num_towers; i++) {
        x = x_base+(((towers[i].pos_x+24)*x_factor) >> FIXED_POINT_PRECISION);
        y = y_base+(((towers[i].pos_y+24)*y_factor) >> FIXED_POINT_PRECISION);
        PA_PutDouble8bitPixels (1, x, y, c4, c4);
        PA_PutDouble8bitPixels (1, x, y+1, c4, c4);
    }
    
    // draw screen rectangle
    x = x_base+((window_x*x_factor) >> FIXED_POINT_PRECISION);
    y = y_base+((window_y*y_factor) >> FIXED_POINT_PRECISION);
    x2 = x+minimap_x_windowsize;
    y2 = y+minimap_y_windowsize;
    PA_Draw8bitLine(1, x, y, x2, y, c5);
    PA_Draw8bitLine(1, x, y2, x2, y2, c5);
    PA_Draw8bitLine(1, x, y, x, y2, c5);
    PA_Draw8bitLine(1, x2, y, x2, y2, c5);
}

/**********************************************************************************************************************/

// Calculate min & max damage for selected tower
void calcDamages(tower* tow, int* min, int* max) {

    *min = tow->minDamage;
    *max = tow->maxDamage;

    // tower special effects
    int i;
    for(i=0; i < tow->num_sp; i++) {

        // fire magic
        if(tow->special[i] == FIRE) {
            *min += tow->spParams[i][0];
            *max += tow->spParams[i][1];
        }
        // water magic
        else if(tow->special[i] == WATER) {
            *min += tow->spParams[i][0];
            *max += tow->spParams[i][1];
        }
        // lightning magic
        else if(tow->special[i] == LIGHTNING) {
            *min += tow->spParams[i][0];
            *max += tow->spParams[i][1];
        }
        // wind magic
        else if(tow->special[i] == WIND) {
            *min += tow->spParams[i][0];
            *max += tow->spParams[i][1];
        }
    }
}

// Draw special effects icons
void drawSpIcons(u8 num, u8* sp) {
    int i, j, x = 124-((num)*10);
    for(i=0; i<num; i++) {
        for(j=SLOW; j<=IM_MAGIC; j++) {
            if(sp[i] == j) {
                PA_CreateSprite(1, BASE_SP_ICONS_ID+num_sp_icons, (void*)icons_dark_Sprite, OBJ_SIZE_16X16, 1, ICONS_DARK_PAL_ID, x+(20*num_sp_icons), 174);
                PA_SetSpriteAnim(1, BASE_SP_ICONS_ID+num_sp_icons, 115-SLOW+j);
                num_sp_icons++;
                break;
            }
        }
    }
}

// Refresh UI
void refreshUI() {

    // gold & number of towers
    sformat(strtmp, "%d/%d", num_towers, max_towers);
    rightAlignSmartText(1, 197, 7, 245, 17, strtmp, 1, 1, 1);
    sformat(strtmp, "%d", gold);
    rightAlignSmartText(1, 116, 7, 166, 17, strtmp, 1, 1, 1);
    
    // title
    SmartText(1, 3, 27, 139, 37, td->name, 7, 1, 1, MAP_NAME_MAX);

    // timer
    if(timer == 0 && !new_round) {
        SmartText(1, 148, 32, 249, 42, S_ROUND_STARTED, 7, 1, 1, 20);
    } else {
        sformat(strtmp, "%02d:%02d", timer/60, timer%60);
        rightAlignSmartText(1, 192, 32, 249, 42, strtmp, 7, 1, 1);
        SmartText(1, 148, 32, 216, 42, S_NEXT_ROUND, 7, 1, 1, 20);
    }
        
    // kills
    SmartText(1, 148, 44, 249, 50, S_KILLS, 1, 0, 1, 10);
    SmartText(1, 148, 50, 249, 56, (char*)PA_UserInfo.Name, 2, 0, 1, 20);
    sformat(strtmp, "    %d", kills);
    rightAlignSmartText(1, 200, 50, 249, 56, strtmp, 2, 0, 1);

    // lives
    SmartText(1, 148, 64, 249, 70, S_LIVES, 1, 0, 1, 10);
    SmartText(1, 148, 72, 249, 78, (char*)PA_UserInfo.Name, 2, 0, 1, 20);
    sformat(strtmp, "    %d", lifes);
    rightAlignSmartText(1, 200, 72, 249, 78, strtmp, 2, 0, 1);

    // round information, comments & next round
    if(current_round == -1) {
        SmartText(1, 3, 41, 139, 71, td->welcome, 1, 1, 1, WELCOME_MAX);
    } else {

        if(timer == 0 && !new_round) {
            sformat(strtmp, S_CURRENT_ROUND, current_round+1);
            SmartText(1, 3, 41, 139, 51, strtmp, 6, 1, 1, 20);
            SmartText(1, 3, 51, 139, 61, td->monsters[td->rounds[current_round].type].name, 1, 1, 1, NAME_MAX);
            SmartText(1, 3, 64, 139, 82, td->rounds[current_round].comment, 1, 0, 1, COMMENTS_MAX);
        } else {
            SmartText(1, 3, 41, 139, 51, S_NEXT_ROUND_2, 6, 1, 1, 16);
            SmartText(1, 3, 51, 139, 61, td->monsters[td->rounds[current_round+1].type].name, 1, 1, 1, NAME_MAX);
        }
    }

    // message
    if(msg_timer > 0) {
        msg_timer--;
        SmartText(1, 3, 82, 139, 102, msg, 12, 1, 1, 255);
    }

    // clear sp icons
    int i;
    for(i=0; i<num_sp_icons; i++)
        PA_DeleteSprite(1, BASE_SP_ICONS_ID+i);
    num_sp_icons = 0;

    // show if a new evolution is available
    if(evo_timer > 0) {
        if(evo_timer%64 > 24)
            centerAlignSmartText(1, 172, 98, 254, 104, S_EVO_AVAILABLE, 15, 0, 1);
        evo_timer--;
    }

    // current time
    sformat(strtmp, "%02d:%02d", PA_RTC.Hour, PA_RTC.Minutes);
    rightAlignSmartText(1, 215, 112, 255, 118, strtmp, 6, 0, 1);

    // show tips box
    if(boxhidden) {
        PA_DeleteSprite(1, HIDE_BOX_ID);
        PA_DeleteSprite(1, HIDE_BOX_ID+1);
        PA_DeleteSprite(1, HIDE_BOX_ID+2);
        boxhidden = false;
    }

    // selected tower / monster information
    if(!menu) {
    
        if(selected_clan != -1) {

            sformat(strtmp, S_CONFIRM);
            SmartText(1, 172, 112, 225, 118, strtmp, 11, 0, 0, 20);
            centerAlignSmartText(1, 78, 111, 165, 121, td->clans[selected_clan].name, 6, 1, 1);
            centerAlignSmartText(1, 85, 144, 159, 187, td->clans[selected_clan].info, 1, 0, 1);

        } else if(selected_tower != -1) {
        
            tower* sel_t;
            int dmg_min, dmg_max;

            if(upgrade_to == -1) {
                sel_t = &(td->towers[towers[BASE_TOWERS_ID-selected_tower].type]);
                if(to_sell != -1) {
                    sformat(strtmp, S_SELL);
                    SmartText(1, 172, 106, 254, 112, strtmp, 11, 0, 1, 20);
                    sformat(strtmp, S_PRICE, sel_t->price*td->sell_pct/100);
                    SmartText(1, 172, 112, 225, 118, strtmp, 11, 0, 1, 20);
                }
            } else {

                u8 new_upg_to = upgrade_to;
                if(new_upg_to >= 8)
                    new_upg_to -= 8;
            
                sel_t = &(td->towers[td->towers[towers[BASE_TOWERS_ID-selected_tower].type].upgrades[new_upg_to]]);
                sformat(strtmp, S_UPGRADE);
                SmartText(1, 172, 106, 254, 112, strtmp, 11, 0, 1, 20);
                sformat(strtmp, S_PRICE, sel_t->price);
                SmartText(1, 172, 112, 225, 118, strtmp, 11, 0, 1, 20);
            }
            centerAlignSmartText(1, 78, 111, 165, 121, sel_t->name, 6, 1, 1);
            
            if(desc) {
                centerAlignSmartText(1, 85, 144, 159, 180, sel_t->desc, 1, 0, 1);
                if(sel_t->air) {
                    if(sel_t->ground)
                        strcopy(strtmp, S_ATTACKS_ALL);
                    else
                        strcopy(strtmp, S_ATTACKS_AIR_ONLY);
                } else {
                    if(sel_t->ground)
                        strcopy(strtmp, S_ATTACKS_GROUND_ONLY);
                    else
                        strcopy(strtmp, S_DONT_ATTACK);
                }
                centerAlignSmartText(1, 84, 181, 159, 187, strtmp, 1, 0, 1);
            } else {
                calcDamages(sel_t, &dmg_min, &dmg_max);
                sformat(strtmp, S_TOWER_INFOS, dmg_min, dmg_max, (540/sel_t->reload)*sel_t->speed, sel_t->range);
                centerAlignSmartText(1, 80, 142, 164, 172, strtmp, 1, 1, 1);
                // draw special effects icons
                drawSpIcons(sel_t->num_sp, sel_t->special);
            }
            
        } else if(selected_monster != -1) {
            monster* sel_m = &(td->monsters[monsters[BASE_MONSTERS_ID-selected_monster].type]);
            centerAlignSmartText(1, 78, 111, 165, 121, sel_m->name, 6, 1, 1);
            sformat(strtmp, S_MONSTER_INFOS, monsters[BASE_MONSTERS_ID-selected_monster].life, sel_m->life * difficulty / 100, sel_m->armor);
            centerAlignSmartText(1, 80, 142, 164, 172, strtmp, 1, 1, 1);
            // draw special effects icons
            drawSpIcons(sel_m->num_immunes, sel_m->immune);
        } else {
            if(selected_evo != -1) {
                centerAlignSmartText(1, 78, 111, 165, 121, evolutions[selected_evo].name, 6, 1, 1);
                centerAlignSmartText(1, 84, 145, 159, 187, evolutions[selected_evo].info, 1, 0, 1);
            } else if(upgrade_base != -1) {
                sformat(strtmp, S_UPGRADE);
                SmartText(1, 172, 106, 254, 112, strtmp, 11, 0, 1, 20);
                sformat(strtmp, S_PRICE, evolutions[current_evo+1].price);
                SmartText(1, 172, 112, 225, 118, strtmp, 11, 0, 1, 20);

                centerAlignSmartText(1, 78, 111, 165, 121, evolutions[current_evo+1].name, 6, 1, 1);
                centerAlignSmartText(1, 84, 145, 159, 187, evolutions[current_evo+1].info, 1, 0, 1);
            } else {
                PA_CreateSprite(1, HIDE_BOX_ID, (void*)hide_box_Sprite, OBJ_SIZE_32X32, 1, HIDE_BOX_PAL_ID, 77, 107);
                PA_CreateSprite(1, HIDE_BOX_ID+1, (void*)hide_box_Sprite, OBJ_SIZE_32X32, 1, HIDE_BOX_PAL_ID, 77+32, 107);
                PA_CreateSprite(1, HIDE_BOX_ID+2, (void*)hide_box_Sprite, OBJ_SIZE_32X32, 1, HIDE_BOX_PAL_ID, 77+64, 107);
                PA_SetSpriteAnim(1, HIDE_BOX_ID+1, 1);
                PA_SetSpriteAnim(1, HIDE_BOX_ID+2, 2);
                boxhidden = true;
            }
        }
    } else {
        int dmg_min, dmg_max;
        tower* sel_t = &(td->towers[towerToBuild]);
        centerAlignSmartText(1, 78, 111, 165, 121, sel_t->name, 6, 1, 1);
        sformat(strtmp, S_PRICE, sel_t->price);
        SmartText(1, 172, 112, 225, 118, strtmp, 11, 0, 1, 20);
        
        if(desc) {
            centerAlignSmartText(1, 85, 144, 159, 180, sel_t->desc, 1, 0, 1);
            if(sel_t->air) {
                if(sel_t->ground)
                    strcopy(strtmp, S_ATTACKS_ALL);
                else
                    strcopy(strtmp, S_ATTACKS_AIR_ONLY);
            } else {
                if(sel_t->ground)
                    strcopy(strtmp, S_ATTACKS_GROUND_ONLY);
                else
                    strcopy(strtmp, S_DONT_ATTACK);
            }
            centerAlignSmartText(1, 84, 181, 159, 187, strtmp, 1, 0, 1);
        } else {
            calcDamages(sel_t, &dmg_min, &dmg_max);
            sformat(strtmp, S_TOWER_INFOS, dmg_min, dmg_max, (540/sel_t->reload)*sel_t->speed, sel_t->range);
            centerAlignSmartText(1, 80, 142, 164, 172, strtmp, 1, 1, 1);
            // draw special effects icons
            drawSpIcons(sel_t->num_sp, sel_t->special);
        }
    }
    
}

/**********************************************************************************************************************/
// Spawn a monster
void spawnMonster(u8 num) {

    int i;

    monsters[num].type = td->rounds[current_round].type;
    monsters[num].pos_x = waypoints[current_path][0]*16;
    monsters[num].pos_y = waypoints[current_path][1]*16;
    monsters[num].last_x = -1;
    monsters[num].last_y = -1;
    monsters[num].life = td->monsters[monsters[num].type].life * difficulty / 100;

    if(monsters[num].life <= 0)
        monsters[num].life = 1;

    monsters[num].id = BASE_MONSTERS_ID - num;
    monsters[num].wait = 0;
    monsters[num].reverse = false;
    monsters[num].dead = false;
    monsters[num].step = false;
    monsters[num].transp = false;
    monsters[num].visible = true;
    monsters[num].slow = 0;
    monsters[num].slow2 = 0;
    monsters[num].slow_time = 0;
    monsters[num].slow_param1 = 0;
    monsters[num].slow_param2 = 0;
    monsters[num].poison = 0;
    monsters[num].poison_time = 0;
    monsters[num].poison_param1 = 0;
    monsters[num].poison_param2 = 0;
    monsters[num].dir = NONE;
    monsters[num].last_dir = NONE;
    monsters[num].steps = 0;
    monsters[num].ai_count = 0;
    monsters[num].wp = 0;
    monsters[num].cycle = 0;
    monsters[num].path = current_path;

    num_spawned[current_path]++;
    if(num_monsters < MONSTERS_INST_MAX)
        num_monsters++;
    
    // multiple spawn management
    if(num_paths > 1) {
        switch(td->rounds[current_round].mps_method) {
            case ALTERNATE:
                for(i=0; i<num_paths; i++) {
                    current_path = (current_path+1) % num_paths;
                    if(num_spawned[current_path] < td->rounds[current_round].number[current_path])
                        break;
                }
                break;
            case SIMULTANEOUS:
                for(i=0; i<num_paths; i++) {
                    current_path = (current_path+1) % num_paths;
                    if(num_spawned[current_path] < td->rounds[current_round].number[current_path])
                        break;
                }
                if(current_path != 0)
                    spawntimer = 1;
                break;
            case ORDERED:
                if(num_spawned[current_path] == td->rounds[current_round].number[current_path])
                    for(i=0; i<num_paths; i++) {
                        current_path = (current_path+1) % num_paths;
                        if(num_spawned[current_path] < td->rounds[current_round].number[current_path])
                            break;
                    }
                break;
        }
    }
    
    // manage monster size (fix sprite glitch)
    if(td->monsters[monsters[num].type].size == 0) {
        PA_SetSpriteRotDisable(0, BASE_MONSTERS_ID - num);
    } else {
        PA_SetSpriteRotEnable(0, BASE_MONSTERS_ID - num, ZOOMSET_MONSTERS);
    }
    
    // update gfx
    PA_LoadSpritePal(0, MONSTERS_PAL_ID, monster_pals[td->monsters[td->rounds[current_round].type].pal-8]);
    PA_UpdateSpriteGfxAndMem(0, monsters[num].id, td->monsters[td->rounds[current_round].type].gfx);

    // activate transparency if ghost mode is set
    if(td->monsters[td->rounds[current_round].type].ghost)
        PA_SetSpriteMode(0, monsters[num].id, 1);
    else
        PA_SetSpriteMode(0, monsters[num].id, 0);

}

// Spawn monsters
void spawnMonsters() {

    int i, num;
    bool spawn_finished = true;     // is spawn finished ?

    for(i=0; i < num_paths; i++)
        if(num_spawned[i] != td->rounds[current_round].number[i]) {
            spawn_finished = false;
            break;
        }

    if(spawntimer == 0)
        spawntimer = td->rounds[current_round].spawn_rate[current_path];
    else
        spawntimer--;

    if(m_vanish == -1 && timer == 0 && spawntimer == 0 && !new_round && !spawn_finished) {

        if(num_monsters < MONSTERS_INST_MAX) {
            spawnMonster(num_monsters);
        } else {
            // search for a dead monster
            num = -1;
            for(i=num_monsters-1; i>=0; i--)
                if(monsters[i].dead) {
                    num = i;
                    break;
                }
            // spawn a monster if possible
            if(num != -1)
                spawnMonster(num);
        }
    }

/*    int test = 0;
    for(i=num_monsters-1; i>=0; i--)
        if(monsters[i].life <= 0)
            test++;

    sformat(strtmp, "spf:%d, cp:%d, num:%d, d:%d\nded:%d" , spawn_finished, spawntimer,
        current_path, num_spawned[0], deadmonsters);
    SmartText(1, 3, 70, 256, 192, strtmp, 6, 1, 1, 100);
*/

}

/**********************************************************************************************************************/

// find a monster to attack, returns monster id or -1 if no monster to attack
int findMonsterToAttack(u16 from_x, u16 from_y, u16 range, bool ground, bool air) {
    int i;
    for(i=0; i < num_monsters; i++)
        if(monsters[i].life > 0 && monsters[i].visible && ((td->monsters[monsters[i].type].air && air) || (!td->monsters[monsters[i].type].air && ground)))
            if(PA_Distance(from_x, from_y, monsters[i].pos_x, monsters[i].pos_y) <= range*range)
                return i;

    return -1;
}

// apply splash damage to near monsters
void doSplashDamage(u16 radius, u16 min_dmg, u16 max_dmg, u16 x, u16 y, bool ground, u8 m_id) {
    int i, j, damage, radius2 = radius*radius;
    bool not_immune = true;
    monster* mons;
    
    for(i=0; i < num_monsters; i++)
        if(i != m_id && monsters[i].life > 0 && !td->monsters[monsters[i].type].air && ground)
            if(PA_Distance(x, y, monsters[i].pos_x, monsters[i].pos_y) <= radius2) {
            
                mons = &td->monsters[monsters[i].type];
                
                // check if monster has immune to normal damage
                for(j=0; j < mons->num_immunes; j++)
                    if(mons->immune[j] == IM_NORMAL)
                        not_immune = false;
                
                if(not_immune) {
                    damage = rand(min_dmg, max_dmg) - td->monsters[monsters[i].type].armor;
                    
                    if(damage > 0)
                        monsters[i].life -= damage;
                }
            }
}

// AI, animation & attacks for a tower
void towerEngine(u8 t) {

    // attack sprite id & tower type
    u8 attack_id = towers[t].id - TOWERS_INST_MAX;
    tower* tow = &td->towers[towers[t].type];
    bool play_attack_snd = false;
    int i;

    // reload delay
    if(towers[t].reload > 0)
        towers[t].reload--;

    // attacks
    if(!towers[t].attack && towers[t].reload == 0) {
    
        s16 to_attack = findMonsterToAttack(towers[t].pos_x+16, towers[t].pos_y+16, tow->range, tow->ground, tow->air);
        
        if(to_attack != -1) {
            towers[t].attack = true;
            towers[t].reload = tow->reload;
//            PA_CreateSprite(0, attack_id, attack_Sprites[tow->attack_type], OBJ_SIZE_16X16, 1, ATTACKS_PAL_ID, 0, 0);
//            PA_SetSpritePrio(0, attack_id, 2);
            PA_UpdateSpriteGfxAndMem(0, attack_id, (void*)attack_Sprites[tow->attack_type]);
            attacks[t].pos_x = towers[t].pos_x+8+8;
            attacks[t].pos_y = towers[t].pos_y+8+4;
//            attacks[t].cycle = 0;
            attacks[t].explode = false;
            attacks[t].monster_idx = to_attack;
//            attacks[t].tower_idx = t;
            attacks[t].id = attack_id;

            // choose the direction of the animation
            int diff_x, diff_y, abs_dx, abs_dy, dir;
            diff_x = attacks[t].pos_x - monsters[to_attack].pos_x;
            diff_y = attacks[t].pos_y - monsters[to_attack].pos_y;

            if(diff_x < 0) abs_dx = -diff_x; else abs_dx = diff_x;
            if(diff_y < 0) abs_dy = -diff_y; else abs_dy = diff_y;

            if(abs_dx > 2*abs_dy) {
                if(diff_x < 0)
                    dir = A_RIGHT;
                else
                    dir = A_LEFT;
            } else if(abs_dy > 2*abs_dx) {
                if(diff_y < 0)
                    dir = A_DOWN;
                else
                    dir = A_UP;
            } else if(diff_x < 0) {
                if(diff_y < 0)
                    dir = A_DOWN_RIGHT;
                else
                    dir = A_UP_RIGHT;
            } else {
                if(diff_y < 0)
                    dir = A_DOWN_LEFT;
                else
                    dir = A_UP_LEFT;
            }
            
            if(dir > A_DOWN) { 
                if(!PA_GetSpriteHflip(0, attack_id))
                    PA_SetSpriteHflip(0, attack_id, 1);
                dir -= 4;
            } else if(PA_GetSpriteHflip(0, attack_id)) {
                PA_SetSpriteHflip(0, attack_id, 0);
            }
            
            PA_SetSpriteAnimEx(0, attack_id, 16, 16, 1, dir);
            attacks[t].dir = dir;
            
            play_attack_snd = true;
        }
    }

    // if the towers is attacking
    if(towers[t].attack) {
    
        // if the attack has reach the target
        if(attacks[t].explode) {
        
//            PA_DeleteSprite(0, attacks[t].id);  // delete the attack
            PA_SetSpriteXY(0, attacks[t].id, -32, -32);
            towers[t].attack = false;
//            attacks[t].cycle = 0;
            
            u8 m_id = attacks[t].monster_idx;
            int damage = 0, fire_dmg = 0, water_dmg = 0, lightning_dmg = 0, wind_dmg = 0;
            int normal_dmg = rand(tow->minDamage, tow->maxDamage);
            int armor = td->monsters[monsters[m_id].type].armor;
            bool critic = false;
            
            // tower special effects
            for(i=0; i<tow->num_sp; i++) {
                
                // slow
                if(tow->special[i] == SLOW) {
                    monsters[m_id].slow_time = time_to_vbl(tow->spParams[i][0]);
                    monsters[m_id].slow_param1 = tow->spParams[i][1];
                    monsters[m_id].slow_param2 = tow->spParams[i][2];
                }
                // poison
                else if(tow->special[i] == POISON) {
                    monsters[m_id].poison_time = time_to_vbl(tow->spParams[i][0]);
                    monsters[m_id].poison_param1 = tow->spParams[i][1];
                    monsters[m_id].poison_param2 = tow->spParams[i][2];
                    monsters[m_id].poison = 60;     // wait 1 sec before activation
                }
                // critic
                else if(tow->special[i] == CRITIC) {
                    if(rand(0, 100) <= tow->spParams[i][0])
                        critic = true;
                }
                // pierce
                else if(tow->special[i] == PIERCE) {
                    armor -= tow->spParams[i][0];
                }
                // fire magic
                else if(tow->special[i] == FIRE) {
                    fire_dmg = rand(tow->spParams[i][0], tow->spParams[i][1]);
                }
                // water magic
                else if(tow->special[i] == WATER) {
                    water_dmg = rand(tow->spParams[i][0], tow->spParams[i][1]);
                }
                // lightning magic
                else if(tow->special[i] == LIGHTNING) {
                    lightning_dmg = rand(tow->spParams[i][0], tow->spParams[i][1]);
                }
                // wind magic
                else if(tow->special[i] == WIND) {
                    wind_dmg = rand(tow->spParams[i][0], tow->spParams[i][1]);
                }
                // splash damage
                else if(tow->special[i] == SPLASH) {
                    doSplashDamage(tow->spParams[i][0], tow->minDamage, tow->maxDamage, monsters[m_id].pos_x, monsters[m_id].pos_y, tow->ground, m_id);
                }
            }
            
            // monster resistance & immunes
            monster* mon = &td->monsters[monsters[m_id].type];
            for(i=0; i<mon->num_immunes; i++) {
                
                // slow resistance
                if(mon->immune[i] == SLOW) {
                    if(time_to_vbl(mon->imParams[i][0]) > monsters[m_id].slow_time) {
                        monsters[m_id].slow_time = 0;
                    } else {
                        monsters[m_id].slow_time -= time_to_vbl(mon->imParams[i][0]);
                        if(mon->imParams[i][1] > monsters[m_id].slow_param1) {
                            monsters[m_id].slow_time = 0;
                        } else {
                            monsters[m_id].slow_param1 -= mon->imParams[i][1];
                            monsters[m_id].slow_param2 += mon->imParams[i][2];
                        }
                    }
                }
                // poison
                else if(mon->immune[i] == POISON) {
                    if(time_to_vbl(mon->imParams[i][0]) > monsters[m_id].poison_time) {
                        monsters[m_id].poison_time = 0;
                    } else {
                        monsters[m_id].poison_time -= time_to_vbl(mon->imParams[i][0]);
                        if(mon->imParams[i][1] > monsters[m_id].poison_param1) {
                            monsters[m_id].poison_param1 = 0;
                        } else {
                            monsters[m_id].poison_param1 -= mon->imParams[i][1];
                            if(mon->imParams[i][2] > monsters[m_id].poison_param2) {
                                monsters[m_id].poison_time = 0;
                            } else {
                                monsters[m_id].poison_param2 -= mon->imParams[i][2];
                            }
                        }
                    }
                }
                // fire magic
                else if(mon->immune[i] == FIRE) {
                    if(mon->imParams[i][0] > fire_dmg) 
                        fire_dmg = 0;
                    else
                        fire_dmg -= mon->imParams[i][0];
                }
                // water magic
                else if(mon->immune[i] == WATER) {
                    if(mon->imParams[i][0] > water_dmg) 
                        water_dmg = 0;
                    else
                        water_dmg -= mon->imParams[i][0];
                }
                // lightning magic
                else if(mon->immune[i] == LIGHTNING) {
                    if(mon->imParams[i][0] > lightning_dmg) 
                        lightning_dmg = 0;
                    else
                        lightning_dmg -= mon->imParams[i][0];
                }
                // wind magic
                else if(mon->immune[i] == WIND) {
                    if(mon->imParams[i][0] > wind_dmg) 
                        wind_dmg = 0;
                    else
                        wind_dmg -= mon->imParams[i][0];
                }
                // immumity to normal damage
                else if(mon->immune[i] == IM_NORMAL) {
                    normal_dmg = 0;
                }
                // immunity to pierce
                else if(mon->immune[i] == IM_PIERCE) {
                    armor = td->monsters[monsters[m_id].type].armor;
                }
                // immunity to magic
                else if(mon->immune[i] == IM_MAGIC) {
                    fire_dmg = 0;
                    water_dmg = 0;
                    lightning_dmg = 0;
                    wind_dmg = 0;
                }
            }
            
            if(armor < 0)
                armor = 0;

            // apply monster's armor
            if(armor > normal_dmg)
                normal_dmg = 0;
            else
                normal_dmg -= armor;
            
            // calculate damage
            damage = normal_dmg + fire_dmg + water_dmg + lightning_dmg + wind_dmg;

            if(critic)
                damage += damage;

            if(damage < 0)
                damage = 0;
                
            // W3 damage style
            if(damage == 0 && td->w3_damage_style)
                damage = 1;
                
            monsters[attacks[t].monster_idx].life -= damage;
            

        } else {

            u16 goal_x = monsters[attacks[t].monster_idx].pos_x;
            u16 goal_y = monsters[attacks[t].monster_idx].pos_y;
            u16 pos_x = attacks[t].pos_x;
            u16 pos_y = attacks[t].pos_y;
            u16 speed = tow->speed;
            int diff_x, diff_y, abs_dx, abs_dy;

            // move the attack
            for(i=0; i<speed; i++) {

                // if monster reached
                if(pos_x == goal_x && pos_y == goal_y) {
                    attacks[t].explode = true;
                    break;

                } else {

                    // choose the direction to move to
                    diff_x = pos_x - goal_x;
                    diff_y = pos_y - goal_y;

                    if(diff_x < 0) abs_dx = -diff_x; else abs_dx = diff_x;
                    if(diff_y < 0) abs_dy = -diff_y; else abs_dy = diff_y;

                    if(abs_dx > abs_dy) {
                        if(diff_x < 0)
                            pos_x++;
                        else
                            pos_x--;
                    } else {
                        if(diff_y < 0)
                            pos_y++;
                        else
                            pos_y--;
                    }
                }
            }
            
            // display or not the attack sprite
            if(pos_x >= window_x-32 && pos_y >= window_y-32 && pos_x < window_x+VIEW_W && pos_y < window_y+VIEW_H) {
                PA_SetSpriteXY(0, attacks[t].id, pos_x-window_x, pos_y-window_y);
                if(play_attack_snd) {
                    // stereo placement
                    int x = (pos_x-window_x)*128/VIEW_W, y = ((pos_y-window_y)*32/VIEW_H)-16;
                    if(x>127)
                        x = 127;
                    if(x<0)
                        x = 0;
                    if(y<0)
                        y = -y;
                    soundMix(tow->sound, tow->sound_size, SND_ATTACK, 70-y, x);
                }
            } else {
                PA_SetSpriteXY(0, attacks[t].id, -16, -16);
            }

            // update  the position
            attacks[t].pos_x = pos_x;
            attacks[t].pos_y = pos_y;
//            attacks[t].cycle++;
        }
    }

    // display or not the tower sprite
    if(towers[t].pos_x >= window_x-32 && towers[t].pos_y >= window_y-32 && towers[t].pos_x < window_x+VIEW_W && towers[t].pos_y < window_y+VIEW_H) {
        PA_SetSpriteXY(0, towers[t].id, towers[t].pos_x-window_x+8, towers[t].pos_y-window_y+4);
    } else {
        PA_SetSpriteXY(0, towers[t].id, -32, -32);
    }

}

/**********************************************************************************************************************/

// do dark effect on towers
void doDarkTowers() {
    int i;
    for(i=0; i < num_towers; i++) {
        tower* tow = &td->towers[towers[i].type];
        // switch palette if the tower is set to dark variation
        if(tow->dark) {
            if(dark)
                PA_SetSpritePal(0, towers[i].id, T_DARK_PAL_ID);
            else
                PA_SetSpritePal(0, towers[i].id, tow->pal);
        }
    }
}

// show invisible monsters
void revealMonsters(u16 radius, u16 x, u16 y) {
    int i, radius2 = radius*radius;
    for(i=0; i < num_monsters; i++)
        if(monsters[i].life > 0)
            if(PA_Distance(x, y, monsters[i].pos_x, monsters[i].pos_y) <= radius2)
                monsters[i].visible = true;
}

// towers reveal monsters
void towerReveal() {
    int i, j;
    tower* tow;

    for(i=0; i < num_towers; i++) {
        tow = &td->towers[towers[i].type];
        // check if reveal invisible monsters
        for(j=0; j < tow->num_sp; j++) {
            if(tow->special[j] == REVEAL) {
                revealMonsters(tow->spParams[j][0], towers[i].pos_x+16, towers[i].pos_y+16);
            }
        }
    }
}

/**********************************************************************************************************************/

// AI, animation & attacks for all tower
void towersEngine() {
    int i;
    for(i=0; i < num_towers; i++)
        towerEngine(i);
}

/**********************************************************************************************************************/

// Load text palette on desired screen
void loadTextPalette(u8 screen, u8 base_index) {
    // Text colors
    PA_SetBgPalCol(screen, base_index+1, PA_RGB(31, 31, 31));  // white
    PA_SetBgPalCol(screen, base_index+2, PA_RGB(31, 0, 0));    // red
    PA_SetBgPalCol(screen, base_index+3, PA_RGB(0, 0, 31));    // blue
    PA_SetBgPalCol(screen, base_index+4, PA_RGB(0, 31, 0));    // green
    PA_SetBgPalCol(screen, base_index+5, 32768);                // black
    PA_SetBgPalCol(screen, base_index+6, 52850);                // grey
    PA_SetBgPalCol(screen, base_index+7, 35679);                // dark yellow
    PA_SetBgPalCol(screen, base_index+8, 34499);                // dark green
    PA_SetBgPalCol(screen, base_index+9, 33375);                // orange
    PA_SetBgPalCol(screen, base_index+10, 65504);               // cyan
    PA_SetBgPalCol(screen, base_index+11, 33791);               // yellow
    PA_SetBgPalCol(screen, base_index+12, 58136);               // light grey
    PA_SetBgPalCol(screen, base_index+13, 64965);               // soft blue
    PA_SetBgPalCol(screen, base_index+14, 37247);               // dark orange
    PA_SetBgPalCol(screen, base_index+15, 38015);               // soft red
    PA_SetBgPalCol(screen, base_index+16, PA_RGB(1, 1, 17));  // dark blue
}

/**********************************************************************************************************************/

// Make all monsters disappear and reset monsters number for a new round
void vanishMonsters() {

    int i;

/*    if(m_vanish == 9) {
        m_vanish--;
//        PA_EnableSpecialFx(0, SFX_ALPHA, 0, SFX_BG0 | SFX_BG1 | SFX_BG2 | SFX_BG3 | SFX_BD); 
        PA_SetSFXAlpha(0, m_vanish, 15);
        selected_monster = -1;

        // activate transparency
        for(i=0; i < num_monsters; i++) {
//            monsters[i].life = 0;
            PA_SetSpriteMode(0, monsters[i].id, 1);
        }
        
    } else */
    if(m_vanish > 0) {

        // activate transparency
        for(i=0; i < num_monsters; i++) {
//            monsters[i].life = 0;
            PA_SetSpriteMode(0, monsters[i].id, 1);
        }

        if(vblcount%4 == 0)
            m_vanish--;

        PA_SetSFXAlpha(0, m_vanish, 15);
        
    } else if(m_vanish == 0) {
        m_vanish = -1;
        
        // delete all monsters
        for(i=0; i < num_monsters; i++) {
//            PA_DeleteSprite(0, monsters[i].id);
            PA_SetSpriteXY(0, monsters[i].id, -32, -32);
        }
        
        for(i=0; i < num_paths; i++)
            num_spawned[i] = 0;
            
        // set transparency if ghost mode on
        if(td->monsters[td->rounds[current_round].type].ghost)
            PA_SetSFXAlpha(0, 11, 15);
    
        // set size of monsters
        switch(td->monsters[td->rounds[current_round].type].size) {
//            case 0:
//                break;
            case 2:
                PA_SetRotsetNoAngle(0, ZOOMSET_MONSTERS, 384, 384);
                break;
            default:
                PA_SetRotsetNoAngle(0, ZOOMSET_MONSTERS, 256, 256);
        }

        current_path = 0;
        num_monsters = 0;
        spawntimer = 1;  
    }
}

/**********************************************************************************************************************/

// Check if a new round starts
void checkForNewRound() {

    int i;
    u8 total_monsters = 0;
    
    for(i=0; i<num_paths; i++)
        total_monsters += td->rounds[current_round].number[i];

    if(timer == 0 && new_round) {
        new_round = false;
        
        if(td->monsters[td->rounds[current_round].type].ghost)
            m_vanish = 11;
        else
            m_vanish = 9;

        PA_SetSFXAlpha(0, m_vanish, 15);
        selected_monster = -1;

        // activate transparency
//        for(i=0; i < num_monsters; i++) {
//            PA_SetSpriteMode(0, monsters[i].id, 1);
//        }

        current_round++;
        
        if(current_clan != -1) {
            if(current_evo < td->clans[current_clan].num_evolutions-1 && evolutions[current_evo+1].minRound <= current_round) {
                if(!evolution_available)
                    evo_timer = NEW_EVO_MSG_TIME;
                evolution_available = true;
            } else {
                evolution_available = false;
            }
        }

        playSound(CreepAggroWhat);
    } else if(current_round >= 0 && deadmonsters == total_monsters) {
        if(current_round+1 == td->numRounds) {
            // you won!
            gameWin();
        } else {
            new_round = true;
            deadmonsters = 0;
            timer = td->rounds[current_round].nextround_delay;

            if(!td->timed_gold)
                gold += td->rounds[current_round].gold_bonus;
                
            playSound(Hint);
        }
    }
}

/**********************************************************************************************************************/

// Interface interactions
void interfaceInteract() {

    int i, j, shift_x, shift_y, tmp;
    bool canbuild;
    
    // !!!DEBUG!!! : win game
//    if (Pad.Newpress.Select) { gameWin(); }

    // pause
    if(Pad.Newpress.Start) {
        gamePause();
    }

    // switch to interface
    if(Pad.Newpress.L || Pad.Newpress.R) {
        if(!interface) {
            interface = true;
            PA_SwitchScreens();
        } else if(interface_switch) {
            interface = false;
            PA_SwitchScreens();
        }
    }
        
    // return to normal
    if(!interface_switch && (Pad.Released.L || Pad.Released.R) && interface) {
        interface = false;
        PA_SwitchScreens();
    }

    // interface icons
    for(i=0; i<12; i++) {

        shift_x = 178 + (i%4)*19;
        shift_y = 132 + (i/4)*20;
    
        if(interface && (Stylus.Newpress && PA_StylusInZone(shift_x, shift_y, shift_x+16, shift_y+16))) {
            
            if(current_clan == -1) {

                // validate clan
                if(i == selected_clan) {
                    current_clan = i;
                    selected_clan = -1;
                    evolutions = td->clans[i].evolutions;
                    max_towers += evolutions[0].max_towers;
                    if(max_towers > TOWERS_INST_MAX)
                        max_towers = TOWERS_INST_MAX;
                    if(current_round >= 0)
                        evolution_available = (current_evo < td->clans[current_clan].num_evolutions-1 && evolutions[current_evo+1].minRound <= current_round);
                    else 
                        evolution_available = (current_evo < td->clans[current_clan].num_evolutions-1 && evolutions[current_evo+1].minRound <= 0);
                    playSound(MouseClick);
                }
                // select clan
                else if(i < td->numClans) {
                        selected_clan = i;
                        playSound(MouseClick);
                }
            
            } else if(selected_evo == -1 && selected_monster == -1 && selected_tower == -1) {  // choose evolution
                if(i <= current_evo)
                    if(evolutions[i].num_towers > 0) {
                        selected_evo = i;
                        playSound(MouseClick);
                    }
                    
                // upgrade base
                if(current_round == -1)
                    tmp = 0;
                else
                    tmp = current_round;

                if(i == 11 && upgrade_base == -1 && current_evo < td->clans[current_clan].num_evolutions-1 && evolutions[current_evo+1].minRound <= tmp) {
                    upgrade_base = i;
                    playSound(MouseClick);
                } else if(upgrade_base == i) {
                    if(gold >= evolutions[current_evo+1].price) {
                        current_evo++;
                        gold -= evolutions[current_evo].price;
                        max_towers += evolutions[current_evo].max_towers;
                        if(max_towers > TOWERS_INST_MAX)
                            max_towers = TOWERS_INST_MAX;
                        upgrade_base = -1;
                        evolution_available = (current_evo < td->clans[current_clan].num_evolutions-1 && evolutions[current_evo+1].minRound <= current_round);

                    } else {
                        // pas assez d'or!
                        strcopy(msg, S_NOT_ENOUGH_GOLD);
                        msg_timer = MSG_TIME;
                        soundMix((void*)KnightNoGold, (u32)KnightNoGold_size, SND_VOICE, 127, 64);
                    }
                    playSound(MouseClick);
                }

            } else if(selected_evo != -1 && selected_monster == -1 && selected_tower == -1 && i < evolutions[selected_evo].num_towers) {  // choose tower to build
    
                // check if you can build this tower
                if(current_evo >= td->towers[evolutions[selected_evo].towers[i]].evo_min) {

                    towerToBuild = evolutions[selected_evo].towers[i];
                    if(!menu) {
                        
                        // check if the tower is set to dark variation 
                        if(td->towers[towerToBuild].dark)
                            build_pal = td->towers[towerToBuild].pal;
                        else
                            build_pal = -1;
                        
//                        PA_CreateSprite(0, BUILD_OK_ID, (void*)t_select24x24_Sprite, OBJ_SIZE_32X32, 1, BUILD_SELEC_PAL_ID, 32, 32);
//                        PA_SetSpritePrio(0, BUILD_OK_ID, 2);

//                        PA_CreateSprite(0, BUILD_TOWER_ID, (void*)t_all_normal_Sprite, OBJ_SIZE_32X32, 1, td->towers[towerToBuild].pal, 0, 0);
//                        PA_SetSpritePrio(0, BUILD_TOWER_ID, 2);
                        PA_SetSpritePal(0, BUILD_TOWER_ID, td->towers[towerToBuild].pal);
                        PA_SetSpriteAnim(0, BUILD_TOWER_ID, td->towers[towerToBuild].gfx_idx);
                
                        build_x = round16(window_x + VIEW_W/2 - 16);
                        build_y = round16(window_y + VIEW_H/2 - 16);
                        menu = true;
                    } else {
                        // check if the tower is set to dark variation 
                        if(td->towers[towerToBuild].dark)
                            build_pal = td->towers[towerToBuild].pal;
                        else
                            build_pal = -1;

                        PA_SetSpritePal(0, BUILD_TOWER_ID, td->towers[towerToBuild].pal);
                        PA_SetSpriteAnim(0, BUILD_TOWER_ID, td->towers[towerToBuild].gfx_idx);
                    }

                } else {
                    // evolution insuffisante
                    sformat(msg, S_EVOLUTION_REQUIRED, evolutions[td->towers[towerToBuild].evo_min].name);
                    msg_timer = MSG_TIME;
                }
                playSound(MouseClick);

            } else if(selected_evo != -1 && selected_monster == -1 && selected_tower == -1 && i == 11) {  // return to evolution menu
                selected_evo = -1;
            
            } else if(selected_tower != -1) {       // tower information & icons

                tower* sel_t = &(td->towers[towers[BASE_TOWERS_ID-selected_tower].type]);

                if(upgrade_to == -1) {
                    // sell tower
                    if(i==3) {
                        if(to_sell == -1) {
                            to_sell = selected_tower;
                            playSound(MouseClick);
                        } else if(to_sell == selected_tower)
                            sellTower();
                    }
                    // upgrade
                    else if(i >= 4) {
                    
                        s16 new_i = i;
                        if(i >= 8)
                            new_i -= 8;

                        if(new_i < sel_t->num_upgrades) {
                    
                            s16 upg = sel_t->upgrades[new_i];
                    
                            if(current_evo >= td->towers[upg].evo_min) {
                                upgrade_to = i;
                            } else {
                                // evolution insuffisante
                                sformat(msg, S_CANT_UPGRADE_EVO_REQ, evolutions[td->towers[upg].evo_min].name);
                                msg_timer = MSG_TIME;
                            }
                            playSound(MouseClick);
                        }
                    }
                    
                } else if(zoom_tower == -1) {

                    if(i == upgrade_to) {
                        upgradeTower();
                    } else {
                        upgrade_to = -1;
                    }
                }
        
            } /*else if(selected_monster != -1) {     // monster information & icons
    
            }*/
        
        }   
    }
    
    // scroll window when click on minimap
    if(interface && (Stylus.Held && PA_StylusInZone(MINIMAP_X, MINIMAP_Y, MINIMAP_X+MINIMAP_WIDTH, MINIMAP_Y+MINIMAP_HEIGHT))) {
        shift_x = Stylus.X-minimap_x_windowsize/2;
        shift_y = Stylus.Y-minimap_y_windowsize/2;
//        TO_ABS(shift_x, shift_y);
        window_x = round4(((shift_x-minimap_x_base) << FIXED_POINT_PRECISION)/minimap_x_factor);
        window_y = round4(((shift_y-minimap_y_base) << FIXED_POINT_PRECISION)/minimap_y_factor);
    }

    // switch tower description
    if(interface && (Stylus.Newpress && PA_StylusInZone(79, 140, 165, 191))) {
        desc = !desc;
    }
    
    // build menu: build
    else if (!interface && menu && ((double_clic && Stylus.DblClick && (PA_Distance(stylus_old_x, stylus_old_y, Stylus.X, Stylus.Y) <= DBL_TAP_ZONE) && !PA_StylusInZone(dbl_click_menu_x1, build_sel_y1, dbl_click_menu_x2, build_sel_y2)) || (Stylus.Newpress && PA_StylusInZone(build_sel_x1, build_sel_y1, build_sel_x2, build_sel_y2))) && (zoom_tower == -1)) {
        
        build_menu_sel_id = BUILD_MENU_ID;
        PA_SetSpriteAnim(0, build_menu_sel_id, 2);
/*        PA_LoadSpritePal(0, SELECTED_PAL_ID, (void*)build_menu_selected_Pal);
        PA_CreateSprite(0, SELECTED_ID, (void*)build_menu_selected_Sprite, OBJ_SIZE_64X32, 1, SELECTED_PAL_ID, 131, 1);
        PA_SetSpriteMode(0, SELECTED_ID, 1);
        PA_EnableSpecialFx(0, SFX_ALPHA, 0, SFX_BG0 | SFX_BG1 | SFX_BG2 | SFX_BG3 | SFX_BD); 
        PA_SetSFXAlpha(0, 7, 15);*/
        menutimer = time_to_vbl(100);
//        playSound(BigButtonClick);

        canbuild = true;
        for(j=0; j<3; j++)
            for(i=0; i<3; i++)
                if((t_path[((build_x >> 4)+i) + ((build_y >> 4)+j)*path_width] > 0))
                    canbuild = false;

//        if((t_path[((build_x >> 4)+1) +( (build_y >> 4)+1)*path_width] == 0) && num_towers < TOWERS_INST_MAX ) {
        if(canbuild) {
            if(num_towers < max_towers) {
                buildTower();
            } else {
                // pas assez de fermes!
                soundMix((void*)KnightNoFood, (u32)KnightNoFood_size, SND_VOICE, 127, 64);
                sformat(msg, S_NOT_ENOUGH_FARMS);
                msg_timer = MSG_TIME;
            }
        } else {
            // impossible de construire ici!
            strcopy(msg, S_CANT_BUILD_HERE);
            msg_timer = MSG_TIME;
            soundMix((void*)PeonCannotBuildThere, (u32)PeonCannotBuildThere_size, SND_VOICE, 127, 64);
        }
    }

    // build menu: cancel
    else if (!interface && menu && (Stylus.Newpress && PA_StylusInZone(cancel_sel_x1, build_sel_y1, cancel_sel_x2, build_sel_y2))) {
        build_menu_sel_id = BUILD_MENU_ID+1;
        PA_SetSpriteAnim(0, build_menu_sel_id, 3);
/*        PA_LoadSpritePal(0, SELECTED_PAL_ID, (void*)build_menu_selected_Pal);
        PA_CreateSprite(0, SELECTED_ID, (void*)build_menu_selected_Sprite, OBJ_SIZE_64X32, 1, SELECTED_PAL_ID, 195, 1);
        PA_SetSpriteMode(0, SELECTED_ID, 1);
        PA_EnableSpecialFx(0, SFX_ALPHA, 0, SFX_BG0 | SFX_BG1 | SFX_BG2 | SFX_BG3 | SFX_BD); 
        PA_SetSFXAlpha(0, 7, 15);*/
        menutimer = time_to_vbl(100);
        playSound(BigButtonClick);
        menu = false;

        PA_SetSpriteXY(0, BUILD_OK_ID, -32, -32);
        PA_SetSpriteXY(0, BUILD_TOWER_ID, -32, -32);
//        PA_DeleteSprite(0, BUILD_OK_ID);
//        PA_DeleteSprite(0, BUILD_TOWER_ID);
    }

    // menu button
    else if ((interface && (Stylus.Newpress && PA_StylusInZone(3, 2, 53, 18))) || Pad.Newpress.Select) {

        // draw UI texts & minimap
        refreshUI();
        updateMinimap();

        PA_LoadSpritePal(1, SELECTED_PAL_ID, (void*)interface_selected_Pal);
        PA_CreateSprite(1, SELECTED_ID, (void*)interface_selected_Sprite, OBJ_SIZE_64X32, 1, SELECTED_PAL_ID, 4, 2);
        PA_SetSpriteMode(1, SELECTED_ID, 1);
//        PA_EnableSpecialFx(1, SFX_ALPHA, 0, SFX_BG0 | SFX_BG1 | SFX_BG2 | SFX_BG3 | SFX_BD); 
        PA_SetSFXAlpha(1, 7, 15);

        while(Stylus.Held) 
            PA_WaitForVBL();
    
        wait(100);
        PA_DeleteSprite(1, SELECTED_ID);
        playSound(BigButtonClick);
            
        showIngameMenu();
//        exit_engine = true;
    }
    
    // [Android port] bottom-screen game-speed button: 1x -> 2x -> 4x
    else if (!interface && Stylus.Newpress && PA_StylusInZone(MENUBTN_X1, MENUBTN_Y1, MENUBTN_X2, MENUBTN_Y2)) {
        shim_game_speed = (shim_game_speed == 1) ? 2 : (shim_game_speed == 2) ? 4 : 1;
        playSound(MouseClick);
    }

    // move build tower
    else if (Stylus.Held && !interface && menu && !PA_StylusInZone(dbl_click_menu_x1, build_sel_y1, dbl_click_menu_x2, build_sel_y2)) {
        build_x = round16(Stylus.X-16+window_x);
        build_y = round16(Stylus.Y-16+window_y);
    }

}

/**********************************************************************************************************************/

// Build new tower
void buildTower() {

    int i, j, tow_x, tow_y;
    
    if(gold >= td->towers[towerToBuild].price) {
            
        gold -= td->towers[towerToBuild].price;

        towers[num_towers].pos_x = build_x;
        towers[num_towers].pos_y = build_y;
        towers[num_towers].attack = false;
        towers[num_towers].reload = 0;
        towers[num_towers].id = BASE_TOWERS_ID - num_towers;
        towers[num_towers].type = towerToBuild;
    
//        PA_CreateSprite(0, towers[num_towers].id, (void*)t_all_normal_Sprite, OBJ_SIZE_32X32, 1, td->towers[towerToBuild].pal, -32, -32);
//        PA_SetSpritePrio(0, towers[num_towers].id, 2);
        PA_SetSpritePal(0, towers[num_towers].id, td->towers[towerToBuild].pal);
        PA_SetSpriteAnim(0, towers[num_towers].id, td->towers[towerToBuild].gfx_idx+TOTAL_TOWERS);

        for(j=0; j<3; j++)
            for(i=0; i<3; i++) 
                m_path[(build_x/16)+i + ((build_y/16)+j)*path_width]++;
        
        t_path[(build_x/16)+1 + ((build_y/16)+1)*path_width]++;

        // for zoom effect
        if(zoom_tower > 0 && zoom_tower < num_towers)
            PA_SetSpriteRotDisable(0, zoom_tower);

        zoom_tower = towers[num_towers].id;
        zoom_timer = BUILD_ZOOM_TIME;        
        PA_SetRotsetNoAngle(0, ZOOMSET_BUILD_TOWER, 384, 384);
        PA_SetSpriteRotEnable(0, zoom_tower, ZOOMSET_BUILD_TOWER);
        
        if(!multiple_builds) {
            menu = false;
//            PA_DeleteSprite(0, BUILD_OK_ID);
//            PA_DeleteSprite(0, BUILD_TOWER_ID);
            PA_SetSpriteXY(0, BUILD_OK_ID, -32, -32);
            PA_SetSpriteXY(0, BUILD_TOWER_ID, -32, -32);
        }
        
        num_towers++;

        // stereo placement
        int x = (build_x-window_x)*128/VIEW_W, y = ((build_y-window_y)*32/VIEW_H)-16;
        if(x>127)
            x = 127;
        if(x<0)
            x = 0;
        if(y<0)
            y = -y;
//        soundMix(tow->sound, tow->sound_size, SND_ATTACK, x/2, 70-y);
        soundMix((void*)BuildingPlacement, (u32)BuildingPlacement_size, SND_STANDARD, 127-y, x);
//        playSound(BuildingPlacement);

        stylus_old_x = -32;
        stylus_old_y = -32;
        
        // check if building this tower allow more max towers
        tower* tow = &td->towers[towerToBuild];
        for(j=0; j < tow->num_sp; j++) {
            if(tow->special[j] == ADD_MAX_T) {
                max_towers += tow->spParams[j][0];
                if(max_towers > TOWERS_INST_MAX)
                    max_towers = TOWERS_INST_MAX;
            }
        }
            
        // hide monsters under the tower
        tow_x = build_x+16;
        tow_y = build_y+16;
        if(!td->monsters[td->rounds[current_round].type].air) {
            for(i=0; i < num_monsters; i++)
                if(monsters[i].life <= 0 && PA_Distance(tow_x, tow_y, monsters[i].pos_x, monsters[i].pos_y) <= TOWER_SIZE) {
                    monsters[i].pos_x = -32*16;
                    monsters[i].pos_y = -32*16;
                }
        }

    } else {
        // not enough gold!
        strcopy(msg, S_NOT_ENOUGH_GOLD);
        msg_timer = MSG_TIME;
        soundMix((void*)KnightNoGold, (u32)KnightNoGold_size, SND_VOICE, 127, 64);
    }
}

/**********************************************************************************************************************/

// Display interface icons
void refreshIcons() {

    int i, j;
    
    // delete icons
    for(i=0; i<num_icons; i++) {
        PA_DeleteSprite(1, BASE_ICONS_ID+i);
    }

    if(current_clan == -1) {

        num_icons = 0;
        // choice of clans
        for(i=0; i < td->numClans; i++) {
            PA_CreateSprite(1, BASE_ICONS_ID+i, (void*)icons_Sprite, OBJ_SIZE_16X16, 1, ICONS_PAL_ID, 178 + (i%4)*19, 132 + (i/4)*20);
            PA_SetSpriteAnim(1, BASE_ICONS_ID+i, td->clans[i].icon_idx);
        }
        num_icons = i;

    } else if(selected_evo == -1 && selected_monster == -1 && selected_tower == -1) {  // choose evolution

        num_icons = 0;
        i = 0;
        // choice of evolution set
        for(j=0; j <= current_evo; j++) {
            if(evolutions[j].num_towers > 0) {
                PA_CreateSprite(1, BASE_ICONS_ID+i, (void*)icons_Sprite, OBJ_SIZE_16X16, 1, ICONS_PAL_ID, 178 + (i%4)*19, 132 + (i/4)*20);
                PA_SetSpriteAnim(1, BASE_ICONS_ID+i, evolutions[j].build_icon_idx);
                i++;
            }
        }
        if(evolution_available) {
            // evolution of the base
            PA_CreateSprite(1, BASE_ICONS_ID+i, (void*)icons_Sprite, OBJ_SIZE_16X16, 1, ICONS_PAL_ID, 235, 172);
            PA_SetSpriteAnim(1, BASE_ICONS_ID+i, evolutions[current_evo+1].icon_idx);
            i++;
        }
        num_icons = i;

    } else if(selected_evo != -1 && selected_monster == -1 && selected_tower == -1) {  // choose tower to build

        num_icons = 0;
        // choice of towers
        for(i=0; i < evolutions[selected_evo].num_towers; i++) {
            if(current_evo >= td->towers[evolutions[selected_evo].towers[i]].evo_min) {
                PA_CreateSprite(1, BASE_ICONS_ID+i, (void*)icons_Sprite, OBJ_SIZE_16X16, 1, ICONS_PAL_ID, 178 + (i%4)*19, 132 + (i/4)*20);
            } else {
                PA_CreateSprite(1, BASE_ICONS_ID+i, (void*)icons_dark_Sprite, OBJ_SIZE_16X16, 1, ICONS_DARK_PAL_ID, 178 + (i%4)*19, 132 + (i/4)*20);
            }
                
            PA_SetSpriteAnim(1, BASE_ICONS_ID+i, td->towers[evolutions[selected_evo].towers[i]].icon_idx);
        }
        // return to evolution menu
        PA_CreateSprite(1, BASE_ICONS_ID+i, (void*)icons_Sprite, OBJ_SIZE_16X16, 1, ICONS_PAL_ID, 235, 172);
        PA_SetSpriteAnim(1, BASE_ICONS_ID+i, 0);
        num_icons = i+1;
    
    } else if(selected_tower != -1) {       // tower icons & upgrades

        num_icons = 0;
        // selected tower menu
        tower* sel_t = &(td->towers[towers[BASE_TOWERS_ID-selected_tower].type]);
        // upgrades
        for(i=0; i < sel_t->num_upgrades; i++) {
            if(current_evo >= td->towers[sel_t->upgrades[i]].evo_min) {
                PA_CreateSprite(1, BASE_ICONS_ID+i, (void*)icons_Sprite, OBJ_SIZE_16X16, 1, ICONS_PAL_ID, 178 + (i%4)*19, 172 - (i/4)*20);
            } else {
                PA_CreateSprite(1, BASE_ICONS_ID+i, (void*)icons_dark_Sprite, OBJ_SIZE_16X16, 1, ICONS_DARK_PAL_ID, 178 + (i%4)*19, 172 - (i/4)*20);
            }
            
            PA_SetSpriteAnim(1, BASE_ICONS_ID+i, td->towers[sel_t->upgrades[i]].icon_idx);
        }
        // sell tower
        PA_CreateSprite(1, BASE_ICONS_ID+i, (void*)icons_Sprite, OBJ_SIZE_16X16, 1, ICONS_PAL_ID, 235, 132);
        PA_SetSpriteAnim(1, BASE_ICONS_ID+i, 2);
        num_icons = i+1;

    } else if(selected_monster != -1) {     // monster information & icons
        num_icons = 0;
    }
}

/**********************************************************************************************************************/

// test if a tower or a monster is selected
void checkForSelection() {
    int i;

    if(!menu) {

        // reset selection if player touched anywhere else
        if(Stylus.Held && !PA_StylusInZone(176, 129, 252, 190) && !PA_StylusInZone(79, 140, 165, 191)) {
            selected_tower = -1;
            selected_monster = -1;
            upgrade_to = -1;
            upgrade_base = -1;
            to_sell = -1;
        }

        // check for towers
        for(i=BASE_TOWERS_ID; i > BASE_TOWERS_ID-num_towers; i--)
            if(PA_SpriteTouched(i)) {
                selected_tower = i;
                break;
            }

        // check for monsters
        if(selected_tower == -1) {
            for(i=BASE_MONSTERS_ID; i > BASE_MONSTERS_ID-num_monsters; i--)
                if(PA_SpriteTouched(i) && monsters[BASE_MONSTERS_ID-i].life > 0) {
                    selected_monster = i;
                    break;
                }
        }
    }
}

/**********************************************************************************************************************/

// Won
void gameWin() {
    simpleDialog(td->name, 7, S_VICTORY, 1, true, S_SHOW_STATS, 1);
    stats = true;
    exit_engine = true;
}

/**********************************************************************************************************************/

// Lose
void gameLose() {
    simpleDialog(td->name, 7, S_DEFEAT, 1, true, S_SHOW_STATS, 1);
    stats = true;
    exit_engine = true;
}

/**********************************************************************************************************************/

// Pause
void gamePause() {
    simpleDialog(td->name, 7, S_PAUSE, 1, true, S_RETURN, 1);
}

/**********************************************************************************************************************/

// show a simple modal dialog on the screen
void simpleDialog(char* title, u8 title_color, char* text, u8 text_color, bool center_text, char* button, u8 button_color) {

    shim_ModalBegin();   // [port] dialog at classic 256x192

    // draw UI texts & minimap
    refreshUI();
    updateMinimap();

    // change 8 bit bg priority
//    _REG16(REG_BGCNT(0, 3)) = 0 | BG_BMP8_256x256 | BG_BMP_BASE(5);

    // check if screens are flipped
    if(interface) {
        interface = false;
        PA_SwitchScreens();
    }

    // show the dialog
    PA_Load8bitBgPal(0, (void*)ingame_menu_Pal);
    PA_Load8bitBitmap(0, ingame_menu_Bitmap); 

    // text palette
    loadTextPalette(0, base_color);

    // texts
    centerAlignSmartText(0, 65, 60, 191, 70, title, base_color+title_color, 1, 1);

    if(center_text)
        centerAlignSmartText(0, 65, 73, 191, 103, text, base_color+text_color, 1, 1);
    else
        SmartText(0, 65, 73, 190, 103, text, base_color+text_color, 1, 1, 255);

    centerAlignSmartText(0,77, 111, 179, 121, button, base_color+button_color, 1, 1);

    while(!(Stylus.Newpress && PA_StylusInZone(70, 103, 190, 127)) || Pad.Newpress.Start) {
        PA_WaitForVBL();
//        playQueue();
        checkLid();
    }

    // selected button effect
//    PA_LoadSpritePal(0, SELECTED_PAL_ID, (void*)ingame_menu_selected_Pal);
//    PA_CreateSprite(0, SELECTED_ID, (void*)ingame_menu_selected_Sprite, OBJ_SIZE_64X32, 1, SELECTED_PAL_ID, 74, 108);
//    PA_CreateSprite(0, SELECTED_ID+1, (void*)ingame_menu_selected_Sprite, OBJ_SIZE_64X32, 1, SELECTED_PAL_ID, 74+64, 108);
//    PA_SetSpriteMode(0, SELECTED_ID, 1);
//    PA_SetSpriteMode(0, SELECTED_ID+1, 1);
//    PA_SetSpriteAnim(0, SELECTED_ID+1, 1);
    PA_SetSpriteXY(0, SELECTED_ID, 74, 108);
    PA_SetSpriteXY(0, SELECTED_ID+1, 74+64, 108);
//    PA_EnableSpecialFx(0, SFX_ALPHA, 0, SFX_BG0 | SFX_BG1 | SFX_BG2 | SFX_BG3 | SFX_BD); 
    PA_SetSFXAlpha(0, 7, 15);

    while(Stylus.Held) {
//        playQueue();
        PA_WaitForVBL();
    }
    
//    PA_DeleteSprite(0, SELECTED_ID);
//    PA_DeleteSprite(0, SELECTED_ID+1);
    PA_SetSpriteXY(0, SELECTED_ID, -32, -32);
    PA_SetSpriteXY(0, SELECTED_ID+1, -32, -32);

    // set transparency if ghost mode on
    if(td->monsters[td->rounds[current_round].type].ghost)
        PA_SetSFXAlpha(0, 11, 15);

//    PA_StopSound(15);
    playSound(BigButtonClick);

    // clear the dialog
    int i, j;
    for(j=51; j<136; j++)
        for(i=61; i<195; i+=4)
            *(u32*)(PA_DrawBg[0] + (i >> 1) + (j << 7)) = 0;

    shim_ModalEnd();   // [port]
            
    // change 8 bit bg priority
//    _REG16(REG_BGCNT(0, 3)) = 1 | BG_BMP8_256x256 | BG_BMP_BASE(5);
}

/**********************************************************************************************************************/

// Choose game difficulty
void chooseDifficulty() {

    shim_ModalBegin();   // [port] dialog at classic 256x192

    int choice = 2;
    char *diff_texts[5] = { S_DIFF_VERYEASY, S_DIFF_EASY, S_DIFF_NORMAL, S_DIFF_HARD, S_DIFF_VERYHARD };    
    int diff_value[5] = { 50, 75, 100, 125, 150 };

    // draw UI texts & minimap
    refreshUI();
    updateMinimap();

    // check if screens are flipped
    if(interface) {
        interface = false;
        PA_SwitchScreens();
    }

    // show the dialog
    PA_Load8bitBgPal(0, (void*)ingame_menu_Pal);
    

    // create selection buttons
    PA_LoadSpritePal(0, SELECT_DIFF_ID, (void*)ingame_menu_arrow_Pal);
    PA_CreateSprite(0, BUILD_TOWER_ID, (void*)ingame_menu_arrow_Sprite, OBJ_SIZE_16X16, 1, SELECT_DIFF_ID, 76, 88);
    PA_CreateSprite(0, BUILD_TOWER_ID+1, (void*)ingame_menu_arrow_Sprite, OBJ_SIZE_16X16, 1, SELECT_DIFF_ID, 165, 88);
    PA_SetSpriteHflip(0, BUILD_TOWER_ID+1, true);

    // text palette
    loadTextPalette(0, base_color);

    while(1) {
        PA_Clear8bitBg(0);

        // dialog box
        PA_Load8bitBitmap(0, ingame_menu_Bitmap); 

        // texts
        centerAlignSmartText(0, 65, 60, 191, 70, S_DIFFICULTY, base_color+7, 1, 1);
        centerAlignSmartText(0, 65, 73, 191, 84, S_CHOOSE_DIFFICULTY, base_color+1, 1, 1);
        centerAlignSmartText(0, 65, 90, 191, 103, diff_texts[choice], base_color+1, 1, 1);
        centerAlignSmartText(0,77, 111, 179, 121, S_START, base_color+1, 1, 1);           
    
        PA_WaitForVBL();
//        playQueue();
        checkLid();
        
        if((Stylus.Newpress && PA_StylusInZone(70, 103, 190, 127)) || Pad.Newpress.Start)
            break;
        
        if((Stylus.Newpress && (PA_StylusInZone(76, 88, 85, 101) || Pad.Newpress.Left)) && choice > 0)
            --choice;

        if((Stylus.Newpress && (PA_StylusInZone(165, 88, 182, 101) || Pad.Newpress.Right)) && choice < 4)
            ++choice;
    }

    // selected button effect
    PA_SetSpriteXY(0, SELECTED_ID, 74, 108);
    PA_SetSpriteXY(0, SELECTED_ID+1, 74+64, 108);
    PA_SetSFXAlpha(0, 7, 15);

    while(Stylus.Held) {
//        playQueue();
        PA_WaitForVBL();
    }

    PA_SetSpriteXY(0, SELECTED_ID, -32, -32);
    PA_SetSpriteXY(0, SELECTED_ID+1, -32, -32);

    // set transparency if ghost mode on
    if(td->monsters[td->rounds[current_round].type].ghost)
        PA_SetSFXAlpha(0, 11, 15);

//    PA_StopSound(15);
    playSound(BigButtonClick);

    // clear the dialog
    int i, j;
    for(j=51; j<136; j++)
        for(i=61; i<195; i+=4)
            *(u32*)(PA_DrawBg[0] + (i >> 1) + (j << 7)) = 0;

    shim_ModalEnd();   // [port]

    PA_DeleteSprite(0, BUILD_TOWER_ID);
    PA_DeleteSprite(0, BUILD_TOWER_ID+1);  

    // set difficulty
    difficulty = diff_value[choice];
    difficulty_value = choice;
}

/**********************************************************************************************************************/

// Highlight a button from the menu
void highlightButton(u8 y) {

    // selected button effect
    PA_SetSpriteXY(0, SELECTED_ID, 74, y);
    PA_SetSpriteXY(0, SELECTED_ID+1, 74+64, y);
    PA_SetSFXAlpha(0, 7, 15);

    while(Stylus.Held) {
//        playQueue();
        PA_WaitForVBL();
    }

    PA_SetSpriteXY(0, SELECTED_ID, -32, -32);
    PA_SetSpriteXY(0, SELECTED_ID+1, -32, -32);

    // set transparency if ghost mode on
    if(td->monsters[td->rounds[current_round].type].ghost)
        PA_SetSFXAlpha(0, 11, 15);

//    PA_StopSound(15);
    playSound(BigButtonClick);
}


// Show in-game menu
void showIngameMenu() {

    shim_ModalBegin();   // [port] dialog at classic 256x192

    // draw UI texts & minimap
    refreshUI();
    updateMinimap();

    // check if screens are flipped
    if(interface) {
        interface = false;
        PA_SwitchScreens();
    }

    // show the dialog
    PA_Load8bitBgPal(0, (void*)ingame_menu_Pal);
    PA_Load8bitBitmap(0, ingame_full_menu_Bitmap); 
    
    // text palette
    loadTextPalette(0, base_color);

    while(1) {
        PA_WaitForVBL();
//        playQueue();
        checkLid();
        
        if(Stylus.Newpress && PA_StylusInZone(70, 51, 186, 73)) {
            highlightButton(55);

            // restart current map
            retry = true;
            exit_engine = true;
            break;
        }
        
        if(Stylus.Newpress && PA_StylusInZone(70, 75, 186, 96)) {
            highlightButton(78);

            // save current map status
            saveGame();
            exit_engine = true;
            break;
        }

        if(Stylus.Newpress && PA_StylusInZone(70, 98, 186, 119)) {
            highlightButton(101);
            exit_engine = true;
            break;
        }

        if((Stylus.Newpress && PA_StylusInZone(70, 121, 186, 142)) || Pad.Newpress.Start) {
            highlightButton(124);
            break;
        }
    }

    // clear the dialog
    PA_Clear8bitBg(0);

    shim_ModalEnd();   // [port]    
}

/**********************************************************************************************************************/

// Sell a tower
void sellTower() {

    u8 sel_t = BASE_TOWERS_ID-selected_tower;
    gold += td->towers[towers[sel_t].type].price * td->sell_pct / 100;

    // remove path entries
    int i, j;
    for(j=0; j<3; j++)
        for(i=0; i<3; i++)
            m_path[(towers[sel_t].pos_x >> 4)+i + ((towers[sel_t].pos_y >> 4)+j)*path_width]--;

    t_path[(towers[sel_t].pos_x >> 4)+1 + ((towers[sel_t].pos_y >> 4)+1)*path_width]--;
//    m_path[(towers[sel_t].pos_x >> 4)+1 + ((towers[sel_t].pos_y >> 4)+1)*path_width]--;

    // rebuild towers array
    num_towers--;
    if(sel_t != num_towers) {
        towers[sel_t].pos_x = towers[num_towers].pos_x;
        towers[sel_t].pos_y = towers[num_towers].pos_y;
        towers[sel_t].attack = towers[num_towers].attack;
        towers[sel_t].reload = towers[num_towers].reload;
        towers[sel_t].type = towers[num_towers].type;
        PA_SetSpriteAnim(0, towers[sel_t].id, td->towers[towers[num_towers].type].gfx_idx+TOTAL_TOWERS);
        PA_SetSpritePal(0, towers[sel_t].id, td->towers[towers[num_towers].type].pal);

        if(towers[num_towers].attack) {
            // update attack
            attacks[sel_t].pos_x = attacks[num_towers].pos_x;
            attacks[sel_t].pos_y = attacks[num_towers].pos_y;
            attacks[sel_t].dir = attacks[num_towers].dir;
            attacks[sel_t].explode = attacks[num_towers].explode;
//            attacks[sel_t].cycle = attacks[num_towers].cycle;
            attacks[sel_t].monster_idx = attacks[num_towers].monster_idx;
//            attacks[sel_t].tower_idx = attacks[num_towers].tower_idx;
                
            PA_UpdateGfx (0, PA_GetSpriteGfx(0, attacks[sel_t].id), attack_Sprites[td->towers[towers[num_towers].type].attack_type]);
            towers[sel_t].attack = true;
            
/*            if(towers[sel_t].attack) {
                PA_UpdateGfx (0, PA_GetSpriteGfx(0, attacks[sel_t].id), attack_Sprites[td->towers[towers[num_towers].type].attack_type]);
            } else {
//                PA_CreateSprite(0, (towers[sel_t].id - TOWERS_INST_MAX), attack_Sprites[td->towers[towers[num_towers].type].attack_type], OBJ_SIZE_16X16, 1, ATTACKS_PAL_ID, attacks[sel_t].pos_x, attacks[sel_t].pos_y);
                PA_UpdateGfx (0, PA_GetSpriteGfx(0, attacks[sel_t].id), attack_Sprites[td->towers[towers[num_towers].type].attack_type]);
                towers[sel_t].attack = true;
            }*/
        } else {
            towers[sel_t].attack = false;
        }
    } 

    // cancel zoom effect
    if(zoom_tower == towers[num_towers].id)
        zoom_tower = -1;

    // delete the tower & its attack
//    PA_DeleteSprite(0, towers[num_towers].id);
    PA_SetSpriteXY(0, towers[num_towers].id, -32, -32);
//    if(towers[num_towers].attack) {
//        PA_DeleteSprite(0, towers[num_towers].id - TOWERS_INST_MAX);
        PA_SetSpriteXY(0, towers[num_towers].id - TOWERS_INST_MAX, -32, -32);
//    }
    
    selected_tower = -1;
    to_sell = -1;
    
    playSound(RallyPointPlace);
}

/**********************************************************************************************************************/

// upgrade selected tower
void upgradeTower() {
    
    u8 new_upg_to = upgrade_to;
    if(new_upg_to >= 8)
        new_upg_to -= 8;

    u8 upg_tow = td->towers[towers[BASE_TOWERS_ID-selected_tower].type].upgrades[new_upg_to];

    if(gold >= td->towers[upg_tow].price) {

        gold -= td->towers[upg_tow].price;
        
        u8 sel_t = BASE_TOWERS_ID-selected_tower;
    
        // upgrade the tower
        PA_SetSpriteAnim(0, towers[sel_t].id, td->towers[upg_tow].gfx_idx+TOTAL_TOWERS);
        PA_SetSpritePal(0, towers[sel_t].id, td->towers[upg_tow].pal);
//        towers[sel_t].attack = false;
        towers[sel_t].reload = 0;
        towers[sel_t].type = upg_tow;

        upgrade_to = -1;
        
        // for zoom effect
        if(zoom_tower > 0 && zoom_tower < num_towers)
            PA_SetSpriteRotDisable(0, zoom_tower);

        zoom_tower = towers[sel_t].id;
        zoom_timer = BUILD_ZOOM_TIME;
        PA_SetRotsetNoAngle(0, ZOOMSET_BUILD_TOWER, 384, 384);
        PA_SetSpriteRotEnable(0, zoom_tower, ZOOMSET_BUILD_TOWER);

        playSound(RallyPointPlace);

    } else {
        // pas assez d'or!
        strcopy(msg, S_NOT_ENOUGH_GOLD);
        msg_timer = MSG_TIME;
        playSound(KnightNoGold);
    }
}

/**********************************************************************************************************************/

// manage build menu
void manageBuildMenu() {

    bool go_up, go_down;

    if(build_menu_pos > 1) {
        go_up = menu && (menu_y > 176);
        go_down = !menu && (menu_y < 192) && menutimer == 0;
    } else {
        go_up = !menu && (menu_y > -32) && menutimer == 0;
        go_down = menu && (menu_y < 0);
    }

    if(go_down) {
        menu_y += 2;
        if(build_menu_pos%2 == 0) {
            PA_SetSpriteXY(0, BUILD_MENU_ID, menu_x, menu_y);
            PA_SetSpriteXY(0, BUILD_MENU_ID+1, menu_x+64, menu_y);
        } else {
            PA_SetSpriteXY(0, BUILD_MENU_ID, menu_x+64, menu_y);
            PA_SetSpriteXY(0, BUILD_MENU_ID+1, menu_x, menu_y);
        }
    } else if(go_up) {
        menu_y -= 2;
        if(build_menu_pos%2 == 0) {
            PA_SetSpriteXY(0, BUILD_MENU_ID, menu_x, menu_y);
            PA_SetSpriteXY(0, BUILD_MENU_ID+1, menu_x+64, menu_y);
        } else {
            PA_SetSpriteXY(0, BUILD_MENU_ID, menu_x+64, menu_y);
            PA_SetSpriteXY(0, BUILD_MENU_ID+1, menu_x, menu_y);
        }
    }
        
    if(menutimer > 0) {
        menutimer--;
        if(menutimer == 0) {
//            PA_DeleteSprite(0, SELECTED_ID);
            PA_SetSpriteAnim(0, build_menu_sel_id, build_menu_sel_id%2);
        }
    }
}

/**********************************************************************************************************************/

// draw shadow for air monster
/*void drawAirShadow(u16 x, u16 y) {
//            PA_16c8X6(0, monsters[i].pos_x, monsters[i].pos_y, air_shadow);
    int shadow_color = base_color+5;

    if(x <= 253) {
    PA_PutDouble8bitPixels(0, x, y, 0, shadow_color);
    PA_PutDouble8bitPixels(0, x, y+1, shadow_color, shadow_color);
    PA_PutDouble8bitPixels(0, x, y+2, shadow_color, shadow_color);
    PA_PutDouble8bitPixels(0, x, y+3, shadow_color, shadow_color);
    PA_PutDouble8bitPixels(0, x, y+4, shadow_color, shadow_color);
    PA_PutDouble8bitPixels(0, x, y+5, 0, shadow_color);
    }
    
    if(x <= 251) {
    PA_PutDouble8bitPixels(0, x+2, y, shadow_color, shadow_color);
    PA_PutDouble8bitPixels(0, x+2, y+1, shadow_color, shadow_color);
    PA_PutDouble8bitPixels(0, x+2, y+2, shadow_color, shadow_color);
    PA_PutDouble8bitPixels(0, x+2, y+3, shadow_color, shadow_color);
    PA_PutDouble8bitPixels(0, x+2, y+4, shadow_color, shadow_color);
    PA_PutDouble8bitPixels(0, x+2, y+5, shadow_color, shadow_color);
    }
    
    if(x <= 249) {
    PA_PutDouble8bitPixels(0, x+4, y, shadow_color, shadow_color);
    PA_PutDouble8bitPixels(0, x+4, y+1, shadow_color, shadow_color);
    PA_PutDouble8bitPixels(0, x+4, y+2, shadow_color, shadow_color);
    PA_PutDouble8bitPixels(0, x+4, y+3, shadow_color, shadow_color);
    PA_PutDouble8bitPixels(0, x+4, y+4, shadow_color, shadow_color);
    PA_PutDouble8bitPixels(0, x+4, y+5, shadow_color, shadow_color);
    }
    
    if(x <= 247) {
    PA_PutDouble8bitPixels(0, x+6, y, shadow_color, 0);
    PA_PutDouble8bitPixels(0, x+6, y+1, shadow_color, shadow_color);
    PA_PutDouble8bitPixels(0, x+6, y+2, shadow_color, shadow_color);
    PA_PutDouble8bitPixels(0, x+6, y+3, shadow_color, shadow_color);
    PA_PutDouble8bitPixels(0, x+6, y+4, shadow_color, shadow_color);
    PA_PutDouble8bitPixels(0, x+6, y+5, shadow_color, 0);
    }


}*/

/**********************************************************************************************************************/

// save game
void saveGame() {

    bool save_present = true;

    // save all game parameters & variables
    EFS_FILE* file = EFS_fopen(FILE_GAMESAVE);
     
    EFS_fwrite(&save_present, sizeof(bool), 1, file);
    EFS_fwrite(curr_map_path, sizeof(char), 256, file);    
    EFS_fwrite(&curr_map_size, sizeof(int), 1, file);
    EFS_fwrite(&curr_map_sum, sizeof(int), 1, file);    
    
    EFS_fwrite(msg, sizeof(char), 256, file);
    EFS_fwrite(&difficulty, sizeof(int), 1, file);
    EFS_fwrite(&difficulty_value, sizeof(u8), 1, file);
    EFS_fwrite(&interface, sizeof(bool), 1, file);
    EFS_fwrite(&menu, sizeof(bool), 1, file);
    EFS_fwrite(&desc, sizeof(bool), 1, file);
    EFS_fwrite(&dark, sizeof(bool), 1, file);
    EFS_fwrite(&blocked, sizeof(bool), 1, file);
    EFS_fwrite(&evolution_available, sizeof(bool), 1, file);

    EFS_fwrite(&build_pal, sizeof(s16), 1, file);
    EFS_fwrite(&menu_x, sizeof(s32), 1, file);
    EFS_fwrite(&menu_y, sizeof(s32), 1, file);
    EFS_fwrite(&window_x, sizeof(s32), 1, file);
    EFS_fwrite(&window_y, sizeof(s32), 1, file);
    EFS_fwrite(&build_x, sizeof(s32), 1, file);
    EFS_fwrite(&build_y, sizeof(s32), 1, file);
    EFS_fwrite(&menutimer, sizeof(u16), 1, file);
    EFS_fwrite(&spawntimer, sizeof(s16), 1, file);
    EFS_fwrite(&msg_timer, sizeof(u16), 1, file);
    EFS_fwrite(&evo_timer, sizeof(u16), 1, file);
    EFS_fwrite(&total_time, sizeof(int), 1, file);
    EFS_fwrite(&stylus_old_x, sizeof(int), 1, file);
    EFS_fwrite(&stylus_old_y, sizeof(int), 1, file);

    EFS_fwrite(&current_clan, sizeof(s16), 1, file);
    EFS_fwrite(&current_round, sizeof(s16), 1, file);
    EFS_fwrite(&current_evo, sizeof(s16), 1, file);
    EFS_fwrite(&gold, sizeof(u16), 1, file);
    EFS_fwrite(&max_towers, sizeof(u8), 1, file);
    EFS_fwrite(&lifes, sizeof(s16), 1, file);
    EFS_fwrite(&timer, sizeof(u16), 1, file);
    EFS_fwrite(&vblcount, sizeof(u8), 1, file);
    EFS_fwrite(&deadmonsters, sizeof(u8), 1, file);
    EFS_fwrite(&kills, sizeof(u16), 1, file);
    EFS_fwrite(&m_vanish, sizeof(s16), 1, file);
    EFS_fwrite(&selected_evo, sizeof(s16), 1, file);
    EFS_fwrite(&selected_monster, sizeof(s16), 1, file);
    EFS_fwrite(&selected_tower, sizeof(s16), 1, file);
    EFS_fwrite(&selected_clan, sizeof(s16), 1, file);
    EFS_fwrite(&towerToBuild, sizeof(u8), 1, file);
    EFS_fwrite(&upgrade_to, sizeof(s16), 1, file);
    EFS_fwrite(&upgrade_base, sizeof(s16), 1, file);
    EFS_fwrite(&to_sell, sizeof(s16), 1, file);
    EFS_fwrite(&num_icons, sizeof(u8), 1, file);
    EFS_fwrite(&num_sp_icons, sizeof(u8), 1, file);
    EFS_fwrite(&blocked_monsters, sizeof(u8), 1, file);
    EFS_fwrite(&build_menu_sel_id, sizeof(u8), 1, file);

    EFS_fwrite(&boxhidden, sizeof(bool), 1, file);
    EFS_fwrite(&new_round, sizeof(bool), 1, file);
    EFS_fwrite(&confirm, sizeof(bool), 1, file);
    EFS_fwrite(&zoom_timer, sizeof(u8), 1, file);
    EFS_fwrite(&zoom_tower, sizeof(s8), 1, file);
    EFS_fwrite(&current_path, sizeof(u8), 1, file);
    EFS_fwrite(num_spawned, sizeof(u8), MAX_PATHS, file);

    EFS_fwrite(&num_towers, sizeof(u8), 1, file);
    EFS_fwrite(towers, sizeof(tower_instance), TOWERS_INST_MAX, file);
    EFS_fwrite(attacks, sizeof(attack_instance), ATTACKS_INST_MAX, file);
    EFS_fwrite(&num_monsters, sizeof(u16), 1, file);
    EFS_fwrite(monsters, sizeof(monster_instance), MONSTERS_INST_MAX, file);

    EFS_fwrite(m_path, sizeof(u8), (MAP_WIDTH_MAX*MAP_HEIGHT_MAX), file);
    EFS_fwrite(t_path, sizeof(u8), (MAP_WIDTH_MAX*MAP_HEIGHT_MAX), file);
    EFS_fwrite(&path_width, sizeof(u16), 1, file);
    EFS_fwrite(&path_height, sizeof(u16), 1, file);    

    EFS_fclose(file);    
}

/**********************************************************************************************************************/

// load game
void loadGame() {

    bool save_present;

    // save all game parameters & variables
    EFS_FILE* file = EFS_fopen(FILE_GAMESAVE);
    
    EFS_fread(&save_present, sizeof(bool), 1, file);
    EFS_fread(curr_map_path, sizeof(char), 256, file);    
    EFS_fread(&curr_map_size, sizeof(int), 1, file);
    EFS_fread(&curr_map_sum, sizeof(int), 1, file);    
    
    EFS_fread(msg, sizeof(char), 256, file);
    EFS_fread(&difficulty, sizeof(int), 1, file);
    EFS_fread(&difficulty_value, sizeof(u8), 1, file);
    EFS_fread(&interface, sizeof(bool), 1, file);
    EFS_fread(&menu, sizeof(bool), 1, file);
    EFS_fread(&desc, sizeof(bool), 1, file);
    EFS_fread(&dark, sizeof(bool), 1, file);
    EFS_fread(&blocked, sizeof(bool), 1, file);
    EFS_fread(&evolution_available, sizeof(bool), 1, file);

    EFS_fread(&build_pal, sizeof(s16), 1, file);
    EFS_fread(&menu_x, sizeof(s32), 1, file);
    EFS_fread(&menu_y, sizeof(s32), 1, file);
    EFS_fread(&window_x, sizeof(s32), 1, file);
    EFS_fread(&window_y, sizeof(s32), 1, file);
    EFS_fread(&build_x, sizeof(s32), 1, file);
    EFS_fread(&build_y, sizeof(s32), 1, file);
    EFS_fread(&menutimer, sizeof(u16), 1, file);
    EFS_fread(&spawntimer, sizeof(s16), 1, file);
    EFS_fread(&msg_timer, sizeof(u16), 1, file);
    EFS_fread(&evo_timer, sizeof(u16), 1, file);
    EFS_fread(&total_time, sizeof(int), 1, file);
    EFS_fread(&stylus_old_x, sizeof(int), 1, file);
    EFS_fread(&stylus_old_y, sizeof(int), 1, file);

    EFS_fread(&current_clan, sizeof(s16), 1, file);
    EFS_fread(&current_round, sizeof(s16), 1, file);
    EFS_fread(&current_evo, sizeof(s16), 1, file);
    EFS_fread(&gold, sizeof(u16), 1, file);
    EFS_fread(&max_towers, sizeof(u8), 1, file);
    EFS_fread(&lifes, sizeof(s16), 1, file);
    EFS_fread(&timer, sizeof(u16), 1, file);
    EFS_fread(&vblcount, sizeof(u8), 1, file);
    EFS_fread(&deadmonsters, sizeof(u8), 1, file);
    EFS_fread(&kills, sizeof(u16), 1, file);
    EFS_fread(&m_vanish, sizeof(s16), 1, file);
    EFS_fread(&selected_evo, sizeof(s16), 1, file);
    EFS_fread(&selected_monster, sizeof(s16), 1, file);
    EFS_fread(&selected_tower, sizeof(s16), 1, file);
    EFS_fread(&selected_clan, sizeof(s16), 1, file);
    EFS_fread(&towerToBuild, sizeof(u8), 1, file);
    EFS_fread(&upgrade_to, sizeof(s16), 1, file);
    EFS_fread(&upgrade_base, sizeof(s16), 1, file);
    EFS_fread(&to_sell, sizeof(s16), 1, file);
    EFS_fread(&num_icons, sizeof(u8), 1, file);
    EFS_fread(&num_sp_icons, sizeof(u8), 1, file);
    EFS_fread(&blocked_monsters, sizeof(u8), 1, file);
    EFS_fread(&build_menu_sel_id, sizeof(u8), 1, file);

    EFS_fread(&boxhidden, sizeof(bool), 1, file);
    EFS_fread(&new_round, sizeof(bool), 1, file);
    EFS_fread(&confirm, sizeof(bool), 1, file);
    EFS_fread(&zoom_timer, sizeof(u8), 1, file);
    EFS_fread(&zoom_tower, sizeof(s8), 1, file);
    EFS_fread(&current_path, sizeof(u8), 1, file);
    EFS_fread(num_spawned, sizeof(u8), MAX_PATHS, file);

    EFS_fread(&num_towers, sizeof(u8), 1, file);
    EFS_fread(towers, sizeof(tower_instance), TOWERS_INST_MAX, file);
    EFS_fread(attacks, sizeof(attack_instance), ATTACKS_INST_MAX, file);
    EFS_fread(&num_monsters, sizeof(u16), 1, file);
    EFS_fread(monsters, sizeof(monster_instance), MONSTERS_INST_MAX, file);

    EFS_fread(m_path, sizeof(u8), (MAP_WIDTH_MAX*MAP_HEIGHT_MAX), file);
    EFS_fread(t_path, sizeof(u8), (MAP_WIDTH_MAX*MAP_HEIGHT_MAX), file);
    EFS_fread(&path_width, sizeof(u16), 1, file);
    EFS_fread(&path_height, sizeof(u16), 1, file);    

    // erase saved game
    save_present = false;
    EFS_fseek(file, 0, SEEK_SET);     
    EFS_fwrite(&save_present, sizeof(bool), 1, file);

    EFS_fclose(file);   

    // restore evolution
    evolutions = td->clans[current_clan].evolutions;

    // restore correct towers sprites
    int i;
    for(i=0; i < num_towers; i++) {
        PA_SetSpritePal(0, towers[i].id, td->towers[towers[i].type].pal);
        PA_SetSpriteAnim(0, towers[i].id, td->towers[towers[i].type].gfx_idx + TOTAL_TOWERS);
        
        // restore attack sprite
        if(towers[i].attack)
            PA_UpdateSpriteGfxAndMem(0, towers[i].id - TOWERS_INST_MAX, (void*)attack_Sprites[td->towers[towers[i].type].attack_type]);
    }
    
    // restore correct monsters sprites
    PA_LoadSpritePal(0, MONSTERS_PAL_ID, monster_pals[td->monsters[td->rounds[current_round].type].pal-8]);
    for(i=0; i < num_monsters; i++) {
        PA_UpdateSpriteGfxAndMem(0, monsters[i].id, td->monsters[td->rounds[current_round].type].gfx);
        if(monsters[i].life <= 0) {
            if(monsters[i].last_dir != NONE) 
                PA_SetSpriteAnimEx(0, monsters[i].id, 32, 32, 1, monsters[i].last_dir + 4*(monsters[i].cycle - 1));
            else
                PA_SetSpriteAnimEx(0, monsters[i].id, 32, 32, 1, UP + 4*(monsters[i].cycle - 1));
        }        
    }    
    
    // restore hiding box sprite state
    if(!boxhidden) {
        PA_DeleteSprite(1, HIDE_BOX_ID);
        PA_DeleteSprite(1, HIDE_BOX_ID+1);
        PA_DeleteSprite(1, HIDE_BOX_ID+2);    
    }
    
    num_icons = 0;
}

/**********************************************************************************************************************/
