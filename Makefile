TARGET = termgl_test
COMPILER = gcc

SRC = *.c
LINUX_CFLAGS := -lm -std=c11 -march=native -Wall -Wextra -Wdouble-promotion -Wpedantic -Wstrict-prototypes -Wshadow
RELEASE_CFLAGS := -O3
DEBUG_CFLAGS := -g -Og

test:
	$(COMPILER) termgl.c termgl3d.c termgl_test.c -o $(TARGET) $(LINUX_CFLAGS) $(RELEASE_CFLAGS)
clean:
	rm -rf $(TARGET)
