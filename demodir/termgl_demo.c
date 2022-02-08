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
#define __USE_POSIX199309
#define _POSIX_C_SOURCE 199309L
#include <time.h>
#endif

typedef struct  STLTriangle {
	float normal[3]; //normal is unreliable so it is not used.
	float vertices[3][3];
	uint16_t attribute_bytes; //attribute bytes is unreliable so it is not used.
} __attribute__ ((packed)) STLTriangle;

#define xstr(str_) str(str_)
#define str(str_) #str_

const char *HELPTEXT_HEADER = "TermGL v" xstr(TGL_VERSION_MAJOR) "." xstr(TGL_VERSION_MINOR) " Demo Utility";
const char *HELPTEXT_BODY = "\
Select a Demo:\n\
1. Utah Teapot\n\
    Renders a rotating 3D Utah Teapot.\n\
2. Star Polygon\n\
    Renders a star polygon in steps using random colors.\n\
3. Color Palette\n\
    Renders a palette of various text colors and styles.\n\
4. Mandelbrot\n\
    Renders an infinitely zooming-in Mandelbrot set.\n\
5. Realtime Keyboard\n\
    Displays keyboard input in realtime.\
";

const uint16_t fg_colors[] = {
	TGL_BLACK,
	TGL_RED,
	TGL_GREEN,
	TGL_YELLOW,
	TGL_BLUE,
	TGL_PURPLE,
	TGL_CYAN,
	TGL_WHITE,
};
const uint16_t bkg_colors[] = {
	TGL_BLACK_BKG,
	TGL_RED_BKG,
	TGL_GREEN_BKG,
	TGL_YELLOW_BKG,
	TGL_BLUE_BKG ,
	TGL_PURPLE_BKG,
	TGL_CYAN_BKG,
	TGL_WHITE_BKG,
};

void teapot_intermediate_shader(TGLTriangle *trig, void *data);
uint32_t stl_load(FILE *file, TGLTriangle **triangles);
void sleep_ms(const unsigned long ms);

void demo_mandelbrot(const unsigned res_x, const unsigned res_y, const unsigned frametime_ms);
void demo_teapot(const unsigned res_x, const unsigned res_y, const unsigned frametime_ms);
void demo_keyboard(const unsigned res_x, const unsigned res_y, const unsigned frametime_ms);
void demo_star(const unsigned res_x, const unsigned res_y, const unsigned frametime_ms);
void demo_color(const unsigned res_x, const unsigned res_y, const unsigned frametime_ms);

void teapot_intermediate_shader(TGLTriangle *trig, void *data)
{
	TGLVec3 light_direction = {1.f, 0.f, 0.f};
	TGLTriangle *in = data;
	TGLVec3 ab, ac, cp;
	tgl_sub3v(in->vertices[1], in->vertices[0], ab);
	tgl_sub3v(in->vertices[2], in->vertices[0], ac);
	tgl_cross(ab, ac, cp);
	float dp = tgl_dot3(cp, light_direction);
	float light_mul;
	if (signbit(dp))
		light_mul = 0.15f;
	else
		light_mul = acosf(dp / (tgl_mag3(cp) * tgl_mag3(light_direction))) / -3.14159f + 1.f;
	trig->intensity[0] *= light_mul;
	trig->intensity[1] *= light_mul;
	trig->intensity[2] *= light_mul;
}

uint32_t stl_load(FILE *file, TGLTriangle **triangles)
{
	//ensure that file is binary instead of ascii
	char header[5];
	assert(fread(header, sizeof(char), 5, file) == 5);
	assert(strncmp("solid", header, 5));

	assert(fseek(file, sizeof(uint8_t[80]), SEEK_SET) == 0);
	uint32_t num_triangles;
	assert(fread(&num_triangles, sizeof(uint32_t), 1, file) == 1);
	*triangles = malloc(sizeof(TGLTriangle) * num_triangles);

	uint32_t i;
	for (i = 0; i < num_triangles; i++) {
		STLTriangle stl_triangle;
		assert(fread(&stl_triangle, sizeof(STLTriangle), 1, file) == 1);
		memcpy((*triangles)[i].vertices, stl_triangle.vertices, sizeof(float[3][3]));
		memset((*triangles)[i].intensity, 255, 3);
	}
	return num_triangles;
}

void sleep_ms(const unsigned long ms)
{
#ifdef TGL_OS_WINDOWS
	Sleep(ms);
#else
	struct timespec ts = (struct timespec) {
		.tv_sec = ms / 1000,
		.tv_nsec = (ms % 1000ul) * 1000000,
	};
	nanosleep(&ts, NULL);
#endif
}

