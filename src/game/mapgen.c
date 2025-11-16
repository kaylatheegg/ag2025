#include "desertbus.h"

/* Map of 50x50 tiles */
uint16_t map[50][50] = {0};

point find_closest_grid_point(size_t direction, box box);

bool box_is_colliding(box a, box b) {
    if(a.x <= b.x + b.width && a.x + a.width >= b.x &&
       a.y <= b.y + b.height && a.y + a.height >= b.y)
        return true;
    
    return false;
}

bool point_inside_box(box a, point b) {
    if (a.x <= b.x && b.x <= a.x + a.width &&
        a.y <= b.y && b.y <= a.y + a.height)
        return true;

    return false;
}

box room_boxes[ROOM_COUNT];

bool is_point_in_any_box(point a) {
    for (size_t i = 0; i < ROOM_COUNT; i++) {
        if (room_boxes[i].width == 0) {
            continue;
        }

        if (point_inside_box(room_boxes[i], a)) {
            return true;
        }
    }
    return false;
}


void generate_map() {
    /* Clean up room boxes */
    for (size_t i = 0; i < ROOM_COUNT; i++) {
        room_boxes[i] = (box){0, 0, 0, 0, 0};
    }

    /* Decide on room count */
    size_t room_count = rand() % 4 + 6;

    /* Generate room_count random rooms */
    /* NOTE: terrible alg */
    size_t runs = 0;
    for (size_t i = 0; i < room_count; i++) {
        if (runs > 1024) break;
        /* Generate random box */
        box new_box;
        new_box.x = rand() % 50;
        new_box.y = rand() % 50;
        new_box.width = rand() % 5 + 5;
        new_box.height = rand() % 5 + 5;
        new_box.invalid = false;
        
        if (new_box.x + new_box.width >= 50) {
            new_box.x -= new_box.width - 1;
        }

        if (new_box.y + new_box.height >= 50) {
            new_box.y -= new_box.height - 1;
        }
        
        bool rerun = false;

        /* Check that this doesn't intersect with any other boxes. If so, redo. */
        for (size_t j = 0; j < room_count; j++) {
            if (room_boxes[j].x == 0)
                continue;

            if (box_is_colliding(new_box, room_boxes[j]), box_is_colliding(room_boxes[j], new_box)) {
                rerun = true;
                break;
            }
        }

        if (rerun) {
            runs++;
            i--;
            continue;
        }

        runs = 0;

        room_boxes[i] = new_box;
    }

    /* Rooms are generated, pick two rooms and make a path between them */
    size_t path_count = 1.5 * (float)room_count;

    uint8_t old_paths[128][128] = {0};
    uint8_t old_path_sources[128][2] = {0};
    uint16_t old_path_count = 0;
    runs = 0;
    for (size_t i = 0; i < path_count; i++) {
        /* Pick two random rooms */
        size_t room_one_idx = rand() % room_count;
        if (path_count < room_count) {
            room_one_idx = i;
        }
        size_t room_two_idx = rand() % room_count;

        while (room_one_idx == room_two_idx)
            room_two_idx = rand() % room_count;
        
        box room_one = room_boxes[room_one_idx];
        box room_two = room_boxes[room_two_idx];

        /* Choose a random side on each room, and generate the closest corridor to the room */
        size_t room_one_side = rand() % 4;
        size_t room_two_side = rand() % 4;

        /* Generate room one corridor */
        point room_one_point = find_closest_grid_point(room_one_side, room_one);
        
        bool rerun = false;
        if (room_one_point.x == 0 && room_one_point.y == 0) {
            rerun = true;
            for (size_t i = 0; i < 4; i++) {
                room_one_point = find_closest_grid_point(room_one_side, room_one);
                if (room_one_point.x != 0 && room_one_point.y != 0) {
                    rerun = false;
                    break;
                }
            }
        }

        if (rerun) {
            runs++;
            i--;
            continue;
        }

        /* Generate room two corridor */
        point room_two_point = find_closest_grid_point(room_two_side, room_two);
        
        bool found_new = false;
        if (room_two_point.x == 0 && room_two_point.y == 0) {
            rerun = true;
            for (size_t j = 0; j < 4; j++) {
                room_two_point = find_closest_grid_point(room_two_side, room_two);
                if (room_two_point.x != 0 && room_two_point.y != 0) {
                    rerun = false;
                    found_new = true;
                    break;
                }
            }
        }

        if (rerun) {
            runs++;
            i--;
            continue;
        }
        runs = 0;
    
        /* Run Dijkstra's algorithm*/
        bool has_failed = false;
        int16_t adjacencies[50/3][50/3] = {0};
        uint16_t dist[50/3][50/3] = {0};
        int16_t prev[50/3][50/3][2];
        uint16_t cell_count = 50/3 * 50/3;
        for (size_t y = 0; y < 50/3; y++) {
            for (size_t x = 0; x < 50/3; x++) {
                dist[y][x] = 1000;
                prev[y][x][0] = -1;
                prev[y][x][1] = -1;
                adjacencies[y][x] = 0xF;
                uint8_t real_x = x * 3;
                uint8_t real_y = y * 3;

                if (y == 0)      adjacencies[y][x] &= ~0b1;
                if (x == 50/3-1) adjacencies[y][x] &= ~0b10;
                if (y == 50/3-1) adjacencies[y][x] &= ~0b100;
                if (x == 0)      adjacencies[y][x] &= ~0b1000;

                /* North */ if (real_y != 0  && is_point_in_any_box((point){real_x,     real_y - 1})) adjacencies[y][x] &= ~0b1;
                /* East */  if (real_x < 49 && is_point_in_any_box((point){real_x + 1, real_y}))     adjacencies[y][x] &= ~0b10;
                /* South */ if (real_y < 49 && is_point_in_any_box((point){real_x,     real_y + 1})) adjacencies[y][x] &= ~0b100;
                /* West */  if (real_x != 0  && is_point_in_any_box((point){real_x - 1, real_y}))     adjacencies[y][x] &= ~0b1000;
            
                char* hexbuf = "0123456789ABCDEF";
                //map[y*3][x*3] = hexbuf[adjacencies[y][x]];
            }
        }

        if (room_one_point.x / 3 > 50/3-1 || room_one_point.y / 3 > 50/3-1 ||
            room_two_point.x / 3 > 50/3-1 || room_two_point.y / 3 > 50/3-1) {
            cell_count = 0;
        } else {
            dist[room_one_point.y / 3][room_one_point.x / 3] = 0;
        }


        while (cell_count != 0) {
            int8_t u_x = -1;
            int8_t u_y = -1;
            uint16_t min_dist = 10001;
            for (size_t y = 0; y < 50/3; y++) {
                for (size_t x = 0; x < 50/3; x++) {
                    if (adjacencies[y][x] == -1)  
                        continue;

                    if (dist[y][x] < min_dist) {
                        /* new minimum! */
                        u_x = x;
                        u_y = y;
                        min_dist = dist[y][x];
                    }
                }
            }
            
            if (u_y == -1 && u_x == -1)
                break;

            if (min_dist == 1000) {
                adjacencies[u_y][u_x] = -1;
                cell_count--;
                continue;
            }

            /* north */ 
            if (BIT(adjacencies[u_y][u_x], 0) && u_y != 0 && adjacencies[u_y - 1][u_x] != -1 && (dist[u_y][u_x] + 1) < dist[u_y - 1][u_x]) {
                dist[u_y - 1][u_x] = dist[u_y][u_x] + 1;

                prev[u_y - 1][u_x][0] = u_x;
                prev[u_y - 1][u_x][1] = u_y;
            }
            /* east */
            if (BIT(adjacencies[u_y][u_x], 1) && u_x < 50/3-1 && adjacencies[u_y][u_x + 1] != -1 && (dist[u_y][u_x] + 1) < dist[u_y][u_x + 1]) {
                dist[u_y][u_x + 1] = dist[u_y][u_x] + 1;
                prev[u_y][u_x + 1][0] = u_x;
                prev[u_y][u_x + 1][1] = u_y;
            }
            /* south */
            if (BIT(adjacencies[u_y][u_x], 2) && u_y < 50/3-1 && adjacencies[u_y + 1][u_x] != -1 && (dist[u_y][u_x] + 1) < dist[u_y + 1][u_x]) {
                dist[u_y + 1][u_x] = dist[u_y][u_x] + 1;
                prev[u_y + 1][u_x][0] = u_x;
                prev[u_y + 1][u_x][1] = u_y;
            }
            /* west */
            if (BIT(adjacencies[u_y][u_x], 3) && u_x != 0 && adjacencies[u_y][u_x - 1] != -1 && (dist[u_y][u_x] + 1) < dist[u_y][u_x - 1]) {
                dist[u_y][u_x - 1] = dist[u_y][u_x] + 1;
                prev[u_y][u_x - 1][0] = u_x;
                prev[u_y][u_x - 1][1] = u_y;
            }

            adjacencies[u_y][u_x] = -1;
            cell_count--;
        }

        uint8_t path_size = 0;
        uint8_t dir_to_go[50/3 * 50/3] = {0};
        size_t iter = 0;
        point converted_point = (point){room_two_point.x/3, room_two_point.y/3};
        if (converted_point.y == 16 || converted_point.x == 16) continue;
        if ((prev[room_two_point.y / 3][room_two_point.x / 3][0] != -1 && prev[room_two_point.y / 3][room_two_point.x / 3][1] != -1) || 
        (room_one_point.x == room_two_point.x && room_one_point.y == room_two_point.y)) {
            int8_t u_x = room_two_point.x / 3;
            int8_t u_y = room_two_point.y / 3;
            while (prev[u_y][u_x][0] != -1 && prev[u_y][u_x][1] != -1) {
                /* find direction prev points to current */
                int8_t prev_x = prev[u_y][u_x][0];
                int8_t prev_y = prev[u_y][u_x][1];
                int8_t dir = -1;
                iter++;
                /* north */ if (prev_y != 0      && u_x == prev_x       && u_y == (prev_y - 1)) dir = 0;
                /* east  */ if (prev_x < 50/3-1 && (u_x + 1) == prev_x && u_y == prev_y)       dir = 1;
                /* south */ if (prev_y < 50/3-1 && u_x == prev_x       && u_y == (prev_y + 1)) dir = 2;
                /* west  */ if (prev_x != 0      && (u_x - 1) == prev_x && u_y == prev_y)       dir = 3;
                if (dir == -1) {
                    has_failed = true;
                }
                
                dir_to_go[path_size] = dir;
                path_size++;
                u_x = prev_x;
                u_y = prev_y;
            }
        }

        if (!has_failed) {
            uint16_t curr_x = room_two_point.x;
            uint16_t curr_y = room_two_point.y;
            old_path_sources[old_path_count][0] = curr_x;
            old_path_sources[old_path_count][1] = curr_y;
            for (size_t j = 0; j < path_size; j++) {
                old_paths[old_path_count][j] = dir_to_go[j];

                map[curr_y][curr_x] = '.';
                
                if (dir_to_go[j] == 0) {
                    for (size_t k = 0; k < 3; k++) map[curr_y + k][curr_x] = '.';
                    curr_y += 3;
                }
                if (dir_to_go[j] == 1) {
                    for (size_t k = 0; k < 3; k++) map[curr_y][curr_x + k] = '.';
                    curr_x += 3;
                }
                if (dir_to_go[j] == 2) {
                    for (size_t k = 0; k < 3; k++) map[curr_y - k][curr_x] = '.';
                    curr_y -= 3;
                }
                if (dir_to_go[j] == 3) {
                    for (size_t k = 0; k < 3; k++) map[curr_y][curr_x - k] = '.';
                    curr_x -= 3;
                }
            }
            old_paths[old_path_count][path_size] = 5;
            old_path_count++;
        }
    }

    /* Mark orphan rooms as invalid */
    /* This is fucking awful.*/
    for (size_t i = 0; i < room_count; i++) {
        bool is_invalid = true;
        for (size_t y = 0; y < room_boxes[i].height; y++) {
            for (size_t x = 0; x < room_boxes[i].width; x++) {
                if (map[y + room_boxes[i].y][x + room_boxes[i].x] == '.') {
                    /* Find next grid point */
                    uint8_t grid_x = x + room_boxes[i].x;
                    uint8_t grid_y = y + room_boxes[i].y;
                    uint8_t new_grid_x = 0;
                    uint8_t new_grid_y = 0;
                    uint8_t prev_dir = 5;
                    /* North */ 
                    if (grid_y != 0 && map[grid_y - 3][grid_x] == '.') {
                        new_grid_x = grid_x;
                        new_grid_y = grid_y - 3;
                        prev_dir = 3;
                    }
                    /* East */
                    if (grid_x != 50/3-1 && map[grid_y][grid_x + 3] == '.') {
                        new_grid_x = grid_x + 3;
                        new_grid_y = grid_y;
                        prev_dir = 2;
                    }
                    /* South */
                    if (grid_y != 50/3-1 && map[grid_y + 3][grid_x] == '.') {
                        new_grid_x = grid_x;
                        new_grid_y = grid_y + 3;
                        prev_dir = 0;
                    }
                    /* West*/
                    if (grid_x != 0 && map[grid_y][grid_x - 3] == '.') {
                        new_grid_x = grid_x - 3;
                        new_grid_y = grid_y;
                        prev_dir = 1;
                    }

                    if (new_grid_x == 0 && new_grid_y == 0) {
                        is_invalid = true;
                        y = 10000;
                        x = 10000;
                        break;
                    }
                    
                    /* Check again*/
                    uint8_t second_grid_x = 0;
                    uint8_t second_grid_y = 0;

                    /* North */ 
                    if (prev_dir != 0 && new_grid_y != 0 && map[new_grid_y - 3][new_grid_x] == '.') {
                        second_grid_x = new_grid_x;
                        second_grid_y = new_grid_y - 3;
                    }
                    /* East */
                    if (prev_dir != 1 && new_grid_x != 50/3-1 && map[new_grid_y][new_grid_x + 3] == '.') {
                        second_grid_x = new_grid_x + 3;
                        second_grid_y = new_grid_y;
                    }
                    /* South */
                    if (prev_dir != 2 && new_grid_y != 50/3-1 && map[new_grid_y + 3][new_grid_x] == '.') {
                        second_grid_x = new_grid_x;
                        second_grid_y = new_grid_y + 3;
                    }
                    /* West*/
                    if (prev_dir != 3 && new_grid_x != 0 && map[new_grid_y][new_grid_x - 3] == '.') {
                        second_grid_x = new_grid_x - 3;
                        second_grid_y = new_grid_y;
                    }
                    
                    if (second_grid_x == 0 && second_grid_y == 0) {
                        is_invalid = true;
                        y = 10000;
                        x = 10000;
                        break;
                    } else {
                        is_invalid = false;
                        y = 10000;
                        x = 10000;
                        break;
                    }

                    
                }
                
            }
        }

        if (is_invalid) {
            //map[room_boxes[i].y][room_boxes[i].x] = 'H';
            room_boxes[i].invalid = true;
        }
    }

    /* Draw rooms */
    for (size_t i = 0; i < room_count; i++) {
        box curr_box = room_boxes[i];
    
        for (size_t y = 1; y < curr_box.height - 1; y++) {
            for (size_t x = 1; x < curr_box.width - 1; x++) {
                map[y + curr_box.y][x + curr_box.x] = '-';
            }
        }

        /* Draw doors */
        /* Horiz doors */
        for (size_t x = 0; x < curr_box.width; x++) {
            if (map[curr_box.y][curr_box.x + x] == '.')
                map[curr_box.y][curr_box.x + x] = 'd';
            
            if (map[curr_box.y + curr_box.height - 1][curr_box.x + x] == '.')
                map[curr_box.y + curr_box.height - 1][curr_box.x + x] = 'd';
        
        }
        /* Vertical doors */
        for (size_t y = 0; y < curr_box.height; y++) {
            if (map[curr_box.y + y][curr_box.x + curr_box.width - 1] == '.')
                map[curr_box.y + y][curr_box.x + curr_box.width - 1] = 'D'; 

            if (map[curr_box.y + y][curr_box.x] == '.')
                map[curr_box.y + y][curr_box.x] = 'D'; 
            
        }
    }
}


