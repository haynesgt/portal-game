#pragma once

#define DEBUG(level, s, args...) \
	if (level <= 5) fprintf(stderr, s, args)
