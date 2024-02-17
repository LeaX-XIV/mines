#include "mines.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "ansi.h"

// Private interface declaration

struct cursor {
	unsigned int x, y;
};

struct mines_ctx {
	struct cursor cursor;
	struct cell* grid;
	size_t rows;
	size_t cols;
	unsigned int n_bombs;
};

struct {
	enum ansi_style style;
	enum ansi_color fgc;
	enum ansi_color bgc;
	char buff[2];
} CELL_STR_INFO;

void init_cell(struct mines_ctx* ctx, unsigned int x, unsigned int y);
unsigned int count_nearby_bombs(struct mines_ctx* ctx, unsigned int x, unsigned int y);
void place_random_bombs(struct mines_ctx* ctx);

void cell_str_init(struct cell* cell);
char* show_cell(struct cell* cell);
struct cell* cell_at(struct mines_ctx* ctx, unsigned int x, unsigned int y);
struct cell* cell_at_cursor(struct mines_ctx* ctx);

void highlight_at(unsigned int x, unsigned int y);
void highlight_at_cursor(struct mines_ctx* ctx);

void unhide_at(struct mines_ctx* ctx, unsigned int x, unsigned int y);
void unhide_at_cursor(struct mines_ctx* ctx);
void unhide_all_bombs(struct mines_ctx* ctx);

void toggle_flag_at(struct mines_ctx* ctx, unsigned int x, unsigned int y);
void toggle_flag_at_cursor(struct mines_ctx* ctx);

bool try_move_cursor(struct mines_ctx* ctx, enum action action);
bool check_boundary(struct mines_ctx* ctx, int x, int y);
void print_cell(struct cell* cell);

// Public interface implementation

struct mines_ctx* mines_init(struct settings settings) {
	struct mines_ctx* ctx = calloc(1, sizeof *ctx);

	if (settings.rows * settings.cols < settings.bombs)
		return NULL;

	ctx->rows = settings.rows;
	ctx->cols = settings.cols;
	ctx->n_bombs = settings.bombs;

	ctx->grid = calloc(ctx->rows * ctx->cols, sizeof *(ctx->grid));

	ansi_setup();

	return ctx;
}

void mines_destroy(struct mines_ctx* ctx) {
	if (ctx == NULL)
		return;

	ansi_teardown(ctx->rows);

	free(ctx->grid);
	free(ctx);
}

void reset_game(struct mines_ctx* ctx) {
	srand(time(NULL));

	// Init cells
	for (size_t y = 0; y < ctx->rows; ++y) {
		for (size_t x = 0; x < ctx->cols; ++x) {
			init_cell(ctx, x, y);
		}
	}

	// Place bombs
	for (unsigned int i = 0; i < ctx->n_bombs; ++i) {
		place_random_bombs(ctx);
	}

	// Update nearby bombs counter
	for (size_t y = 0; y < ctx->rows; ++y) {
		for (size_t x = 0; x < ctx->cols; ++x) {
			struct cell* cell = cell_at(ctx, x, y);
			cell->n_bombs_nearby = count_nearby_bombs(ctx, x, y);
		}
	}

	// Draw grid on screen
	char* buf = calloc(ctx->rows * ( 3 * ctx->cols + 2 ) + 32, sizeof *buf);
	size_t idx = 0;

	struct cell* cell = cell_at(ctx, 0, 0);
	cell_str_init(cell);
	idx += sprintf(
		buf + idx, SET_STL,
		CELL_STR_INFO.style, CELL_STR_INFO.fgc, CELL_STR_INFO.bgc
	);

	for (size_t y = 0; y < ctx->rows; ++y) {
		for (size_t x = 0; x < ctx->cols; ++x) {
			struct cell* cell = cell_at(ctx, x, y);
			cell_str_init(cell);
			idx += sprintf(buf + idx, " %c ", CELL_STR_INFO.buff[0]);
		}
		// CRLF needed for Linux compatibility
		idx += sprintf(buf + idx, "\r\n");
	}
	idx += sprintf(buf + idx, RST_STL, NULL);

	ansi_move_cursor(0, 0);
	printf("%s", buf);
	free(buf);

	ctx->cursor.x = 0;
	ctx->cursor.y = 0;
	highlight_at_cursor(ctx);
}

