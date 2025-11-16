#include "desertbus.h"
#include "render/colours.h"

int current_frame = 0;

int speed = 10;

void render_scene();

bool running = false;
bool is_initialised = false;

char buf[128] = {0};

extern uint16_t map[50][50];
extern uint8_t display_map[50][50];

window* entities;
window* player_stuff;

void run_game_loop();

void timer_tick() {
    get_render_dimensions();

    if (running) {
        run_game_loop();
    }

    render_scene();
    player_render();
    current_frame++;
}

void run_game_loop() {
    char c;
    size_t buf_idx = 0;
    size_t runs = 0;

    if ((c = readKey())) {
        if (c == CTRL_KEY('q')) {
            is_initialised = false;
            running = false;
        }
        if (c == 'W' || c == 'w') move_player(0);
        if (c == 'D' || c == 'd') move_player(1);
        if (c == 'S' || c == 's') move_player(2);
        if (c == 'A' || c == 'a') move_player(3);
        if (c == 'P' || c == 'p') move_player(4);
        if (c == 'G' || c == 'g') move_player(5);

        runs++;
    }
}

window* world;

void initGame() {
    add_colour("player",   ANSI_FG_BG(15, 0));
    add_colour("kobold",   ANSI_FG_BG(196, 0));
    add_colour("skeleton", ANSI_FG_BG(229, 0));
    add_colour("snake",    ANSI_FG_BG(106, 0));
    add_colour("wisp",     ANSI_FG_BG(45, 0));
    add_colour("dead",     ANSI_FG_BG(0, 160));
    add_colour("infected", ANSI_FG_BG(165, 0));
    add_colour("coin",     ANSI_FG_BG(214, 0));
    add_colour("gem",      ANSI_FG_BG(125, 0));
    add_colour("hpup",     ANSI_FG_BG(196, 0));
    add_colour("hprest",   ANSI_FG_BG(125, 0));
    add_colour("infcore1", ANSI_FG_BG(202, 0));
    add_colour("warp",     ANSI_FG_BG(20, 0));
    
    
    generate_map();

    world = new_window(50, 50);
    entities = new_window(50, 50);
    player_stuff = new_window(40, 50);
    player_stuff->width_offset = 50;

    is_initialised = true;
    running = true;

    init_game_ticker();

    init_player();

    while (running) {
        /* Grab all keys*/
        grabKeys();
    }

    destroy_window(entities);
    destroy_window(world);
    db_exit();
}

void render_scene() {
    /* Check that we're not actually running early */
    if (!is_initialised) 
        return;

    for (size_t i = 0; i < 50; i++) {
        for (size_t j = 0; j < 50; j++) {
            uint8_t palette = tile_to_palette((point){i, j});
            plot(world, i, j, palette, display_map[j][i] & 0xFF);
        }
    }

    /* Erase entire screen */
    //write(STDOUT_FILENO, "\x1b[2J", 4);

    render_win(world);
    render_win(entities);
    render_win(player_stuff);

    fsync(STDOUT_FILENO);

}