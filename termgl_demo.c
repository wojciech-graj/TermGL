/*
 * Copyright (c) 2021-2025 Wojciech Graj
 *
 * Licensed under the MIT license: https://opensource.org/licenses/MIT
 * Permission is granted to use, copy, modify, and redistribute the work.
 * Full license information available in the project LICENSE file.
 **/

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "termgl.h"

// OS-specific imports used for sleep_ms
#ifdef TGL_OS_WINDOWS
#include <synchapi.h>
#else
#include <time.h>
#endif

typedef struct STLTriangle {
	float normal[3]; //normal is unreliable so it is not used.
	float vertices[3][3];
	uint16_t attribute_bytes; //attribute bytes is unreliable so it is not used.
} __attribute__((packed)) STLTriangle;

#define xstr(str_) str(str_)
#define str(str_) #str_

#define MIN(a_, b_) (((a_) < (b_)) ? (a_) : (b_))

static const char *const HELPTEXT_HEADER =
	"TermGL v" xstr(TGL_VERSION_MAJOR) "." xstr(TGL_VERSION_MINOR) " Demo Utility";
static const char *const HELPTEXT_BODY = "\
Select a Demo:\n\
1. Utah Teapot\n\
    Renders a rotating 3D Utah Teapot.\n\
2. Color Palette\n\
    Renders a palette of indexed text colors and styles.\n\
3. Mandelbrot\n\
    Renders an infinitely zooming-in Mandelbrot set.\n\
4. Realtime Keyboard\n\
    Displays keyboard input in realtime.\n\
5. Textured Cube\n\
    Renders a texture-mapped cube.\n\
6. RGB\n\
    Renders overlapping red, green, and blue circles.\n\
7. Mouse\n\
    Displays mouse position and button state.\
";

static const uint8_t colors[] = {
	TGL_BLACK,
	TGL_RED,
	TGL_GREEN,
	TGL_YELLOW,
	TGL_BLUE,
	TGL_PURPLE,
	TGL_CYAN,
	TGL_WHITE,
};

static void teapot_pixel_shader(uint8_t u, uint8_t v, TGLPixFmt *color, char *c, const void *data);
static uint32_t stl_load(FILE *file, TGLTriangle **triangles);
static uint8_t rgb_map_circle(int dx, int dy);
static void sleep_ms(unsigned long ms);

static void demo_mandelbrot(unsigned res_x, unsigned res_y, unsigned frametime_ms);
static void demo_teapot(unsigned res_x, unsigned res_y, unsigned frametime_ms);
static void demo_keyboard(unsigned res_x, unsigned res_y, unsigned frametime_ms);
static void demo_color(unsigned res_x, unsigned res_y, unsigned frametime_ms);
static void demo_texture(unsigned res_x, unsigned res_y, unsigned frametime_ms);
static void demo_rgb(unsigned res_x, unsigned res_y, unsigned frametime_ms);
static void demo_mouse(unsigned res_x, unsigned res_y, unsigned frametime_ms);

void teapot_pixel_shader(const uint8_t u, const uint8_t v, TGLPixFmt *const color, char *const c,
	const void *const data)
{
	const TGLVec3 light_direction = { 1.F, 0.F, 0.F };
	const TGLVec3 *const in = (const TGLVec3 *)data;
	TGLVec3 ab, ac, cp;
	tgl_sub3v(in[1], in[0], ab);
	tgl_sub3v(in[2], in[0], ac);
	tgl_cross(ab, ac, cp);
	const float dp = tgl_dot3(cp, light_direction);
	float light_mul;
	if (signbit(dp))
		light_mul = 0.15F;
	else
		light_mul =
			acosf(dp / (tgl_mag3(cp) * tgl_mag3(light_direction))) / -3.14159F + 1.F;
	*color = TGL_PIXFMT(TGL_IDX(TGL_WHITE, TGL_BOLD));
	*c = tgl_grad_char(&gradient_min, light_mul * 255);
	(void)u;
	(void)v;
}

