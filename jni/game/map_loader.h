/************************************/
/* Warcraft Tower Defense - by Noda */
/* Map Loader              13/02/07 */
/************************************/

// Includes
#include <PA9.h>        // Include for PA_Lib

#include "types.h"      // Types definitions


// List all maps from filesystem
void listMaps(map_desc** maplist, int* num);

// Return a pointer to the map's name
//char* getMapName(int map);

// Return a pointer to the map's minimap palette
u16* getMapPreviewPal(int map);

// Return a pointer to the map's minimap sprite
u8* getMapPreviewSprite(int map);

// Load map BG from file
void initMapGfx();

// Load a map
void loadMap(char *map_filename);
//void loadMap(int map);

// Load a previously saved game
void loadSavedGame();

