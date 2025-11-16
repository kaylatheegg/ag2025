# Aber Game Jam 2025 submission: The Ooze
The Ooze is a simple dungeon crawler, with a slowly spreading "ooze" that spreads through the dungeon, attempting to halt your progress

## Building
Make sure you have make, gcc (clang untested) (or just grab your distro's version of build-essential)

Navigate to the root of the repo, and then run `make clean` to build the build structure, then run `make` to build the game. It is jank. I do not care.

## Controls
- WASD for movement
- g for warping when over a Warp
- p for warping inside a dungeon (if you get spawned in a glitched room)

## Known bugs
- This crashes vscode due to some kind of terminal jank (maybe it doesn't emulate the ASB correctly?)
- The room generator is jank as all hell (mapgen.c), so it can misbehave

