#include "desertbus.h"

void* dbmalloc(size_t size) {
    void* data = malloc(size);
    if (data == NULL) crash("could not allocate memory block of size: %ld, error: %s\n", size, strerror(errno));

    memset(data, 0, size);
    return data;
}

void dbfree(void* data) {
    if (data == NULL) crash("attempted to free null ptr!\n");
    free(data);
}

void* dbrealloc(void* data, size_t size) {
    void* new_data = realloc(data, size);
    if (new_data == NULL) crash("could not reallocate memory block of size %ld, error: %s\n", size, strerror(errno));

    return new_data;
}