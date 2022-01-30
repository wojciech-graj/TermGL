SRC = src/termgl.c
DEMO = demodir/termgl_demo.c
LIB_BIN = lib/libtermgl.so
DEMO_BIN = demo
WARNINGS := -Wall -Wextra -Wpedantic -Wdouble-promotion -Wstrict-prototypes -Wshadow -Wduplicated-cond -Wduplicated-branches -Wjump-misses-init -Wnull-dereference -Wrestrict -Wlogical-op -Wno-maybe-uninitialized -Walloc-zero -Wformat-security -Wformat-signedness -Winit-self -Wlogical-op -Wmissing-declarations -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wswitch-enum -Wundef -Wwrite-strings -Wno-address-of-packed-member -Wno-discarded-qualifiers
CFLAGS += -std=c99 -march=native
LINK_CFLAGS := -Ilib -lm
RELEASE_CFLAGS := -O3

ifndef COMPILER
COMPILER = gcc
endif

shared:
	$(COMPILER) -c $(SRC) -shared -o $(LIB_BIN) $(WARNINGS) $(CFLAGS) $(LINK_CFLAGS) $(RELEASE_CFLAGS) -fPIC
demo:
	$(COMPILER) $(SRC) $(DEMO) -o $(DEMO_BIN) $(WARNINGS) $(CFLAGS) $(LINK_CFLAGS) $(RELEASE_CFLAGS) -DTERMGL3D -DTERMGLUTIL
clean:
	rm -f $(DEMO_BIN)
	rm -f $(LIB_BIN)
