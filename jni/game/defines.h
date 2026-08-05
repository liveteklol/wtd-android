/************************************/
/* Warcraft Tower Defense - by Noda */
/* Defines                 12/02/08 */
/************************************/

#ifndef __DEFINES__
#define __DEFINES__

// Gfx
#include "gfx/all_gfx.h"

// Sfx
#include "BasicHumanDead.h"
#include "BasicOrcDead.h"
#include "BuildingDeathLargeOrc.h"
#include "Explosion.h"
#include "GoblinSapperDeath.h"
#include "GoblinZeppelinDeath.h"
#include "GruntDeath.h"
#include "HeroFirelordDeath.h"
#include "MurlocDeath.h"
#include "NetherDragonDeath.h"
#include "OgreDeath.h"
#include "OrcWarlockDeath.h"
#include "RiddenHippogryphDeath.h"
#include "SealDeath.h"
#include "SheepDeath.h"
#include "ShipSinking.h"
#include "SkeletonDeath.h"
#include "TinkerDeath.h"
#include "WendigoDeath.h"
#include "ZombieDeath.h"


// map special
#define MAP_HEADER_SIZE   5         // map header size
#define MAP_HEADER_V3     "TDMv3"   // header for maps version 3 (new format)

// used files
#define FILE_SETTINGS     "/settings"   // file used for settings
#define FILE_GAMESAVE     "/last_game"  // file used for the quick save
#define FILE_HIGHSCORES   "/highscores" // file used for high scores

// Values
#define NOTHING           0     // default value
#define TOTAL_TOWERS      64    // total number of towers
#define MAP_NAME_MAX      24    // max number of characters for map names
#define NAME_MAX          20    // max number of characters for names
#define INFO_MAX          128   // max number of characters for evolution info
#define DESC_MAX          80    // max number of characters for tower description
#define WELCOME_MAX       96    // max number of characters for welcome message
#define PRESENTATION_MAX  256   // max number of characters for presentation message
#define COMMENTS_MAX      64    // max number of characters for comments
#define SPECIAL_MAX       4     // max number of special effects for towers
#define SP_PARAMS         3     // number of parameters for special effects
#define EVO_TOWERS        11    // number of towers for an evolution
#define IMMUNE_MAX        4     // max number of immune for a monster
#define IM_PARAMS         3     // number of parameters for immunes
#define ROUNDS_MAX        256   // max number of rounds
#define TOWERS_MAX        256   // max number of towers
#define EVOLUTIONS_MAX    8     // max number of evolutions
#define MONSTERS_MAX      256   // max number of monsters
#define CLANS_MAX         8     // max number of clans
#define UPGRADES_MAX      8     // max number of upgrades per tower
#define MAP_WIDTH_MAX     128   // max horizontal tiles for a map
#define MAP_HEIGHT_MAX    128   // max vertical tiles for a map
#define TOWER_SIZE        576   // (tower space range from its center)² = 24*24, used for hiding monster when building towers
#define MONSTER_PALS_NUM  4     // number of monsters palettes
#define MAX_PATHS         4     // number of max paths
#define MINIMAP_COLORSETS_NUM   4   // number of minimap colorsets
#define FIXED_POINT_PRECISION   16   // number of bits for fixed point decimals
#define DBL_TAP_ZONE      48    // double tap detection zone

// Special effects (effects parameters) 
#define NO_SP       0
#define ADD_MAX_T   1           // adds more max towers (add to max towers, 0, 0)
#define SLOW        2           // slow down (effect time in msec, move delay in vbl, effect delay in vbl)
#define POISON      3           // poison, damage every sec (effect time in msec, min damage, max damage)
#define CRITIC      4           // critic damage (percentage to do, 0, 0)
#define PIERCE      5           // ignore armor (armor malus, 0, 0)
#define FIRE        6           // fire magic (min damage, max damage, 0)
#define WATER       7           // water magic (min damage, max damage, 0)
#define LIGHTNING   8           // lightning magic (min damage, max damage, 0)
#define WIND        9           // wind magic (min damage, max damage, 0)
#define SPLASH      10          // splash damage (radius, 0, 0) ONLY EFFECTIVE WITH GROUND MONSTERS
#define REVEAL      11          // reveal invisible monsters (radius, 0, 0)

