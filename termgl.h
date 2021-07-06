#ifndef TERMGL_H
#define TERMGL_H

#define CLEAR_SCREEN puts("\033[1;1H\033[2J")
#define TYPEOF __typeof__

typedef unsigned char ubyte;

typedef struct TGL TGL;
typedef struct Gradient Gradient;

extern const Gradient gradient_full;
extern const Gradient gradient_min;

enum colors {
	TGL_BLACK = 0x00,
	TGL_RED = 0x01,
	TGL_GREEN = 0x02,
	TGL_YELLOW = 0x03,
	TGL_BLUE = 0x04,
	TGL_PURPLE = 0x05,
	TGL_CYAN = 0x06,
	TGL_WHITE = 0x07,
	TGL_BLACK_BKG = 0x00,
	TGL_RED_BKG = 0x10,
	TGL_GREEN_BKG = 0x20,
	TGL_YELLOW_BKG = 0x30,
	TGL_BLUE_BKG = 0x40,
	TGL_PURPLE_BKG = 0x50,
	TGL_CYAN_BKG = 0x60,
	TGL_WHITE_BKG = 0x70,
};

#define TGL_FRAME_BUFFER 0x01
#define TGL_Z_BUFFER 0x40
#define TGL_DOUBLE_CHARS 0x80

TGL *tgl_init(const unsigned width, const unsigned height, const Gradient *gradient);
void tgl_delete(TGL *tgl);
void tgl_flush(TGL *tgl);
void tgl_point(TGL *tgl, int x, int y, float z, ubyte i, ubyte color);
void tgl_line(TGL *tgl, int x0, int y0, float z0, ubyte i0, int x1, int y1, float z1, ubyte i1, const ubyte color);
void tgl_triangle(TGL *tgl, int x0, int y0, float z0, ubyte i0, int x1, int y1, float z1, ubyte i1, int x2, int y2, float z2, int i2, const ubyte color);
void tgl_triangle_fill(TGL *tgl, int x0, int y0, float z0, ubyte i0, int x1, int y1, float z1, ubyte i1, int x2, int y2, float z2, int i2, const ubyte color);
void tgl_clear(TGL *tgl, ubyte buffers);
void tgl_enable(TGL *tgl, ubyte settings);
void tgl_disable(TGL *tgl, ubyte settings);

#endif