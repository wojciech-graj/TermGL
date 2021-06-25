TARGET = console_game_engine
COMPILER = gcc

SRC = *.c
LINUX_CFLAGS := -lm -std=c11 -march=native
RELEASE_CFLAGS := -O3
DEBUG_CFLAGS := -Wall -Wextra -Wdouble-promotion -Wpedantic -Wstrict-prototypes -Wshadow -g -Ofast -DDEBUG -fsanitize=address -fsanitize=undefined -Wno-maybe-uninitialized

debug:
	$(COMPILER) console_game_engine.c $(LINUX_CFLAGS) $(DEBUG_CFLAGS)
test:
	$(COMPILER) console_game_engine.c console_game_engine_test.c -o $(TARGET) $(LINUX_CFLAGS) $(RELEASE_CFLAGS)
clean:
	rm -rf $(TARGET)
