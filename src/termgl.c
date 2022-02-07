#include "termgl.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef TGL_OS_WINDOWS
#include <windows.h>
#endif

#ifdef TERMGL3D
typedef struct TGL3D TGL3D;
#endif

typedef struct Pixel {
	char v_char;
	TGLubyte color;
} Pixel;

struct TGL {
	unsigned width;
	unsigned height;
	int max_x;
	int max_y;
	unsigned frame_size;
	Pixel *frame_buffer;
	float *z_buffer;
	char *output_buffer;
	unsigned output_buffer_size;
	bool z_buffer_enabled;
	TGLubyte settings;
	const TGLGradient *gradient;
#ifdef TERMGL3D
	TGL3D *tgl3d;
#endif
};

#define SWAP(a_, b_) do {TGL_TYPEOF(a_) temp_ = a_; a_ = b_; b_ = temp_;} while(0)
#define MIN(a_, b_) (((a_)<(b_))?(a_):(b_))
#define MAX(a_, b_) (((a_)>(b_))?(a_):(b_))
#define XOR(a_, b_) (((bool)(a_))!=((bool)(b_)))

#define SET_PIXEL_RAW(tgl_, x_, y_, v_char_, color_)\
	do {\
		*(&tgl_->frame_buffer[y_ * tgl->width + x_]) = (Pixel) {\
			.v_char = v_char_,\
			.color = color_,\
		};\
	} while (0)

#define SET_PIXEL(tgl_, x_, y_, z_, v_char_, color_)\
	do {\
		if (!tgl_->z_buffer_enabled) {\
			SET_PIXEL_RAW(tgl_, x_, y_, v_char_, color_);\
		} else if (z_ >= tgl_->z_buffer[y_ * tgl_->width + x_]) {\
			SET_PIXEL_RAW(tgl_, x_, y_, v_char_, color_);\
			tgl->z_buffer[y_ * tgl->width + x_] = z_;\
		}\
	} while (0)

#define INTENSITY_TO_CHAR(tgl_, intensity_) tgl_->gradient->grad[tgl_->gradient->length * intensity_ / 256u]

const char grad_full_chars[] = " .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
const TGLGradient gradient_full = {
	.length = 70,
	.grad = grad_full_chars,
};

const char grad_min_chars[] = " .:-=+*#%@";
const TGLGradient gradient_min = {
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

	[TGL_BLACK | TGL_BOLD] =        "\033[1;30m",
	[TGL_RED | TGL_BOLD] =          "\033[1;31m",
	[TGL_GREEN | TGL_BOLD] =        "\033[1;32m",
	[TGL_YELLOW | TGL_BOLD] =       "\033[1;33m",
	[TGL_BLUE | TGL_BOLD] =         "\033[1;34m",
	[TGL_PURPLE | TGL_BOLD] =       "\033[1;35m",
	[TGL_CYAN | TGL_BOLD] =         "\033[1;36m",
	[TGL_WHITE | TGL_BOLD] =        "\033[1;37m",
};

const char *color_codes_bkg[] = {
	[TGL_BLACK_BKG >> 4] =  "\033[40m",
	[TGL_RED_BKG >> 4] =    "\033[41m",
	[TGL_GREEN_BKG  >> 4] = "\033[42m",
	[TGL_YELLOW_BKG >> 4] = "\033[43m",
	[TGL_BLUE_BKG >> 4] =   "\033[44m",
	[TGL_PURPLE_BKG >> 4] = "\033[45m",
	[TGL_CYAN_BKG >> 4] =   "\033[46m",
	[TGL_WHITE_BKG >> 4] =  "\033[47m",
};

void itgl_clip(TGL *tgl, int *x, int *y);
void itgl_horiz_line(TGL *tgl, int x0, float z0, TGLubyte i0, int x1, float z1, TGLubyte i1, int y, const TGLubyte color);

inline void itgl_clip(TGL *tgl, int *x, int *y)
{
	*x = MAX(MIN(tgl->max_x, *x), 0);
	*y = MAX(MIN(tgl->max_y, *y), 0);
}

