TARGET = bin
COMPILER = gcc

SRC = ../src/termgl.c termgl_test.c
WARNINGS := -Wall -Wextra -Wpedantic -Wdouble-promotion -Wstrict-prototypes -Wshadow -Wduplicated-cond -Wduplicated-branches -Wjump-misses-init -Wnull-dereference -Wrestrict -Wlogical-op -Wno-maybe-uninitialized -Walloc-zero -Wformat-security -Wformat-signedness -Winit-self -Wlogical-op -Wmissing-declarations -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wswitch-enum -Wundef -Wwrite-strings -Wno-address-of-packed-member -Wno-discarded-qualifiers
CFLAGS := -std=c11 -march=native -DTERMGL3D
LINK_CFLAGS := -lm
RELEASE_CFLAGS := -O3

test:
	$(COMPILER) $(SRC) -o $(TARGET) $(WARNINGS) $(CFLAGS) $(LINK_CFLAGS) $(DEBUG_CFLAGS)
clean:
	rm -rf $(TARGET)