// Immunes (immunes parameters)
#define NO_IMM      0
#define SLOW        2           // slow (effect time malus in msec, move delay malus in vbl, effect delay bonus in vbl)
#define POISON      3           // poison (effect time malus in msec, damage malus, 0)
#define FIRE        6           // fire magic (damage malus, 0, 0)
#define WATER       7           // water magic (damage malus, 0, 0)
#define LIGHTNING   8           // lightning magic (damage malus, 0, 0)
#define WIND        9           // wind magic (damage malus, 0, 0)
#define IM_NORMAL   12          // immune to normal damage
#define IM_PIERCE   13          // immune to pierce effects
#define IM_MAGIC    14          // immune to magic [fire, water, lightning & wind] (0, 0, 0)

// Game engine defines
#define MONSTERS_INST_MAX   40  // Maximum monsters instances
#define TOWERS_INST_MAX     40  // Maximum tower instances
#define ATTACKS_INST_MAX    40  // Maximum attack instances

// Sprites IDs (screen 0)
#define BASE_MONSTERS_ID    43  // base sprite id for monsters
#define BASE_TOWERS_ID      123 // base sprite id for towers
#define BASE_ATTACK_ID      83  // base sprite id for attacks
#define BUILD_MENU_ID       124 // build menu sprite id (use 2 sprites)
#define SELECTED_ID         126 // selected flash sprite id (use 2 sprites)
#define BUILD_OK_ID         2   // sprite id for build ok sprite
#define BUILD_TOWER_ID      1   // sprite id for building temp tower
#define SELECTION_ID        0   // sprite id for selection round

// Sprites IDs (screen 1)
#define BASE_ICONS_ID       10  // base sprite id for icons
#define BASE_SP_ICONS_ID    32  // base sprite id for special effects icons
#define MINIMAP_ID          5   // sprite id for minimap
#define HIDE_BOX_ID         1   // base id for hiding box

// Palettes IDs (screen 0)
#define MONSTERS_PAL_ID     8   // palette id for monsters
#define SELECT_DIFF_ID      9    // palette id for difficulty selection buttons
#define M_FROZ_PAL_ID       6   // palette id for monsters (frozen)
#define M_POIS_PAL_ID       7   // palette id for monsters (poison)
//#define M_RED_PAL_ID        8   // palette id for monsters (red)
//#define M_BLUE_PAL_ID       9   // palette id for monsters (blue)
//#define M_GREEN_PAL_ID      10  // palette id for monsters (green)
//#define M_DARK_PAL_ID       11  // palette id for monsters (dark)
#define ATTACKS_PAL_ID      12  // palette id for attacks
#define T_NORM_PAL_ID       1   // palette id for towers (normal)
#define T_FROZ_PAL_ID       2   // palette id for towers (frozen)
#define T_POIS_PAL_ID       3   // palette id for towers (poison)
#define T_DARK_PAL_ID       4   // palette id for towers (dark)
#define BUILD_SELEC_PAL_ID  5   // palette id for build selector
#define BUILD_MENU_PAL_ID   14  // palette id for build menu
#define SELECTED_PAL_ID     15  // palette id for selected flash sprite

// Palettes IDs (screen 1)
#define ICONS_PAL_ID        12  // palette id for icons
#define ICONS_DARK_PAL_ID   13  // palette id for dark icons
#define MINIMAP_PAL_ID      4   // palette id for minimap
#define HIDE_BOX_PAL_ID     14  // palette id for hiding box

// Zoom sets
#define ZOOMSET_BUILD_TOWER 0   // zoom set for building towers
#define ZOOMSET_MONSTERS    1   // zoom set for monsters

// Minimap position and default build position
#define MINIMAP_WIDTH       58  // width of the minimap
#define MINIMAP_HEIGHT      62  // height of the minimap
#define MINIMAP_X           5   // x base of the minimap
#define MINIMAP_Y           126 // y base of the minimap
#define BUILD_X_OFFSET      120 // default x offset for new towers             
#define BUILD_Y_OFFSET      88  // default y offset for new towers             

// Attacks direction
#define A_UP                0
#define A_UP_RIGHT          1
#define A_RIGHT             2
#define A_DOWN_RIGHT        3
#define A_DOWN              4
#define A_UP_LEFT           5
#define A_LEFT              6
#define A_DOWN_LEFT         7

