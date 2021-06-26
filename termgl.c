#include "termgl.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Pixel {
	char v_char;
	ubyte color;
} Pixel;

typedef struct TGL {
	unsigned width;
	unsigned height;
	unsigned frame_size;
	Pixel *frame_buffer;
	const Gradient *gradient;
} TGL;

typedef struct Gradient {
	unsigned length;
	unsigned divisor;
	const char *grad;
} Gradient;

#define SET_PIXEL(tgl, x, y, v_char_, color_)\
	do {\
		*(&tgl->frame_buffer[y * tgl->width + x]) = (Pixel) {\
			.v_char = v_char_,\
			.color = color_,\
		};\
	} while (0)

#define INTENSITY_TO_CHAR(tgl, intensity) tgl->gradient->grad[(tgl->gradient->length * intensity) / 256u]

#define SWAP(a, b) do {TYPEOF(a) temp = a; a = b; b = temp;} while(0)

const char grad_full_chars[] = " .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
const Gradient gradient_full = {
	.length = 70,
	.grad = grad_full_chars,
};

const char grad_min_chars[] = " .:-=+*#%@";
const Gradient gradient_min = {
	.length = 10,
	.grad = grad_min_chars,
};

const char *color_codes[] = {
	[TGL_BLACK] =      "\033[0;30m",
	[TGL_RED] =        "\033[0;31m",
	[TGL_GREEN] =      "\033[0;32m",
	[TGL_YELLOW] =     "\033[0;33m",
	[TGL_BLUE] =       "\033[0;34m",
	[TGL_PURPLE] =     "\033[0;35m",
	[TGL_CYAN] =       "\033[0;36m",
	[TGL_WHITE] =      "\033[0;37m",
};

const char *color_codes_bkg[] = {
	[TGL_BLACK_BKG >> 4] =  "\033[40m",
	[TGL_RED_BKG >> 4] =    "\033[41m",
	[TGL_GREEN_BKG  >> 4] =  "\033[42m",
	[TGL_YELLOW_BKG >> 4] = "\033[43m",
	[TGL_BLUE_BKG >> 4] =   "\033[44m",
	[TGL_PURPLE_BKG >> 4] = "\033[45m",
	[TGL_CYAN_BKG >> 4] =   "\033[46m",
	[TGL_WHITE_BKG >> 4] =  "\033[47m",
};

inline void i_tgl_clear_frame_buffer(TGL *tgl)
{
	unsigned i;
	for (i = 0; i < tgl->frame_size; i++) {
		*(&tgl->frame_buffer[i]) = (Pixel) {
			.v_char = ' ',
			.color = 0x00
		};
	}
}

TGL *tgl_init(const unsigned width, const unsigned height, const Gradient *gradient)
{
	TGL *tgl = malloc(sizeof(TGL));
	*tgl = (TGL) {
		.width = width,
		.height = height,
		.frame_size = width * height,
		.frame_buffer = malloc(sizeof(Pixel) * width * height),
		.gradient = gradient,
	};
	i_tgl_clear_frame_buffer(tgl);
	return tgl;
}

void tgl_flush(TGL *tgl)
{
	CLEAR_SCREEN;
	ubyte color = 0xFF;
	unsigned row, col;
	Pixel *pixel = tgl->frame_buffer;

	for (row = 0; row < tgl->height; row++) {
		for (col = 0; col < tgl->width; col++) {
			if (color != pixel->color) {
				color = pixel->color;
				fputs(color_codes[color & 0x0F], stdout);
				fputs(color_codes_bkg[color >> 4], stdout);
			}
			putchar(pixel->v_char);
			pixel++;
		}
		putchar('\n');
	}
	fflush(stdout);
	i_tgl_clear_frame_buffer(tgl);
}

void tgl_point(TGL *tgl, int x, int y, ubyte i, ubyte color)
{
	SET_PIXEL(tgl, x, y, INTENSITY_TO_CHAR(tgl, i), color);
}

