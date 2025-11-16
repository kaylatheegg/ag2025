#include "desertbus.h"

/* taken from enclave v5 */
char* name[] = {
    "Adelaide", "Clotilde", "Euphemia", "Vivian",
    "Jeanne", "Poppy", "Alaric", "Constance",
    "Finnian", "Jessamine", "Remaine", "Alton",
    "Corbett", "Gertrude", "Johan", "Rosalind",
    "Balthazar", "Denis", "Godfrey", "Lavinia",
    "Rose", "Bard", "Desirae", "Grimwald",
    "Lucien", "Sabina", "Basil", "Destrian",
    "Hannibal", "Madelena", "Sif", "Beatrix",
    "Ellis", "Henrietta", "Margot", "Thaddeus",
    "Bianca", "Elric", "Hester", "Mortimer",
    "Trillby", "Cadman", "Emil", "Ione",
    "Olivia", "Urban", "Charlotte", "Erasmus", 
    "Jasper" ,"Penegrin"
};

char* moninker_suffix[] = {
    "of the Deep Haze",
    "of the Acidic Citadel",
    "of the True Scribes",
    "of Jouster's Bridge",
    "of Dirgehorn",
    "of Ashpit",
    "of Nowhere",
    "of Raefus",
    "of Sorveil",
    "of Vedel",
    "the Superstitious",
    "the Effusive",
    "the Glutton",
    "the Hermit",
    "the Wistful",
    "the Zealous",
    "the Fallen",
    "the Pitiful",
    "the Eager",
    "the Exiled",
    "Mirror-breaker",
    "Gold-tooth",
    "Toe-taker",
    "Cloud-watcher",
    "Scroll-speaker"
};

char* moniker_prefix[] = {
    "Honourable",
    "Serene",
    "Manic",
    "Generous",
    "Misanthrope",
    "Jealous",
    "Imperious",
    "Humourless",
    "Prickly",
    "Chipper",
    "Righteous",
    "Ragged",
    "Pious",
    "Jolly",
    "Eunuch",
    "Outlander",
    "Beggar",
    "Executioner",
    "Cutpurse",
    "Gravedigger",
    "Jailer",
    "Thespian",
    "Vanguard",
    "Daredevil",
    "Inquisitor"
};

char* background[] = {
    "Alchemist", "Cleric ", "Magician",
    "Performer", "Beggar", "Cook", "Mariner",
    "Pickpocket", "Butcher", "Cultist", "Mercenary",
    "Smuggler", "Burglar", "Gambler", "Merchant",
    "Student", "Charlatan", "Herbalist", "Outlaw"
};

uint8_t display_map[50][50] = {0};

point player_pos = {0};
point old_player_pos = {0};
int16_t player_hp = 10;
int16_t player_max_hp = 10;
uint8_t player_damage = 1;
uint8_t new_damage = 0;
uint8_t infection = 0;
uint8_t infection_tolerance = 50;

int16_t infection_core_hp;

uint32_t score = 0;

uint16_t level = 0;

char player_name[50] = {0};
char* chosen_background;

typedef struct {
    point pos;
    int16_t hp;
    int16_t max_hp;
    char pic;
    uint8_t pic_idx;
    uint8_t palette;
    uint8_t state;
    bool is_alive;
    bool is_revealed;
    bool is_infected;
} creature;

extern window* entities;
extern window* player_stuff;

extern box room_boxes[ROOM_COUNT];

extern uint16_t map[50][50];

bool init_once = false;

size_t disqual_room = 1000;
size_t start_room = 10000;

size_t num_enemy = 0;

int16_t armour = 0;

int16_t new_armour_val = 0;

creature* creatures;

char current_message[128] = {0};

point get_point_in_dungeon();

void reveal_spaces();
void tick_creatures();
bool is_collider(point pos);
void run_interaction(point pos, bool is_player);
void render_player_info();
void tick_infection_spread();
void warp_new_map();

char* creature_names[] = {
    "Kobold",
    "Skeleton",
    "Snake",
    "Wisp"
};

char* creature_pics = "KS@*";
char* dead_creature_pics = "ks~\'";

