/************************************/
/* Warcraft Tower Defense - by Noda */
/* AI functions            20/03/07 */
/************************************/

// Includes
#include <PA9.h>       // Include for PA_Lib
#include "defines.h"
#include "f_aux.h"
#include "ai.h"
#include "nds.h"
#include "binheap.h"

// global variables
u16 _hmax, _height, _wmax, _width;
u8 _goal_x, _goal_y;
u8* _wall;
u16* _cost;
u8* _path;


// simple heuristic search for path
bool find_path(u8 sx, u8 sy, u8 gx, u8 gy, u8 last_dir) {

    bool up = false, down = false, right = false, left = false;

    if(_cost[gx+gy*_width] != 0) 
        return true;
        
    int current_cost = _cost[sx+sy*_width]+1;
    int w1 = sx+(sy+1)*_width, w2 = sx+(sy-1)*_width, w3 = sx+1+sy*_width, w4 = sx-1+sy*_width;

    if(sy < _hmax)
        if(_cost[w1]==0 && _wall[w1]<2) {
//            _cost[w1] = current_cost;
            down = true;
        }
    if(sy > 0)
        if(_cost[w2]==0 && _wall[w2]<2) {
//            _cost[w2] = current_cost;
            up = true;
        }
    if(sx < _wmax) 
        if(_cost[w3]==0 && _wall[w3]<2) {
//            _cost[w3] = current_cost;
            right = true;
        }
    if(sx > 0)
        if(_cost[w4]==0 && _wall[w4]<2) {
//            _cost[w4] = current_cost;
            left = true;
        }

    switch(last_dir) {
        case UP:
            down = false;
            break;
        case DOWN:
            up = false;
            break;
        case LEFT:
            right = false;
            break;
        case RIGHT:
            left = false;
            break;
        default:
            break;
    }

    // trace a line to the goal and first try to search the simplest way
    int x = gx - sx, y = gy - sy, abs_x, abs_y;

    if(x < 0) abs_x = -x; else abs_x = x;
    if(y < 0) abs_y = -y; else abs_y = y;
    
    // change direction priority based on current position from goal
    if(abs_x > abs_y) {
    
        if(y == 0) {
    
            if(x < 0 && left) {
                _cost[w4] = current_cost;
                left = false;
                if(find_path(sx-1, sy, gx, gy, LEFT))
                    return true;
            } else if(right) {
                _cost[w3] = current_cost;
                right = false;
                if(find_path(sx+1, sy, gx, gy, RIGHT))
                    return true;
            }

            if(up) _cost[w2] = current_cost;
            if(down) _cost[w1] = current_cost;
            if(left) _cost[w4] = current_cost;
            if(right) _cost[w3] = current_cost;

            // if direct search failed search randomly
            if(left) {
                _cost[w4] = current_cost;
                if(find_path(sx-1, sy, gx, gy, LEFT))
                    return true;
            }
            if(up) {
                _cost[w2] = current_cost;
                if(find_path(sx, sy-1, gx, gy, UP))
                    return true;
            }
            if(down) {
                _cost[w1] = current_cost;
                if(find_path(sx, sy+1, gx, gy, DOWN))
                    return true;
            }

        } else if(y < 0) {

            if(x == 0 && up) {
                _cost[w2] = current_cost;
                up = false;
                if(find_path(sx, sy-1, gx, gy, UP))
                    return true;
            } else if(x < 0 &&  left) {
                _cost[w4] = current_cost;
                left = false;
                if(find_path(sx-1, sy, gx, gy, LEFT))
                    return true;
            } else if(right) {
                _cost[w3] = current_cost;
                right = false;
                if(find_path(sx+1, sy, gx, gy, RIGHT))
                    return true;
            }

            if(up) _cost[w2] = current_cost;
            if(down) _cost[w1] = current_cost;
            if(left) _cost[w4] = current_cost;
            if(right) _cost[w3] = current_cost;

            // if direct search failed search randomly
            if(up) {
                _cost[w2] = current_cost;
                if(find_path(sx, sy-1, gx, gy, UP))
                    return true;
            }
            if(left) {
                _cost[w4] = current_cost;
                if(find_path(sx-1, sy, gx, gy, LEFT))
                    return true;
            }
            if(down) {
                _cost[w1] = current_cost;
                if(find_path(sx, sy+1, gx, gy, DOWN))
                    return true;
            }

        } else {
    
            if(x == 0 && down) {
                _cost[w1] = current_cost;
                down = false;
                if(find_path(sx, sy+1, gx, gy, DOWN))
                    return true;
            } else if(x < 0 && left) {
                _cost[w4] = current_cost;
                left = false;
                if(find_path(sx-1, sy, gx, gy, LEFT))
                    return true;
            } else if(right) {
                _cost[w3] = current_cost;
                right  = false;
                if(find_path(sx+1, sy, gx, gy, RIGHT))
                    return true;
            }

            if(up) _cost[w2] = current_cost;
            if(down) _cost[w1] = current_cost;
            if(left) _cost[w4] = current_cost;
            if(right) _cost[w3] = current_cost;

            // if direct search failed search randomly
            if(down) {
                _cost[w1] = current_cost;
                if(find_path(sx, sy+1, gx, gy, DOWN))
                    return true;
            }
            if(left) {
                _cost[w4] = current_cost;
                if(find_path(sx-1, sy, gx, gy, LEFT))
                    return true;
            }
            if(up) {
                _cost[w2] = current_cost;
                if(find_path(sx, sy-1, gx, gy, UP))
                    return true;
            }
        }
    
    } else {
    
        if(x == 0) {
    
            if(y < 0 && up) {
                _cost[w2] = current_cost;
                up = false;
                if(find_path(sx, sy-1, gx, gy, UP))
                    return true;
            } else if(down) {
                _cost[w1] = current_cost;
                down = false;
                if(find_path(sx, sy+1, gx, gy, DOWN))
                    return true;
            }

            if(up) _cost[w2] = current_cost;
            if(down) _cost[w1] = current_cost;
            if(left) _cost[w4] = current_cost;
            if(right) _cost[w3] = current_cost;

            // if direct search failed search randomly
            if(up) {
                _cost[w2] = current_cost;
                if(find_path(sx, sy-1, gx, gy, UP))
                    return true;
            }
            if(right) {
                _cost[w3] = current_cost;
                if(find_path(sx+1, sy, gx, gy, DOWN))
                    return true;
            }
            if(left) {
                _cost[w4] = current_cost;
                if(find_path(sx-1, sy, gx, gy, LEFT))
                    return true;
            }

        } else if(x < 0) {

            if(y == 0 && left) {
                _cost[w4] = current_cost;
                left = false;
                if(find_path(sx-1, sy, gx, gy, LEFT))
                    return true;
            } else if(y < 0 &&  up) {
                _cost[w2] = current_cost;
                up = false;
                if(find_path(sx, sy-1, gx, gy, UP))
                    return true;
            } else if(down) {
                _cost[w1] = current_cost;
                down = false;
                if(find_path(sx, sy+1, gx, gy, DOWN))
                    return true;
            }

            if(up) _cost[w2] = current_cost;
            if(down) _cost[w1] = current_cost;
            if(left) _cost[w4] = current_cost;
            if(right) _cost[w3] = current_cost;

            // if direct search failed search randomly
            if(left) {
                _cost[w4] = current_cost;
                if(find_path(sx-1, sy, gx, gy, LEFT))
                    return true;
            }
            if(up) {
                _cost[w2] = current_cost;
                if(find_path(sx, sy-1, gx, gy, UP))
                    return true;
            }
            if(right) {
                _cost[w3] = current_cost;
                if(find_path(sx+1, sy, gx, gy, RIGHT))
                    return true;
            }

        } else {
        
            if(y == 0 && right) {
                _cost[w3] = current_cost;
                right = false;
                if(find_path(sx+1, sy, gx, gy, RIGHT))
                    return true;
            } else if(y < 0 && up) {
                _cost[w2] = current_cost;
                up = false;
                if(find_path(sx, sy-1, gx, gy, UP))
                    return true;
            } else if(down) {
                _cost[w1] = current_cost;
                down  = false;
                if(find_path(sx, sy+1, gx, gy, DOWN))
                    return true;
            }

            if(up) _cost[w2] = current_cost;
            if(down) _cost[w1] = current_cost;
            if(left) _cost[w4] = current_cost;
            if(right) _cost[w3] = current_cost;

            // if direct search failed search randomly
            if(right) {
                _cost[w3] = current_cost;
                if(find_path(sx+1, sy, gx, gy, RIGHT))
                    return true;
            }
            if(up) {
                _cost[w2] = current_cost;
                if(find_path(sx, sy-1, gx, gy, UP))
                    return true;
            }
            if(left) {
                _cost[w4] = current_cost;
                if(find_path(sx-1, sy, gx, gy, LEFT))
                    return true;
            }
        }
    }
   
    return false;
}

