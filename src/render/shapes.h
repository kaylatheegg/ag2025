#pragma once
#define SHAPES_H

typedef struct {
    int x;
    int y;
} point;

void drawLine(window* win, point a, point b, char colour, char character);
void drawTriangle(window* win, point a, point b, point c, char colour, char character);
void interruptedLine(window* win, point a, point b, int length, int offset, char colour, char character);
void drawCircle(window* win, point a, int radius, char colour, char character);

#define P(x, y) (point){(x), (y)}