uint32_t stl_load(FILE *const file, TGLTriangle **const triangles)
{
	//ensure that file is binary instead of ascii
	char header[5];
	assert(fread(header, sizeof(char), 5, file) == 5);
	assert(strncmp("solid", header, 5));

	assert(fseek(file, sizeof(uint8_t[80]), SEEK_SET) == 0);
	uint32_t num_triangles;
	assert(fread(&num_triangles, sizeof(uint32_t), 1, file) == 1);
	*triangles = malloc(sizeof(TGLTriangle[num_triangles]));

	uint32_t i;
	for (i = 0; i < num_triangles; i++) {
		STLTriangle stl_triangle;
		assert(fread(&stl_triangle, sizeof(STLTriangle), 1, file) == 1);
		memcpy((*triangles)[i], stl_triangle.vertices, sizeof(float[3][3]));
	}
	return num_triangles;
}

void sleep_ms(const unsigned long ms)
{
#ifdef TGL_OS_WINDOWS
	Sleep(ms);
#else
	const struct timespec ts = (struct timespec){
		.tv_sec = ms / 1000,
		.tv_nsec = (ms % 1000UL) * 1000000,
	};
	nanosleep(&ts, NULL);
#endif
}

void demo_mandelbrot(const unsigned res_x, const unsigned res_y, const unsigned frametime_ms)
{
	TGL *const tgl = tgl_init(res_x, res_y);
	assert(tgl);
	assert(!tgl_enable(tgl, TGL_OUTPUT_BUFFER | TGL_PROGRESSIVE));

	const unsigned frame_max = 90;
	const unsigned i_max = 255;

	const float init_mid_x = -1.F, init_mid_y = 0.F;
	const float end_mid_x = -1.31, end_mid_y = 0.F;
	const float dmid_x = (end_mid_x - init_mid_x) / frame_max,
		    dmid_y = (end_mid_y - init_mid_y) / frame_max;

	const float init_width = .5f, init_height = .5f;
	const float end_width = .12f, end_height = .12f;
	const float dwidth = (end_width - init_width) / frame_max,
		    dheight = (end_height - init_height) / frame_max;

	float mid_x = init_mid_x, mid_y = init_mid_y;
	float width = init_width, height = init_height;

	unsigned frame = 0;
	while (1) {
		float x, y = mid_y - .5f * height;
		float dx = width / res_x, dy = height / res_y;
		float init_x = mid_x - .5f * width;
		unsigned pix_x, pix_y, i;
		for (pix_y = 0; pix_y < res_y; pix_y++) {
			x = init_x;
			for (pix_x = 0; pix_x < res_x; pix_x++) {
				float ix = 0, iy = 0;
				float ix2 = 0, iy2 = 0;
				for (i = 0; (ix2 + iy2) <= 4.F && i < i_max; i++) {
					const float next_ix = ix2 - iy2 + x;
					iy = 2.F * ix * iy + y;
					ix = next_ix;
					ix2 = ix * ix;
					iy2 = iy * iy;
				}
				if (i < i_max) // Set pixel with intensity dependent on i
					tgl_putchar(tgl, pix_x, pix_y,
						tgl_grad_char(&gradient_full, i * 255U / i_max),
						TGL_PIXFMT(TGL_IDX(TGL_WHITE, TGL_BOLD)));
				x += dx;
			}
			y += dy;
		}

		assert(!tgl_flush(tgl));
		tgl_clear(tgl, TGL_FRAME_BUFFER | TGL_OUTPUT_BUFFER);

		if (frame++ < frame_max) {
			mid_x += dmid_x;
			mid_y += dmid_y;
			width += dwidth;
			height += dheight;
		} else {
			frame = 0;
			mid_x = init_mid_x;
			mid_y = init_mid_y;
			width = init_width;
			height = init_height;
		}

		sleep_ms(frametime_ms);
	}

	tgl_delete(tgl);
}

