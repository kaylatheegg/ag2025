#pragma once
#define INIT_H

/** Default character ring buffer size */
#define CHAR_RB_LEN 16

/** 
 * Read a character from the character ringbuffer
 * 
 * @return Character from the ring buffer, or 0 for no character
 */
char readKey();

void grabKeys(void);

void db_exit();

/**
 * Initialise the game tick timer
 */
void init_game_ticker();