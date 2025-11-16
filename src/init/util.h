#pragma once
#define UTIL_H

void crash(char* error, ...);

#define FOR_RANGE_INCL(iterator, start, end) for (intptr_t iterator = (start); iterator <= (end); iterator++)
#define for_range(iterator, start, end) for (intptr_t iterator = (start); iterator < (end); iterator++)

#define FOR_URANGE_INCL(iterator, start, end) for (uintptr_t iterator = (start); iterator <= (end); iterator++)
#define for_urange(iterator, start, end) for (uintptr_t iterator = (start); iterator < (end); iterator++)

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))

#define max3(a, b, c) max(a, max(b, c))
#define min3(a, b, c) min(a, min(b, c))

#define CTRL_KEY(k) ((k) & 0x1f)

#define swap(x, y) \
    x ^= y; \
    y ^= x; \
    x ^= y; \
    while(0)

#define iabs(x) (((x) > 0) ? (x) : -(x))

#define map(x, a, b, A, B) ((x) - (a)) * ((B) - (A)) / ((b) - (a)) + B;
