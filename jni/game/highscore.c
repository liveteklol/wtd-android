/************************************/
/* Warcraft Tower Defense - by Noda */
/* Highscore functions     23/01/08 */
/************************************/

// Includes
#include <PA9.h>       // Include for PA_Lib
#include "types.h"     // Types definitions
#include "defines.h"   // Defines
#include "f_aux.h"
#include "efs_lib.h"
#include "highscore.h"


// search for the given map in the highscore file, get score info and return true if found
bool searchAndGetScore(int map_size, int map_sum, score *score_info) {

    bool found = false;
    int i, total_num = 0;
    score sc;

    EFS_FILE* file = EFS_fopen(FILE_HIGHSCORES);
    EFS_fread(&total_num, sizeof(int), 1, file);

    for(i=0; i < total_num; i++) {
        EFS_fread(&sc, sizeof(score), 1, file);
        
        if(sc.valid && sc.map_size == map_size && sc.map_sum == map_sum) {
            found = true;
            *score_info = sc;
            break;
        }
    }
    EFS_fclose(file);
    
    return found;
}

// set a score for a given map
// if the score block does not exist, a new one is used
// if score limit is reached, the score is ignored and not added
void setScore(score score_info) {

    bool found = false;
    int i, total_num = 0, pos;
    score sc;

    EFS_FILE* file = EFS_fopen(FILE_HIGHSCORES);
    EFS_fread(&total_num, sizeof(int), 1, file);

    for(i=0; i < total_num; i++) {
        EFS_fread(&sc, sizeof(score), 1, file);
        
        if(sc.valid && (sc.map_size == score_info.map_size && sc.map_sum == score_info.map_sum)) {
            found = true;
            
            // update the score
            if(!sc.finished && score_info.finished)
                sc.finished = true;
            
            if(sc.finish_diff < score_info.finish_diff)
                sc.finish_diff = score_info.finish_diff;
                
            if(~sc.score < ~score_info.score) {
                sc.score_diff = score_info.score_diff;
                sc.score = score_info.score;
            }
            
            EFS_fseek(file, -sizeof(score), SEEK_CUR);
            pos = EFS_ftell(file);
            EFS_fclose(file);
            
            // reopen the file because of a libfat bug
            file = EFS_fopen(FILE_HIGHSCORES);            
            EFS_fseek(file, pos, SEEK_SET);
            EFS_fwrite(&sc, sizeof(score), 1, file);
            break;
        }
    }    
    
    pos = EFS_ftell(file);
    EFS_fclose(file);

    // if the score does not already exist, use a new block
    if(!found && i < NUM_SCORES_MAX) {

        // reopen the file because of a libfat bug
        file = EFS_fopen(FILE_HIGHSCORES);            
        EFS_fseek(file, pos, SEEK_SET);

        // append score at the end of previous blocks
        EFS_fwrite(&score_info, sizeof(score), 1, file);
        
        // increment total blocks number
        EFS_fseek(file, 0, SEEK_SET);
        total_num++;
        EFS_fwrite(&total_num, sizeof(int), 1, file);

        EFS_fclose(file);
    }   
    
}



/*
// get the score info at the given index
void getScore(u32 score_index, score *score_info) {

    int i, total_num = 0;

    EFS_FILE* file = EFS_fopen("/highscore");
    EFS_fseek(file, sizeof(int), SEEK_SET);

    for(i=0; i < total_num; i++) {
        EFS_fread(&sc, sizeof(score), 1, file);
        
        if(sc.map_size == map_size && sc.map_sum == map_sum) {
            idx = i;
            break;
        }
    }
    EFS_fclose(file);
}

// set the score info at the given index
void setScore(u32 score_index, score *score_info) {




}
*/
// return the first empty score block & increment the total used blocks number