void init_player() {
    player_pos = get_point_in_dungeon();
    old_player_pos = player_pos;

    /* Clear display map */
    for (size_t y = 0; y < 50; y++) {
        for (size_t x = 0; x < 50; x++) {
            display_map[y][x] = ' ';
        }
    }

    if (!init_once) {
        /* Pick player name */
        if (rand() % 2 == 0) {
            sprintf(player_name, "%s %s", moniker_prefix[rand() % 25], name[rand() % 50]);
        } else {
            sprintf(player_name, "%s %s", name[rand() % 50], moninker_suffix[rand() % 25]);
        }

        /* Pick background */
        chosen_background = background[rand() % 19];
        init_once = true;
    }

    if (num_enemy == 0) {
        /* Create enemies */
        num_enemy = rand() % 10 + 5;

        creatures = dbmalloc(sizeof(*creatures) * num_enemy);
        
        for (size_t i = 0; i < num_enemy; i++) {
            creatures[i].pos = get_point_in_dungeon();
            creatures[i].max_hp = level * 3 + rand() % 5 + 5;
            creatures[i].hp = creatures[i].max_hp;
            creatures[i].is_alive = true;
            uint8_t creature_idx = rand() % strlen(creature_pics);
            creatures[i].pic_idx = creature_idx;
            creatures[i].pic = creature_pics[creature_idx];
            creatures[i].palette = 2 + creature_idx;
            creatures[i].state = 0;
            creatures[i].is_revealed = false;
            creatures[i].is_infected = false;
        }
    }

    /* Place coins on the map */
    size_t coin_count = 5 + rand() % 11;
    for (size_t i = 0; i < coin_count; i++) {
        point coin_loc = get_point_in_dungeon();
        map[coin_loc.y][coin_loc.x] = 'c';
    }
    
    /* Place gems on the map */
    size_t gem_count = rand() % 3;
    for (size_t i = 0; i < gem_count; i++) {
        point gem_loc = get_point_in_dungeon();
        map[gem_loc.y][gem_loc.x] = 'g';
    }

    /* Place health up on map */
    point hp_up = get_point_in_dungeon();
    map[hp_up.y][hp_up.x] = 'h';

    /* Place health restores on map */
    size_t hp_restores = 1 + rand() % 3;
    for (size_t i = 0; i < hp_restores; i++) {
        point hp_rest = get_point_in_dungeon();
        map[hp_rest.y][hp_rest.x] = 'H';
    }

    /* Spawn a piece of armour */
    point new_armour = get_point_in_dungeon();
    map[new_armour.y][new_armour.x] = 'A';
    new_armour_val = level * 2 + rand() % 5 + 1;

    /* Spawn a sword */
    point new_sword = get_point_in_dungeon();
    map[new_sword.y][new_sword.x] = '/';
    new_damage = level * 2 + rand() % 5 + 1;

    /* Spawn a few warps */
    for (size_t i = 0; i < 3; i++) {
        point new_warp = get_point_in_dungeon();
        map[new_warp.y][new_warp.x] = 'W';
    }

    /* Spawn infection nexus */
    point infection_nexus = get_point_in_dungeon();
    map[infection_nexus.y][infection_nexus.x] = ']';
    map[infection_nexus.y][infection_nexus.x] |= 0x100;

    infection_core_hp = level * 2 + 10 + rand() % 15;

    reveal_spaces();
    tick_creatures();
    render_player_info();
}

void tick_infection_spread() {
    uint16_t infected_map[50][50];

    if (infection_core_hp < 0) return;

    for (size_t y = 0; y < 50; y++)
        for (size_t x = 0; x < 50; x++)
            infected_map[y][x] = map[y][x];

    /* Iterate through map, if a tile touches an infected one, spread */
    for (size_t y = 0; y < 50; y++) {
        for (size_t x = 0; x < 50; x++) {
            int16_t infection_chance = 128 - level * 5;
            if (infection_chance < 2) infection_chance = 2;
            if (rand() % infection_chance != 0) continue; // Skip infecting this tile if it could be

            /* North */
            if (y != 0 && BIT(map[y-1][x], 8)) infected_map[y][x] |= 0x100;
            /* East */
            if (x != 49 && BIT(map[y][x+1], 8)) infected_map[y][x] |= 0x100;
            /* South */
            if (y != 49 && BIT(map[y+1][x], 8)) infected_map[y][x] |= 0x100;
            /* West */
            if (x != 0 && BIT(map[y][x-1], 8)) infected_map[y][x] |= 0x100;
            
        }
    }

    for (size_t y = 0; y < 50; y++)
        for (size_t x = 0; x < 50; x++)
            map[y][x] = infected_map[y][x];
}