//Bresenham's line algorithm
void tgl_line(TGL *tgl, int x0, int y0, ubyte i0, int x1, int y1, ubyte i1, const ubyte color)
{
	if (abs(y1 - y0) < abs(x1 - x0)) {
		if (x0 > x1) {
			SWAP(x1, x0);
			SWAP(y1, y0);
			SWAP(i1, i0);
		}
		int dx = x1 - x0;
		int dy = y1 - y0;
		int yi;
		if (dy > 0) {
			yi = 1;
		} else {
			yi = -1;
			dy *= -1;
		}
		int d = (dy + dy) - dx;
		int y = y0;
		int x;
		for (x = x0; x <= x1; x++) {
			SET_PIXEL(tgl, x, y, INTENSITY_TO_CHAR(tgl, ((x - x0) * i1 + (x1 - x) * i0) / dx), color);
			if (d > 0) {
				y += yi;
				d += 2 * (dy - dx);
			} else {
				d += dy + dy;
			}
		}
	} else {
		if (y0 > y1) {
			SWAP(x1, x0);
			SWAP(y1, y0);
			SWAP(i1, i0);
		}
		int dx = x1 - x0;
		int dy = y1 - y0;
		int xi;
		if (dx > 0) {
			xi = 1;
		} else {
			xi = -1;
			dx *= -1;
		}
		int d = (dx + dx) - dy;
		int x = x0;
		int y;
		for (y = y0; y < y1; y++) {
			SET_PIXEL(tgl, x, y, INTENSITY_TO_CHAR(tgl, ((y - y0) * i1 + (y1 - y) * i0) / dy), color);
			if (d > 0) {
				x += xi;
				d += 2 * (dx - dy);
			} else {
				d += dx + dx;
			}
		}
	}
}

void tgl_triangle(TGL *tgl, int x0, int y0, ubyte i0, int x1, int y1, ubyte i1, int x2, int y2, int i2, const ubyte color)
{
	tgl_line(tgl, x0, y0, i0, x1, y1, i1, color);
	tgl_line(tgl, x1, y1, i1, x2, y2, i2, color);
	tgl_line(tgl, x2, y2, i2, x0, y0, i0, color);
}

void itgl_horiz_line(TGL *tgl, int x0, ubyte i0, int x1, ubyte i1, int y, const ubyte color)
{
	if (x0 == x1) {
		SET_PIXEL(tgl, x0, y, INTENSITY_TO_CHAR(tgl, i0), color);
	} else {
		int dx = x1 - x0;
		int x;
		for (x = x0; x <= x1; x++)
			SET_PIXEL(tgl, x, y, INTENSITY_TO_CHAR(tgl, ((x - x0) * i1 + (x1 - x) * i0) / dx), color);
	}

}