void demo_mandelbrot(const unsigned res_x, const unsigned res_y, const unsigned frametime_ms)
{
	TGL *tgl = tgl_init(res_x, res_y, &gradient_full);
	assert(tgl);
	assert(!tgl_enable(tgl, TGL_OUTPUT_BUFFER));

	const unsigned frame_max = 90;
	const unsigned i_max = 255;

	const float init_mid_x = -1.f, init_mid_y = 0.f;
	const float end_mid_x = -1.31, end_mid_y = 0.f;
	const float dmid_x = (end_mid_x - init_mid_x) / frame_max, dmid_y = (end_mid_y - init_mid_y) / frame_max;

	const float init_width = .5f, init_height = .5f;
	const float end_width = .12f, end_height = .12f;
	const float dwidth = (end_width - init_width) / frame_max, dheight = (end_height - init_height) / frame_max;

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
				for (i = 0; (ix2 + iy2) <= 4.f && i < i_max; i++) {
					float next_ix = ix2 - iy2 + x;
					iy = 2.f * ix * iy + y;
					ix = next_ix;
					ix2 = ix * ix;
					iy2 = iy * iy;
					i++;
				}
				if (i < i_max) // Set pixel with intensity dependent on i
					tgl_point(tgl, pix_x, pix_y, 0.f, i * 255u / i_max, TGL_WHITE | TGL_BOLD);
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
	TGL *tgl = tgl_init(res_x, res_y, &gradient_min);
	assert(tgl);
	assert(!tgl3d_init(tgl));
	tgl3d_cull_face(tgl, TGL_BACK | TGL_CCW);
	assert(!tgl_enable(tgl, TGL_DOUBLE_CHARS | TGL_CULL_FACE | TGL_Z_BUFFER | TGL_OUTPUT_BUFFER));
	tgl3d_camera(tgl, 1.57f, 0.1f, 5.f);

	// Load triangles
	TGLTriangle *trigs;
	FILE *stl_file = fopen("demodir/utah_teapot.stl", "rb");
	assert(stl_file);
	uint32_t n_trigs = stl_load(stl_file, &trigs);
	fclose(stl_file);

	// Edit camera transformations
	TGLTransform *camera_t = tgl3d_get_transform(tgl);
	tgl3d_transform_scale(camera_t, 1.0f, 1.0f, 1.0f);
	tgl3d_transform_rotate(camera_t, 2.1f, 0.f, 0.f);
	tgl3d_transform_translate(camera_t, 0.f, 0.f, 2.f);
	tgl3d_transform_update(camera_t);

	// Create transformation matrices for object
	TGLTransform obj_t;
	tgl3d_transform_scale(&obj_t, 0.12f, 0.12f, 0.12f);
	tgl3d_transform_translate(&obj_t, 0.f, 0.f, 0.f);

	const float dn = 0.02f;
	float n = 0;

	while (1) {
		//Edit transformation to move objects
		tgl3d_transform_rotate(&obj_t, 0.f, 0.f, n);
		tgl3d_transform_update(&obj_t);

		unsigned i;
		for (i = 0; i < n_trigs; i++) {
			TGLTriangle temp;
			// Apply object's transformation
			tgl3d_transform_apply(&obj_t, trigs[i].vertices, temp.vertices);
			memcpy(temp.intensity, trigs[i].intensity, 3);

			//Draw to framebuffer
			tgl3d_shader(tgl, &temp, TGL_WHITE | TGL_BOLD, true, &temp, &teapot_intermediate_shader);
		}

		assert(!tgl_flush(tgl));
		tgl_clear(tgl, TGL_FRAME_BUFFER | TGL_Z_BUFFER | TGL_OUTPUT_BUFFER);

		n += dn;

		sleep_ms(frametime_ms);
	}

	tgl_delete(tgl);
	free(trigs);
}

void demo_keyboard(const unsigned res_x, const unsigned res_y, const unsigned frametime_ms)
{
	TGL *tgl = tgl_init(res_x, res_y, &gradient_min);
	assert(tgl);
	assert(!tgl_enable(tgl, TGL_OUTPUT_BUFFER));

	const size_t bufsize = 16;

	char *input_keys = calloc(bufsize, sizeof(char));
	input_keys[0] = '\1';

	while (1) {
		if (tglutil_read(input_keys, bufsize - 1u)) {
			tgl_puts(tgl, 0, 0, "Pressed keys:", TGL_WHITE);
			tgl_puts(tgl, 14, 0, input_keys, TGL_WHITE);

			assert(!tgl_flush(tgl));
			tgl_clear(tgl, TGL_FRAME_BUFFER | TGL_OUTPUT_BUFFER);
		} else if (input_keys[0]) {
			memset(input_keys, 0, bufsize);

			tgl_puts(tgl, 0, 0, "Pressed keys: NONE", TGL_WHITE);

			assert(!tgl_flush(tgl));
			tgl_clear(tgl, TGL_FRAME_BUFFER | TGL_OUTPUT_BUFFER);
		}

		sleep_ms(frametime_ms);
	}

	tgl_delete(tgl);
}

