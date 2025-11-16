#pragma once
#define WINDOW_H

typedef struct {
    int width;
    int height;
    int width_offset;
    int height_offset;
    char** window_data;
} window;

typedef struct {
    char* name;
    char* colour;
} entry;

typedef struct {
    int width;
    int height;
} term_info;

da_typedef(entry);


void init_render();
void add_colour(char* string, char* colour);
window* new_window(int width, int height);
void render_win(window* win);
void exit_render();
void get_render_dimensions();
void destroy_window(window* win);


#define plot(window, x, y, colour, character) \
    (window)->window_data[(y)][2*(x) + 0] = (colour); \
    (window)->window_data[(y)][2*(x) + 1] = (character);

//window data is stored in char pairs:
//first char is colour index, second char is actual character.