void demo_teapot(const unsigned res_x, const unsigned res_y, const unsigned frametime_ms)
{
	TGL *const tgl = tgl_init(res_x, res_y);
	assert(tgl);
	tgl_cull_face(tgl, TGL_BACK | TGL_CCW);
	assert(!tgl_enable(tgl,
		TGL_DOUBLE_CHARS | TGL_CULL_FACE | TGL_Z_BUFFER | TGL_OUTPUT_BUFFER
			| TGL_PROGRESSIVE));

	TGLMat camera;
	tgl_camera(camera, res_x, res_y, 1.57F, 0.1F, 5.F);

	// Load triangles
	TGLTriangle *trigs;
	FILE *const stl_file = fopen("demo/utah_teapot.stl", "rb");
	assert(stl_file);
	const uint32_t n_trigs = stl_load(stl_file, &trigs);
	assert(!fclose(stl_file));

	TGLMat temp;

	// Create transformation matrices for camera
	TGLMat camera_scale, camera_rotate, camera_translate, camera_t;
	tgl_scale(camera_scale, 1.0F, 1.0F, 1.0F);
	tgl_rotate(camera_rotate, 2.1F, 0.F, 0.F);
	tgl_translate(camera_translate, 0.F, 0.F, 2.F);
	tgl_mulmat((const TGLVec4 *)camera_translate, (const TGLVec4 *)camera_scale, temp);
	tgl_mulmat((const TGLVec4 *)temp, (const TGLVec4 *)camera_rotate, camera_t);

	// Create transformation matrices for object
	TGLMat obj_scale, obj_rotate, obj_translate, obj_t;
	tgl_scale(obj_scale, 0.12F, 0.12F, 0.12F);
	tgl_translate(obj_translate, 0.F, 0.F, 0.F);

	const float dn = 0.02F;
	float n = 0;

	// Determine UVs for triangle vertices
	const uint8_t uv[3][2] = {
		{
			0,
			0,
		},
		{
			255,
			0,
		},
		{
			0,
			255,
		},
	};

	while (1) {
		//Edit transformation to move objects
		tgl_rotate(obj_rotate, 0.F, 0.F, n);
		tgl_mulmat((const TGLVec4 *)obj_translate, (const TGLVec4 *)obj_scale, temp);
		tgl_mulmat((const TGLVec4 *)temp, (const TGLVec4 *)obj_rotate, obj_t);

		unsigned i;
		for (i = 0; i < n_trigs; i++) {
			// Generate final transformation matrix
			TGLMat to_view;
			TGLVertexShaderSimple vertex_shader_data;
			tgl_mulmat((const TGLVec4 *)camera_t, (const TGLVec4 *)obj_t, to_view);
			tgl_mulmat((const TGLVec4 *)camera, (const TGLVec4 *)to_view,
				vertex_shader_data.mat);

			// Draw to framebuffer
			tgl_triangle_3d(tgl, (const TGLVec3 *)trigs[i], uv, true,
				&tgl_vertex_shader_simple, &vertex_shader_data,
				&teapot_pixel_shader, &trigs[i]);
		}

		assert(!tgl_flush(tgl));
		tgl_clear(tgl, TGL_FRAME_BUFFER | TGL_Z_BUFFER | TGL_OUTPUT_BUFFER);

		n += dn * .5;

		sleep_ms(frametime_ms);
	}

	tgl_delete(tgl);
	free(trigs);
}

void demo_keyboard(const unsigned res_x, const unsigned res_y, const unsigned frametime_ms)
{
	TGL *const tgl = tgl_init(res_x, res_y);
	assert(tgl);
	assert(!tgl_enable(tgl, TGL_OUTPUT_BUFFER));

	const size_t bufsize = 16;

	char *const input_keys = calloc(bufsize, sizeof(char));
	input_keys[0] = '\1';

	assert(!tglutil_set_echo_input(false));

	while (1) {
		TGL_SSIZE_T chars_read = tglutil_read(input_keys, bufsize - 1U, NULL, 0, NULL);
		assert(chars_read >= 0);
		if (chars_read) {
			tgl_puts(tgl, 0, 0, "Pressed keys:", TGL_PIXFMT(TGL_IDX(TGL_WHITE)));
			tgl_puts(tgl, 14, 0, input_keys, TGL_PIXFMT(TGL_IDX(TGL_WHITE)));

			assert(!tgl_flush(tgl));
			tgl_clear(tgl, TGL_FRAME_BUFFER | TGL_OUTPUT_BUFFER);
		} else if (input_keys[0]) {
			memset(input_keys, 0, bufsize);

			tgl_puts(tgl, 0, 0, "Pressed keys: NONE", TGL_PIXFMT(TGL_IDX(TGL_WHITE)));

			assert(!tgl_flush(tgl));
			tgl_clear(tgl, TGL_FRAME_BUFFER | TGL_OUTPUT_BUFFER);
		}

		sleep_ms(frametime_ms);
	}

	assert(!tglutil_set_echo_input(true));

	free(input_keys);

	tgl_delete(tgl);
}

