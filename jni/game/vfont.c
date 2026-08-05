/************************************/
/* Warcraft Tower Defense - by Noda */
/* Variable width fonts    04/09/06 */
/************************************/

// Based on PAlib code

#include <PA9.h>       // Include for PA_Lib

#include "font/font.c" // Variable width font
#include "font/text0.c" // Variable width font

// The fonts
const u8 *text_Data[2] = {(u8*)(text0Data), (u8*)(fontTiles)};
const u8 police_height[2] = {6, 10};
const u8 police_size[2][256] = {
    { 2, 2, 4, 6, 6, 6, 0, 2, 3, 3, 2, 4, 3, 4, 2, 6, 5, 3, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2, 4, 4, 4, 5,
      6, 5, 5, 4, 5, 4, 4, 5, 5, 4, 5, 5, 4, 6, 5, 5, 5, 5, 5, 5, 4, 5, 5, 6, 5, 5, 4, 3, 6, 3, 4, 5,
      2, 5, 5, 4, 5, 5, 4, 4, 5, 2, 3, 5, 2, 6, 5, 5, 4, 4, 4, 5, 4, 5, 5, 6, 4, 4, 5, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,        
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,        
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,        
      5, 0, 0, 0, 0, 0, 0, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0},        
    { 3, 4, 5, 6, 6, 9, 9, 2, 4, 4, 6, 6, 3, 4, 3, 3, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 3, 3, 6, 6, 6, 5,
      10, 8, 7, 7, 8, 7, 6, 7, 8, 4, 4, 8, 7, 10, 8, 8, 7, 8, 7, 5, 8, 8, 7, 11, 8, 8, 7, 4, 3, 4, 5, 6,
      4, 5, 5, 5, 5, 5, 4, 5, 6, 3, 3, 6, 3, 9, 6, 6, 6, 5, 4, 4, 4, 5, 6, 7, 6, 6, 5, 5, 2, 5, 6, 9,
      6, 9, 4, 6, 5, 10, 5, 6, 4, 11, 5, 4, 10, 9, 7, 9, 9, 4, 4, 6, 5, 4, 6, 11, 4, 12, 4, 4, 8, 9, 5, 8,
      3, 4, 6, 6, 6, 6, 2, 5, 4, 8, 4, 6, 6, 4, 8, 6, 4, 6, 3, 3, 4, 5, 5, 3, 4, 3, 3, 6, 8, 8, 8, 5,
      8, 8, 8, 8, 8, 8, 11, 7, 7, 7, 7, 7, 4, 4, 4, 4, 8, 8, 8, 8, 8, 8, 8, 6, 8, 8, 8, 8, 8, 8, 6, 5,
      5, 5, 5, 5, 5, 5, 8, 5, 5, 5, 5, 5, 3, 3, 3, 3, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 6, 6, 6 },
};

// Prototypes
void _Letter(u8 size, u8 screen, u16 x, u16 y, char letter, u8 color);
void Transp_Letter(u8 size, u8 screen, u16 x, u16 y, char letter, u8 color);
void No_Letter(u8 size, u8 screen, u16 x, u16 y, char letter, u8 color);
void Rot_Letter(u8 size, u8 screen, u16 x, u16 y, char letter, u8 color);
void Rot_Letter2(u8 size, u8 screen, u16 x, u16 y, char letter, u8 color);

letterfp _letters[5] = {(letterfp)&_Letter, (letterfp)&Transp_Letter, (letterfp)&No_Letter, (letterfp)&Rot_Letter, (letterfp)&Rot_Letter2};
    
extern inline void _normala(u8 screen, u16 i, s8 ly, s16 pos, u8 *data, u8 color){
    u8 j;
    s16 temp;

    for (j = 0; j < 8; j++){
        temp = (data[i + (j << 3)]) << 8;
            PA_DrawBg[screen][pos] &= 255;
        PA_DrawBg[screen][pos] |= temp*color;    
        pos+=128;        
    }
    ly -= 8;
    for (j = 0; j < ly; j++){
        temp = (data[128 + i + (j << 3)]) << 8;
            PA_DrawBg[screen][pos] &= 255;
        PA_DrawBg[screen][pos] |= temp*color;    
        pos+=128;        
    }            
}