void tgl_clear(TGL *tgl, const TGLubyte buffers)
{
	unsigned i;
	if (buffers & TGL_FRAME_BUFFER) {
		for (i = 0; i < tgl->frame_size; i++) {
			*(&tgl->frame_buffer[i]) = (Pixel) {
				.v_char = ' ',
				.color = 0x00
			};
		}
	}
	if (buffers & TGL_Z_BUFFER)
		for (i = 0; i < tgl->frame_size; i++)
			tgl->z_buffer[i] = -1.f;
	if (buffers & TGL_OUTPUT_BUFFER)
		memset(tgl->output_buffer, '\0', tgl->output_buffer_size);
}

TGL *tgl_init(const unsigned width, const unsigned height, const TGLGradient *gradient)
{
#ifdef TGL_OS_WINDOWS
	static bool win_init = false;
	if (!win_init) {
		win_init = true;

		HANDLE hOutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD mode;
		GetConsoleMode(hOutputHandle, &mode);
		mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		SetConsoleMode(hOutputHandle, mode);

		HANDLE hInputHandle = GetStdHandle(STD_INPUT_HANDLE);
		GetConsoleMode(hInputHandle, &mode);
		mode &= ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT | ENABLE_QUICK_EDIT_MODE);
		SetConsoleMode(hInputHandle, mode);
	}
#endif
	TGL *tgl = TGL_MALLOC(sizeof(TGL));
	*tgl = (TGL) {
		.width = width,
		.height = height,
		.max_x = width - 1,
		.max_y = height - 1,
		.frame_size = width * height,
		.frame_buffer = TGL_MALLOC(sizeof(Pixel) * width * height),
		.gradient = gradient,
	};
	tgl_clear(tgl, TGL_FRAME_BUFFER);
	return tgl;
}

void tgl_flush(TGL *tgl)
{
	TGL_CLEAR_SCREEN;
	TGLubyte color = 0xFF;
	unsigned row, col;
	Pixel *pixel = tgl->frame_buffer;
	bool double_chars = tgl->settings & TGL_DOUBLE_CHARS;

	if (tgl->output_buffer_size) {
		char *output_buffer_loc = tgl->output_buffer;
		for (row = 0; row < tgl->height; row++) {
			for (col = 0; col < tgl->width; col++) {
				if (color != pixel->color) {
					color = pixel->color;
					memcpy(output_buffer_loc, color_codes[color & 0x0F], 7);
					output_buffer_loc += 7;
					memcpy(output_buffer_loc, color_codes_bkg[color >> 4], 5);
					output_buffer_loc += 5;
				}
				*(output_buffer_loc++) = pixel->v_char;
				if (double_chars)
					*(output_buffer_loc++) = pixel->v_char;
				pixel++;
			}
			*(output_buffer_loc++) = '\n';
		}
		memcpy(output_buffer_loc, color_codes[TGL_WHITE], 7);
		output_buffer_loc += 7;
		memcpy(output_buffer_loc, color_codes_bkg[TGL_BLACK_BKG], 5);
		fputs(tgl->output_buffer, stdout);
	} else {
		for (row = 0; row < tgl->height; row++) {
			for (col = 0; col < tgl->width; col++) {
				if (color != pixel->color) {
					color = pixel->color;
					fputs(color_codes[color & 0x0F], stdout);
					fputs(color_codes_bkg[color >> 4], stdout);
				}
				putchar(pixel->v_char);
				if (double_chars)
					putchar(pixel->v_char);
				pixel++;
			}
			putchar('\n');
		}
		fputs(color_codes[TGL_WHITE], stdout);
		fputs(color_codes_bkg[TGL_BLACK_BKG], stdout);
	}

	fflush(stdout);
}

void tgl_putchar(TGL *tgl, int x, int y, char c, TGLubyte color)
{
	itgl_clip(tgl, &x, &y);
	SET_PIXEL_RAW(tgl, x, y, c, color);
}

void tgl_puts(TGL *tgl, int x, int y, char *str, TGLubyte color)
{
	itgl_clip(tgl, &x, &y);
	char *c_ptr = str;
	while (*c_ptr) {
		SET_PIXEL_RAW(tgl, x, y, *c_ptr, color);
		x++;
		itgl_clip(tgl, &x, &y);
		c_ptr++;
	}
}

