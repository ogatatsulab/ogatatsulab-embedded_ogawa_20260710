#ifndef RENDER_H
#define RENDER_H

#include "game.h"

void console_init(void);
void console_cleanup(void);
void render_game(const GameState *game);

#endif