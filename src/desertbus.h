#pragma once
#define DESERTBUS_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <stddef.h>
#include <errno.h>
#include <stdarg.h>
#include <signal.h>
#include <term.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <stdbool.h>

#include "init/util.h"
#include "init/init.h"

#include "orbit_types.h"
#include "orbit_da.h"
#include "orbit_string.h"

#include "init/memalloc.h"
#include "init/signal.h"
#include "render/window.h"
#include "render/shapes.h"

#include "game/display.h"
#include "game/mapgen.h"
#include "game/player.h"