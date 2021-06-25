#include "console_game_engine.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct Pixel {
	char v_char;
	ubyte color;
} Pixel;

typedef struct TGE {
	unsigned width;
	unsigned height;
	unsigned frame_size;
	Pixel *frame_buffer;
	const Gradient *gradient;
} TGE;

typedef struct Gradient {
	unsigned length;
	unsigned divisor;
	const char *grad;
} Gradient;

#define SET_PIXEL(tge, x, y, v_char_, color_)\
	*(&tge->frame_buffer[y * tge->width + x]) = (Pixel) {\
		.v_char = v_char_,\
		.color = color_,\
	}

#define INTENSITY_TO_CHAR(tge, intensity)\
	tge->gradient->grad[(tge->gradient->length * intensity) / 255u]

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
	[TGE_BLACK] =      "\033[0;30m",
	[TGE_RED] =        "\033[0;31m",
	[TGE_GREEN] =      "\033[0;32m",
	[TGE_YELLOW] =     "\033[0;33m",
	[TGE_BLUE] =       "\033[0;34m",
	[TGE_PURPLE] =     "\033[0;35m",
	[TGE_CYAN] =       "\033[0;36m",
	[TGE_WHITE] =      "\033[0;37m",
};

const char *color_codes_bkg[] = {
	[TGE_BLACK_BKG >> 4] =  "\033[40m",
	[TGE_RED_BKG >> 4] =    "\033[41m",
	[TGE_GREEN_BKG  >> 4] =  "\033[42m",
	[TGE_YELLOW_BKG >> 4] = "\033[43m",
	[TGE_BLUE_BKG >> 4] =   "\033[44m",
	[TGE_PURPLE_BKG >> 4] = "\033[45m",
	[TGE_CYAN_BKG >> 4] =   "\033[46m",
	[TGE_WHITE_BKG >> 4] =  "\033[47m",
};

inline void i_tge_clear_frame_buffer(TGE *tge)
{
	unsigned i;
	for (i = 0; i < tge->frame_size; i++) {
		*(&tge->frame_buffer[i]) = (Pixel) {
			.v_char = ' ',
			.color = 0x00
		};
	}
}

TGE *tge_init(const unsigned width, const unsigned height, const Gradient *gradient)
{
	TGE *tge = malloc(sizeof(TGE));
	*tge = (TGE) {
		.width = width,
		.height = height,
		.frame_size = width * height,
		.frame_buffer = malloc(sizeof(Pixel) * width * height),
		.gradient = gradient,
	};
	i_tge_clear_frame_buffer(tge);
	return tge;
}

void tge_flush(TGE *tge)
{
	CLEAR_SCREEN;
	ubyte color = 0xFF;
	unsigned row, col;
	Pixel *pixel = tge->frame_buffer;

	for (row = 0; row < tge->height; row++) {
		for (col = 0; col < tge->width; col++) {
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
	i_tge_clear_frame_buffer(tge);
}

void tge_point(TGE *tge, int x, int y, ubyte i, ubyte color)
{
	SET_PIXEL(tge, x, y, INTENSITY_TO_CHAR(tge, i), color);
}

void i_tge_line_low(TGE *tge, const int x0, const int y0, const ubyte i0, const int x1, const int y1, const ubyte i1, const ubyte color)
{
	int dx = x1 - x0;
	int dy = y1 - y0;
	int yi;
	if (dy > 0) {
		yi = 1;
	} else {
		yi = -1;
		dy *= -1;
	}
	int d = (2 * dy) - dx;
	int y = y0;
	int x;
	for (x = x0; x <= x1; x++) {
		SET_PIXEL(tge, x, y, INTENSITY_TO_CHAR(tge, ((x - x0) * i1 + (x1 - x) * i0) / dx), color);
		if (d > 0) {
			y += yi;
			d += 2 * (dy - dx);
		} else {
			d += 2 * dy;
		}
	}
}

void i_tge_line_high(TGE *tge, const int x0, const int y0, const ubyte i0, const int x1, const int y1, const ubyte i1, const ubyte color)
{
	int dx = x1 - x0;
	int dy = y1 - y0;
	int xi;
	if (dx > 0) {
		xi = 1;
	} else {
		xi = -1;
		dx *= -1;
	}
	int d = (2 * dx) - dy;
	int x = x0;
	int y;
	for (y = y0; y < y1; y++) {
		SET_PIXEL(tge, x, y, INTENSITY_TO_CHAR(tge, ((y - y0) * i1 + (y1 - y) * i0) / dx), color);
		if (d > 0) {
			x += xi;
			d += 2 * (dx - dy);
		} else {
			d += 2 * dx;
		}
	}
}

void tge_line(TGE *tge, const int x0, const int y0, const ubyte i0, const int x1, const int y1, const ubyte i1, const ubyte color)
{
	if (abs(y1 - y0) < abs(x1 - x0)) {
		if (x0 > x1)
			i_tge_line_low(tge, x1, y1, i1, x0, y0, i0, color);
		else
			i_tge_line_low(tge, x0, y0, i0, x1, y1, i1, color);
	} else {
		if (y0 > y1)
			i_tge_line_high(tge, x1, y1, i1, x0, y0, i0, color);
		else
			i_tge_line_high(tge, x0, y0, i0, x1, y1, i1, color);
	}
}

void tge_delete(TGE *tge)
{
	free(tge->frame_buffer);
	free(tge);
}
