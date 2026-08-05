/************************************/
/* Warcraft Tower Defense - by Noda */
/* Map Loader              12/02/08 */
/************************************/

// Includes
#include <PA9.h>        // Include for PA_Lib
#include <sys/dir.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "nds.h"

// Gfx
#include "gfx/all_gfx.h"

// Modules
#include "f_aux.h"      // Auxiliary functions
#include "engine.h"     // Game engine
#include "types.h"      // Types definitions
#include "defines.h"    // Defines
#include "map_loader.h"
#include "efs_lib.h"

// Sounds
#include "AncestralGuardianMissileLaunch.h"
#include "ArrowAttack.h"
#include "BansheeMissileLaunch.h"
#include "BoatAttack.h"
#include "BristleBackMissileLaunch.h"
#include "CannonMissileLaunch.h"
#include "DestroyerMissile.h"
#include "FarseerMissileLaunch.h"
#include "FeralSpiritTarget.h"
#include "FireBallMissileLaunch.h"
#include "FlakCannon.h"
#include "FragBombs.h"
#include "GryphonRiderMissileLaunch.h"
#include "GuardTowerMissileLaunch.h"
#include "HeroDemonMissileLaunch.h"
#include "ImpaleLaunch.h"
#include "NecromancerMissileLaunch.h"
#include "PhaseShift.h"
#include "PhoenixAttack.h"
#include "SearingArrowMissileLaunch.h"
#include "SorceressCastAttack.h"
#include "SorceressMissileLaunch.h"
#include "StormBoltLaunch.h"
#include "TrollBatriderMissile.h"
#include "ZigguratMissileLaunch.h"


// Attack sounds
u8* attack_snds[] = {
    (u8*)AncestralGuardianMissileLaunch,
    (u8*)ArrowAttack,
    (u8*)BansheeMissileLaunch,
    (u8*)BoatAttack,
    (u8*)BristleBackMissileLaunch,
    (u8*)CannonMissileLaunch,
    (u8*)DestroyerMissile,
    (u8*)FarseerMissileLaunch,
    (u8*)FeralSpiritTarget,
    (u8*)FireBallMissileLaunch,
    (u8*)FlakCannon,
    (u8*)FragBombs,
    (u8*)GryphonRiderMissileLaunch,
    (u8*)GuardTowerMissileLaunch,
    (u8*)HeroDemonMissileLaunch,
    (u8*)ImpaleLaunch,
    (u8*)NecromancerMissileLaunch,
    (u8*)PhaseShift,
    (u8*)PhoenixAttack,
    (u8*)SearingArrowMissileLaunch,
    (u8*)SorceressCastAttack,
    (u8*)SorceressMissileLaunch,
    (u8*)StormBoltLaunch,
    (u8*)TrollBatriderMissile,
    (u8*)ZigguratMissileLaunch,
};

u32 attack_snds_size[] = {
    (u32)AncestralGuardianMissileLaunch_size,
    (u32)ArrowAttack_size,
    (u32)BansheeMissileLaunch_size,
    (u32)BoatAttack_size,
    (u32)BristleBackMissileLaunch_size,
    (u32)CannonMissileLaunch_size,
    (u32)DestroyerMissile_size,
    (u32)FarseerMissileLaunch_size,
    (u32)FeralSpiritTarget_size,
    (u32)FireBallMissileLaunch_size,
    (u32)FlakCannon_size,
    (u32)FragBombs_size,
    (u32)GryphonRiderMissileLaunch_size,
    (u32)GuardTowerMissileLaunch_size,
    (u32)HeroDemonMissileLaunch_size,
    (u32)ImpaleLaunch_size,
    (u32)NecromancerMissileLaunch_size,
    (u32)PhaseShift_size,
    (u32)PhoenixAttack_size,
    (u32)SearingArrowMissileLaunch_size,
    (u32)SorceressCastAttack_size,
    (u32)SorceressMissileLaunch_size,
    (u32)StormBoltLaunch_size,
    (u32)TrollBatriderMissile_size,
    (u32)ZigguratMissileLaunch_size,
};

