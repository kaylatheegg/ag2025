#pragma once
#ifndef MAPGEN_H
#define MAPGEN_H

/** Extract bit n of value */
#define BIT(value, n) (((value) >> (n)) & 1)

/** Generate a random 50x50 map */
void generate_map();

typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    bool invalid;
} box;

#define ROOM_COUNT 128

bool point_inside_box(box a, point b);

#endif