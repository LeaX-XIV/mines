#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "settings.h"
#include "mines.h"
#include "system.h"

struct settings game_settings;

void print_usage(char* filename) {
	printf("USAGE: %s help | [DIFFICULTY] [SIZE [<bombs>]]\n", filename);
	printf("\tDIFFICULTY = easy | medium | hard\n");
	printf("\teasy    Sets easy difficulty   (%2lux%2lu, %3u bombs) (Default)\n", EASY.rows, EASY.cols, EASY.bombs);
	printf("\tmedium  Sets medium difficulty (%2lux%2lu, %3u bombs)\n", MEDIUM.rows, MEDIUM.cols, MEDIUM.bombs);
	printf("\thard    Sets hard difficulty   (%2lux%2lu, %3u bombs)\n", HARD.rows, HARD.cols, HARD.bombs);
	printf("\n");
	printf("\tSIZE = full | (<rows> [<cols>])\n");
	printf("\tfull    Sets the grid size to fill the current terminal view\n");
	printf("\t<rows>  Number of rows of the grid\n");
	printf("\t<cols>  Number of columns of the grid\n");
	printf("\t<bombs> Number of bombs placed in the grid\n");
	printf("\n");
	printf("\thelp    Displays this message\n");
	printf("\n");
	printf("The game ends when you try to reveal a mine, or when all non-mine cells are\nrevealed and mine cells are flagged.\n");
	printf("\tIN GAME COMMANDS:\n");
	printf("\tq       Quit the game\n");
	printf("\tr       Restart\n");
	printf("\tarrows  Move the cursor\n");
	printf("\tspace   Reveal cell at cursor\n");
	printf("\tenter   Reveal cell at cursor\n");
	printf("\tf       Flag cell at cursor\n");
}

void parse_args(int n, char** _args) {
	int i = 1;
	int num_found = 0;
	char** args = _args + 1;

	while (i < n) {
		char* arg = args[0];
		args = args + 1;
		++i;

		if (0 == strcmp(arg, "help")) {
			print_usage(_args[0]);
			exit(EXIT_SUCCESS);
		} else if (0 == strcmp(arg, "easy")) {
			apply_settings(&game_settings, &EASY);
		} else if (0 == strcmp(arg, "medium")) {
			apply_settings(&game_settings, &MEDIUM);
		} else if (0 == strcmp(arg, "hard")) {
			apply_settings(&game_settings, &HARD);
		} else if (0 == strcmp(arg, "full")) {
			int _w, _h;
			if (!get_terminal_size(&_w, &_h)) {
				exit(1);
			}

			size_t n_cells = game_settings.rows * game_settings.cols;
			double bombs_ratio = (double)game_settings.bombs / n_cells;

			game_settings.rows = _h;
			game_settings.cols = _w / 3;
			n_cells = game_settings.rows * game_settings.cols;
			game_settings.bombs = n_cells * bombs_ratio + 1;

			num_found = 2;
		} else {
			int n = atoi(arg);
			if (n < 0)
				exit(1);

			switch (num_found) {
				case 0: game_settings.rows = n;
				break;
				case 1: game_settings.cols = n;
				break;
				case 2: game_settings.bombs = n;
				break;
				default: exit(1);
				break;
			}
			++num_found;
		}
	}
}

int main(int argc, char** argv) {
	apply_settings(&game_settings, &EASY);
	parse_args(argc, argv);

	struct mines_ctx* ctx = mines_init(game_settings);
	if (ctx == NULL)
		return 1;

	reset_game(ctx);

	mines_game_loop(ctx);
	bool game_won = check_win(ctx);

	mines_destroy(ctx);

	if (game_won) {
		printf("You Win\n");
	} else {
		printf("You lost :(\n");
	}

	return EXIT_SUCCESS;
}