extern inline void _normalb(u8 screen, u16 i, s8 ly, s16 pos, u8 *data, u8 color){
    u8 j;
    s16 temp;

    for (j = 0; j < 8; j++){
        temp = data[i + (j << 3)];
        PA_DrawBg[screen][pos] &= ~255;
        PA_DrawBg[screen][pos] |= temp*color;
        pos+=128;    
    }
    ly -= 8;
    for (j = 0; j < ly; j++){
        temp = (data[128 + i + (j << 3)]);
        PA_DrawBg[screen][pos] &= ~255;
        PA_DrawBg[screen][pos] |= temp*color;
        pos+=128;        
    }    
}

void No_Letter(u8 size, u8 screen, u16 x, u16 y, char letter, u8 color) {}

void Rot_Letter(u8 size, u8 screen, u16 x, u16 y, char letter, u8 color) {
    u8 lx = police_size[size][(u8)letter];
    u8 ly = police_height[size];
    u8 *data;
    if (size > 0) data = (u8*)(text_Data[size] + (letter << 8)); // Big font
    else data = (u8*)(text_Data[size] + (letter << 6));          // Small

    s16 i, j;
    s16 temp = x;
    x = 255 - y;
    y = temp;

    if (lx > 8){
        for (i = 0; i < 8; i++){
            for (j = 0; j < 8; j++)
                PA_Put8bitPixel(screen, x - j, y + i, data[i + (j << 3)]*color);
            for (j = 0; j < ly-8; j++)
                PA_Put8bitPixel(screen, x - j - 8, y + i, data[128 + i + (j << 3)]*color);    
        }
        data = (u8*)(data+64);
        lx -= 8;
        y+= 8;
    }

    for (i = 0; i < lx; i++){
        for (j = 0; j < 8; j++)
            PA_Put8bitPixel(screen, x - j, y + i, data[i + (j << 3)]*color);
        for (j = 0; j < ly-8; j++)
            PA_Put8bitPixel(screen, x - j - 8, y + i, data[128 + i + (j << 3)]*color);
    }
}


void Rot_Letter2(u8 size, u8 screen, u16 x, u16 y, char letter, u8 color) {
    u8 lx = police_size[size][(u8)letter];
    u8 ly = police_height[size];
    u8 *data;
    if (size > 0) data = (u8*)(text_Data[size] + (letter << 8)); // Big font
    else data = (u8*)(text_Data[size] + (letter << 6));          // Small

    s16 i, j;
    s16 temp = x;
    x = y;
    y = 191 - temp;

    if (lx > 8){
        for (i = 0; i < 8; i++){
            for (j = 0; j < 8; j++)
                PA_Put8bitPixel(screen, x + j, y - i, data[i + (j << 3)]*color);
            for (j = 0; j < ly-8; j++)
                PA_Put8bitPixel(screen, x + j + 8, y - i, data[128 + i + (j << 3)]*color);    
        }
        data = (u8*)(data+64);
        lx -= 8;
        y-= 8;
    }

    for (i = 0; i < lx; i++){
        for (j = 0; j < 8; j++)
            PA_Put8bitPixel(screen, x + j, y - i, data[i + (j << 3)]*color);
        for (j = 0; j < ly-8; j++)
            PA_Put8bitPixel(screen, x + j + 8, y - i, data[128 + i + (j << 3)]*color);
    }
}

void _Letter(u8 size, u8 screen, u16 x, u16 y, char letter, u8 color) {
    u8 lx = police_size[size][(u8)letter];
    u8 ly = police_height[size];
    u8 *data;
    if (size > 0) data = (u8*)(text_Data[size] + (letter << 8));
    else data = (u8*)(text_Data[size] + (letter << 6));

    u16 i;
    u16 pos;

    if (lx > 8) {
        for (i = 0; i < 8; i++){
            pos = ((i + x) >> 1) + (y << 7);
            if ((i + x)&1){
                _normala(screen, i, ly, pos, data, color);    
            }
            else{
                _normalb(screen, i, ly, pos, data, color);
            }
        }
        x += 8;
        lx -= 8;
        data = (u8*)(data+64);
    }

    for (i = 0; i < lx; i++){
        pos = ((i + x) >> 1) + (y << 7);
        if ((i + x)&1){
            _normala(screen, i, ly, pos, data, color);
        }
        else{
            _normalb(screen, i, ly, pos, data, color);
        }
    }
}

