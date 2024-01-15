#include "settings.h"

const struct settings EASY = {
	.rows = 10,
	.cols = 10,
	.bombs = 10
};

const struct settings MEDIUM = {
	.rows = 15,
	.cols = 15,
	.bombs = 40
};

const struct settings HARD = {
	.rows = 15,
	.cols = 30,
	.bombs = 100
};

void apply_settings(struct settings* dst, const struct settings* src) {
	dst->rows = src->rows;
	dst->cols = src->cols;
	dst->bombs = src->bombs;
}