void draw_text(char* buf, int y) {
    size_t y_incs = (int)ceilf((float)strlen(buf)/(float)40);

    for (size_t j = 0; j < y_incs; j++) {
        for (size_t i = 0; i < 40; i++) {
            if (buf[j * 40 + i] == '\0') break;

            plot(player_stuff, i, y + j, 1, buf[j * 40 + i]);
        }
    }
}

void render_player_info() {
    /* Print player name */
    char buf[256] = {0};
    sprintf(buf, "Name: %s", player_name);
    draw_text(buf, 1);

    /* Print background */
    sprintf(buf, "Background: %s", chosen_background);
    draw_text(buf, 3);

    sprintf(buf, "HP: %d/%d   ", player_hp, player_max_hp);
    draw_text(buf, 5);

    /* TODO: make the ooze purple in the infection bar */
    /* Infection status bar */
    /* Calculate % of bar filled */
    float infection_amount = (float)infection/(float)infection_tolerance;
    size_t infected_bars = (int)20 * infection_amount;
    
    char infect_buf[64] = {0};
    infect_buf[0] = '<';

    for (size_t i = 0; i < 20; i++) {
        infect_buf[i + 1] = '-'; 
    }
    infect_buf[20] = '>';
    infect_buf[21] = '\0';

    for (size_t i = 0; i < infected_bars && i < 20; i++) {
        infect_buf[1 + i] = '|';
    }

    sprintf(buf, "Ooze: %s", infect_buf);

    draw_text(buf, 7);

    sprintf(buf, "Score: %d", score);
    draw_text(buf, 9);
    sprintf(buf, "Damage: %d", player_damage);
    draw_text(buf, 10);
    sprintf(buf, "Armour: %d", armour);
    draw_text(buf, 11);
    sprintf(buf, "Level: %d", level + 1);
    draw_text(buf, 12);

    /* Clear out buf */
    for (size_t i = 0; i < 256; i++) {
        buf[i] = ' ';
    }
    buf[255] = '\0';
    draw_text(buf, 14);

    /* Print current message */
    sprintf(buf, "%s", current_message);
    draw_text(buf, 14);

}

void tick_creatures() {
    for (size_t i = 0; i < num_enemy; i++) {
        if (!creatures[i].is_alive) goto render;
        point old_pos = creatures[i].pos;
        point c_pos = creatures[i].pos;

        run_interaction((point){c_pos.x, c_pos.y}, false);

        /* move creature... */
        switch (creatures[i].state) {
            case 0: {
                /* Wandering */
                /* Choose move direction */
                uint8_t dir = rand() % 4;
                switch(dir) {
                case 0:
                    if (c_pos.y != 0  && !is_collider((point){c_pos.x, c_pos.y-1})) c_pos.y--;
                    if (c_pos.y != 0) run_interaction((point){c_pos.x, c_pos.y-1}, false);
                    break;
                case 1:
                    if (c_pos.x != 49 && !is_collider((point){c_pos.x + 1, c_pos.y})) c_pos.x++;
                    if (c_pos.x != 49) run_interaction((point){c_pos.x + 1, c_pos.y}, false);
                    break;
                case 2:
                    if (c_pos.y != 49 && !is_collider((point){c_pos.x, c_pos.y+1})) c_pos.y++;
                    if (c_pos.y != 49) run_interaction((point){c_pos.x, c_pos.y+1}, false);
                    break;
                case 3:
                    if (c_pos.x != 0  && !is_collider((point){c_pos.x - 1, c_pos.y})) c_pos.x--;
                    if (c_pos.x != 0) run_interaction((point){c_pos.x - 1, c_pos.y}, false);
                    break;
                }
                /* Write back creature pos */
                creatures[i].pos = c_pos;
                break;
            }
            default:
                break;
        }

        if (display_map[creatures[i].pos.y][creatures[i].pos.x] != ' ') {
            creatures[i].is_revealed = true;
        }


render:
        if (creatures[i].is_revealed) {
            /* If c_pos or old_pos is invalid, skip */
            if (c_pos.x > 49 || c_pos.x < 0 || c_pos.y > 49 || c_pos.x < 0 ||
                old_pos.x > 49 || old_pos.x < 0 || old_pos.y > 49 || old_pos.x < 0)
                continue;

            /* Erase old player position */
            plot(entities, old_pos.x, old_pos.y, 1, '\0');
            
            /* Draw new player position */
            plot(entities, creatures[i].pos.x, creatures[i].pos.y, creatures[i].palette, creatures[i].pic);
        }
    }

    tick_infection_spread();
}

