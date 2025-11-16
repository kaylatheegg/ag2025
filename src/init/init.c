#define ORBIT_IMPLEMENTATION

#include "desertbus.h"

void db_exit();

char readKey();

timer_t timerid;

int main(int argc, char *argv[]) {   
    init_signal_handler();
    init_render();

    time_t time_val = time(0);

    srand(time_val);

    initGame();

    db_exit();
}

void init_game_ticker() {
    //setup the 10ms timer for ticking the road paint forward
    struct sigevent sev;
    struct itimerspec its;
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGRTMIN;
    sev.sigev_value.sival_ptr = &timerid;

    if (timer_create(CLOCK_MONOTONIC, &sev, &timerid) == -1) crash("timer could not be created!");
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = 1;

    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 10000000; //10ms

    timer_settime(timerid, 0, &its, NULL);
}

static size_t char_insert = 0;
static size_t char_extract = 0;

char char_rb[CHAR_RB_LEN] = {0};

/** 
 * Grab any keys the user is pressing
 */
void grabKeys(void) {
    int c;

    while ((c = getchar())) {
        if (c == EOF || feof(stdin)) return;

        char_rb[char_insert] = c;
        char_insert++;
        char_insert %= CHAR_RB_LEN;
    }
}

char readKey(void) {
    char c;

    if (char_insert == char_extract)
        return 0;

    c = char_rb[char_extract];
    char_extract++;

    char_extract %= CHAR_RB_LEN;

    return c;
}

void db_exit() {
    exit_render();
}