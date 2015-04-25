#pragma once

#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdbool.h>

#include "colors.h"

typedef struct game game_t;

typedef struct game {
	int width;
	int height;
	char *window_name;
	void (*update_callback)(game_t *game) ;
	void (*draw_callback)(game_t *game) ;
	bool disable_cursor;

	GLFWwindow *window;
	void *user_data;
} game_t;

extern char keys[256];
extern double green[3];
extern double lime[3];

extern int window_width, window_height;

int key(char c);

int get_mouse_x(void);
int get_mouse_y(void);
int get_mouse_button(game_t *game, int button);
double sign(double x);
double norm2d(double x, double y);

void game_init(game_t *game);
void game_run(game_t *game);
