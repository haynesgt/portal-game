file:
	gcc -g -Wall `pkg-config --cflags glfw3` -o portal main.c game.c colors.c map.c glDraw.c `pkg-config --static --libs glfw3`