void demo_color(const unsigned res_x, const unsigned res_y, const unsigned frametime_ms)
{
	(void)frametime_ms;
	TGL *const tgl = tgl_init(res_x, res_y);
	assert(tgl);
	assert(!tgl_enable(tgl, TGL_OUTPUT_BUFFER));

	static const uint8_t modifiers[5][2] = {
		{ 0, 0 },
		{ TGL_HIGH_INTENSITY, 0 },
		{ 0, TGL_BOLD },
		{ TGL_HIGH_INTENSITY, TGL_BOLD },
		{ 0, TGL_UNDERLINE },
	};

	tgl_puts(tgl, 9, 0, "NULL", TGL_PIXFMT(TGL_IDX(TGL_WHITE)));
	tgl_puts(tgl, 9, 2, "TGL_HIGH_INTENSITY", TGL_PIXFMT(TGL_IDX(TGL_WHITE)));
	tgl_puts(tgl, 9, 4, "TGL_BOLD", TGL_PIXFMT(TGL_IDX(TGL_WHITE)));
	tgl_puts(tgl, 9, 6, "TGL_BOLD + TGL_HIGH_INTENSITY", TGL_PIXFMT(TGL_IDX(TGL_WHITE)));
	tgl_puts(tgl, 9, 8, "TGL_UNDERLINE", TGL_PIXFMT(TGL_IDX(TGL_WHITE)));

	unsigned m, c;
	for (m = 0; m < 5; m++) {
		unsigned y_start = m * 2;
		for (c = 0; c < 8; c++) {
			const char color = "KRGYBPCW"[c];
			tgl_putchar(tgl, c, y_start, color,
				TGL_PIXFMT(TGL_IDX(colors[c] | modifiers[m][0], modifiers[m][1]),
					TGL_IDX(TGL_BLACK | modifiers[m][0], modifiers[m][1])));
			tgl_putchar(tgl, c, y_start + 1, color,
				TGL_PIXFMT(TGL_IDX(TGL_BLACK | modifiers[m][0], modifiers[m][1]),
					TGL_IDX(colors[c] | modifiers[m][0], modifiers[m][1])));
		}
	}

	assert(!tgl_flush(tgl));

	tgl_delete(tgl);

	/* Wait for user input */
	assert(getchar() != EOF);
	assert(getchar() != EOF);
}