// Monsters basics
monster_base monsters_basics[] = {
    { M_PEON },
    { M_AXETHROWER },
    { M_GRUNT },
    { M_PEASANT },
    { M_ROGUE },
    { M_SHEEP },
    { M_DAEMON },
    { M_DEATH_KNIGHT },
    { M_GOBLINS },
    { M_DWARVES },
    { M_FOOTMAN },
    { M_KNIGHT },
    { M_MAGE },
    { M_OGRE },
    { M_BIG_SHEEP },
    { M_SKELETON },
    { M_GRYPHON },
    { M_HELICOPTER },
    { M_ZEPPELIN },
    { M_DRAGON },
    { M_BALLISTA },
    { M_BATTLESHIP },
    { M_BUFFALO },
    { M_CATAPULT },
    { M_CLERIC },
    { M_ELVEN_DESTROYER },
    { M_ELVEN_TANKER },
    { M_HUMAN_TANKER },
    { M_HUMAN_TRANSPORT },
    { M_JUGGERNAUT },
    { M_NECROLYTE },
    { M_ORC_TRANSPORT },
    { M_SEAL },
    { M_SUBMARINE },
    { M_TROLL_DESTROYER },
    { M_TURTLE },
};

// Global variables
//static bool use_fat;            // use FATlib (true) or PAFS
//static bool init_done = false; // to init FS only once
static u32 numMaps;             // total number of available maps
static map_desc** maps = NULL;   // map list
static u8 tiles[1024*64];       // loaded map tileset
static u8 tow_path[MAP_WIDTH_MAX*MAP_HEIGHT_MAX];    // loaded map tower collision map
static u8 mon_path[MAP_WIDTH_MAX*MAP_HEIGHT_MAX];    // loaded map monster collision map
static u32 paths_offset;        // offset to reload collision maps
static u8* file;                // map file

static char path[256];          // path + filename holder
static map loaded_map;          // the loaded map
static u8* map_pal;             // map palette pointer
static u8* map_tilemap;         // map tilemap pointer
static u32 map_info[3];         // map infos
static u32 map_tiles_size;      // tileset size
static u16 minimap_pal[256] ;        // minimap palette (when using fatlib)
static u8 minimap_sprite[64*64] ;      // minimap sprite (when using fatlib)

// External variables
extern bool load_game;           // load saved game
char curr_map_path[256];         // current map path
int curr_map_size;               // current map file size
int curr_map_sum;                // current map magic sum


//static int num_temp;

// List all maps from filesystem
void listMaps(map_desc** maplist, int* num) {

    int i;
    numMaps = 0;
//    use_fat = true;
  
/*    if(!use_fat) {
    
        if(!init_done) {
            num_temp = PA_FSInit(); // Inits PA File System, and returns the number of files
            init_done = true;
        }
        numMaps = num_temp;
        
        *maplist = (map_desc*)calloc(numMaps, sizeof(map_desc));   // allocate memory
    
        for(i=0; i<numMaps; i++) {
            
            // map name
            strcopy((*maplist)[i].name, getMapName(i));
            (*maplist)[i].file = PA_PAFSFile(i);
        }
        
    } else*/ {

/*        if(!init_done) {
            if(fatInitDefault()) {
                init_done = true;
            } else {
                *maplist = (map_desc*)calloc(1, sizeof(map_desc));
                *num = 1;
                strcopy((*maplist)[0].name, "FAT init error !");
                (*maplist)[0].file = NULL;
                return;
            }
        }
*/
        char filename[256];
//        char header[MAP_HEADER_SIZE];
//        int seek_off = 0;
        DIR_ITER* dir = NULL;   // directory handler
        FILE* fd;               // file descriptor

        if ((dir = diropen("/maps"))) {
        
            dirreset(dir);

            while(dirnext(dir, filename, NULL) == 0) {
                if((strncomp(filename+strlength(filename)-4, ".tdm", 4) == 0)) {
                    numMaps++;
                }
            }
            
            // allocate memory
            *maplist = (map_desc*)calloc(numMaps, sizeof(map_desc));
            
            dirreset(dir);
            
            i = 0;
            while (dirnext(dir, filename, NULL) == 0) {
                if((strncomp(filename+strlength(filename)-4, ".tdm", 4) == 0)) {

                    // construct file path+name
                    path[0] = '\0';
                    strccat(path, "/maps/");
                    strccat(path, filename);
                    
                    fd = fopen(path, "rb");

                    // initialize map
                    strcopy((*maplist)[i].filename, path);
                    (*maplist)[i].file = (void*)true;
//                    strcopy((*maplist)[i].name, de->d_name);
/*
                    // check for new map format
                    seek_off = 0;
                    fread(header, sizeof(char), MAP_HEADER_SIZE, fd);    
                    if(strncomp(header, MAP_HEADER_V3, MAP_HEADER_SIZE) == 0)
                        seek_off = 8+MAP_HEADER_SIZE;
                        
                    // read filename
                    fseek(fd, seek_off, SEEK_SET);
*/                    fread((*maplist)[i].name, 1, MAP_NAME_MAX, fd); 
                    fclose(fd);
                    i++;
                }
            }
            
            dirclose(dir);
            
        } else {
            *maplist = (map_desc*)calloc(1, sizeof(map_desc));
            *num = 1;
            strcopy((*maplist)[0].name, "/maps not found");
            (*maplist)[0].file = NULL;
            return;
        }

    }

    maps = maplist;  
    *num = numMaps;
}