// pathfinder for monsters : returns a direction
u8 pathfinder(u16 width, u16 height, u8* wall, u16 start_x, u16 start_y, u16 goal_x, u16 goal_y, bool air, u8 last_dir, u8 count) {

    // init globals
    _goal_x = goal_x; _goal_y = goal_y;
    _hmax = height-1; _height = height;
    _wmax = width-1; _width = width;

    // allocate memory
    _cost = (u16*)calloc(_width*_height, sizeof(u16));
    _path = (u8*)calloc(_width*_height, sizeof(u8));

    // set start
    _cost[start_x + start_y*width] = 1;

    // ignore walls if monster fly
    if(air)
        _wall = _path;
    else
        _wall = wall;

    // exits if on goal or if no path was found
    if((start_x == goal_x && start_y == goal_y) || !find_path(start_x, start_y, goal_x, goal_y, last_dir)) {
        free(_cost);
        free(_path);
        return NONE;
    }

    // to even path resolution
    if(count % 2) {

        while(_path[start_x + start_y*width] != 1) {
            // find the path
            _path[goal_x + goal_y*width] = 1;
            if(goal_y<_hmax && _cost[goal_x + (goal_y+1)*width]<_cost[goal_x + goal_y*width] && _cost[goal_x + (goal_y+1)*width]>0)
                goal_y++;
            else if(goal_y>0 && _cost[goal_x + (goal_y-1)*width]<_cost[goal_x + goal_y*width] && _cost[goal_x + (goal_y-1)*width]>0)
                goal_y--;
            else if(goal_x<_wmax && _cost[goal_x+1 + goal_y*width]<_cost[goal_x + goal_y*width] && _cost[goal_x+1 + goal_y*width]>0)
                goal_x++;
            else /*if(goal_x<_wmax && _cost[goal_x-1 + goal_y*width]<_cost[goal_x + goal_y*width] && _cost[goal_x-1 + goal_y*width]>0)*/
                goal_x--;
        }
    
    } else {

        while(_path[start_x + start_y*width] != 1) {
            // find the path
            _path[goal_x + goal_y*width] = 1;
            if(goal_x<_wmax && _cost[goal_x+1 + goal_y*width]<_cost[goal_x + goal_y*width] && _cost[goal_x+1 + goal_y*width]>0)
                goal_x++;
            else if(goal_x<_wmax && _cost[goal_x-1 + goal_y*width]<_cost[goal_x + goal_y*width] && _cost[goal_x-1 + goal_y*width]>0)
                goal_x--;
            else if(goal_y<_hmax && _cost[goal_x + (goal_y+1)*width]<_cost[goal_x + goal_y*width] && _cost[goal_x + (goal_y+1)*width]>0)
                goal_y++;
            else /*if(goal_y>0 && _cost[goal_x + (goal_y-1)*width]<_cost[goal_x + goal_y*width] && _cost[goal_x + (goal_y-1)*width]>0)*/
                goal_y--;
        }
        
    }
    
    // free memory
    free(_cost);
    
    // return the good direction
    if(_path[start_x-1 + start_y*width] == 1) {
        free(_path);
        return LEFT;
    }
    else if(_path[start_x+1 + start_y*width] == 1) {
        free(_path);
        return RIGHT;
    }
    else if(_path[start_x + (start_y-1)*width] == 1) {
        free(_path);
        return UP;
    }
    else /*if(_path[start_x + (start_y+1)*width] == 1)*/ {
        free(_path);
        return DOWN;
    }

    return NONE;
}