//Solution based on Bresenham's line algorithm
//adapted from: https://github.com/OneLoneCoder/videos/blob/master/olcConsoleGameEngine.h
void tgl_triangle_fill(TGL *tgl, int x0, int y0, ubyte i0, int x1, int y1, ubyte i1, int x2, int y2, int i2, const ubyte color)
{
	if (y1 < y0) {
		SWAP(x1, x0);
		SWAP(y1, y0);
		SWAP(i1, i0);
	}
	if (y2 < y0) {
		SWAP(x2, x0);
		SWAP(y2, y0);
		SWAP(i2, i0);
	}
	if (y2 < y1) {
		SWAP(x1, x2);
		SWAP(y1, y2);
		SWAP(i1, i2);
	}

	int t0xp, t1xp, minx, maxx, t0x, t1x;
	t0x = t1x = x0;
	int y = y0;
	int dx0 = x1 - x0;
	int signx0, signx1;
	bool changed0, changed1;
	changed0 = changed1 = false;
	if (dx0 < 0) {
		dx0 *= -1;
		signx0 = -1;
	} else {
		signx0 = 1;
	}
	int dy0 = y1 - y0;
	int dx1 = x2 - x0;
	if (dx1 < 0) {
		dx1 = -dx1;
		signx1 = -1;
	} else {
		signx1 = 1;
	}
	int dy1 = y2 - y0;

	if (dy0 > dx0) {
		SWAP(dx0, dy0);
		changed0 = true;
	}
	if (dy1 > dx1) {
		SWAP(dx1, dy1);
		changed1 = true;
	}
	int e1 = dx1 >> 1;
	if (y0 == y1)
		goto LBL_NEXT;
	int e0 = dx0 >> 1;

	for (int i = 0; i < dx0;) {
		t0xp = t1xp = 0;
		if (t0x < t1x) {
			minx = t0x;
			maxx = t1x;
		} else {
			minx = t1x;
			maxx = t0x;
		}
		while (i < dx0) {
			i++;
			e0 += dy0;
			while (e0 >= dx0) {
				e0 -= dx0;
				if (changed0)
					t0xp = signx0;
				else
					goto LBL_NEXT1;
			}
			if (changed0)
				break;
			else
				t0x += signx0;
		}
		// Move line
LBL_NEXT1:
		// process second line until y value is about to change
		while (true) {
			e1 += dy1;
			while (e1 >= dx1) {
				e1 -= dx1;
				if (changed1)
					t1xp = signx1;
				else
					goto LBL_NEXT2;
			}
			if (changed1)
				break;
			else
				t1x += signx1;
		}
LBL_NEXT2:
		if (minx > t0x)
			minx = t0x;
		if (minx > t1x)
			minx = t1x;
		if (maxx < t0x)
			maxx = t0x;
		if (maxx < t1x)
			maxx = t1x;

		int vi0 = ((y - y0) * i1 + (y1 - y) * i0) / (y1 - y0);
		int vi1 = ((y - y0) * i2 + (y2 - y) * i0) / (y2 - y0);
		if (t0x < t1x)
			itgl_horiz_line(tgl, minx, vi0, maxx, vi1, y, color);
		else
			itgl_horiz_line(tgl, minx, vi1, maxx, vi0, y, color);


		if (!changed0)
			t0x += signx0;
		t0x += t0xp;
		if (!changed1)
			t1x += signx1;
		t1x += t1xp;
		y += 1;
		if (y == y1)
			break;
	}
LBL_NEXT:
	// Second half
	dx0 = x2 - x1;
	if (dx0 < 0) {
		dx0 *= -1;
		signx0 = -1;
	} else {
		signx0 = 1;
	}
	dy0 = y2 - y1;
	t0x = x1;
	if (dy0 > dx0) {
		SWAP(dy0, dx0);
		changed0 = true;
	} else {
		changed0 = false;
	}
	e0 = dx0 >> 1;

	for (int i = 0; i <= dx0; i++) {
		t0xp = t1xp = 0;
		if (t0x < t1x) {
			minx = t0x;
			maxx = t1x;
		} else {
			minx = t1x;
			maxx = t0x;
		}
		while (i < dx0) {
			e0 += dy0;
			while (e0 >= dx0) {
				e0 -= dx0;
				if (changed0) {
					t0xp = signx0;
					break;
				} else {
					goto LBL_NEXT3;
				}
			}
			if (changed0)
				break;
			else
				t0x += signx0;
			if (i < dx0)
				i++;
		}
LBL_NEXT3:
		while (t1x != x2) {
			e1 += dy1;
			while (e1 >= dx1) {
				e1 -= dx1;
				if (changed1)
					t1xp = signx1;
				else
					goto LBL_NEXT4;
			}
			if (changed1)
				break;
			else
				t1x += signx1;
		}
LBL_NEXT4:
		if (minx > t0x)
			minx = t0x;
		if (minx > t1x)
			minx = t1x;
		if (maxx < t0x)
			maxx = t0x;
		if (maxx < t1x)
			maxx = t1x;

		if (y1 != y2) {
			int vi0 = ((y - y0) * i2 + (y2 - y) * i0) / (y2 - y0);
			int vi1 = ((y - y1) * i2 + (y2 - y) * i1) / (y2 - y1);
			if (t1x < t0x)
				itgl_horiz_line(tgl, minx, vi0, maxx, vi1, y, color);
			else
				itgl_horiz_line(tgl, minx, vi1, maxx, vi0, y, color);
		} else {
			itgl_horiz_line(tgl, minx, i1, maxx, i2, y, color);
		}

		if (!changed0)
			t0x += signx0;
		t0x += t0xp;
		if (!changed1)
			t1x += signx1;
		t1x += t1xp;
		y += 1;
		if (y > y2)
			return;
	}

}

void tgl_delete(TGL *tgl)
{
	free(tgl->frame_buffer);
	free(tgl);
}