int round_down(int value, int rounding) {
    return value - value % rounding;
}

int round_up(int value, int rounding) {
    if (value % rounding == 0)
        return value;
    
    return value + rounding - value % rounding;
}

int round_to_closest(int value, int rounding) {
    float val = value;
    float rounder = rounding;
    double nothing;
    if (modf(val/rounder, &nothing) < 0.5) 
        return round_down(value, rounding);
    else
        return round_up(value, rounding);
}

point find_closest_grid_point(size_t direction, box box) {
    /* Find closest grid point on that side */

    /* N0 E1 S2 W3
    */
    switch(direction) {
        case 0: {
            if (box.y < 3) {
                return (point){0, 0};
            }
            size_t x_pos = box.x + box.width/2;
            x_pos = round_to_closest(x_pos, 3);
            
            size_t y_pos = box.y - 1;
            y_pos = round_down(y_pos, 3);

            if (is_point_in_any_box((point){x_pos, y_pos}))
                return (point){0, 0};

            /* Draw from box to pos */
            for (size_t j = 0; j < box.y - y_pos + 1; j++) {
                map[j + y_pos][x_pos] = '.';
            }

            return (point){x_pos, y_pos};
            break;
        }
        case 1: {
            if (box.x + box.width + 1 >= 50)
                return (point){0, 0};
                
            size_t y_pos = box.y + box.height / 2;
            y_pos = round_to_closest(y_pos, 3);
            
            size_t x_pos = box.x + box.width;
            x_pos = round_up(x_pos, 3);

            if (is_point_in_any_box((point){x_pos, y_pos}))
                return (point){0, 0};
                
            /* Draw from box to map pos */
            for (size_t j = 0; j < x_pos - (box.x + box.width) + 2; j++) {
                map[y_pos][box.x + box.width - 1 + j] = '.';
            }
            

            return (point){x_pos, y_pos};
        }
        case 2: {
            if (box.y + box.height + 1 >= 50)
                return (point){0, 0};

            size_t y_pos = box.y + box.height;
            y_pos = round_to_closest(y_pos, 3);

            size_t x_pos = box.x + box.width / 2;
            x_pos = round_up(x_pos, 3);

            if (is_point_in_any_box((point){x_pos, y_pos}))
                return (point){0, 0};

            /* Draw from box to map pos */
            for (size_t j = 0; j < y_pos - (box.y + box.height) + 2; j++) {
                map[box.y + box.height - 1 + j][x_pos] = '.';
            }

            return (point){x_pos, y_pos};
        }
        case 3: {
            if (box.x < 3) {
                return (point){0, 0};
            }
            size_t y_pos = box.y + box.height/2;
            y_pos = round_to_closest(y_pos, 3);
            
            size_t x_pos = box.x - 1;
            x_pos = round_down(x_pos, 3);

            if (is_point_in_any_box((point){x_pos, y_pos}))
                return (point){0, 0};

            /* Draw from box to pos */
            for (size_t j = 0; j < box.x - x_pos + 1; j++) {
                map[y_pos][x_pos + j] = '.';
            }

            return (point){x_pos, y_pos};
            break;
        }
        
    }

    return (point){0, 0};
}