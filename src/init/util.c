#include "desertbus.h"

void crash(char* error, ...) {
    exit_render();
    
    va_list args;
    va_start(args, error);
    vprintf(error, args);
    va_end(args);
    exit(-1);
}