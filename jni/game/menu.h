/************************************/
/* Warcraft Tower Defense - by Noda */
/* Menu functions          11/03/07 */
/************************************/

// Includes
#include <PA9.h>       // Include for PA_Lib


// Menu pop up (screen 0, BG1)
void menuPopUp();

// Menu pop down (screen 0, BG1)
void menuPopDown();

// Init snow flakes
void initSnowFlakes();

// Moves snow flakes & flash the eyes
void moveSnowFlakes();

// Waits & moves snow flakes & flash the eyes
void waitNFlakes(int time);

// Init the flashing eyes
void initFlashEyes();

// Flash the eyes
void FlashEyes();

// Display the selected button
void dispSelected(s16 x, s16 y);

// Display the selected button for options
void dispSelectedOption(s16 x, s16 y);

// Main menu - returns selected menu item
u8 mainMenu();

// Credits
void credits();

// Options
void options();

// display map list
void dispMapList();

// Map selection
bool selectMap();