static void demo_texture(const unsigned res_x, const unsigned res_y, const unsigned frametime_ms)
{
	TGL *const tgl = tgl_init(res_x, res_y);
	assert(tgl);
	tgl_cull_face(tgl, TGL_BACK | TGL_CCW);
	assert(!tgl_enable(tgl,
		TGL_DOUBLE_CHARS | TGL_CULL_FACE | TGL_Z_BUFFER | TGL_OUTPUT_BUFFER
			| TGL_PROGRESSIVE));

	// Triangle vertices for cube faces
	const TGLTriangle trigs[] = {
		{
			{
				0,
				0,
				0,
			},
			{
				0,
				1,
				0,
			},
			{
				1,
				0,
				0,
			},
		},
		{
			{
				0,
				1,
				0,
			},
			{
				1,
				1,
				0,
			},
			{
				1,
				0,
				0,
			},
		},
		{
			{
				0,
				0,
				0,
			},
			{
				1,
				0,
				0,
			},
			{
				0,
				0,
				1,
			},
		},
		{
			{
				1,
				0,
				0,
			},
			{
				1,
				0,
				1,
			},
			{
				0,
				0,
				1,
			},
		},
		{
			{
				0,
				0,
				0,
			},
			{
				0,
				0,
				1,
			},
			{
				0,
				1,
				0,
			},
		},
		{

			{
				0,
				0,
				1,
			},
			{
				0,
				1,
				1,
			},
			{
				0,
				1,
				0,
			},
		},
		{
			{
				0,
				0,
				1,
			},
			{
				1,
				0,
				1,
			},
			{
				0,
				1,
				1,
			},
		},
		{
			{
				1,
				0,
				1,
			},
			{
				1,
				1,
				1,
			},
			{
				0,
				1,
				1,
			},
		},
		{
			{
				0,
				1,
				0,
			},
			{
				0,
				1,
				1,
			},
			{
				1,
				1,
				0,
			},
		},
		{
			{
				0,
				1,
				1,
			},
			{
				1,
				1,
				1,
			},
			{
				1,
				1,
				0,
			},
		},
		{
			{
				1,
				0,
				0,
			},
			{
				1,
				1,
				0,
			},
			{
				1,
				0,
				1,
			},
		},
		{
			{
				1,
				1,
				0,
			},
			{
				1,
				1,
				1,
			},
			{
				1,
				0,
				1,
			},

		},
	};

	const uint8_t uvs[][3][2] = {
		{
			{
				0,
				0,
			},
			{
				0,
				255,
			},
			{
				255,
				0,
			},
		},
		{
			{
				0,
				255,
			},
			{
				255,
				255,
			},
			{
				255,
				0,
			},
		},
	};

	// Create transformation matrices
	TGLVertexShaderSimple vertex_shader_data;
	TGLMat rotate, translate, translate2, transform;
	TGLMat camera;
	tgl_camera(camera, res_x, res_y, 1.57F, .1F, 10.F);
	tgl_translate(translate, -.5F, -.5F, -.5F);
	tgl_translate(translate2, 0.F, 0.F, 1.3F);

	// Create texture
	const TGLPixelShaderTexture tex = {
		.width = 6,
		.height = 6,
		.chars = "\
######\
# 1 2#\
#3 4 #\
# 5 6#\
#7 8 #\
######",
		.colors =
			(TGLPixFmt[]){
				TGL_PIXFMT(TGL_IDX(TGL_WHITE, TGL_BOLD)),
				TGL_PIXFMT(TGL_IDX(TGL_WHITE, TGL_BOLD)),
				TGL_PIXFMT(TGL_IDX(TGL_WHITE, TGL_BOLD)),
				TGL_PIXFMT(TGL_IDX(TGL_WHITE, TGL_BOLD)),
				TGL_PIXFMT(TGL_IDX(TGL_WHITE, TGL_BOLD)),
				TGL_PIXFMT(TGL_IDX(TGL_WHITE, TGL_BOLD)),
				TGL_PIXFMT(TGL_IDX(TGL_WHITE, TGL_BOLD)),
				TGL_PIXFMT(TGL_IDX(TGL_WHITE, TGL_BOLD)),
				TGL_PIXFMT(TGL_IDX(TGL_RED, TGL_BOLD | TGL_UNDERLINE)),
				TGL_PIXFMT(TGL_IDX(TGL_WHITE, TGL_BOLD)),
				TGL_PIXFMT(TGL_IDX(TGL_GREEN, TGL_BOLD)),
				TGL_PIXFMT(TGL_IDX(TGL_WHITE, TGL_BOLD)),
				TGL_PIXFMT(TGL_IDX(TGL_WHITE, TGL_BOLD)),
				TGL_PIXFMT(TGL_IDX(TGL_YELLOW, TGL_BOLD)),
				TGL_PIXFMT(TGL_IDX(TGL_WHITE, TGL_BOLD)),
				TGL_PIXFMT(TGL_IDX(TGL_BLUE, TGL_BOLD | TGL_UNDERLINE)),
				TGL_PIXFMT(TGL_IDX(TGL_WHITE, TGL_BOLD)),
				TGL_PIXFMT(TGL_IDX(TGL_WHITE, TGL_BOLD)),
				TGL_PIXFMT(TGL_IDX(TGL_WHITE, TGL_BOLD)),
				TGL_PIXFMT(TGL_IDX(TGL_WHITE, TGL_BOLD)),
				TGL_PIXFMT(TGL_IDX(TGL_PURPLE, TGL_BOLD | TGL_UNDERLINE)),
				TGL_PIXFMT(TGL_IDX(TGL_WHITE, TGL_BOLD)),
				TGL_PIXFMT(TGL_IDX(TGL_CYAN, TGL_BOLD)),
				TGL_PIXFMT(TGL_IDX(TGL_WHITE, TGL_BOLD)),
				TGL_PIXFMT(TGL_IDX(TGL_WHITE, TGL_BOLD)),
				TGL_PIXFMT(TGL_IDX(TGL_GREEN, TGL_BOLD)),
				TGL_PIXFMT(TGL_IDX(TGL_WHITE, TGL_BOLD)),
				TGL_PIXFMT(TGL_IDX(TGL_RED, TGL_BOLD | TGL_UNDERLINE)),
				TGL_PIXFMT(TGL_IDX(TGL_WHITE, TGL_BOLD)),
				TGL_PIXFMT(TGL_IDX(TGL_WHITE, TGL_BOLD)),
				TGL_PIXFMT(TGL_IDX(TGL_WHITE, TGL_BOLD)),
				TGL_PIXFMT(TGL_IDX(TGL_WHITE, TGL_BOLD)),
				TGL_PIXFMT(TGL_IDX(TGL_WHITE, TGL_BOLD)),
				TGL_PIXFMT(TGL_IDX(TGL_WHITE, TGL_BOLD)),
				TGL_PIXFMT(TGL_IDX(TGL_WHITE, TGL_BOLD)),
				TGL_PIXFMT(TGL_IDX(TGL_WHITE, TGL_BOLD)),
			},
	};

	const float dn = 0.02F;
	float n = 0;

	while (1) {
		// Calculate final transformation matrix
		TGLMat temp;
		tgl_rotate(rotate, n / 3.F, n, 0.F);
		tgl_mulmat((const TGLVec4 *)translate2, (const TGLVec4 *)rotate, temp);
		tgl_mulmat((const TGLVec4 *)temp, (const TGLVec4 *)translate, transform);
		tgl_mulmat((const TGLVec4 *)camera, (const TGLVec4 *)transform,
			vertex_shader_data.mat);

		unsigned i;
		for (i = 0; i < 12; i++) {
			// Draw to framebuffer
			tgl_triangle_3d(tgl, trigs[i], uvs[i % 2], true, &tgl_vertex_shader_simple,
				&vertex_shader_data, &tgl_pixel_shader_texture, &tex);
		}

		assert(!tgl_flush(tgl));
		tgl_clear(tgl, TGL_FRAME_BUFFER | TGL_Z_BUFFER | TGL_OUTPUT_BUFFER);

		n += dn;

		sleep_ms(frametime_ms);
	}

	tgl_delete(tgl);
}