point get_point_in_dungeon() {
    /* Find room count */
    size_t room_count = 0;
    for (; room_count < ROOM_COUNT; room_count++) {
        if (room_boxes[room_count].width == 0) break;
    }

    size_t runs = 0;

    /* TODO: make a better spawner (from center?)*/
    /* Choose a room at random */
    size_t chosen_room = rand() % room_count;
    while (!room_boxes[chosen_room].invalid && chosen_room == start_room) {
        if (runs > 16) {
            /* Just pick a random room */
            chosen_room = rand() % room_count;
            break;
        }
        
        chosen_room = rand() % room_count;
        runs++;
    }

    start_room = chosen_room;

    point ret_point;
    /* Place the player in this room */
    ret_point.x = room_boxes[chosen_room].x + 1 + rand() % (room_boxes[chosen_room].width - 2);
    ret_point.y = room_boxes[chosen_room].y + 1 + rand() % (room_boxes[chosen_room].height - 2);

    return ret_point;
}

char convert_from_map(point pos) {
    switch(map[pos.y][pos.x] & 0xFF) {
        case '.':  return '.';
        case '-':  return '.';
        case 'd':  return '-';
        case 'D':  return '|';
        case 'e':  return '-';
        case 'E':  return '|';
        case 'O':  return '~';
        case 'c':  return 'o';
        case 'g':  return '&';
        case 'h':  return '6';
        case 'H':  return '9';
        case ']':  return '^';
        case '\0': return '#';
        case 'A':  return ']';
        case '/':  return '/';
        case 'W':  return 'W';
        default:   return '?';
    }
}

char tile_to_palette(point pos) {
    if (BIT(map[pos.y][pos.x], 8)) 
        return 7;

    switch(map[pos.y][pos.x] & 0xFF) {
        case 'c': return 8;
        case 'g': return 9;
        case 'h': return 10;
        case 'H': return 11;
        case ']': return 12;
        case 'W': return 13;
        default: return 1;
    }
}

void reveal_spaces() {
    /* First, reveal spaces around the player */
    
    if ((map[player_pos.y][player_pos.x] & 0xFF) == '.') {
        /* Reveal other tiles around this point */
        /* Create 9 sample points around player */
        point points[9] = {0};
        
        for (size_t i = 0; i < 9; i++) points[i] = (point){-1, -1};

        /* 012
           345
           678 */

        if (player_pos.y != 0 && player_pos.x != 0)   points[0] = (point){player_pos.x - 1, player_pos.y - 1};
        if (player_pos.y != 0)                        points[1] = (point){player_pos.x,     player_pos.y - 1};
        if (player_pos.y != 0 && player_pos.x != 49)  points[2] = (point){player_pos.x + 1, player_pos.y - 1};
        if (player_pos.x != 0)                        points[3] = (point){player_pos.x - 1, player_pos.y};
        if (1)                                        points[4] = (point){player_pos.x,     player_pos.y};
        if (player_pos.x != 49)                       points[5] = (point){player_pos.x + 1, player_pos.y};
        if (player_pos.y != 49 && player_pos.x != 0)  points[6] = (point){player_pos.x - 1, player_pos.y + 1};
        if (player_pos.y != 49)                       points[7] = (point){player_pos.x,     player_pos.y + 1};
        if (player_pos.y != 49 && player_pos.x != 49) points[8] = (point){player_pos.x + 1, player_pos.y + 1};
        
        for (size_t i = 0; i < 9; i++) {
            if (points[i].x == -1) continue;
            point curr = points[i];
            
            /* Check if tile is already revealed */
            if ((display_map[curr.y][curr.x] & 0xFF) == convert_from_map(curr)) continue;

            /* Reveal tile */
            display_map[curr.y][curr.x] = convert_from_map(curr);
        }
    }

    else if ((map[player_pos.y][player_pos.x] & 0xFF) == '-') {
        /* Reveal room if not exposed */
        /* Get room the player is in */
        size_t room_idx = 128;
        for (size_t i = 0; i < ROOM_COUNT; i++) {
            if (point_inside_box(room_boxes[i], player_pos)) {
                /* Found box */
                room_idx = i;
                break;
            }
        }

        if (room_idx == 128)
            return;

        for (size_t y = 0; y < room_boxes[room_idx].height; y++) {
            for (size_t x = 0; x < room_boxes[room_idx].width; x++) {
                /* Check if tile is already revealed */
                point curr = (point){room_boxes[room_idx].x + x, room_boxes[room_idx].y + y};
                
                if ((display_map[curr.y][curr.x] & 0xFF) == convert_from_map(curr)) continue;

                /* Reveal tile */
                display_map[curr.y][curr.x] = convert_from_map(curr);
            }
        }
    }
}

