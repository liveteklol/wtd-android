/************************************/
/* Warcraft Tower Defense - by Noda */
/* Highscore functions     23/01/08 */
/************************************/

// Includes
#include <PA9.h>       // Include for PA_Lib
#include <nds.h>

#include "types.h"

#define NUM_SCORES_MAX  4096    // the maximum number of scores blocks


// search for the given map in the highscore file, get score info and return true if found
bool searchAndGetScore(int map_size, int map_sum, score *score_info);

// set a score for a given map
// if the score block does not exist, a new one is used
// if score limit is reached, the score is ignored and not added
void setScore(score score_info);