// advanced heuristic search for path (A*)
bool adv_find_path(u8 sx, u8 sy, u8 gx, u8 gy, u8 last_dir) {

    int px, py, w1, w2, w3, w4, dx = 2, dy = 0, current_cost;
    point p;
    PriorityQueue queue = Initialize(/*_width*_height*/32*32*16);

    // Add the starting point
    Insert((point){sx, sy, 1}, queue);

    while(!_cost[gx+gy*_width] && !IsEmpty(queue)) {

        // Find & delete the most relevant point
        p = DeleteMin(queue);
        px = p.x;
        py = p.y;        

        current_cost = _cost[px+py*_width]+1;
        w1 = px+(py+1)*_width;
        w2 = px+(py-1)*_width;
        w3 = px+1+py*_width;
        w4 = px-1+py*_width;

        if(py < _hmax)
            if(_cost[w1]==0 && _wall[w1]<1) {
                _cost[w1] = current_cost;

                // calculate heuristic
                dx = gx-px; dy = gy-(py+1);
                TO_ABS(dx, dy);
                
                Insert((point){px, py+1, current_cost+dx+dy}, queue);
            }
        if(py > 0)
            if(_cost[w2]==0 && _wall[w2]<1) {
                _cost[w2] = current_cost;

                // calculate heuristic
                dx = gx-px; dy = gy-(py-1);
                TO_ABS(dx, dy);
                
                Insert((point){px, py-1, current_cost+dx+dy}, queue);
            }
        if(px < _wmax) 
            if(_cost[w3]==0 && _wall[w3]<1) {
                _cost[w3] = current_cost;

                // calculate heuristic
                dx = gx-(px+1); dy = gy-py;
                TO_ABS(dx, dy);
                
                Insert((point){px+1, py, current_cost+dx+dy}, queue);
            }
        if(px > 0)
            if(_cost[w4]==0 && _wall[w4]<1) {
                _cost[w4] = current_cost;

                // calculate heuristic
                dx = gx-(px-1); dy = gy-py;
                TO_ABS(dx, dy);
                
                Insert((point){px-1, py, current_cost+dx+dy}, queue);
            }
    }

    // Free memory
    Destroy(queue);

    return _cost[gx+gy*_width];
}

