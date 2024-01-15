#ifndef _SETTINGS_H
#define _SETTINGS_H

#include <stddef.h>

struct settings {
	size_t rows;
	size_t cols;
	unsigned int bombs;
};

extern const struct settings EASY;
extern const struct settings MEDIUM;
extern const struct settings HARD;

void apply_settings(struct settings* dst, const struct settings* src);

#endif