// Return a pointer to the map name
/*char* getMapName(int map) {
    return (char*)PA_PAFSFile(map);
}*/

// Return a pointer to the map's minimap palette
u16* getMapPreviewPal(int map) {
//    if(use_fat) {
        FILE* fd;
        int pal_size;//, seek_off = 0;
//        char header[MAP_HEADER_SIZE];
        
        fd = fopen((*maps)[map].filename, "rb");
/*
        // check for new map format
        fread(header, sizeof(char), MAP_HEADER_SIZE, fd);    
        if(strncomp(header, MAP_HEADER_V3, MAP_HEADER_SIZE) == 0)
            seek_off += 8+MAP_HEADER_SIZE;
*/
        // read minimap palette size
        fseek(fd, MAP_NAME_MAX+PRESENTATION_MAX+8, SEEK_SET);
        fread(&pal_size, sizeof(u32), 1, fd);
        fread(minimap_pal, sizeof(u8), pal_size, fd);
        fclose(fd);
        return minimap_pal;
/*        
    } else {
        return (u16*)(PA_PAFSFile(map)+MAP_NAME_MAX+PRESENTATION_MAX+12);
    }*/
}

// Return a pointer to the map's minimap sprite
u8* getMapPreviewSprite(int map) {
//    if(use_fat) {
        FILE* fd;
        int pal_size;//, seek_off = 0;
//        char header[MAP_HEADER_SIZE];
        
        fd = fopen((*maps)[map].filename, "rb");
/*
        // check for new map format
        fread(header, sizeof(char), MAP_HEADER_SIZE, fd);    
        if(strncomp(header, MAP_HEADER_V3, MAP_HEADER_SIZE) == 0)
            seek_off += 8+MAP_HEADER_SIZE;
*/    
        // read minimap palette size
        fseek(fd, MAP_NAME_MAX+PRESENTATION_MAX+8, SEEK_SET);
        fread(&pal_size, sizeof(u32), 1, fd);

        // read minimap sprite
        fseek(fd, MAP_NAME_MAX+PRESENTATION_MAX+pal_size+16, SEEK_SET);
        fread(minimap_sprite, sizeof(u8), 64*64, fd);
        fclose(fd);
        
        return minimap_sprite;
        
/*    } else {
        u32 pal_size = *(u32*)(PA_PAFSFile(map)+MAP_NAME_MAX+PRESENTATION_MAX+8);
        return (u8*)(PA_PAFSFile(map)+MAP_NAME_MAX+PRESENTATION_MAX+pal_size+16);
    }*/
}

// Reload monsters & towers collision maps
void reloadCollisionsMaps() {

    int i, j;
    u32 read_offset = paths_offset;

    // Collision map - monsters
    for(j=0; j<loaded_map.map_Height/16; j++)
        for(i=0; i<loaded_map.map_Width/16; i++) {
            mon_path[i+j*loaded_map.map_Width/16] = (u8)*(u32*)(file+read_offset);
            read_offset += 4; // skip the int
        }

    // Collision map - towers
    for(j=0; j<loaded_map.map_Height/16; j++)
        for(i=0; i<loaded_map.map_Width/16; i++) {
            tow_path[i+j*loaded_map.map_Width/16] = (u8)*(u32*)(file+read_offset);
            read_offset += 4; // skip the int
        }
}

