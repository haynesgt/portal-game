sources = src/*.c

all:
	gcc -g -Wall `pkg-config --cflags glfw3` -o portal ${sources} `pkg-config --static --libs glfw3`
