#include "game.h"

char keys[256];

int window_width, window_height;

double aspect = 1;

int key(char c) { return keys[(size_t)c]; }

int cursor_x, cursor_y;
int mouse_x, mouse_y;

int get_mouse_x(void) { return mouse_x; }
int get_mouse_y(void) { return mouse_y; }
int get_mouse_button(game_t *game, int button) { return glfwGetMouseButton(game->window, button); }
double sign(double x) { if (x > 0) return 1; if (x < 0) return -1; return 0; }
double norm2d(double x, double y) {return sqrt(x*x+y*y);}

void resize(GLFWwindow *windo, int w, int h)
{
	glViewport(0, 0, w, h);
	aspect = (double) w / h;
	window_width = w;
	window_height = h;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	keys[key] = action != GLFW_RELEASE;
}

void cursor_pos_callback(GLFWwindow *window, double x, double y )
{
	cursor_x = x;
	cursor_y = y;
}

void game_init(game_t *game) {
	GLFWmonitor *monitor;
	const GLFWvidmode *mode;
	GLFWwindow *window;
	glfwInit();
	glfwWindowHint(GLFW_SAMPLES, 4);
	monitor = glfwGetPrimaryMonitor();
	mode = glfwGetVideoMode(monitor);
	window_width = game->width;
	window_height = game->height;
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
	window = glfwCreateWindow(window_width, window_height, game->window_name, NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetWindowSizeCallback(window, resize);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_pos_callback);
	cursor_pos_callback(window, 0, 0);
	
	if (game->disable_cursor)
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glLineWidth(1.25);

	resize(window, window_width, window_height);
	glClearColor(1, 1, 1, 1);

	int i;
	for (i = 0; i < 256; i++) keys[i] = 0;

	game->window = window;
}

void game_run(game_t *game) {
	GLFWwindow *window;
	window = game->window;
	while (!glfwWindowShouldClose(window)) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, window_width, 0, window_height, -1, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		double x, y;
		x = cursor_x;
		y = cursor_y;
		// glfwSetCursorPos(window, x*max_d/d, y*max_d/d);
		
		mouse_x = x;
		mouse_y = window_height - y;

		game->update_callback(game);
		game->draw_callback(game);

		glfwSwapBuffers(window);
		glfwPollEvents();
		usleep(17e3);
	}
	glfwTerminate();
}
