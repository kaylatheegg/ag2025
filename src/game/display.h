#pragma once
#define DISPLAY_H

void initGame();

void timer_tick();

#define PI 3.1415926535

#define ROT_P(p, angle) P(cos(angle) * (p).x - sin(angle) * (p).y, sin(angle) * (p).x + cos(angle) * (p).y)