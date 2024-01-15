#include "ansi.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "system.h"

const char* CLR_SCR = "\033[2J";

const char* RST_CRS = "\033[H";
const char* MOV_CRS = "\033[%d;%dH";
const char* CRS_LFT = "\033[%dD";

const char* SET_STL = "\033[%d;%d;%dm";
const char* RST_STL = "\033[0m";

void ansi_setup() {
	system_init();

	setbuf(stdout, NULL);

	printf(CLR_SCR, NULL);
	printf(RST_CRS, NULL);
}

void ansi_teardown(size_t grid_rows) {
	ansi_move_cursor(1, grid_rows);

	system_teardown();
}

enum action read_action() {
	enum action action = A_NONE;
	int c = conio_getch();

	if (c == 224) {
		c = conio_getch();
	}

	switch (c) {
		case 'q': action = A_QUIT;    break;
		case 'r': action = A_RESTART; break;
		case  13:
		case  10:
		case ' ': action = A_REVEAL;  break;
		case 'f': action = A_FLAG;    break;
		case  72: action = A_UP;      break;
		case  80: action = A_DOWN;    break;
		case  77: action = A_RIGHT;   break;
		case  75: action = A_LEFT;    break;
		default: /* noop */;          break;
	}

	return action;
}

char BUFFER1[16] = { 0 };
char* ansi_color_str(char* str, enum ansi_color fg, enum ansi_color bg, enum ansi_style style) {
	snprintf(BUFFER1, 16, SET_STL, style, fg, bg);
	snprintf(&BUFFER1[strlen(BUFFER1)], 16, "%s%s", str, RST_STL);
	return BUFFER1;
}

char BUFFER2[8] = { 0 };
void ansi_print_reset_cursor(char* str) {
	int n = printf(str, NULL);
	snprintf(BUFFER2, 8, CRS_LFT, n);
	printf(BUFFER2, NULL);
}

char BUFFER3[8] = { 0 };
void ansi_print_reset_cursor_n(char* str, unsigned int n) {
	snprintf(BUFFER3, 8, CRS_LFT, n);
	printf("%s%s", str, BUFFER3);
}

char BUFFER4[12] = { 0 };
void ansi_move_cursor(int x, int y) {
	snprintf(BUFFER4, 12, MOV_CRS, y + 1, x + 1);
	printf(BUFFER4, NULL);
}
