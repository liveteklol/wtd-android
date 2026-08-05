/************************************/
/* Warcraft Tower Defense - by Noda */
/* Engine functions        21/01/08 */
/************************************/

// Includes
#include <PA9.h>       // Include for PA_Lib
#include "types.h"     // Types definitions

// Presentation of the map
void showPresentation(map* map);

// Init the game engine
void initEngine(map* map);

// Main loop
bool startEngine();

// Almost everything of the game engine...
inline bool engineActions();

// Display statistics of the game
void statScreen();

// Animate a monster
void monsterAnim(u8 num);

// AI, animation & movement for a monster
void monsterEngine(u8 m);

// display monsters
inline void showMonsters();

// AI, animation & movement for all monsters
inline void monstersEngine();

// Update the minimap display
inline void updateMinimap();

// Refresh UI
inline void refreshUI();

// Spawn monsters
inline void spawnMonsters();

// AI, animation & attacks for a tower
inline void towerEngine(u8 t);

// do dark effect on towers
inline void doDarkTowers();

// towers reveal monsters
inline void towerReveal();

// AI, animation & attacks for all tower
inline void towersEngine();

// Load text palette on desired screen
void loadTextPalette(u8 screen, u8 base_index);

// Make all monsters disappear and reset monsters number for a new round
inline void vanishMonsters();

// Check if a new round starts
inline void checkForNewRound();

// Interface interactions
inline void interfaceInteract();

// Build new tower
inline void buildTower();

// Display interface icons
inline void refreshIcons();

// test if a tower or a monster is selected
inline void checkForSelection();

// Won
void gameWin();

// Loose
void gameLose();

// Pause
void gamePause();

// Choose game difficulty
void chooseDifficulty();    

// Show in-game menu
void showIngameMenu();

// show a simple modal dialog on the screen
void simpleDialog(char* title, u8 title_color, char* text, u8 text_color, bool center_text, char* button, u8 button_color);

// Sell a tower
inline void sellTower();

// upgrade selected tower
inline void upgradeTower();

// manage build menu
inline void manageBuildMenu();

// draw shadow for air monster
//inline void drawAirShadow(u16 x, u16 y);

// save game
inline void saveGame();

// load game
inline void loadGame();

