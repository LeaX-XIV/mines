#ifndef _MINES_H
#define _MINES_H

#include <stdbool.h>

#include "settings.h"

enum action {
	A_NONE = 0,
	A_QUIT = 1,
	A_RESTART = 2,
	A_UP = 3,
	A_DOWN = 4,
	A_LEFT = 5,
	A_RIGHT = 6,
	A_REVEAL = 7,
	A_FLAG = 8
};

#include "ansi.h"

struct mines_ctx;

struct cell {
	unsigned int x, y, n_bombs_nearby;
	bool is_hidden, is_bomb, is_flag;
};

struct mines_ctx* mines_init(struct settings settings);
void mines_destroy(struct mines_ctx* ctx);

bool check_win(struct mines_ctx* ctx);
void reset_game(struct mines_ctx* ctx);
void mines_game_loop(struct mines_ctx *ctx);

#endif