bool is_collider(point pos) {
    /* Check if this map tile is a collider */
    switch (map[pos.y][pos.x] & 0xFF) {
        case '\0':
        case 'e':
        case 'E': 
        case 'd':
        case 'D': 
        case 'K':
        case 'S':
        case '@':
        case '*':
        case ']':
        return true;
        default:   return false;
    }
}

extern bool running;

void kill_player() {
    running = false;
    db_exit();
    printf("\nYou died! Final score: %d\n", score);
    exit(-1);
}

void move_player(uint8_t dir) {
    old_player_pos = player_pos;

    /* If player starts in infection zone, increase infection */
    if (BIT(map[player_pos.y][player_pos.x], 8)) {
        if (rand() % 3 == 0) infection++;
    }

    /* Damage player based on ooze */
    if (infection > infection_tolerance/2) {
        sprintf(current_message, "The ooze digs into you!");
        player_hp--;
    }

    if (player_hp <= 0) {
        kill_player();
        return;
    }

    run_interaction((point){player_pos.x, player_pos.y}, true);

    tick_creatures();
    switch(dir) {
        case 0:
            if (player_pos.y != 0  && !is_collider((point){player_pos.x, player_pos.y-1})) player_pos.y--;
            if (player_pos.y != 0) run_interaction((point){player_pos.x, player_pos.y-1}, true);
            break;
        case 1:
            if (player_pos.x != 49 && !is_collider((point){player_pos.x + 1, player_pos.y})) player_pos.x++;
            if (player_pos.x != 49) run_interaction((point){player_pos.x + 1, player_pos.y}, true);
            break;
        case 2:
            if (player_pos.y != 49 && !is_collider((point){player_pos.x, player_pos.y+1})) player_pos.y++;
            if (player_pos.y != 49) run_interaction((point){player_pos.x, player_pos.y+1}, true);
            break;
        case 3:
            if (player_pos.x != 0  && !is_collider((point){player_pos.x - 1, player_pos.y})) player_pos.x--;
            if (player_pos.x != 0) run_interaction((point){player_pos.x - 1, player_pos.y}, true);
            break;
        case 4:
            plot(entities, player_pos.x, player_pos.y, 1, '\0');
            disqual_room = start_room;
            init_player();
            break;
        case 5:
            if ((map[player_pos.y][player_pos.x ] & 0xFF) == 'W') {
                /* Warp to new map */
                warp_new_map();
                return;
            }
            break;
        default:
            break;
    }

    reveal_spaces();

    render_player_info();
}

void warp_new_map() {
    /* Clean up enemy list */
    num_enemy = 0;
    /* Destroy old maps */
    for (size_t i = 0; i < 50; i++) {
        for (size_t j = 0; j < 50; j++) {
            map[i][j] = '\0';
            plot(entities, i, j, 1, '\0');
        }
    }

    level++;
    generate_map();
    /* Call new player init*/
    init_player();
}