// advanced pathfinder for monsters : returns a direction
u8 adv_pathfinder(u16 width, u16 height, u8* wall, u16 start_x, u16 start_y, u16 goal_x, u16 goal_y, bool air, u8 last_dir, u8 count) {

    // init globals
    _goal_x = goal_x; _goal_y = goal_y;
    _hmax = height-1; _height = height;
    _wmax = width-1; _width = width;

    // allocate memory
    _cost = (u16*)calloc(_width*_height, sizeof(u16));
    _path = (u8*)calloc(_width*_height, sizeof(u8));

    // set start
    _cost[start_x + start_y*width] = 1;

    // ignore walls if monster fly
    if(air)
        _wall = _path;
    else
        _wall = wall;

    // exits if on goal or if no path was found
    if((start_x == goal_x && start_y == goal_y) || !adv_find_path(start_x, start_y, goal_x, goal_y, last_dir)) {
        free(_cost);
        free(_path);
        return NONE;
    }

    // to even path resolution
    if(count % 2) {

        while(_path[start_x + start_y*width] != 1) {
            // find the path
            _path[goal_x + goal_y*width] = 1;
            if(goal_y<_hmax && _cost[goal_x + (goal_y+1)*width]<_cost[goal_x + goal_y*width] && _cost[goal_x + (goal_y+1)*width]>0)
                goal_y++;
            else if(goal_y>0 && _cost[goal_x + (goal_y-1)*width]<_cost[goal_x + goal_y*width] && _cost[goal_x + (goal_y-1)*width]>0)
                goal_y--;
            else if(goal_x<_wmax && _cost[goal_x+1 + goal_y*width]<_cost[goal_x + goal_y*width] && _cost[goal_x+1 + goal_y*width]>0)
                goal_x++;
            else /*if(goal_x<_wmax && _cost[goal_x-1 + goal_y*width]<_cost[goal_x + goal_y*width] && _cost[goal_x-1 + goal_y*width]>0)*/
                goal_x--;
        }
    
    } else {

        while(_path[start_x + start_y*width] != 1) {
            // find the path
            _path[goal_x + goal_y*width] = 1;
            if(goal_x<_wmax && _cost[goal_x+1 + goal_y*width]<_cost[goal_x + goal_y*width] && _cost[goal_x+1 + goal_y*width]>0)
                goal_x++;
            else if(goal_x<_wmax && _cost[goal_x-1 + goal_y*width]<_cost[goal_x + goal_y*width] && _cost[goal_x-1 + goal_y*width]>0)
                goal_x--;
            else if(goal_y<_hmax && _cost[goal_x + (goal_y+1)*width]<_cost[goal_x + goal_y*width] && _cost[goal_x + (goal_y+1)*width]>0)
                goal_y++;
            else /*if(goal_y>0 && _cost[goal_x + (goal_y-1)*width]<_cost[goal_x + goal_y*width] && _cost[goal_x + (goal_y-1)*width]>0)*/
                goal_y--;
        }
        
    }
    
    // free memory
    free(_cost);
    
    // return the good direction
    if(_path[start_x-1 + start_y*width] == 1) {
        free(_path);
        return LEFT;
    }
    else if(_path[start_x+1 + start_y*width] == 1) {
        free(_path);
        return RIGHT;
    }
    else if(_path[start_x + (start_y-1)*width] == 1) {
        free(_path);
        return UP;
    }
    else /*if(_path[start_x + (start_y+1)*width] == 1)*/ {
        free(_path);
        return DOWN;
    }

    return NONE;
}
