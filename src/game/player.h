#pragma once
#ifndef PLAYER_H
#define PLAYER_H

void move_player(uint8_t dir);
void player_render();
void init_player();

char tile_to_palette(point pos);

#endif