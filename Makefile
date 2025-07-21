DEMO = termgl_demo
SO = libtermgl.so
HEADER = termgl.h
DEMO_SRC = termgl.c termgl_demo.c
CFLAGS += -Wall
LDFLAGS += -lm

ifeq ($(OS),Windows_NT)
CC = cl
else
CFLAGS += -std=c99 -O3 -Wextra -Wpedantic
endif

lib%.so: %.pic.o
	$(CC) $^ -shared -o $@ $(LDFLAGS)

%.pic.o: %.c
	$(CC) -c $^ -o $@ $(CFLAGS) -fPIC

%.o: %.c
	$(CC) -c $^ -o $@ $(CFLAGS)

# MSVC
%.obj: %.c
	$(CC) -c $^ $(CFLAGS)

.PHONY: shared
shared: $(SO)

.PHONY: install
install: $(SO) $(HEADER)
	cp $(SO) /usr/local/lib/$(SO)
	cp $(HEADER) /usr/local/include/$(HEADER)

.PHONY: uninstall
uninstall:
	rm -f /usr/local/lib/$(SO) /usr/local/include/$(HEADER)

.PHONY: demo
demo: $(DEMO)

$(DEMO): $(DEMO_SRC)
	$(CC) $^ -o $@ $(CFLAGS) -DTERMGL3D -DTERMGLUTIL -D_POSIX_C_SOURCE=199309L $(LDFLAGS)

.PHONY: clean
clean:
	rm -f *.so *.o *.obj $(DEMO)
