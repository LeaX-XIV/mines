#ifndef _ANSI_H
#define _ANSI_H

#include <stddef.h>

#include "mines.h"

enum ansi_color {
	COLOR_RESET = 0,
	COLOR_FG_BLACK = 30,
	COLOR_FG_RED = 31,
	COLOR_FG_GREEN = 32,
	COLOR_FG_YELLOW = 33,
	COLOR_FG_BLUE = 34,
	COLOR_FG_MAGENTA = 35,
	COLOR_FG_CYAN = 36,
	COLOR_FG_WHITE = 37,
	COLOR_FG_DEFAULT = 39,
	COLOR_BG_BLACK = 40,
	COLOR_BG_RED = 41,
	COLOR_BG_GREEN = 42,
	COLOR_BG_YELLOW = 43,
	COLOR_BG_BLUE = 44,
	COLOR_BG_MAGENTA = 45,
	COLOR_BG_CYAN = 46,
	COLOR_BG_WHITE = 47,
	COLOR_BG_DEFAULT = 49,
};

enum ansi_style {
	STYLE_NONE = 0,
	STYLE_BOLD = 1,
	STYLE_DIMMED = 2,
};

extern const char* CLR_SCR;
extern const char* RST_CRS;
extern const char* MOV_CRS;
extern const char* CRS_LFT;
extern const char* SET_STL;
extern const char* RST_STL;

void ansi_setup();
void ansi_teardown(size_t grid_rows);

enum action read_action();

char* ansi_color_str(char* str, enum ansi_color fg, enum ansi_color bg, enum ansi_style style);

void ansi_print_reset_cursor(char* str);
void ansi_print_reset_cursor_n(char* str, unsigned int n);
void ansi_move_cursor(int x, int y);

#endif