// Spawn rate for monsters
#define SR_SLOW             119
#define SR_NORMAL           59
#define SR_FAST             29
#define SR_VERYFAST         15
#define SR_CONTINUOUS       9

// Walk speed for monsters
#define W_NORMAL            1
#define W_FAST              2
#define W_VERYFAST          4

// Animation speed for monsters
#define MA_SLOW             8
#define MA_NORMAL           4
#define MA_FAST             2
#define MA_VERYFAST         1

// Projectiles speed for towers
#define A_VERYSLOW          4
#define A_SLOW              8
#define A_NORMAL            16
#define A_FAST              32
#define A_VERYFAST          64

// Multiple spawn methods
#define ALTERNATE           0
#define SIMULTANEOUS        1
#define ORDERED             2

// Monsters
// --------
// define MONSTER           sprite, death sound, death sound size, num of death anim, hide corpse, air, no anim
#define M_PEON              (u8*)m_peon_Sprite, (u8*)BasicHumanDead, (u32)BasicHumanDead_size, 3, false, false, false
#define M_AXETHROWER        (u8*)m_axethrower_Sprite, (u8*)BasicHumanDead, (u32)BasicHumanDead_size, 3, false, false, false
#define M_GRUNT             (u8*)m_grunt_Sprite, (u8*)BasicOrcDead, (u32)BasicOrcDead_size, 3, false, false, false
#define M_PEASANT           (u8*)m_peasant_Sprite, (u8*)BasicHumanDead, (u32)BasicHumanDead_size, 3, false, false, false
#define M_ROGUE             (u8*)m_archer_Sprite, (u8*)OrcWarlockDeath, (u32)OrcWarlockDeath_size, 3, false, false, false
#define M_SHEEP             (u8*)m_sheep_Sprite, (u8*)SheepDeath, (u32)SheepDeath_size, 1, false, false, false
#define M_DAEMON            (u8*)m_daemon_Sprite, (u8*)HeroFirelordDeath, (u32)HeroFirelordDeath_size, 5, false, false, false
#define M_DEATH_KNIGHT      (u8*)m_death_knight_Sprite, (u8*)ZombieDeath, (u32)ZombieDeath_size, 4, true, false, false
#define M_GOBLINS           (u8*)m_goblins_Sprite, (u8*)MurlocDeath, (u32)MurlocDeath_size, 7, true, false, false
#define M_DWARVES           (u8*)m_dwarves_Sprite, (u8*)MurlocDeath, (u32)MurlocDeath_size, 4, true, false, false
#define M_FOOTMAN           (u8*)m_footman_Sprite, (u8*)TinkerDeath, (u32)TinkerDeath_size, 3, false, false, false
#define M_KNIGHT            (u8*)m_knight_Sprite, (u8*)GruntDeath, (u32)GruntDeath_size, 5, false, false, false
#define M_MAGE              (u8*)m_mage_Sprite, (u8*)BuildingDeathLargeOrc, (u32)BuildingDeathLargeOrc_size, 7, true, false, false
#define M_OGRE              (u8*)m_ogre_Sprite, (u8*)OgreDeath, (u32)OgreDeath_size, 5, false, false, false
#define M_BIG_SHEEP         (u8*)m_sheep_big_Sprite, (u8*)SheepDeath, (u32)SheepDeath_size, 1, false, false, false
#define M_SKELETON          (u8*)m_skeleton_Sprite, (u8*)SkeletonDeath, (u32)SkeletonDeath_size, 5, false, false, false
#define M_GRYPHON           (u8*)m_gryphon_Sprite, (u8*)RiddenHippogryphDeath, (u32)RiddenHippogryphDeath_size, 6, true, true, false
#define M_HELICOPTER        (u8*)m_helicopter_Sprite, (u8*)GoblinSapperDeath, (u32)GoblinSapperDeath_size, 2, true, true, false
#define M_ZEPPELIN          (u8*)m_zeppelin_Sprite, (u8*)GoblinZeppelinDeath, (u32)GoblinZeppelinDeath_size, 2, true, true, false
#define M_DRAGON            (u8*)m_dragon_Sprite, (u8*)NetherDragonDeath, (u32)NetherDragonDeath_size, 5, true, true, false
#define M_BALLISTA          (u8*)m_ballista_Sprite, (u8*)Explosion, (u32)Explosion_size, 2, true, false, false
#define M_BATTLESHIP        (u8*)m_battleship_Sprite, (u8*)ShipSinking, (u32)ShipSinking_size, 2, true, false, true
#define M_BUFFALO           (u8*)m_buffalo_Sprite, (u8*)WendigoDeath, (u32)WendigoDeath_size, 1, false, false, false
#define M_CATAPULT          (u8*)m_catapult_Sprite, (u8*)Explosion, (u32)Explosion_size, 2, true, false, false
#define M_CLERIC            (u8*)m_cleric_Sprite, (u8*)TinkerDeath, (u32)TinkerDeath_size, 3, true, false, false
#define M_ELVEN_DESTROYER   (u8*)m_elven_destroyer_Sprite, (u8*)ShipSinking, (u32)ShipSinking_size, 2, true, false, true
#define M_ELVEN_TANKER      (u8*)m_elven_tanker_Sprite, (u8*)ShipSinking, (u32)ShipSinking_size, 2, true, false, true
#define M_HUMAN_TANKER      (u8*)m_human_tanker_Sprite, (u8*)ShipSinking, (u32)ShipSinking_size, 2, true, false, true
#define M_HUMAN_TRANSPORT   (u8*)m_human_transport_Sprite, (u8*)ShipSinking, (u32)ShipSinking_size, 2, true, false, true
#define M_JUGGERNAUT        (u8*)m_juggernaut_Sprite, (u8*)ShipSinking, (u32)ShipSinking_size, 2, true, false, true
#define M_NECROLYTE         (u8*)m_necrolyte_Sprite, (u8*)TinkerDeath, (u32)TinkerDeath_size, 3, false, false, false
#define M_ORC_TRANSPORT     (u8*)m_orc_transport_Sprite, (u8*)ShipSinking, (u32)ShipSinking_size, 2, true, false, true
#define M_SEAL              (u8*)m_seal_Sprite, (u8*)SealDeath, (u32)SealDeath_size, 1, false, false, false
#define M_SUBMARINE         (u8*)m_submarine_Sprite, (u8*)ShipSinking, (u32)ShipSinking_size, 2, true, false, true
#define M_TROLL_DESTROYER   (u8*)m_troll_destroyer_Sprite, (u8*)ShipSinking, (u32)ShipSinking_size, 2, true, false, true
#define M_TURTLE            (u8*)m_turtle_Sprite, (u8*)ShipSinking, (u32)ShipSinking_size, 2, true, false, true

