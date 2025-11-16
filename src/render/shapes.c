#include "desertbus.h"

void drawLine(window* win, point a, point b, char colour, char character) {
    if (win == NULL) return;
    //bresenham's line alg
    //https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm

    int x_0 = a.x;
    int y_0 = a.y;
    int x_1 = b.x;
    int y_1 = b.y;

    if (x_0 > win->width || x_0 < 0 || 
        x_1 > win->width || x_1 < 0) return;
    if (y_0 > win->height || y_0 < 0 ||
        y_1 > win->height || y_1 < 0) return;


    int dx = iabs(x_1 - x_0);
    int dy = -iabs(y_1 - y_0);
    int sx = x_0 < x_1 ? 1 : -1;
    int sy = y_0 < y_1 ? 1 : -1;
    int err = dx + dy;
    int e2 = 0;
    for (;;) {
        plot(win, x_0, y_0, colour, character);
        if (x_0 == x_1 && y_0 == y_1) break;
        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x_0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y_0 += sy;
        }
    }
}

void interruptedLine(window* win, point a, point b, int length, int offset, char colour, char character) {
    if (win == NULL) return;
    //bresenham's line alg
    //https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm

    int x_0 = a.x;
    int y_0 = a.y;
    int x_1 = b.x;
    int y_1 = b.y;

    if (x_0 > win->width || x_0 < 0 || 
        x_1 > win->width || x_1 < 0) return;
    if (y_0 > win->height || y_0 < 0 ||
        y_1 > win->height || y_1 < 0) return;


    int dx = iabs(x_1 - x_0);
    int dy = -iabs(y_1 - y_0);
    int sx = x_0 < x_1 ? 1 : -1;
    int sy = y_0 < y_1 ? 1 : -1;
    int err = dx + dy;
    int e2 = 0;
    int current_length = offset;
    for (;;) {
        if (current_length <= length) {
            plot(win, x_0, y_0, colour, character);    
        } else if (current_length >= 2 * length) {
            current_length = 0;
        }
        current_length++;
        if (x_0 == x_1 && y_0 == y_1) break;
        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x_0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y_0 += sy;
        }
    }
}

void drawCircle(window* win, point a, int radius, char colour, char character) {
    //we get bounding box of circle
    int minX = max(a.x - radius, 0);
    int maxX = min(a.x + radius, win->width);
    int minY = max(a.y - radius, 0);
    int maxY = min(a.y + radius, win->height);

    for (int y = minY; y < maxY; y++) {
        for (int x = minX; x < maxX; x++) {
            int dx = x - a.x;
            int dy = y - a.y;
            if (dx*dx + dy*dy < radius*radius) {
                plot(win, x, y, colour, character);
            }
        }
    }
}

int wedgeProduct(point a, point b, point c) {
    return (b.x - a.x) * (c.y - a.y) -
           (b.y - a.y) * (c.x - a.x);
}

void drawTriangle(window* win, point a, point b, point c, char colour, char character) {
    //first, we compute the bounding box and then bound it
    int minX = max(min3(a.x, b.x, c.x), 0);
    int minY = max(min3(a.y, b.y, c.y), 0);
    int maxX = min(max3(a.x, b.x, c.x), win->width - 1);
    int maxY = min(max3(a.y, b.y, c.y), win->height - 1);

    for (int x = minX; x <= maxX; x++) {
        for (int y = minY; y <= maxY; y++) {
            int w0 = wedgeProduct(b, c, P(x, y));
            int w1 = wedgeProduct(c, a, P(x, y));
            int w2 = wedgeProduct(a, b, P(x, y));
            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                plot(win, x, y, colour, character);
            }
        }
    }    
}
