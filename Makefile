SRC = src/termgl.c
DEMO = test/termgl_test.c
WARNINGS := -Wall -Wextra -Wpedantic -Wdouble-promotion -Wstrict-prototypes -Wshadow -Wduplicated-cond -Wduplicated-branches -Wjump-misses-init -Wnull-dereference -Wrestrict -Wlogical-op -Wno-maybe-uninitialized -Walloc-zero -Wformat-security -Wformat-signedness -Winit-self -Wlogical-op -Wmissing-declarations -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wswitch-enum -Wundef -Wwrite-strings -Wno-address-of-packed-member -Wno-discarded-qualifiers
CFLAGS += -std=c11 -march=native
LINK_CFLAGS := -Ilib -lm
RELEASE_CFLAGS := -O3

ifndef COMPILER
COMPILER = gcc
endif

shared:
	$(COMPILER) -c $(SRC) -shared -o lib/libtermgl.so $(WARNINGS) $(CFLAGS) $(LINK_CFLAGS) $(RELEASE_CFLAGS) -fPIC
demo:
	$(COMPILER) $(SRC) $(DEMO) -o demo $(WARNINGS) $(CFLAGS) $(LINK_CFLAGS) $(RELEASE_CFLAGS) -DTERMGL3D -DTERMGLUTIL
