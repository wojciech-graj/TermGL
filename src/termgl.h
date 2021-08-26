#ifndef TERMGL_H
#define TERMGL_H

/**
 * Base TermGL library. Handles printing, 2D drawing, buffers.
 */

#define TGL_CLEAR_SCREEN puts("\033[1;1H\033[2J")
#define TGL_TYPEOF __typeof__
#define TGL_MALLOC malloc
#define TGL_FREE free

typedef unsigned char ubyte;

typedef struct TGL TGL;
typedef struct Gradient {
	unsigned length;
	const char *grad;
} Gradient;

extern const Gradient gradient_full;
extern const Gradient gradient_min;

enum /*colors*/ {
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
#define TGL_OUTPUT_BUFFER 0x20
#define TGL_DOUBLE_CHARS 0x80

/**
 * Initializes a TGL struct which must be passed to all functions as context
 * @param gradient: pointer to a gradient struct which holds characters which will be used when rendering. Gradients provided by default are gradient_min and gradient_full
 */
TGL *tgl_init(const unsigned width, const unsigned height, const Gradient *gradient);

/**
 * Frees a TGL context
 */
void tgl_delete(TGL *tgl);

/**
 * Prints frame buffer to terminal
 */
void tgl_flush(TGL *tgl);

/**
 * Clears frame buffers
 * @param buffers: bitwise combination of buffers:
 *   TGL_FRAME_BUFFER - frame buffer
 *   TGL_Z_BUFFER - depth buffer
 */
void tgl_clear(TGL *tgl, ubyte buffers);

/**
 * Enables or disables certain settings
 * @param settings: bitwise combination of settings:
 *   TGL_Z_BUFFER - depth buffer
 *   TGL_DOUBLE_CHARS - square pixels by printing 2 characters per pixel
 *   TGL_CULL_FACE - (3D ONLY) cull specified triangle faces
 *   TGL_OUTPUT_BUFFER - output buffer allowing for just one print to flush. Mush faster on most terminals, but requires a few hundred kilobytes of memory
 */
void tgl_enable(TGL *tgl, ubyte settings);
void tgl_disable(TGL *tgl, ubyte settings);

/**
 * Various drawing functions
 * @param i: intensity of pixel which will be mapped to character on gradient
 * @param color: bitwise combination of colors defined in above enum. Can use one foreground (TGL_COLOR) and one background (TGL_COLOR_BKG)
 */
void tgl_putchar(TGL *tgl, int x, int y, char c, ubyte color);
void tgl_puts(TGL *tgl, int x, int y, char *str, ubyte color);
void tgl_point(TGL *tgl, int x, int y, float z, ubyte i, ubyte color);
void tgl_line(TGL *tgl, int x0, int y0, float z0, ubyte i0, int x1, int y1, float z1, ubyte i1, ubyte color);
void tgl_triangle(TGL *tgl, int x0, int y0, float z0, ubyte i0, int x1, int y1, float z1, ubyte i1, int x2, int y2, float z2, int i2, ubyte color);
void tgl_triangle_fill(TGL *tgl, int x0, int y0, float z0, ubyte i0, int x1, int y1, float z1, ubyte i1, int x2, int y2, float z2, int i2, ubyte color);

#endif
