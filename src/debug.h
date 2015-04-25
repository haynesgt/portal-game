#pragma once

#define DEBUG_LEVEL 5

#define DEBUG(level, s, ...) \
	if (level <= DEBUG_LEVEL) fprintf(stderr, s, ##__VA_ARGS__)