void demo_star(const unsigned res_x, const unsigned res_y, const unsigned frametime_ms)
{
	TGL *tgl = tgl_init(res_x, res_y, &gradient_min);
	assert(tgl);
	assert(!tgl_enable(tgl, TGL_OUTPUT_BUFFER));

	const float pi2 = 6.28319f;
	const unsigned n = 8, d = 3;

	unsigned half_res_x = res_x / 2u;
	unsigned half_res_y = res_y / 2u;

	unsigned vert = 0;

	while (1) {
		unsigned next_vert = (vert + d) % n;

		float angle0 = pi2 * vert / n;
		float angle1 = pi2 * next_vert / n;

		unsigned x0 = half_res_x + half_res_x * cosf(angle0) * 0.9f;
		unsigned x1 = half_res_x + half_res_x * cosf(angle1) * 0.9f;
		unsigned y0 = half_res_y + half_res_y * sinf(angle0) * 0.9f;
		unsigned y1 = half_res_y + half_res_y * sinf(angle1) * 0.9f;

		TGLubyte i0 = rand() % 256;
		TGLubyte i1 = rand() % 256;

		uint16_t color = fg_colors[rand() % 8]
			| bkg_colors[rand() % 8];

		tgl_line(tgl, x0, y0, 0, i0, x1, y1, 0, i1, color);

		assert(!tgl_flush(tgl));
		// Buffer clear not yet required

		vert = next_vert;

		if (!vert)
			tgl_clear(tgl, TGL_FRAME_BUFFER | TGL_OUTPUT_BUFFER);

		sleep_ms(frametime_ms);
	}

	tgl_delete(tgl);
}

void demo_color(const unsigned res_x, const unsigned res_y, const unsigned frametime_ms)
{
	(void)frametime_ms;
	TGL *tgl = tgl_init(res_x, res_y, &gradient_min);
	assert(tgl);
	assert(!tgl_enable(tgl, TGL_OUTPUT_BUFFER));

	static const uint16_t modifiers[5][2] = {
		{0, 0},
		{TGL_HIGH_INTENSITY, TGL_HIGH_INTENSITY_BKG},
		{TGL_BOLD, 0},
		{TGL_BOLD | TGL_HIGH_INTENSITY, TGL_HIGH_INTENSITY_BKG},
		{TGL_UNDERLINE, TGL_UNDERLINE},
	};

	tgl_puts(tgl, 9, 0, "NULL", TGL_WHITE);
	tgl_puts(tgl, 9, 2, "TGL_HIGH_INTENSITY", TGL_WHITE);
	tgl_puts(tgl, 9, 4, "TGL_BOLD", TGL_WHITE);
	tgl_puts(tgl, 9, 6, "TGL_BOLD + TGL_HIGH_INTENSITY", TGL_WHITE);
	tgl_puts(tgl, 9, 8, "TGL_UNDERLINE", TGL_WHITE);

	unsigned m, c;
	for (m = 0; m < 5; m++) {
		unsigned y_start = m * 2;
		tgl_putchar(tgl, 0, y_start, 'K', TGL_BLACK | TGL_WHITE_BKG | modifiers[m][0]);
		tgl_putchar(tgl, 0, y_start + 1, 'K', TGL_WHITE | TGL_BLACK_BKG | modifiers[m][1]);
		for (c = 1; c < 8; c++) {
			char color = "KRGYBPCW"[c];
			tgl_putchar(tgl, c, y_start, color, fg_colors[c] | TGL_BLACK_BKG | modifiers[m][0]);
			tgl_putchar(tgl, c, y_start + 1, color, TGL_BLACK | bkg_colors[c] | modifiers[m][1]);
		}
	}

	assert(!tgl_flush(tgl));

	tgl_delete(tgl);

	getchar();
}

int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	puts(HELPTEXT_HEADER);
	unsigned col, row;
	tglutil_get_console_size(&col, &row, true);
	printf("Console size: %ux%u\n", col, row);
	puts(HELPTEXT_BODY);

	unsigned n = 0;
	assert(scanf("%u", &n) == 1);

	switch (n) {
	case 1u:
		demo_teapot(40, 40, 33);
		break;
	case 2u:
		demo_star(80, 40, 500);
		break;
	case 3u:
		demo_color(40, 10, 0);
		break;
	case 4u:
		demo_mandelbrot(80, 40, 33);
		break;
	case 5u:
		demo_keyboard(80, 5, 200);
		break;
	default:
		puts("Invalid Input.\n");
		break;
	}

	return 0;
}
