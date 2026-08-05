/************************************/
/* Warcraft Tower Defense - by Noda */
/* AI functions            20/03/07 */
/************************************/

// Includes
#include <PA9.h>       // Include for PA_Lib

// Some defines
#define UP      0
#define DOWN    1
#define LEFT    2
#define RIGHT   3
#define NONE    4

// simple pathfinder for monsters : returns a direction
u8 pathfinder(u16 width, u16 height, u8* wall, u16 start_x, u16 start_y, u16 goal_x, u16 goal_y, bool air, u8 last_dir, u8 count);

// advanced pathfinder for monsters : returns a direction
u8 adv_pathfinder(u16 width, u16 height, u8* wall, u16 start_x, u16 start_y, u16 goal_x, u16 goal_y, bool air, u8 last_dir, u8 count);
