TARGET_DEMO = termgl_demo
TARGET_SO = libtermgl.so

SRC = termgl.c
SRC_DEMO = demo/termgl_demo.c
HEADER = termgl.h
CFLAGS += -std=c99 -O3 -Wall -Wextra -Wpedantic
LDFLAGS += -lm

$(TARGET_SO): $(SRC)
	$(CC) -c $^ -shared -o $@ $(CFLAGS) $(LDFLAGS) -fPIC -DTERMGL3D -DTERMGLUTIL

clean:
	rm -f $(TARGET_DEMO)
	rm -f $(TARGET_SO)

shared: $(TARGET_SO)

install: $(TARGET_SO) $(HEADER)
	cp $(TARGET_SO) /usr/lib/$(TARGET_SO)
	cp $(HEADER) /usr/include/$(HEADER)

uninstall:
	rm -f /usr/lib/$(TARGET_SO)
	rm -f /usr/include/$(HEADER)

demo: $(SRC_DEMO) $(SRC)
	$(CC) $^ -o $(TARGET_DEMO) $(CFLAGS) $(LDFLAGS) -DTERMGL3D -DTERMGLUTIL