extern inline void _transpa(u8 screen, u16 i, s8 ly, s16 pos, u8 *data, u8 color){
    u8 j;
    s16 temp;

    for (j = 0; j < 8; j++){
        temp = (data[i + (j << 3)]) << 8;
        PA_DrawBg[screen][pos] &= ~(temp*255);
        PA_DrawBg[screen][pos] |= temp*color;    
        pos+=128;        
    }
    ly -= 8;
    for (j = 0; j < ly; j++){
        temp = (data[128 + i + (j << 3)]) << 8;
        PA_DrawBg[screen][pos] &= ~(temp*255);
        PA_DrawBg[screen][pos] |= temp*color;    
        pos+=128;        
    }            
}

extern inline void _transpb(u8 screen, u16 i, s8 ly, s16 pos, u8 *data, u8 color){
    u8 j;
    s16 temp;

    for (j = 0; j < 8; j++){
        temp = data[i + (j << 3)];
        PA_DrawBg[screen][pos] &= ~(temp*255);
        PA_DrawBg[screen][pos] |= temp*color;
        pos+=128;    
    }
    ly -= 8;
    for (j = 0; j < ly; j++){
        temp = (data[128 + i + (j << 3)]);
        PA_DrawBg[screen][pos] &= ~(temp*255);
        PA_DrawBg[screen][pos] |= temp*color;
        pos+=128;        
    }    
}

void Transp_Letter(u8 size, u8 screen, u16 x, u16 y, char letter, u8 color) {
    u8 lx = police_size[size][(u8)letter];
    u8 ly = police_height[size];
    u8 *data;
    if (size > 0) data = (u8*)(text_Data[size] + (letter << 8));
    else data = (u8*)(text_Data[size] + (letter << 6));

    u16 i;
    u16 pos;

    if (lx > 8) {
        for (i = 0; i < 8; i++){
            pos = ((i + x) >> 1) + (y << 7);
            if ((i + x)&1){
                _transpa(screen, i, ly, pos, data, color);    
            }
            else{
                _transpb(screen, i, ly, pos, data, color);
            }
        }
        x += 8;
        lx -= 8;
        data = (u8*)(data+64);
    }

    for (i = 0; i < lx; i++){
        pos = ((i + x) >> 1) + (y << 7);
        if ((i + x)&1){
            _transpa(screen, i, ly, pos, data, color);
        }
        else{
            _transpb(screen, i, ly, pos, data, color);
        }
    }
}

