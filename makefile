sources = src/*.c
output = bin/portal

all:
	gcc -g -Wall `pkg-config --cflags glfw3` -o ${output} ${sources} `pkg-config --static --libs glfw3`

clean:
	@rm bin/portal