void run_interaction(point pos, bool is_player) {
    bool was_map = true;
    switch (map[pos.y][pos.x] & 0xFF) {
        case 'd':
            map[pos.y][pos.x] = 'e';
            break;
        case 'D':
            map[pos.y][pos.x] = 'E';
            break;
        case 'e':
        case 'E':
            /* Convert door to O */
            map[pos.y][pos.x] = 'O';
            break;
        case 'c':
            if (!is_player) break;
            map[pos.y][pos.x] = '.';
            sprintf(current_message, "You pick up a coin!");
            score += 1 + rand() % 3;
            break;
        case 'g':
            if (!is_player) break;
            map[pos.y][pos.x] = '.';
            sprintf(current_message, "You pick up a gem!");
            score += 15 + rand() % 10;
            break;
        case 'h':
            if (!is_player) break;
            map[pos.y][pos.x] = '.';
            uint8_t extra_max = 5 + rand() % 6;
            sprintf(current_message, "You gained %d extra max hp!", extra_max);
            player_max_hp += extra_max;
            player_hp += extra_max;
            break;
        case 'H':
            if (!is_player) break;
            map[pos.y][pos.x] = '.';
            uint8_t hp_rest = 15 + rand() % 10;
            sprintf(current_message, "You restored %d hp!", hp_rest);
            player_hp += hp_rest;
            if (player_hp > player_max_hp) {
                player_hp = player_max_hp;
            }
            break;
        case ']':
            if (!is_player) break;
            infection_core_hp -= player_damage;
            sprintf(current_message, "You attack the infection core for %d damage!\0", player_damage);

            /* Kill creature possibly */
            if (infection_core_hp < 0) {
                map[pos.y][pos.x] = '.';
                uint8_t score_inc = 50 + rand() % 40;
                sprintf(current_message, "You kill the infection core! You are cured (for now)!\0");
                score += score_inc;
                infection = 0;
            }
            break;
        case 'A':
            if (!is_player) break;
            map[pos.y][pos.x] = '.';
            if (armour > new_armour_val) {
                sprintf(current_message, "You ignore the weaker armour");
                break;
            }
            armour = new_armour_val;
            sprintf(current_message, "You pick up a new piece of armour!");
            break;
        case '/':
            if (!is_player) break;
            map[pos.y][pos.x] = '.';
            if (player_damage > new_damage) {
                sprintf(current_message, "You ignore the weaker sword");
                break;
            }
            player_damage = new_damage;
            sprintf(current_message, "You pick up a new sword!");
            break;

        default:
            was_map = false;
        }
    
    if (!was_map && is_player) {
        /* Entity interactable */
        /* Find the entity interacted with */
        for (size_t i = 0; i < num_enemy; i++) {
            if (creatures[i].pos.x == pos.x && creatures[i].pos.y == pos.y) {
                if (!creatures[i].is_alive) continue;
                
                /* Attack the enemy */
                creatures[i].hp -= player_damage;
                sprintf(current_message, "You attack the %s (%d/%d) for %d damage!\0", creature_names[creatures[i].pic_idx], creatures[i].hp, creatures[i].max_hp, player_damage);

                /* Kill creature possibly */
                if (creatures[i].hp < 0) {
                    creatures[i].is_alive = false;
                    creatures[i].pic = dead_creature_pics[creatures[i].pic_idx];
                    creatures[i].palette = 6;
                    uint8_t score_inc = 5 + rand() % 15;
                    sprintf(current_message, "You kill the %s! It drops %d coins!\0", creature_names[creatures[i].pic_idx], score_inc);
                    score += score_inc;
                }
            }
        }
    } else if (!was_map) {
        /* Enemy on player interaction */
        if (player_pos.x == pos.x && player_pos.y == pos.y) {
            /* Attack the player */
            int16_t damage = level * 3 + rand() % 2;
            int16_t potential_damage = damage - armour;
            if (potential_damage < 0)
                potential_damage = 0;
            if (potential_damage == 0) {
                sprintf(current_message, "The enemy missed!");
            } else {
                player_hp -= potential_damage;
                sprintf(current_message, "You were attacked for %d damage!", potential_damage);
            }
            
        }
    }
}

void player_render() {
    /* Erase old player position */
    plot(entities, old_player_pos.x, old_player_pos.y, 1, '\0');
    
    /* Draw new player position */
    plot(entities, player_pos.x, player_pos.y, 1, '@');
}