bool check_win(struct mines_ctx* ctx) {
	for (size_t y = 0; y < ctx->rows; ++y) {
		for (size_t x = 0; x < ctx->cols; ++x) {
			struct cell* cell = cell_at(ctx, x, y);
			if (
				(!cell->is_bomb && cell->is_hidden) ||
				(cell->is_bomb && !cell->is_flag)
			) {
				return false;
			}
		}
	}

	return true;
}

void mines_game_loop(struct mines_ctx *ctx) {
	bool close = false;
	do {
		struct cell* cell = cell_at_cursor(ctx);
		enum action action = read_action();

		switch (action) {
			case A_QUIT:
				close = true;
				break;

			case A_RESTART:
				reset_game(ctx);
				break;

			case A_REVEAL: {
				if (cell->is_flag)
					break;
				
				unhide_at_cursor(ctx);
				if (cell->is_bomb) {
					close = true;
					unhide_all_bombs(ctx);
				}
				try_move_cursor(ctx, A_NONE);
			} break;

			case A_FLAG: {
				if (!cell->is_hidden)
					break;

				toggle_flag_at_cursor(ctx);
				print_cell(cell);
			} break;

			case A_UP:
			case A_DOWN:
			case A_LEFT:
			case A_RIGHT:
			default:
				try_move_cursor(ctx, action);
				break;
		}

		if (check_win(ctx))
			close = true;
	} while (!close);
}

// Private interface implementation

const enum ansi_color COLOR_PER_NEARBY_BOMBS[9] = {
	COLOR_FG_BLACK,
	COLOR_FG_GREEN,
	COLOR_FG_YELLOW,
	COLOR_FG_BLUE,
	COLOR_FG_MAGENTA,
	COLOR_FG_CYAN,
	COLOR_FG_YELLOW,
	COLOR_FG_BLUE,
	COLOR_FG_MAGENTA,
};

const enum ansi_style STYLE_PER_NEARBY_BOMBS[9] = {
	STYLE_NONE,
	STYLE_BOLD,
	STYLE_BOLD,
	STYLE_BOLD,
	STYLE_BOLD,
	STYLE_BOLD,
	STYLE_NONE,
	STYLE_NONE,
	STYLE_NONE,
};

void init_cell(struct mines_ctx* ctx, unsigned int x, unsigned int y) {
	struct cell* cell = cell_at(ctx, x, y);
	cell->x = x;
	cell->y = y;
	cell->n_bombs_nearby = 0;
	cell->is_hidden = true;
	cell->is_bomb = false;
	cell->is_flag = false;
}

unsigned int count_nearby_bombs(struct mines_ctx* ctx, unsigned int x, unsigned int y) {
	unsigned int n_bombs = 0;

	for (int dy = -1; dy <= +1 ; ++dy) {
		for (int dx = -1; dx <= +1; ++dx) {
			if (dx == 0 && dy == 0)
				continue;

			int cell_x = x + dx;
			int cell_y = y + dy;

			if (!check_boundary(ctx, cell_x, cell_y))
				continue;

			const struct cell* cell = cell_at(ctx, cell_x, cell_y);
			if (cell->is_bomb)
				++n_bombs;
		}
	}

	return n_bombs;
}

void cell_str_init(struct cell* cell) {
	CELL_STR_INFO.style = STYLE_NONE;
	CELL_STR_INFO.fgc = COLOR_FG_DEFAULT;
	CELL_STR_INFO.bgc = COLOR_BG_DEFAULT;
	CELL_STR_INFO.buff[0] = 0;
	CELL_STR_INFO.buff[1] = 0;

	if (cell->is_flag) {
		*CELL_STR_INFO.buff = 'P';
		CELL_STR_INFO.fgc = COLOR_FG_BLUE;
		CELL_STR_INFO.style = STYLE_NONE;
	} else if (cell->is_hidden) {
		*CELL_STR_INFO.buff = '.';
		CELL_STR_INFO.fgc = COLOR_FG_WHITE;
		CELL_STR_INFO.style = STYLE_DIMMED;
	} else if (cell->is_bomb) {
		*CELL_STR_INFO.buff = '#';
		CELL_STR_INFO.fgc = COLOR_FG_RED;
		CELL_STR_INFO.style = STYLE_BOLD;
	} else {
		*CELL_STR_INFO.buff = cell->n_bombs_nearby + '0';
		CELL_STR_INFO.fgc = COLOR_PER_NEARBY_BOMBS[cell->n_bombs_nearby];
		CELL_STR_INFO.style = STYLE_PER_NEARBY_BOMBS[cell->n_bombs_nearby];
	}
}