void tgl_point(TGL *tgl, int x, int y, float z, TGLubyte i, TGLubyte color)
{
	itgl_clip(tgl, &x, &y);
	SET_PIXEL(tgl, x, y, z, INTENSITY_TO_CHAR(tgl, i), color);
}

//Bresenham's line algorithm
void tgl_line(TGL *tgl, int x0, int y0, float z0, TGLubyte i0, int x1, int y1, float z1, TGLubyte i1, const TGLubyte color)
{
	itgl_clip(tgl, &x0, &y0);
	itgl_clip(tgl, &x1, &y1);
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
			SET_PIXEL(tgl, x, y,
				((x - x0) * z1 + (x1 - x) * z0) / dx,
				INTENSITY_TO_CHAR(tgl, ((x - x0) * i1 + (x1 - x) * i0) / dx), color);
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
			SET_PIXEL(tgl, x, y,
				((y - y0) * z1 + (y1 - y) * z0) / dx,
				INTENSITY_TO_CHAR(tgl, ((y - y0) * i1 + (y1 - y) * i0) / dy), color);
			if (d > 0) {
				x += xi;
				d += 2 * (dx - dy);
			} else {
				d += dx + dx;
			}
		}
	}
}

void tgl_triangle(TGL *tgl, int x0, int y0, float z0, TGLubyte i0, int x1, int y1, float z1, TGLubyte i1, int x2, int y2, float z2, int i2, const TGLubyte color)
{
	tgl_line(tgl, x0, y0, z0, i0, x1, y1, z1, i1, color);
	tgl_line(tgl, x1, y1, z1, i1, x2, y2, z2, i2, color);
	tgl_line(tgl, x2, y2, z2, i2, x0, y0, z0, i0, color);
}

void itgl_horiz_line(TGL *tgl, int x0, float z0, TGLubyte i0, int x1, float z1, TGLubyte i1, int y, const TGLubyte color)
{
	if (x0 == x1) {
		SET_PIXEL(tgl, x0, y, z0, INTENSITY_TO_CHAR(tgl, i0), color);
	} else {
		int dx = x1 - x0;
		int x;
		for (x = x0; x <= x1; x++)
			SET_PIXEL(tgl, x, y,
				((x - x0) * z1 + (x1 - x) * z0) / dx,
				INTENSITY_TO_CHAR(tgl, ((x - x0) * i1 + (x1 - x) * i0) / dx), color);
	}

}