// Load map BG from file
void initMapGfx() {

    int screen = 0;     // touch screen
    int bg_number = 2;

    PA_BgInfo[screen][bg_number].BgMode = map_info[0];   
    PA_LoadBgPal(screen, bg_number, map_pal); 
    PA_DeleteBg(screen, bg_number);
    
    if (PA_BgInfo[screen][bg_number].BgMode == BG_TILEDBG) {       
       PA_LoadBgMap(screen, bg_number, map_tilemap, PA_GetPAGfxBgSize(map_info[1], map_info[2])); 
       PA_LoadBgTilesEx(screen, bg_number, tiles, map_tiles_size);
       PA_InitBg(screen, bg_number, PA_GetPAGfxBgSize(map_info[1], map_info[2]), 0, 1);
    } else {
       PA_BgInfo[screen][bg_number].NTiles = map_tiles_size>>6;
       if (PA_BgInfo[screen][bg_number].NTiles < MAX_TILES) { 
              PA_LoadBgTilesEx(screen, bg_number, tiles, map_tiles_size);
       } else {
              PA_LoadBgTilesEx(screen, bg_number, (void*)Blank, (1008<<5));
       }
       PA_BgInfo[screen][bg_number].Tiles = tiles;
       PA_LoadBgMap(screen, bg_number, Blank, BG_512X256); 
       PA_InitBg(screen, bg_number, BG_512X256, 0, 1);
       PA_InitLargeBg(screen, bg_number, map_info[1]>>3, map_info[2]>>3, map_tilemap);
    }
    PA_BGScrollXY(screen, bg_number, 0, 0);
}