char* show_cell(struct cell* cell) {
	cell_str_init(cell);

	return ansi_color_str(
		CELL_STR_INFO.buff,
		CELL_STR_INFO.fgc,
		CELL_STR_INFO.bgc,
		CELL_STR_INFO.style
	);
}

void place_random_bombs(struct mines_ctx* ctx) {
	int n_cells = ctx->rows * ctx->cols;
	int index = rand() % n_cells;
	for (int i = 0; i < n_cells; ++i) {
		int idx = (index + i) % n_cells;
		if (ctx->grid[idx].is_bomb)
			continue;

		ctx->grid[idx].is_bomb = true;
		break;
	}
}

struct cell* cell_at(struct mines_ctx* ctx, unsigned int x, unsigned int y) {
	const size_t offset = y * ctx->cols + x;
	return &(ctx->grid[offset]);
}

struct cell* cell_at_cursor(struct mines_ctx* ctx) {
	return cell_at(ctx, ctx->cursor.x, ctx->cursor.y);
}

void highlight_at(unsigned int x, unsigned int y) {
	int abs_x = x * 3 + 1;
	int abs_y = y;

	ansi_move_cursor(abs_x, abs_y);
}

void highlight_at_cursor(struct mines_ctx* ctx) {
	highlight_at(ctx->cursor.x, ctx->cursor.y);
}

void unhide_at(struct mines_ctx* ctx, unsigned int x, unsigned int y) {
	struct cell* cell = cell_at(ctx, x, y);

	if (!cell->is_hidden)
		return;

	cell->is_hidden = false;

	highlight_at(x, y);

	print_cell(cell);

	// Also unhide neighbors of 0 cells
	if (!cell->is_bomb && cell->n_bombs_nearby == 0) {
		for (int dy = -1; dy <= +1 ; ++dy) {
			for (int dx = -1; dx <= +1; ++dx) {
				if (dx == 0 && dy == 0)
					continue;

				int cell_x = x + dx;
				int cell_y = y + dy;

				if (!check_boundary(ctx, cell_x, cell_y))
					continue;

				unhide_at(ctx, cell_x, cell_y);
			}
		}
	}
}

void unhide_at_cursor(struct mines_ctx* ctx) {
	unhide_at(ctx, ctx->cursor.x, ctx->cursor.y);
}

void unhide_all_bombs(struct mines_ctx* ctx) {
	for (size_t y = 0; y < ctx->rows; ++y) {
		for (size_t x = 0; x < ctx->cols; ++x) {
			struct cell* cell = cell_at(ctx, x, y);
			if (cell->is_bomb) {
				cell->is_flag = false;
				unhide_at(ctx, x, y);
			}
		}
	}
}

void toggle_flag_at(struct mines_ctx* ctx, unsigned int x, unsigned int y) {
	struct cell* cell = cell_at(ctx, x, y);
	cell->is_flag = !cell->is_flag;
}

void toggle_flag_at_cursor(struct mines_ctx* ctx) {
	toggle_flag_at(ctx, ctx->cursor.x, ctx->cursor.y);
}

bool try_move_cursor(struct mines_ctx* ctx, enum action action) {
	int x = ctx->cursor.x;
	int y = ctx->cursor.y;

	switch (action) {
		case A_UP: --y;
		break;
		case A_DOWN: ++y;
		break;
		case A_RIGHT: ++x;
		break;
		case A_LEFT: --x;
		break;
		default: /* noop */ break;
	}

	bool ok = check_boundary(ctx, x, y);
	if (ok) {
		ctx->cursor.x = x;
		ctx->cursor.y = y;
	}

	highlight_at_cursor(ctx);

	return ok;
}

bool check_boundary(struct mines_ctx* ctx, int x, int y) {
	(void)ctx;
	return x >= 0 && x < (int)ctx->cols && y >= 0 && y < (int)ctx->rows;
}

void print_cell(struct cell* cell) {
	char* c = show_cell(cell);
	ansi_print_reset_cursor_n(c, 1);
}
