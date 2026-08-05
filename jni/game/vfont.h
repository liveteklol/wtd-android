/************************************/
/* Warcraft Tower Defense - by Noda */
/* Variable width fonts    04/09/06 */
/************************************/

#include <PA9.h>       // Include for PA_Lib

// Display text with a custom variable-width font
s16 SmartText(u8 screen, s16 basex, s16 basey, s16 maxx, s16 maxy, char* text, u8 initcolor, u8 size, u8 transp, s32 limit);

// Display text with a custom variable-width font with right align
s16 rightAlignSmartText(u8 screen, s16 basex, s16 basey, s16 maxx, s16 maxy, char* text, u8 color, u8 size, u8 transp);

// Display text with a custom variable-width font with center align
s16 centerAlignSmartText(u8 screen, s16 basex, s16 basey, s16 maxx, s16 maxy, char* text, u8 color, u8 size, u8 transp);