s16 SmartText(u8 screen, s16 basex, s16 basey, s16 maxx, s16 maxy, char* text, u8 color, u8 size, u8 transp, s32 limit){
s16 i, j;
s16 x, y;
s16 lx, ly;
s16 letter; 

ly = police_height[size];

x = basex;
y = basey;

s16 length = 0;
s16 ylimiy = maxy - ly;
s16 wordx = 0;
s16 wordletter = 0;

for (i = 0; (text[i] && y <= ylimiy && i < limit); i++) {
    if (text[i] == '\n'){
        if (transp == 0){
            while(x < maxx) { 
                for (j = 0; j < ly; j++) PA_Put8bitPixel(screen, x, y + j, 0);
                x++;
            }
        }
        if (text[i+1] == ' ') i++; 
        x = basex;
        y += ly;    
    } else {
        wordletter = 1;
        wordx = 0;
        
        while(!((text[i+wordletter] <= 32) || (i + wordletter >= limit))) { 
            letter = text[i+wordletter] - 32;
            lx = police_size[size][letter];
            wordx += lx;
            wordletter++;
        }
        
        if (x + wordx >= maxx ) { 
            if (transp == 0){
                while(x < maxx) { 
                    for (j = 0; j < ly; j++) PA_Put8bitPixel(screen, x, y + j, 0);
                    x++;
                }
            }
        
            x = basex;
            y += ly;
        
            if(text[i] != ' ') {
                if(y <= ylimiy) {
                    for (j = i; j < (i + wordletter); j++) {
                        letter = text[j] - 32;
//                        if (letter > 128) letter -= 96;
                        lx = police_size[size][letter];
                        _letters[transp](size, screen, x, y, letter, color);                
                        x += lx;
                    }
                    i+=wordletter-1;
                } 
                else i--;
            }
        } else { 
            s32 jmax = (i + wordletter);
            if (text[(i + wordletter-1)] < 32) jmax--; 
            
            for (j = i; j < jmax; j++) {
                letter = text[j] - 32;
                lx = police_size[size][letter];
                _letters[transp](size, screen, x, y, letter, color);                
                x += lx;
            }
            i+=wordletter-1;
        }

    }
}

length = i;
if (transp == 0){ 
    while(x < maxx) {
        for (i = 0; i < ly+5; i++) PA_Put8bitPixel(screen, x, y + i, 0);
        x++;
    }
    
    y += ly;
    basey = y;
    
    if (basex&1) {
        while(y < maxy) {
            PA_Put8bitPixel(screen, basex, y, 0);
            y++;
        }
        ++basex;
    }
    
    for (x = basex; x < maxx; x++)
        for (y = basey; y < maxy; y++) PA_DrawBg[screen][(x >> 1) + (y << 7)] = 0;
}

return length;
}

s16 rightAlignSmartText(u8 screen, s16 basex, s16 basey, s16 maxx, s16 maxy, char* text, u8 color, u8 size, u8 transp){
s16 i;
s16 x, y;
s16 lx, ly;
u8 letter; 

ly = police_height[size];

x = basex;
y = basey;

s16 length = 0;


s16 nlines = 0; // Nombre de lignes
s16 xsize[30]; // Taille en largeur de chaque ligne

xsize[0] = 0;

for (i = 0; text[i]; i++){ // Calcul du centrage
    if (text[i] == '\n'){
        nlines++;
        xsize[nlines] = 0; // Rien, par défaut
    } else {
        letter = text[i] - 32;
//        if (letter > 128) letter -= 96; // pour les accents...
        xsize[nlines] += police_size[size][letter];
    }
}

++nlines; // Si 0, ca fait 1 ligne
y = basey;
nlines = 0;
x = maxx - xsize[nlines];
for (i = 0; text[i]; i++) {
    if (text[i] == '\n'){
        ++nlines;
        x = maxx - xsize[nlines];
        y += ly;    
    } else {
        letter = text[i] - 32;
        lx = police_size[size][letter];
        _letters[transp](size, screen, x, y, letter, color);                
        x += lx;
    }

}

length = i;

return length;

}

s16 centerAlignSmartText(u8 screen, s16 basex, s16 basey, s16 maxx, s16 maxy, char* text, u8 color, u8 size, u8 transp){
s16 i;
s16 x, y;
s16 lx, ly;
u8 letter; 

ly = police_height[size];

x = basex;
y = basey;

s16 length = 0;


s16 nlines = 0; // Nombre de lignes
s16 xsize[30]; // Taille en largeur de chaque ligne

xsize[0] = 0;

for (i = 0; text[i]; i++){ // Calcul du centrage
    if (text[i] == '\n'){
        nlines++;
        xsize[nlines] = 0; // Rien, par défaut
    } else {
        letter = text[i] - 32;
//        if (letter > 128) letter -= 96; // pour les accents...
        xsize[nlines] += police_size[size][letter];
    }
}

++nlines; // Si 0, ca fait 1 ligne
y = basey;
nlines = 0;
x = basex + (maxx-basex-xsize[nlines])/2;
for (i = 0; text[i]; i++) {
    if (text[i] == '\n'){
        ++nlines;
        x = basex + (maxx-basex-xsize[nlines])/2;
        y += ly;    
    } else {
        letter = text[i] - 32;
        lx = police_size[size][letter];
        _letters[transp](size, screen, x, y, letter, color);                
        x += lx;
    }

}

length = i;

return length;

}