//Solution based on Bresenham's line algorithm
//adapted from: https://github.com/OneLoneCoder/videos/blob/master/olcConsoleGameEngine.h
void tgl_triangle_fill(TGL *tgl, int x0, int y0, float z0, TGLubyte i0, int x1, int y1, float z1, TGLubyte i1, int x2, int y2, float z2, int i2, const TGLubyte color)
{
	itgl_clip(tgl, &x0, &y0);
	itgl_clip(tgl, &x1, &y1);
	itgl_clip(tgl, &x2, &y2);
	if (y1 < y0) {
		SWAP(x1, x0);
		SWAP(y1, y0);
		SWAP(z1, z0);
		SWAP(i1, i0);
	}
	if (y2 < y0) {
		SWAP(x2, x0);
		SWAP(y2, y0);
		SWAP(z2, z0);
		SWAP(i2, i0);
	}
	if (y2 < y1) {
		SWAP(x1, x2);
		SWAP(y1, y2);
		SWAP(z1, z2);
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
	int e0;
	if (y0 == y1)
		goto LBL_NEXT;
	e0 = dx0 >> 1;

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
		float vz0 = ((y - y0) * z1 + (y1 - y) * z0) / (y1 - y0);
		float vz1 = ((y - y0) * z1 + (y1 - y) * z0) / (y1 - y0);

		if (t0x < t1x)
			itgl_horiz_line(tgl, minx, vz0, vi0, maxx, vz1, vi1, y, color);
		else
			itgl_horiz_line(tgl, minx, vz1, vi1, maxx, vz0, vi0, y, color);


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
			float vz0 = ((y - y0) * z2 + (y2 - y) * z0) / (y2 - y0);
			float vz1 = ((y - y1) * z2 + (y2 - y) * z1) / (y2 - y1);

			if (t1x < t0x)
				itgl_horiz_line(tgl, minx, vz0, vi0, maxx, vz1, vi1, y, color);
			else
				itgl_horiz_line(tgl, minx, vz1, vi1, maxx, vz0, vi0, y, color);
		} else {
			itgl_horiz_line(tgl, minx, z1, i1, maxx, z2, i2, y, color);
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

void tgl_enable(TGL *tgl, TGLubyte settings)
{
	tgl->settings |= settings;
	if (settings & TGL_Z_BUFFER) {
		tgl->z_buffer_enabled = true;
		tgl->z_buffer = TGL_MALLOC(sizeof(float) * tgl->frame_size);
		tgl_clear(tgl, TGL_Z_BUFFER);
	}
	if (settings & TGL_OUTPUT_BUFFER) {
		/* Chars for foreground color: 7
		 * Chars for background color: 6
		 * Maximum 14 chars per pixel: foreground + background + char
		 * 1 Newline character per line
		 * 13 Additional characters for resetting colors after flush
		 */
		tgl->output_buffer_size = 14 * tgl->frame_size + tgl->height + 13;
		tgl->output_buffer = TGL_MALLOC(tgl->output_buffer_size);
		tgl_clear(tgl, TGL_OUTPUT_BUFFER);
	}
}

void tgl_disable(TGL *tgl, TGLubyte settings)
{
	tgl->settings &= ~settings;
	if (settings & TGL_Z_BUFFER) {
		tgl->z_buffer_enabled = false;
		TGL_FREE(tgl->z_buffer);
		tgl->z_buffer = NULL;
	}
	if (settings & TGL_OUTPUT_BUFFER) {
		tgl->output_buffer_size = 0;
		TGL_FREE(tgl->output_buffer);
		tgl->output_buffer = NULL;
	}
}

void tgl_delete(TGL *tgl)
{
	TGL_FREE(tgl->frame_buffer);
	TGL_FREE(tgl->z_buffer);
	TGL_FREE(tgl->output_buffer);
#ifdef TERMGL3D
	TGL_FREE(tgl->tgl3d);
#endif
	TGL_FREE(tgl);
}

#ifdef TERMGL3D

#include <math.h>

struct TGL3D {
	TGLubyte settings;
	float aspect_ratio;
	float half_width;
	float half_height;

	TGLTransform transform;
	TGLMat projection;
};

#define TGL_CULL_BIT 0x01

#define TGL_CULL_FACE_BIT 0x01
#define TGL_WINDING_BIT 0x02

#define MAP_COORD(half, val) ((val * half) + half)

enum /*clip planes*/ {
	CLIP_NEAR = 0,
	CLIP_FAR,
	CLIP_LEFT,
	CLIP_RIGHT,
	CLIP_TOP,
	CLIP_BOTTOM,
};

const TGLVec3 clip_plane_normals[6] = {
	[CLIP_NEAR] = {0.f, 0.f, -1.f},
	[CLIP_FAR] = {0.f, 0.f, 1.f},
	[CLIP_LEFT] = {1.f, 0.f, 0.f},
	[CLIP_RIGHT] = {-1.f, 0.f, 0.f},
	[CLIP_BOTTOM] = {0.f, 1.f, 0.f},
	[CLIP_TOP] = {0.f, -1.f, 0.f},
};

void itgl_mulmatvec(TGLMat mat, const TGLVec3 vec, TGLVec3 res);
void itgl_mulmat(TGLMat mat1, TGLMat mat2, TGLMat res);
float itgl_distance_point_plane(const TGLVec3 normal, TGLVec3 point);
float itgl_line_intersect_plane(const TGLVec3 normal, TGLVec3 start, TGLVec3 end, TGLVec3 point);
unsigned itgl_clip_triangle_plane(const TGLVec3 normal, TGLTriangle *in, TGLTriangle out[2]);

__attribute__((const))
float tgl_sqr(const float val)
{
	return val * val;
}

__attribute__((const))
float tgl_mag3(const float vec[3])
{
	return sqrtf(tgl_sqr(vec[0]) + tgl_sqr(vec[1]) + tgl_sqr(vec[2]));
}

__attribute__((const))
float tgl_magsqr3(const float vec[3])
{
	return tgl_sqr(vec[0]) + tgl_sqr(vec[1]) + tgl_sqr(vec[2]);
}

__attribute__((const))
float tgl_dot3(const float vec1[3], const float vec2[3])
{
	return vec1[0] * vec2[0] + vec1[1] * vec2[1] + vec1[2] * vec2[2];
}

__attribute__((const))
float tgl_dot43(const float vec1[4], const float vec2[3])
{
	return vec1[0] * vec2[0] + vec1[1] * vec2[1] + vec1[2] * vec2[2] + vec1[3];
}

void tgl_add3s(const float vec1[3], const float summand, float res[3])
{
	res[0] = vec1[0] + summand;
	res[1] = vec1[1] + summand;
	res[2] = vec1[2] + summand;
}

void tgl_sub3s(const float vec1[3], const float subtrahend, float res[3])
{
	res[0] = vec1[0] - subtrahend;
	res[1] = vec1[1] - subtrahend;
	res[2] = vec1[2] - subtrahend;
}

void tgl_mul3s(const float vec[3], const float mul, float res[3])
{
	res[0] = vec[0] * mul;
	res[1] = vec[1] * mul;
	res[2] = vec[2] * mul;
}

void tgl_add3v(const float vec1[3], const float vec2[3], float res[3])
{
	res[0] = vec1[0] + vec2[0];
	res[1] = vec1[1] + vec2[1];
	res[2] = vec1[2] + vec2[2];
}

void tgl_sub3v(const float vec1[3], const float vec2[3], float res[3])
{
	res[0] = vec1[0] - vec2[0];
	res[1] = vec1[1] - vec2[1];
	res[2] = vec1[2] - vec2[2];
}

void tgl_mul3v(const float vec1[3], const float vec2[3], float res[3])
{
	res[0] = vec1[0] * vec2[0];
	res[1] = vec1[1] * vec2[1];
	res[2] = vec1[2] * vec2[2];
}

void tgl_inv3(const float vec[3], float res[3])
{
	res[0] = 1.f / vec[0];
	res[1] = 1.f / vec[1];
	res[2] = 1.f / vec[2];
}

void tgl_cross(const float vec1[3], const float vec2[3], float res[3])
{
	res[0] = vec1[1] * vec2[2] - vec1[2] * vec2[1];
	res[1] = vec1[2] * vec2[0] - vec1[0] * vec2[2];
	res[2] = vec1[0] * vec2[1] - vec1[1] * vec2[0];
}

void tgl_norm3(float vec[3])
{
	tgl_mul3s(vec, 1.f / tgl_mag3(vec), vec);
}

void itgl_mulmatvec(TGLMat mat, const TGLVec3 vec, TGLVec3 res)
{
	res[0] = tgl_dot43(mat[0], vec);
	res[1] = tgl_dot43(mat[1], vec);
	res[2] = tgl_dot43(mat[2], vec);

	float w = tgl_dot43(mat[3], vec);
	if (w != 0.f) {
		w = 1.f / w;
		res[0] *= w;
		res[1] *= w;
		res[2] *= w;
	}
}

void itgl_mulmat(TGLMat mat1, TGLMat mat2, TGLMat res)
{
	unsigned c, d, k;
#pragma GCC unroll 4
	for (c = 0; c < 4u; c++)
#pragma GCC unroll 4
		for (d = 0; d < 4u; d++) {
			res[c][d] = 0.f;
#pragma GCC unroll 4
			for (k = 0; k < 4u; k++)
				res[c][d] += mat1[c][k] * mat2[k][d];
		}

}

float itgl_distance_point_plane(const TGLVec3 normal, TGLVec3 point)
{
	return tgl_dot3(normal, point) + 1.f;
}

void tgl3d_init(TGL *tgl)
{
	TGL3D *tgl3d = TGL_MALLOC(sizeof(TGL3D));

	tgl3d->aspect_ratio = tgl->height / (float)tgl->width;
	tgl3d->half_width = tgl->width / 2.f;
	tgl3d->half_height = tgl->height / 2.f;

	tgl->tgl3d = tgl3d;
}

void tgl3d_camera(TGL *tgl, float fov, float near_val, float far_val)
{
	TGL3D *tgl3d = tgl->tgl3d;
	float s = 1.f / tanf(fov * .5f);
	float a = 1.f / (far_val - near_val);
	TGLMat projection = {
		{s * tgl3d->aspect_ratio, 0.f, 0.f, 0.f},
		{0.f, s, 0.f, 0.f},
		{0.f, 0.f, -(far_val + near_val) * a, 2.f * far_val * near_val * a},
		{0.f, 0.f, 1.f, 0.f},
	};
	memcpy(tgl->tgl3d->projection, projection, sizeof(TGLMat));
}

void tgl3d_transform_rotate(TGLTransform *transform, float x, float y, float z)
{
	TGLMat rotate = TGL_ROTATION_MATRIX(x, y, z);
	memcpy(transform->rotate, rotate, sizeof(TGLMat));
}

void tgl3d_transform_scale(TGLTransform *transform, float x, float y, float z)
{
	TGLMat scale = TGL_SCALE_MATRIX(x, y, z);
	memcpy(transform->scale, scale, sizeof(TGLMat));
}

void tgl3d_transform_translate(TGLTransform *transform, float x, float y, float z)
{
	TGLMat translate = TGL_TRANSLATE_MATRIX(x, y, z);
	memcpy(transform->translate, translate, sizeof(TGLMat));
}

void tgl3d_transform_update(TGLTransform *transform)
{
	TGLMat temp;
	itgl_mulmat(transform->translate, transform->scale, temp);
	itgl_mulmat(temp, transform->rotate, transform->result);
}

void tgl3d_transform_apply(TGLTransform *transform, TGLVec3 in[3], TGLVec3 out[3])
{
	itgl_mulmatvec(transform->result, in[0], out[0]);
	itgl_mulmatvec(transform->result, in[1], out[1]);
	itgl_mulmatvec(transform->result, in[2], out[2]);
}

float itgl_line_intersect_plane(const TGLVec3 normal, TGLVec3 start, TGLVec3 end, TGLVec3 point)
{
	TGLVec3 line_vec;
	tgl_sub3v(start, end, line_vec);
	float distance = -(tgl_dot3(normal, start) + 1.f) / (tgl_dot3(normal, line_vec));
	tgl_mul3s(line_vec, distance, point);
	tgl_add3v(point, start, point);
	return distance;
}

unsigned itgl_clip_triangle_plane(const TGLVec3 normal, TGLTriangle *in, TGLTriangle out[2])
{
	TGLVec3 di = {
		itgl_distance_point_plane(normal, in->vertices[0]),
		itgl_distance_point_plane(normal, in->vertices[1]),
		itgl_distance_point_plane(normal, in->vertices[2])
	};

	unsigned n_inside = 0, n_outside = 0;
	unsigned inside[3], outside[3];
#pragma GCC unroll 3
	for (unsigned i = 0; i < 3; i++) {
		if (di[i] >= 0.f)
			inside[n_inside++] = i;
		else
			outside[n_outside++] = i;
	}

	float d;

	switch (n_inside) {
	case 0:
		return 0;
	case 3:
		memcpy(&out[0], in, sizeof(TGLTriangle));
		return 1;
	case 1:
		memcpy(out[0].vertices[0], in->vertices[inside[0]], sizeof(TGLVec3));
		out[0].intensity[0] = in->intensity[inside[0]];
#pragma GCC unroll 2
		for (unsigned i = 0; i < 2; i++) {
			d = itgl_line_intersect_plane(normal, out[0].vertices[0], in->vertices[outside[i]], out[0].vertices[i + 1]);
			out[0].intensity[i + 1] = out[0].intensity[0] * (1.f - d) + in->intensity[outside[i]] * (d);
		}
		return 1;
	case 2: ;
		memcpy(out[0].vertices[0], in->vertices[inside[0]], sizeof(TGLVec3));
		memcpy(out[0].vertices[1], in->vertices[inside[1]], sizeof(TGLVec3));
		d = itgl_line_intersect_plane(normal, out[0].vertices[0], in->vertices[outside[0]], out[0].vertices[2]);
		out[0].intensity[0] = in->intensity[inside[0]];
		out[0].intensity[1] = in->intensity[inside[1]];
		out[0].intensity[2] = out[0].intensity[0] * (1.f - d) + in->intensity[outside[0]] * (d);

		memcpy(out[1].vertices[0], in->vertices[inside[1]], sizeof(TGLVec3));
		memcpy(out[1].vertices[1], out->vertices[2], sizeof(TGLVec3));
		d = itgl_line_intersect_plane(normal, out[1].vertices[0], in->vertices[outside[0]], out[1].vertices[2]);
		out[1].intensity[0] = in->intensity[inside[1]];
		out[1].intensity[1] = out[0].intensity[2];
		out[1].intensity[2] = out[1].intensity[0] * (1.f - d) + in->intensity[outside[0]] * (d);

		return 2;
	}
	return 0;
}

void tgl3d_shader(TGL *tgl, TGLTriangle *in, TGLubyte color, bool fill, void *data, void (*intermediate_shader)(TGLTriangle*, void*))
{
	TGL3D *tgl3d = tgl->tgl3d;

	// VERTEX SHADER
	TGLTriangle t, out;

	itgl_mulmatvec(tgl3d->transform.result, in->vertices[0], t.vertices[0]);
	itgl_mulmatvec(tgl3d->transform.result, in->vertices[1], t.vertices[1]);
	itgl_mulmatvec(tgl3d->transform.result, in->vertices[2], t.vertices[2]);

	//TODO: clipping with the near plane should be done before projection, but this solution works well enough for now
	if (t.vertices[0][2] < 1e-6f
		|| t.vertices[1][2] < 1e-6f
		|| t.vertices[2][2] < 1e-6f)
		return;

	itgl_mulmatvec(tgl3d->projection, t.vertices[0], out.vertices[0]);
	itgl_mulmatvec(tgl3d->projection, t.vertices[1], out.vertices[1]);
	itgl_mulmatvec(tgl3d->projection, t.vertices[2], out.vertices[2]);

	memcpy(out.intensity, in->intensity, sizeof(TGLubyte) * 3);

	if (tgl->settings & TGL_CULL_FACE) {
		TGLVec3 ab, ac, cp;
		tgl_sub3v(out.vertices[1], out.vertices[0], ab);
		tgl_sub3v(out.vertices[2], out.vertices[0], ac);
		tgl_cross(ab, ac, cp);
		if (XOR(tgl3d->settings & TGL_CULL_BIT, signbit(cp[2])))
			return;
	}

	TGLTriangle trig_buffer[127]; //the size of this buffer assumes a pathological case which is probably impossible
	memcpy(&trig_buffer[0], &out, sizeof(TGLTriangle));
	unsigned buffer_offset = 0;
	unsigned n_cur_stage = 1;
	unsigned p, i;
	for (p = 0; p < 6; p++) {
		unsigned n_next_stage = 0;
		for (i = 0; i < n_cur_stage; i++)
			n_next_stage += itgl_clip_triangle_plane(clip_plane_normals[p], &trig_buffer[i + buffer_offset], &trig_buffer[buffer_offset + n_cur_stage + n_next_stage]);
		buffer_offset += n_cur_stage;
		n_cur_stage = n_next_stage;
	}

	for (i = 0; i < n_cur_stage; i++) {
		TGLTriangle *trig = &trig_buffer[i + buffer_offset];

		//INTERMEDIATE SHADER
		if (intermediate_shader)
			intermediate_shader(trig, data);

		//FRAGMENT SHADER
		if (fill)
			tgl_triangle_fill(tgl,
				MAP_COORD(tgl3d->half_width, trig->vertices[0][0]),
				MAP_COORD(tgl3d->half_height, trig->vertices[0][1]),
				trig->vertices[0][2],
				trig->intensity[0],
				MAP_COORD(tgl3d->half_width, trig->vertices[1][0]),
				MAP_COORD(tgl3d->half_height, trig->vertices[1][1]),
				trig->vertices[1][2],
				trig->intensity[1],
				MAP_COORD(tgl3d->half_width, trig->vertices[2][0]),
				MAP_COORD(tgl3d->half_height, trig->vertices[2][1]),
				trig->vertices[2][2],
				trig->intensity[2],
				color);
		else
			tgl_triangle(tgl,
				MAP_COORD(tgl3d->half_width, trig->vertices[0][0]),
				MAP_COORD(tgl3d->half_height, trig->vertices[0][1]),
				trig->vertices[0][2],
				trig->intensity[0],
				MAP_COORD(tgl3d->half_width, trig->vertices[1][0]),
				MAP_COORD(tgl3d->half_height, trig->vertices[1][1]),
				trig->vertices[1][2],
				trig->intensity[1],
				MAP_COORD(tgl3d->half_width, trig->vertices[2][0]),
				MAP_COORD(tgl3d->half_height, trig->vertices[2][1]),
				trig->vertices[2][2],
				trig->intensity[2],
				color);
	}
}

void tgl3d_cull_face(TGL *tgl, TGLubyte settings)
{
	tgl->tgl3d->settings = (tgl->tgl3d->settings & ~TGL_CULL_BIT) | (XOR(settings & TGL_CULL_FACE_BIT, settings & TGL_WINDING_BIT) ? TGL_CULL_BIT: 0);
}

TGLTransform *tgl3d_get_transform(TGL *tgl)
{
	return &tgl->tgl3d->transform;
}

#endif /* TERMGL3D */

#ifdef TERMGLUTIL

#ifdef __unix__
#include <termios.h>
#include <sys/ioctl.h>
#endif

TGL_SSIZE_T tglutil_read(char *buf, size_t count)
{
#ifdef __unix__
	struct termios oldt, newt;

	// Disable canonical mode
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON);
	newt.c_cc[VMIN] = 0;
	newt.c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);

	TGL_SSIZE_T retval = read(2, buf, count);

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

	// Flush input buffer to prevent read of previous unread input
	tcflush(STDIN_FILENO, TCIFLUSH);
#else /* defined(TGL_OS_WINDOWS) */
	HANDLE hInputHandle = GetStdHandle(STD_INPUT_HANDLE);

	// Disable canonical mode
	DWORD old_mode, new_mode;
	GetConsoleMode(hInputHandle, &old_mode);
	new_mode = old_mode;
	new_mode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
	SetConsoleMode(hInputHandle, new_mode);

	DWORD event_cnt;
	GetNumberOfConsoleInputEvents(hInputHandle, &event_cnt);

	// ReadConsole is blocking so must manually process events
	size_t retval = 0;
	if (event_cnt) {
		INPUT_RECORD input_records[32];
		ReadConsoleInput(hInputHandle, input_records, 32, &event_cnt);

		DWORD i;
		for (i = 0; i < event_cnt; i++) {
			if (input_records[i].Event.KeyEvent.bKeyDown && input_records[i].EventType == KEY_EVENT) {
				buf[retval++] = input_records[i].Event.KeyEvent.uChar.AsciiChar;
				if (retval == count)
					break;
			}
		}
	}

	SetConsoleMode(hInputHandle, old_mode);
#endif
	return retval;
}

int tglutil_get_console_size(unsigned *col, unsigned *row, bool screen_buffer)
{
#ifdef __unix__
	(void)screen_buffer;
	struct winsize w;
	int retval = ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	*col = w.ws_col;
	*row = w.ws_row;
#else /* defined(TGL_OS_WINDOWS) */
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	int retval = GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi) ? 0 : -1;
	if (screen_buffer) {
		*col = csbi.dwSize.X;
		*row = csbi.dwSize.Y;
	} else {
		*col = csbi.srWindow.Right - csbi.srWindow.Left + 1;
		*row = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	}
#endif
	return retval;
}

int tglutil_set_console_size(unsigned col, unsigned row)
{
#ifdef __unix__
	struct winsize w = (struct winsize) {
		.ws_row = row,
		.ws_col = col,
	};
	int retval = ioctl(STDOUT_FILENO, TIOCSWINSZ, &w);
#else /* defined(TGL_OS_WINDOWS) */
	COORD size = (COORD) {
		.Y = row,
		.X = col,
	};
	int retval = SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), size) ? 0 : -1;
#endif
	return retval;
}


#endif /* TERMGLUTIL */
