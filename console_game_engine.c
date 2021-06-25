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
	unsigned frame_buffer_size;
	Pixel *frame_buffer;
	const Gradient *gradient;

	unsigned primitive;
	unsigned draw_mode;
	unsigned vertex_buffer[3][2];
	unsigned vertex_idx;
} TGE;

typedef struct Gradient {
	unsigned length;
	unsigned divisor;
	const char *grad;
} Gradient;

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
	[TGE_BLACK_BKG] =  "\033[40m",
	[TGE_RED_BKG] =    "\033[41m",
	[TGE_GREEN_BKG] =  "\033[42m",
	[TGE_YELLOW_BKG] = "\033[43m",
	[TGE_BLUE_BKG] =   "\033[44m",
	[TGE_PURPLE_BKG] = "\033[45m",
	[TGE_CYAN_BKG] =   "\033[46m",
	[TGE_WHITE_BKG] =  "\033[47m",
};

TGE *tge_init(const unsigned width, const unsigned height, const Gradient *gradient)
{
	TGE *tge = malloc(sizeof(TGE));
	unsigned frame_buffer_size = sizeof(Pixel) * width * height;
	*tge = (TGE) {
		.width = width,
		.height = height,
		.frame_buffer_size = frame_buffer_size,
		.frame_buffer = malloc(frame_buffer_size),
		.gradient = gradient,
	};
	return tge;
}

void tge_flush(TGE *tge)
{
	CLEAR_SCREEN;
	ubyte color = 255;
	unsigned row, col;
	Pixel *pixel = tge->frame_buffer;
	for (row = 0; row < tge->height; row++) {
		for (col = 0; col < tge->width; col++) {
			if (color != pixel->color) {
				color = pixel->color;
				fputs(color_codes[color], stdout);
			}
			putchar(pixel->v_char);
			pixel++;
		}
		putchar('\n');
	}
	memset(tge->frame_buffer, '\0', tge->frame_buffer_size);
}

void tge_begin(TGE *tge, unsigned primitive)
{
	tge->primitive = primitive;
}

void tge_end(TGE *tge)
{
	tge->primitive = 0;
	tge->vertex_idx = 0;
}

void tge_draw_mode(TGE *tge, unsigned draw_mode)
{
	tge->draw_mode = draw_mode;
}

void tge_vertex(TGE *tge, unsigned x, unsigned y, ubyte intensity, ubyte color)
{
	switch (tge->primitive) {
	case TGE_POINTS: {
		Pixel *pixel = &tge->frame_buffer[y * tge->width + x];
		*pixel = (Pixel) {
			.v_char = tge->gradient->grad[(tge->gradient->length * intensity) / 255u],
			.color = color,
		};
		}
		break;
	}
}

void tge_delete(TGE *tge)
{
	free(tge->frame_buffer);
	free(tge);
}