uint8_t rgb_map_circle(const int dx, const int dy)
{
	return 255 - MIN(255, sqrtf(dx * dx * 4 + dy * dy) * 3.F);
}

void demo_rgb(const unsigned res_x, const unsigned res_y, const unsigned frametime_ms)
{
	(void)frametime_ms;
	TGL *const tgl = tgl_init(res_x, res_y);
	assert(tgl);
	assert(!tgl_enable(tgl, TGL_OUTPUT_BUFFER));

	unsigned x, y;
	for (y = 0; y < 24; y++) {
		for (x = 0; x < 80; x++) {
			const int x_scld = x * 3;
			const int y_scld = y * 10;
			const TGLFmt fmt = TGL_RGB(rgb_map_circle(120 - x_scld, 100 - y_scld),
				rgb_map_circle(100 - x_scld, 140 - y_scld),
				rgb_map_circle(140 - x_scld, 140 - y_scld));
			tgl_putchar(tgl, x, y, '.', TGL_PIXFMT(fmt, fmt));
		}
	}

	assert(!tgl_flush(tgl));

	tgl_delete(tgl);

	/* Wait for user input */
	assert(getchar() != EOF);
	assert(getchar() != EOF);
}

void demo_mouse(const unsigned res_x, const unsigned res_y, const unsigned frametime_ms)
{
	TGL *const tgl = tgl_init(res_x, res_y);
	assert(tgl);
	assert(!tgl_enable(tgl, TGL_OUTPUT_BUFFER));

	const size_t bufsize = 256;
	const size_t count_mouse_events = 8;

	char *const input_keys = calloc(bufsize, sizeof(char));
	TGLMouseEvent *const mouse_events = calloc(count_mouse_events, sizeof(TGLMouseEvent));

	assert(!tglutil_set_echo_input(false));
	assert(!tglutil_set_mouse_tracking_enabled(true));

	tgl_puts(tgl, 0, 0, "Move the mouse.", TGL_PIXFMT(TGL_IDX(TGL_WHITE)));
	assert(!tgl_flush(tgl));
	tgl_clear(tgl, TGL_FRAME_BUFFER | TGL_OUTPUT_BUFFER);

	const char *action = "None";

	while (1) {
		size_t mouse_events_read = 0;
		const TGL_SSIZE_T chars_read = tglutil_read(input_keys, bufsize - 1U, mouse_events,
			count_mouse_events, &mouse_events_read);
		assert(chars_read >= 0);

		if (mouse_events_read) {
			TGLMouseEvent e = mouse_events[mouse_events_read - 1];
			char printf_buf[32];
			assert(snprintf(printf_buf, 32, "Mouse position: %02u, %02u", e.x, e.y)
				>= 0);
			tgl_puts(tgl, 0, 0, printf_buf, TGL_PIXFMT(TGL_IDX(TGL_WHITE)));

			size_t i;
			for (i = mouse_events_read; i > 0; i--) {
				e = mouse_events[i - 1];
				if (e.button & TGL_MOUSE_UNKNOWN)
					continue;
				if (e.button & TGL_MOUSE_RELEASE)
					action = "Release";
				else if (e.button & TGL_MOUSE_1)
					action = "Left Click";
				else if (e.button & TGL_MOUSE_2)
					action = "Right Click";
				else if (e.button & TGL_MOUSE_3)
					action = "Middle Click";
			}
			tgl_puts(tgl, 0, 1, "Latest action:", TGL_PIXFMT(TGL_IDX(TGL_WHITE)));
			tgl_puts(tgl, 15, 1, action, TGL_PIXFMT(TGL_IDX(TGL_WHITE)));

			assert(!tgl_flush(tgl));
			tgl_clear(tgl, TGL_FRAME_BUFFER | TGL_OUTPUT_BUFFER);
		}

		sleep_ms(frametime_ms);
	}

	assert(!tglutil_set_mouse_tracking_enabled(false));
	assert(!tglutil_set_echo_input(true));

	free(input_keys);
	free(mouse_events);

	tgl_delete(tgl);
}

int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	assert(!tgl_boot());

	assert(!tglutil_set_window_title(HELPTEXT_HEADER));

	tgl_clear_screen();
	puts(HELPTEXT_HEADER);
	unsigned col, row;
	tglutil_get_console_size(&col, &row, true);
	printf("Console size: %ux%u\n", col, row);
	puts(HELPTEXT_BODY);

	unsigned n = 0;
	assert(scanf("%u", &n) == 1);
	tgl_clear_screen();

	switch (n) {
	case 1U:
		demo_teapot(40, 40, 33);
		break;
	case 2U:
		demo_color(40, 10, 0);
		break;
	case 3U:
		demo_mandelbrot(80, 40, 33);
		break;
	case 4U:
		demo_keyboard(80, 5, 200);
		break;
	case 5U:
		demo_texture(40, 40, 33);
		break;
	case 6U:
		demo_rgb(80, 24, 0);
		break;
	case 7U:
		demo_mouse(80, 5, 33);
		break;
	default:
		puts("Invalid Input.\n");
		break;
	}

	return 0;
}