// Towers
#define ALL_TOWERS (u8*)t_all_normal_Sprite

// Attacks
#define ATTACKS_NUM         46  // number of different attacks
#define A_ARROW             0
#define A_BULLET            1
#define A_CANNON            2
#define A_FIRE_LIGHTNING    3
#define A_FIREROCK          4
#define A_ICEROCK           5
#define A_LIGHTNING         6
#define A_MUDROCK           7
#define A_POISON_LIGHTNING  8
#define A_POISONROCK        9

// Sound defines
#define SND_QUEUE_SIZE  16
/*#define SND_STANDARD    0
#define SND_DEATH       1
#define SND_ATTACK      2
#define SND_VOICE       3*/
#define SND_STANDARD    20
#define SND_DEATH       1
#define SND_ATTACK      1
#define SND_VOICE       30
#define SND_DEATH_MAX   4
#define SND_ATTACK_MAX  4
#define SND_VOICE_MAX   4

// Some functions
#define TO_ABS(x, y)  if(x<0) x=-x; if(y<0) y=-y;
#define PA_UpdateSpriteGfxAndMem(screen, spr, gfx)  { u16 _gfx_num = PA_GetSpriteGfx(screen, spr); DMA_Copy((gfx), (void*)(SPRITE_GFX1 + (0x200000 *  (screen)) + ((_gfx_num) << NUMBER_DECAL)), (used_mem[screen][_gfx_num] << (MEM_DECAL+1)), DMA_16NOW); PA_SpriteAnimP[screen][_gfx_num] = gfx; }

#endif