// Load a map
void loadMap(char *map_filename) {
//void loadMap(int map) {

    /******************************************************************************/
    /* Init all map variables from a file and create the structure for the engine */
    /******************************************************************************/

    int i, j, k, map_size;
//    int seek_off = 0;
    u32 read_offset = 0;
//    char header[MAP_HEADER_SIZE];
    FILE* fd;
    
    // init some variables
    strcopy(curr_map_path, map_filename);
    calcMapSizeAndSum(curr_map_path, &curr_map_size, &curr_map_sum);

//    if(use_fat) {

        fd = fopen(map_filename, "rb");
//        fd = fopen((*maps)[map].filename, "rb");
/*        
        // check for header
        fread(header, sizeof(char), MAP_HEADER_SIZE, fd);    
        if(strncomp(header, MAP_HEADER_V3, MAP_HEADER_SIZE) == 0)
            seek_off = 8+MAP_HEADER_SIZE;
*/            
        // read map size
        fseek(fd, MAP_NAME_MAX, SEEK_SET);
        fread(&map_size, sizeof(u32), 1, fd);        

        // load map into memory
        fseek(fd, 0, SEEK_SET);
        file = (u8*)calloc(map_size+4, sizeof(u8));   // allocate memory
        fread(file, 1, map_size+4, fd);
        fclose(fd);    
        
/*    } else {
        file = (*maps)[map].file;
    }*/

    // Free memory
    if(!load_game)
        free(*maps);

    // check for new map format
//    if(strncomp((char*)file, MAP_HEADER_V3, MAP_HEADER_SIZE) == 0)
//        read_offset += 8+MAP_HEADER_SIZE;
        
    // Map name
    strcopy(loaded_map.name, (char*)(file+read_offset));
    read_offset += MAP_NAME_MAX; // skip the map name
    
    // Skip built map size 
    map_size = *(int*)(file+read_offset)+4; // map file size
    read_offset += 4;

    // Map presentation
    strcopy(loaded_map.presentation, (char*)(file+read_offset));
    read_offset += PRESENTATION_MAX; // skip the presentation text
    
    // Simple presentation?
    loaded_map.simple_presentation = *(bool*)(file+read_offset);
    read_offset += 4;   // skip the boolean
    
    // Minimap pal
    u32 pal_size = *(u32*)(file+read_offset); // minimap pal size
    read_offset += 4;   // skip the minimap pal size
    loaded_map.minimap_Pal = (u16*)(file+read_offset);
    read_offset += pal_size;    // skip the minimap pal

    // Minimap sprite
    u32 spr_size = *(u32*)(file+read_offset); // minimap sprite size
    read_offset += 4;   // skip the minimap sprite size
    loaded_map.minimap_Sprite = (u8*)(file+read_offset);
    read_offset += spr_size;    // skip the minimap sprite

    // Map info
    map_info[0] = *(u32*)(file+read_offset);
    map_info[1] = *(u32*)(file+read_offset+4);
    map_info[2] = *(u32*)(file+read_offset+8);
    loaded_map.map_Width = map_info[1];
    loaded_map.map_Height = map_info[2];
    read_offset += 12;   // skip the map bg infos

    // Map palette
    u32 map_pal_size = *(u32*)(file+read_offset); // map palette size
    read_offset += 4;   // skip the map palette size
    map_pal = (u8*)(file+read_offset);  // map bg palette pointer
    read_offset += map_pal_size;    // skip the palette
    
    // Map tilemap
    u32 map_tilemap_size = *(u32*)(file+read_offset); // map tilemap size
    read_offset += 4;   // skip the map tilemap size
    map_tilemap = (u8*)(file+read_offset);  // map bg tilemap pointer
    read_offset += map_tilemap_size;    // skip the tilemap

    // Map tiles
    map_tiles_size = *(u32*)(file+read_offset); // map tiles size
    read_offset += 4;   // skip the map tiles size
    u8* map_tiles = (u8*)(file+read_offset);  // map bg tiles pointer
    read_offset += map_tiles_size;    // skip the tiles
    
    for(i=0; i<map_tiles_size; i++) // copy the tiles in memory
        tiles[i] = map_tiles[i];

    // Welcome message
    strcopy(loaded_map.welcome, (char*)(file+read_offset));
    read_offset += WELCOME_MAX; // skip the welcome message

    // Minimap adjustments
    loaded_map.minimap_x_adjust = (u16)*(u32*)(file+read_offset);
    loaded_map.minimap_y_adjust = (u16)*(u32*)(file+read_offset+4);
    read_offset += 8; // skip the minimap adjustments

    // Starting position
    loaded_map.start_x = *(s32*)(file+read_offset);
    loaded_map.start_y = *(s32*)(file+read_offset+4);
    read_offset += 8; // skip the starting position

    // Spawn position
    loaded_map.spawn_x = (*(s32*)(file+read_offset)) * 16;
    loaded_map.spawn_y = (*(s32*)(file+read_offset+4)) * 16;
    read_offset += 8; // skip the spawn position

    // Various variables
    loaded_map.gold_start = (u16)*(u32*)(file+read_offset);
    loaded_map.sell_pct = (u8)*(u32*)(file+read_offset+4);
    loaded_map.max_towers_start = (u16)*(u32*)(file+read_offset+8);
    loaded_map.init_delay = (u16)*(u32*)(file+read_offset+12);
    loaded_map.lifes = (u16)*(u32*)(file+read_offset+16);
    loaded_map.timed_gold = (bool)*(u32*)(file+read_offset+20);
    loaded_map.initial_bonus = (u16)*(u32*)(file+read_offset+24);
    read_offset += 28; // skip these variables

    /******************************************************************************/

    paths_offset = read_offset;
    
    // Collision map - monsters
    for(j=0; j<loaded_map.map_Height/16; j++)
        for(i=0; i<loaded_map.map_Width/16; i++) {
            mon_path[i+j*loaded_map.map_Width/16] = (u8)*(u32*)(file+read_offset);
            read_offset += 4; // skip the int
        }

    // Collision map - towers
    for(j=0; j<loaded_map.map_Height/16; j++)
        for(i=0; i<loaded_map.map_Width/16; i++) {
            tow_path[i+j*loaded_map.map_Width/16] = (u8)*(u32*)(file+read_offset);
            read_offset += 4; // skip the int
        }

    /******************************************************************************/

    // Waypoints
    loaded_map.num_waypoints[0] = ((u8)*(u32*)(file+read_offset))*2;
    read_offset += 4; // skip the waypoints number
    
    u16* waypoints[MAX_PATHS];
    waypoints[0] = (u16*)calloc(loaded_map.num_waypoints[0], sizeof(u16));   // allocate memory
    
    for(i=0; i<loaded_map.num_waypoints[0]; i++) {
        waypoints[0][i] = (u16)*(u32*)(file+read_offset);
        read_offset += 4; // skip the waypoint
    }
    
    /******************************************************************************/

    // Rounds
    loaded_map.numRounds = (u8)*(u32*)(file+read_offset);
    read_offset += 4; // skip the rounds number
    
    round* rounds = (round*)calloc(loaded_map.numRounds, sizeof(round));   // allocate memory
    
    for(i=0; i<loaded_map.numRounds; i++) {
        strcopy(rounds[i].comment, (char*)(file+read_offset));  // comment
        read_offset += COMMENTS_MAX; // skip the comment

        rounds[i].number[0] = (u16)*(u32*)(file+read_offset);          // number of monsters
        rounds[i].type = (u16)*(u32*)(file+read_offset+4);             // type of monsters
        rounds[i].spawn_rate[0] = (u16)*(u32*)(file+read_offset+8);    // spawn rate of monsters
        rounds[i].nextround_delay = (u16)*(u32*)(file+read_offset+12); // delay for next round
        rounds[i].gold_bonus = (u16)*(u32*)(file+read_offset+16);    // gold bonus for finishing the round
        read_offset += 20; // skip the round
    }
    
    /******************************************************************************/

    // Towers
    int num_towers = (u8)*(u32*)(file+read_offset);
    read_offset += 4; // skip the towers number

    tower* towers = (tower*)calloc(num_towers, sizeof(tower));   // allocate memory

    for(i=0; i<num_towers; i++) {
    
        strcopy(towers[i].name, (char*)(file+read_offset));  // name
        read_offset += NAME_MAX; // skip the name
        strcopy(towers[i].desc, (char*)(file+read_offset));  // description
        read_offset += DESC_MAX; // skip the description
        
        towers[i].pal = (u8)*(u32*)(file+read_offset);              // palette num of the tower
        towers[i].gfx_idx = (u8)*(u32*)(file+read_offset+4);        // gfx index
        towers[i].icon_idx = (u8)*(u32*)(file+read_offset+8);       // gfx icon of the tower (index)
        towers[i].attack_type = (u8)*(u32*)(file+read_offset+12);   // attack type
        int snd_idx = (u8)*(u32*)(file+read_offset+16);             // attack sound index
        towers[i].sound = attack_snds[snd_idx];                                  // attack sound
        towers[i].sound_size = attack_snds_size[snd_idx];                        // attack sound size
        towers[i].price = (u16)*(u32*)(file+read_offset+20);        // price of the tower
        towers[i].minDamage = (u16)*(u32*)(file+read_offset+24);    // minimum damage
        towers[i].maxDamage = (u16)*(u32*)(file+read_offset+28);    // maximum damage
        towers[i].speed = (u16)*(u32*)(file+read_offset+32);        // speed of projectiles
        towers[i].reload = (u16)*(u32*)(file+read_offset+36);       // reload delay (in vbl)
        towers[i].range = (u16)*(u32*)(file+read_offset+40);        // attack range (in pixels)
        towers[i].air = *(bool*)(file+read_offset+44);              // attack air
        towers[i].ground = *(bool*)(file+read_offset+48);           // attack ground
        towers[i].dark = *(bool*)(file+read_offset+52);             // dark variation
        
        towers[i].num_sp = (u8)*(u32*)(file+read_offset+56);        // number of special effects
        read_offset += 60; // skip all previous parameters

        for(j=0; j<towers[i].num_sp; j++) {
            towers[i].special[j] = (u8)*(u32*)(file+read_offset);
            towers[i].spParams[j][0] = (s16)*(s32*)(file+read_offset+4);
            towers[i].spParams[j][1] = (s16)*(s32*)(file+read_offset+8);
            towers[i].spParams[j][2] = (s16)*(s32*)(file+read_offset+12);
            read_offset += 16; // skip special effect
        }

        towers[i].num_upgrades = (u8)*(u32*)(file+read_offset);        // number of possible upgrades
        read_offset += 4; // skip the number of possible upgrades

        for(j=0; j<towers[i].num_upgrades; j++) {
            towers[i].upgrades[j] = (u8)*(u32*)(file+read_offset);
            read_offset += 4; // upgrade
        }

        towers[i].evo_min = (u8)*(u32*)(file+read_offset);    // minimum evolution to build this tower
        read_offset += 4; // skip the minimum evolution to build this tower
    }
    
    /******************************************************************************/

    // Evolutions
    int num_evo = (u8)*(u32*)(file+read_offset);
    read_offset += 4; // skip the evolution number

    evolution* evolutions = (evolution*)calloc(num_evo, sizeof(evolution));   // allocate memory

    for(i=0; i<num_evo; i++) {
        
        strcopy(evolutions[i].name, (char*)(file+read_offset));  // name
        read_offset += NAME_MAX; // skip the name
        strcopy(evolutions[i].info, (char*)(file+read_offset));  // information
        read_offset += INFO_MAX; // skip the information
    
        evolutions[i].icon_idx = (u8)*(u32*)(file+read_offset);         // gfx icon of the evolution (index)
        evolutions[i].build_icon_idx = (u8)*(u32*)(file+read_offset+4); // build gfx icon (index)
        evolutions[i].price = (u16)*(u32*)(file+read_offset+8);         // price of the evolution
        evolutions[i].minRound = (u8)*(u32*)(file+read_offset+12);      // round min. for accessing this evolution
        evolutions[i].num_towers = (u8)*(u32*)(file+read_offset+16);    // number of towers for this evolution
        read_offset += 20; // skip all the previous attributes
        
        for(j=0; j<evolutions[i].num_towers; j++) {
            evolutions[i].towers[j] = (u8)*(s32*)(file+read_offset);
            read_offset += 4; // skip the tower
        }
        
        evolutions[i].max_towers = (u8)*(u32*)(file+read_offset); // max towers bonus for this evolution
        read_offset += 4; // skip the max towers bonus
    }

    /******************************************************************************/

    // Clans
    loaded_map.numClans = (u8)*(u32*)(file+read_offset);
    read_offset += 4; // skip the rounds number
    
    clan* clans = (clan*)calloc(loaded_map.numClans, sizeof(clan));   // allocate memory
    
    for(i=0; i<loaded_map.numClans; i++) {
    
        strcopy(clans[i].name, (char*)(file+read_offset));  // name
        read_offset += NAME_MAX; // skip the name
        strcopy(clans[i].info, (char*)(file+read_offset));  // information
        read_offset += INFO_MAX; // skip the information
    
        clans[i].icon_idx = (u8)*(u32*)(file+read_offset);         // gfx icon of the evolution (index)
        clans[i].num_evolutions = (u8)*(u32*)(file+read_offset+4); // number of evolution levels
        read_offset += 8; // skip the previous attributes
        
        clans[i].evolutions = (evolution*)calloc(clans[i].num_evolutions, sizeof(evolution));   // allocate memory

        int evo_idx;
        for(j=0; j<clans[i].num_evolutions; j++) {
            evo_idx = (u8)*(u32*)(file+read_offset);  // evolution index
            read_offset += 4; // skip the evolution

            // Copy the evolution
            strcopy(clans[i].evolutions[j].name, evolutions[evo_idx].name);
            strcopy(clans[i].evolutions[j].info, evolutions[evo_idx].info);
            clans[i].evolutions[j].icon_idx = evolutions[evo_idx].icon_idx;
            clans[i].evolutions[j].build_icon_idx = evolutions[evo_idx].build_icon_idx;
            clans[i].evolutions[j].price = evolutions[evo_idx].price;
            clans[i].evolutions[j].minRound = evolutions[evo_idx].minRound;
            clans[i].evolutions[j].num_towers = evolutions[evo_idx].num_towers;
            
            for(k=0; k<evolutions[evo_idx].num_towers; k++)
                clans[i].evolutions[j].towers[k] = evolutions[evo_idx].towers[k];
            
            clans[i].evolutions[j].max_towers = evolutions[evo_idx].max_towers;
        }
    }
    
    // Free all evolutions
    free(evolutions);

    /******************************************************************************/

    // Monsters
    int num_monsters = (u8)*(u32*)(file+read_offset);
    read_offset += 4; // skip the evolution number

    monster* monsters = (monster*)calloc(num_monsters, sizeof(monster));   // allocate memory

    for(i=0; i<num_monsters; i++) {
    
        strcopy(monsters[i].name, (char*)(file+read_offset));  // name
        read_offset += NAME_MAX; // skip the name
    
        int mons_idx = (u8)*(u32*)(file+read_offset);           // gfx index of the monster
        monsters[i].gfx = monsters_basics[mons_idx].gfx;                     // gfx of the monster
        monsters[i].sound = monsters_basics[mons_idx].sound;                 // death sound
        monsters[i].sound_size = monsters_basics[mons_idx].sound_size;       // death sound size
        monsters[i].death_anims = monsters_basics[mons_idx].death_anims;     // number of death animation frames
        monsters[i].hide_corpse = monsters_basics[mons_idx].hide_corpse;     // don't show the corpse of the monster after its death
        monsters[i].air = monsters_basics[mons_idx].air;                     // is an air monster
        monsters[i].no_anim = monsters_basics[mons_idx].no_anim;             // no animation

        monsters[i].pal = (u8)*(u32*)(file+read_offset+4);      // palette num of the monster
        monsters[i].invisible = *(bool*)(file+read_offset+8);   // is invisible
        monsters[i].gold_bonus = (u16)*(u32*)(file+read_offset+12);    // gold bonus for killing
        monsters[i].life = (int)*(u32*)(file+read_offset+16);   // life of the monster
        monsters[i].armor = (u16)*(u32*)(file+read_offset+20);  // armor of the monster
        monsters[i].speed = (u8)*(u32*)(file+read_offset+24);   // walk speed of the monster (must be a 32 divisor, and <= 32)

        monsters[i].num_immunes = (u8)*(u32*)(file+read_offset+28);    // number of immunes
        read_offset += 32; // skip all the previous attributes

        for(j=0; j<monsters[i].num_immunes; j++) {
            monsters[i].immune[j] = (u8)*(u32*)(file+read_offset);
            monsters[i].imParams[j][0] = (s16)*(s32*)(file+read_offset+4);
            monsters[i].imParams[j][1] = (s16)*(s32*)(file+read_offset+8);
            monsters[i].imParams[j][2] = (s16)*(s32*)(file+read_offset+12);
            read_offset += 16; // skip the immune
        }

        monsters[i].anim_speed = (u8)*(u32*)(file+read_offset);  // animation speed
        read_offset += 4; // skip the animation speed
    }
    
    /******************************************************************************/

    // Loading of new features since version 0.4
    if(read_offset<map_size) {

        // Additional map properties
        loaded_map.w3_damage_style = *(bool*)(file+read_offset);
        loaded_map.fast_pathfinder = *(bool*)(file+read_offset+4);
        loaded_map.minimap_colors = (u8)*(u32*)(file+read_offset+8);
        loaded_map.num_paths = (u8)*(u32*)(file+read_offset+12);
        read_offset += 16;   // skip the properties

        // Additional monsters properties
        for(i=0; i<num_monsters; i++) {
            monsters[i].ghost = *(bool*)(file+read_offset);         // is transparent?
            monsters[i].size = (u16)*(u32*)(file+read_offset+4);    // monster size: 0=normal, 1=mini
            read_offset += 8;   // skip the properties
        }

        // Additional paths
        for(i=1; i<loaded_map.num_paths; i++) {
        
            loaded_map.num_waypoints[i] = ((u8)*(u32*)(file+read_offset))*2;
            read_offset += 4; // skip the waypoints number
    
            waypoints[i] = (u16*)calloc(loaded_map.num_waypoints[i], sizeof(u16));   // allocate memory
    
            for(j=0; j<loaded_map.num_waypoints[i]; j++) {
                waypoints[i][j] = (u16)*(u32*)(file+read_offset);
                read_offset += 4; // skip the waypoint
            }
        }

        // Additional rounds properties
        for(i=0; i<loaded_map.numRounds; i++) {
            rounds[i].mps_method = (u8)*(u32*)(file+read_offset);          // method of spawning when multiple paths are used
            read_offset += 4; // skip the method

            for(j=1; j<loaded_map.num_paths; j++) {
                rounds[i].number[j] = (u8)*(u32*)(file+read_offset);       // number of monsters
                rounds[i].spawn_rate[j] = (u8)*(u32*)(file+read_offset+4); // spawn rate of monsters
                read_offset += 8; // skip the properties
            }
        }

    } else {
        // for backward compatibility
        loaded_map.w3_damage_style = false;
        loaded_map.fast_pathfinder = false;
        loaded_map.minimap_colors = 0;
        loaded_map.num_paths = 1;
        
        for(i=0; i<num_monsters; i++) {
            monsters[i].ghost = false;         // is transparent?
            monsters[i].size = 1;               // monster size: 0=big, 1=normal, 2=mini
        }
    }

    /******************************************************************************/

    // Assign all pointers to the map
    for(i=0; i<loaded_map.num_paths; i++) {
        loaded_map.waypoints[i] = waypoints[i];
    }

    loaded_map.rounds = rounds;
    loaded_map.towers = towers;
    loaded_map.clans = clans;
    loaded_map.monsters = monsters;
    loaded_map.m_path = mon_path;
    loaded_map.t_path = tow_path;
    
    // Launch the engine
    if(!load_game)
        showPresentation(&loaded_map);
        
    bool play = true;
    while(play) {
        reloadCollisionsMaps();
        initEngine(&loaded_map);
        play = startEngine();
    }
    
    // free memory
    for(i=0; i<loaded_map.num_paths; i++) {
        free(waypoints[i]);
    }

//    free(waypoints);
    free(rounds);
    free(towers);
    
    for(i=0; i<loaded_map.numClans; i++)
        free(clans[i].evolutions);

    free(clans);
    free(monsters);
    
//    if(use_fat)
        free(file);        
}

// Load a previously saved game
void loadSavedGame() {

    int size = 0, sum = 0;

    load_game = true;
    
    fadeOut();
    
    // get infos about map
    EFS_FILE* file = EFS_fopen(FILE_GAMESAVE);
    EFS_fseek(file, sizeof(bool), SEEK_SET);
    EFS_fread(curr_map_path, sizeof(char), 256, file);
    EFS_fread(&curr_map_size, sizeof(int), 1, file);
    EFS_fread(&curr_map_sum, sizeof(int), 1, file);
    EFS_fclose(file);
    
    calcMapSizeAndSum(curr_map_path, &size, &sum);
    
    // launch map
    if(size == curr_map_size && sum == curr_map_sum)
        loadMap(curr_map_path);

    //TODO: else show error msg
}

