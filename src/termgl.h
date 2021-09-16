#ifndef TERMGL_H
#	define TERMGL_H

#include <stdbool.h>
#include <inttypes.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TGL TGL;

/// HEADER PROTOTYPES.

/// termgl_vecmath.h - TermGL vecmath
static inline float tgl_sqr(const float val);
static inline float tgl_mag3(const float vec[3]);
static inline float tgl_magsqr3(const float vec[3]);
static inline float tgl_dot3(const float vec1[3], const float vec2[3]);
static inline float tgl_dot43(const float vec1[4], const float vec2[3]);

static inline void tgl_add3s(const float vec1[3], const float summand, float res[3]);
static inline void tgl_sub3s(const float vec1[3], const float subtrahend, float res[3]);
static inline void tgl_mul3s(const float vec[3], const float mul, float res[3]);

static inline void tgl_add3v(const float vec1[3], const float vec2[3], float res[3]);
static inline void tgl_sub3v(const float vec1[3], const float vec2[3], float res[3]);
static inline void tgl_mul3v(const float vec1[3], const float vec2[3], float res[3]);
static inline void tgl_inv3(const float vec[3], float res[3]);

static inline void tgl_cross(const float vec1[3], const float vec2[3], float res[3]);

static inline void tgl_norm3(float vec[3]);
//////////////////////////////////////////////////////////////////////


/// termgl3d.h - 3D helpers for TermGL.
typedef float TGLMat[4][4];
typedef float TGLVec3[3];

/**
 * Struct which stores 3D trasnformation matrices. Operated on via helper functions (tgl3d_transform_...)
 */
typedef struct TGLTransform {
	TGLMat rotate;
	TGLMat scale;
	TGLMat translate;
	TGLMat result;
} TGLTransform;

/**
 * Struct which stores vertices and intensities of a triangle. Passed to rendering function
 */
typedef struct TGLTriangle {
	TGLVec3 vertices[3];
	uint8_t intensity[3];
} TGLTriangle;

#define TGL_CULL_FACE 0x01

#define TGL_BACK  0x00
#define TGL_FRONT 0x01

#define TGL_CW  0x00
#define TGL_CCW 0x02

/**
 * Initializes 3D component of TermGL
 * @param tgl: a TGL context previously created using tgl_init()
 */
static inline void tgl3d_init(TGL *tgl);

/**
 * Sets the camera's perspective projection matrix
 * @param fov: field of view angle in radians
 * @param near: distance to near clipping plane
 * @param far: distance to far clipping plane
 */
static inline void tgl3d_camera(TGL *tgl, float fov, float near, float far);

/**
 * Gets the camera's TGLTransform transformation matrices which can be operated on using tgl3d_transform_... functions
 */
static inline TGLTransform *tgl3d_get_transform(TGL *tgl);

/**
 * Sets which face should be culled. Requires tgl_enable(TGL_CULL_FACE) to be run before faces will be culled
 * @param settings: bitwise combination of:
 *   TGL_BACK OR TGL_FRONT - face to cull
 *   TGL_CW OR TGL_CCW - winding order of triangles
 */
static inline void tgl3d_cull_face(TGL *tgl, uint8_t settings);

/**
 * Renders triangle onto framebuffer
 * @param intermediate_shader: (allow NULL) pointer to a shader function which is executed after vertex shader (projection and clipping) and before fragment shader (drawing onto framebuffer). Parameters are a projected triangle from vertex shader, and optional data. See termgl_test.c for example
 * @param data: (allow NULL) data which is passed to intermediate_shader
 */
static inline void tgl3d_shader(TGL *tgl, TGLTriangle *in, uint8_t color, bool fill, void *data, void (*intermediate_shader)(TGLTriangle*, void*));

/**
 * Various functions to edit TGLTransform matrices
 */
static inline void tgl3d_transform_rotate(TGLTransform *transform, float x, float y, float z);
static inline void tgl3d_transform_scale(TGLTransform *transform, float x, float y, float z);
static inline void tgl3d_transform_translate(TGLTransform *transform, float x, float y, float z);

/**
 * Updates TGLTransform after any contained matrices were changed by above functions
 */
static inline void tgl3d_transform_update(TGLTransform *transform);

/**
 * Applies TGLTransform to array of 3 3D points, typically triangle vertices
 */
static inline void tgl3d_transform_apply(TGLTransform *transform, TGLVec3 in[3], TGLVec3 out[3]);
//////////////////////////////////////////////////////////////////////


/// termgl.h - TermGL general prototypes.
/**
 * Base TermGL library. Handles printing, 2D drawing, buffers.
 */

#define TGL_CLEAR_SCREEN    puts("\033[1;1H\033[2J")
#define TGL_TYPEOF          __typeof__
#define TGL_MALLOC(bytes)   calloc(1, (bytes))
#define TGL_FREE            free


typedef struct Gradient {
	unsigned length;
	const char *grad;
} Gradient;

enum /** colors */ {
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
static inline TGL *tgl_init(const unsigned width, const unsigned height, const Gradient *gradient);

/**
 * Frees a TGL context
 */
static inline void tgl_delete(TGL *tgl);

/**
 * Prints frame buffer to terminal
 */
static inline void tgl_flush(TGL *tgl);

/**
 * Clears frame buffers
 * @param buffers: bitwise combination of buffers:
 *   TGL_FRAME_BUFFER - frame buffer
 *   TGL_Z_BUFFER - depth buffer
 */
static inline void tgl_clear(TGL *tgl, uint8_t buffers);

/**
 * Enables or disables certain settings
 * @param settings: bitwise combination of settings:
 *   TGL_Z_BUFFER - depth buffer
 *   TGL_DOUBLE_CHARS - square pixels by printing 2 characters per pixel
 *   TGL_CULL_FACE - (3D ONLY) cull specified triangle faces
 *   TGL_OUTPUT_BUFFER - output buffer allowing for just one print to flush. Mush faster on most terminals, but requires a few hundred kilobytes of memory
 */
static inline void tgl_enable(TGL *tgl, uint8_t settings);
static inline void tgl_disable(TGL *tgl, uint8_t settings);

/**
 * Various drawing functions
 * @param i: intensity of pixel which will be mapped to character on gradient
 * @param color: bitwise combination of colors defined in above enum. Can use one foreground (TGL_COLOR) and one background (TGL_COLOR_BKG)
 */
static inline void tgl_putchar(TGL *tgl, int x, int y, char c, uint8_t color);
static inline void tgl_puts(TGL *tgl, int x, int y, char *str, uint8_t color);
static inline void tgl_point(TGL *tgl, int x, int y, float z, uint8_t i, uint8_t color);
static inline void tgl_line(TGL *tgl, int x0, int y0, float z0, uint8_t i0, int x1, int y1, float z1, uint8_t i1, uint8_t color);
static inline void tgl_triangle(TGL *tgl, int x0, int y0, float z0, uint8_t i0, int x1, int y1, float z1, uint8_t i1, int x2, int y2, float z2, int i2, uint8_t color);
static inline void tgl_triangle_fill(TGL *tgl, int x0, int y0, float z0, uint8_t i0, int x1, int y1, float z1, uint8_t i1, int x2, int y2, float z2, int i2, uint8_t color);
//////////////////////////////////////////////////////////////////////


/// termgl_intern.h - utilities.
typedef struct Pixel Pixel;
typedef struct TGL3D TGL3D;
typedef struct TGL {
	TGL3D *tgl3d;
	Pixel *frame_buffer;
	float *z_buffer;
	char *output_buffer;
	const Gradient *gradient;

	unsigned width;
	unsigned height;
	int max_x;
	int max_y;
	unsigned frame_size;
	unsigned output_buffer_size;
	bool z_buffer_enabled;
	uint8_t settings;
} TGL;

#define SWAP(a, b)   do {TGL_TYPEOF(a) temp = a; a = b; b = temp;} while(0)

#define MIN(a, b)    (((a)<(b))?(a):(b))
#define MAX(a, b)    (((a)>(b))?(a):(b))

#define XOR(a, b)    (((bool)(a))!=((bool)(b)))
//////////////////////////////////////////////////////////////////////


/// vecmath implementation
__attribute__((const))
static inline float tgl_sqr(const float val) {
	return val * val;
}

__attribute__((const))
static inline float tgl_mag3(const float vec[3]) {
	return sqrtf(tgl_sqr(vec[0]) + tgl_sqr(vec[1]) + tgl_sqr(vec[2]));
}

__attribute__((const))
static inline float tgl_magsqr3(const float vec[3]) {
	return tgl_sqr(vec[0]) + tgl_sqr(vec[1]) + tgl_sqr(vec[2]);
}

__attribute__((const))
static inline float tgl_dot3(const float vec1[3], const float vec2[3]) {
	return vec1[0] * vec2[0] + vec1[1] * vec2[1] + vec1[2] * vec2[2];
}

__attribute__((const))
static inline float tgl_dot43(const float vec1[4], const float vec2[3]) {
	return vec1[0] * vec2[0] + vec1[1] * vec2[1] + vec1[2] * vec2[2] + vec1[3];
}

static inline void tgl_add3s(const float vec1[3], const float summand, float res[3]) {
	res[0] = vec1[0] + summand;
	res[1] = vec1[1] + summand;
	res[2] = vec1[2] + summand;
}

static inline void tgl_sub3s(const float vec1[3], const float subtrahend, float res[3]) {
	res[0] = vec1[0] - subtrahend;
	res[1] = vec1[1] - subtrahend;
	res[2] = vec1[2] - subtrahend;
}

static inline void tgl_mul3s(const float vec[3], const float mul, float res[3]) {
	res[0] = vec[0] * mul;
	res[1] = vec[1] * mul;
	res[2] = vec[2] * mul;
}

static inline void tgl_add3v(const float vec1[3], const float vec2[3], float res[3]) {
	res[0] = vec1[0] + vec2[0];
	res[1] = vec1[1] + vec2[1];
	res[2] = vec1[2] + vec2[2];
}

static inline void tgl_sub3v(const float vec1[3], const float vec2[3], float res[3]) {
	res[0] = vec1[0] - vec2[0];
	res[1] = vec1[1] - vec2[1];
	res[2] = vec1[2] - vec2[2];
}

static inline void tgl_mul3v(const float vec1[3], const float vec2[3], float res[3]) {
	res[0] = vec1[0] * vec2[0];
	res[1] = vec1[1] * vec2[1];
	res[2] = vec1[2] * vec2[2];
}

static inline void tgl_inv3(const float vec[3], float res[3]) {
	res[0] = 1.f / vec[0];
	res[1] = 1.f / vec[1];
	res[2] = 1.f / vec[2];
}

static inline void tgl_cross(const float vec1[3], const float vec2[3], float res[3]) {
	res[0] = vec1[1] * vec2[2] - vec1[2] * vec2[1];
	res[1] = vec1[2] * vec2[0] - vec1[0] * vec2[2];
	res[2] = vec1[0] * vec2[1] - vec1[1] * vec2[0];
}

static inline void tgl_norm3(float vec[3]) {
	tgl_mul3s(vec, 1.f / tgl_mag3(vec), vec);
}
///////////////////////////////////////////////////////////////////////


/// TermGL 3D Implementation:
typedef struct TGL3D {
	uint8_t settings;
	float aspect_ratio;
	float half_width;
	float half_height;

	TGLTransform transform;
	TGLMat projection;
} TGL3D;

#define TGL_CULL_BIT 0x01

#define TGL_CULL_FACE_BIT 0x01
#define TGL_WINDING_BIT 0x02

#define MAP_COORD(half, val) ((val * half) + half)

#define ROTATION_MATRIX(x, y, z) {\
		{cosf(z) * cosf(y), -sinf(z) * cosf(x) + cosf(z) * sinf(y) * sinf(x), sinf(z) * sinf(x) + cosf(z) * sinf(y) * cosf(x), 0.f},\
		{sinf(z) * cosf(y), cosf(z) * cosf(x) + sinf(z) * sinf(y) * sinf(x), -cosf(z) * sinf(x) + sinf(z) * sinf(y) * cosf(x), 0.f},\
		{-sinf(y), cosf(y) * sinf(x), cosf(y) * cosf(x), 0.f},\
		{0.f, 0.f, 0.f, 1.f},\
	}

#define SCALE_MATRIX(x, y, z) {\
		{x,   0.f, 0.f, 0.f},\
		{0.f, y,   0.f, 0.f},\
		{0.f, 0.f, z,   0.f},\
		{0.f, 0.f, 0.f, 1.f},\
	}

#define TRANSLATE_MATRIX(x, y, z) {\
		{1.f, 0.f, 0.f, x},\
		{0.f, 1.f, 0.f, y},\
		{0.f, 0.f, 1.f, z},\
		{0.f, 0.f, 0.f, 1.f},\
	}

enum /*clip planes*/ {
	CLIP_NEAR = 0,
	CLIP_FAR,
	CLIP_LEFT,
	CLIP_RIGHT,
	CLIP_TOP,
	CLIP_BOTTOM,
};

static const TGLVec3 clip_plane_normals[6] = {
	[CLIP_NEAR] = {0.f, 0.f, -1.f},
	[CLIP_FAR] = {0.f, 0.f, 1.f},
	[CLIP_LEFT] = {1.f, 0.f, 0.f},
	[CLIP_RIGHT] = {-1.f, 0.f, 0.f},
	[CLIP_BOTTOM] = {0.f, 1.f, 0.f},
	[CLIP_TOP] = {0.f, -1.f, 0.f},
};

static inline void tgl_mulmatvec(TGLMat mat, const TGLVec3 vec, TGLVec3 res)
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

static inline void tgl_mulmat(TGLMat mat1, TGLMat mat2, TGLMat res) {
#pragma GCC unroll 4
	for (size_t c = 0; c < 4u; c++) {
#pragma GCC unroll 4
		for (size_t d = 0; d < 4u; d++) {
			res[c][d] = 0.f;
#pragma GCC unroll 4
			for (size_t k = 0; k < 4u; k++) {
				res[c][d] += mat1[c][k] * mat2[k][d];
			}
		}
	}
}

static inline float tgl_distance_point_plane(const TGLVec3 normal, TGLVec3 point)
{
	return tgl_dot3(normal, point) + 1.f;
}

static inline void tgl3d_init(TGL *tgl)
{
	TGL3D *tgl3d = TGL_MALLOC(sizeof(TGL3D));

	tgl3d->aspect_ratio = tgl->height / (float)tgl->width;
	tgl3d->half_width = tgl->width / 2.f;
	tgl3d->half_height = tgl->height / 2.f;

	tgl->tgl3d = tgl3d;
}

static inline void tgl3d_camera(TGL *tgl, float fov, float near, float far)
{
	TGL3D *tgl3d = tgl->tgl3d;
	float s = 1.f / tanf(fov * .5f);
	float a = 1.f / (far - near);
	TGLMat projection = {
		{s * tgl3d->aspect_ratio, 0.f, 0.f, 0.f},
		{0.f, s, 0.f, 0.f},
		{0.f, 0.f, -(far + near) * a, 2.f * far * near * a},
		{0.f, 0.f, 1.f, 0.f},
	};
	memcpy(tgl->tgl3d->projection, projection, sizeof(TGLMat));
}

static inline void tgl3d_transform_rotate(TGLTransform *transform, float x, float y, float z)
{
	TGLMat rotate = ROTATION_MATRIX(x, y, z);
	memcpy(transform->rotate, rotate, sizeof(TGLMat));
}

static inline void tgl3d_transform_scale(TGLTransform *transform, float x, float y, float z)
{
	TGLMat scale = SCALE_MATRIX(x, y, z);
	memcpy(transform->scale, scale, sizeof(TGLMat));
}

static inline void tgl3d_transform_translate(TGLTransform *transform, float x, float y, float z)
{
	TGLMat translate = TRANSLATE_MATRIX(x, y, z);
	memcpy(transform->translate, translate, sizeof(TGLMat));
}

static inline void tgl3d_transform_update(TGLTransform *transform)
{
	TGLMat temp;
	tgl_mulmat(transform->translate, transform->scale, temp);
	tgl_mulmat(temp, transform->rotate, transform->result);
}

static inline void tgl3d_transform_apply(TGLTransform *transform, TGLVec3 in[3], TGLVec3 out[3])
{
	tgl_mulmatvec(transform->result, in[0], out[0]);
	tgl_mulmatvec(transform->result, in[1], out[1]);
	tgl_mulmatvec(transform->result, in[2], out[2]);
}

static inline float tgl_line_intersect_plane(const TGLVec3 normal, TGLVec3 start, TGLVec3 end, TGLVec3 point)
{
	TGLVec3 line_vec;
	tgl_sub3v(start, end, line_vec);
	float distance = -(tgl_dot3(normal, start) + 1.f) / (tgl_dot3(normal, line_vec));
	tgl_mul3s(line_vec, distance, point);
	tgl_add3v(point, start, point);
	return distance;
}

static inline unsigned clip_triangle_plane(const TGLVec3 normal, TGLTriangle *in, TGLTriangle out[2])
{
	TGLVec3 di = {
		tgl_distance_point_plane(normal, in->vertices[0]),
		tgl_distance_point_plane(normal, in->vertices[1]),
		tgl_distance_point_plane(normal, in->vertices[2])
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
			d = tgl_line_intersect_plane(normal, out[0].vertices[0], in->vertices[outside[i]], out[0].vertices[i + 1]);
			out[0].intensity[i + 1] = out[0].intensity[0] * (1.f - d) + in->intensity[outside[i]] * (d);
		}
		return 1;
	case 2: ;
		memcpy(out[0].vertices[0], in->vertices[inside[0]], sizeof(TGLVec3));
		memcpy(out[0].vertices[1], in->vertices[inside[1]], sizeof(TGLVec3));
		d = tgl_line_intersect_plane(normal, out[0].vertices[0], in->vertices[outside[0]], out[0].vertices[2]);
		out[0].intensity[0] = in->intensity[inside[0]];
		out[0].intensity[1] = in->intensity[inside[1]];
		out[0].intensity[2] = out[0].intensity[0] * (1.f - d) + in->intensity[outside[0]] * (d);

		memcpy(out[1].vertices[0], in->vertices[inside[1]], sizeof(TGLVec3));
		memcpy(out[1].vertices[1], out->vertices[2], sizeof(TGLVec3));
		d = tgl_line_intersect_plane(normal, out[1].vertices[0], in->vertices[outside[0]], out[1].vertices[2]);
		out[1].intensity[0] = in->intensity[inside[1]];
		out[1].intensity[1] = out[0].intensity[2];
		out[1].intensity[2] = out[1].intensity[0] * (1.f - d) + in->intensity[outside[0]] * (d);

		return 2;
	}
	return 0;
}

static inline void tgl3d_shader(TGL *tgl, TGLTriangle *in, uint8_t color, bool fill, void *data, void (*intermediate_shader)(TGLTriangle*, void*))
{
	TGL3D *tgl3d = tgl->tgl3d;

	/// VERTEX SHADER
	TGLTriangle t, out;

	tgl_mulmatvec(tgl3d->transform.result, in->vertices[0], t.vertices[0]);
	tgl_mulmatvec(tgl3d->transform.result, in->vertices[1], t.vertices[1]);
	tgl_mulmatvec(tgl3d->transform.result, in->vertices[2], t.vertices[2]);

	/// TODO: clipping with the near plane should be done before projection, but this solution works well enough for now
	if (t.vertices[0][2] < 1e-6f
		|| t.vertices[1][2] < 1e-6f
		|| t.vertices[2][2] < 1e-6f)
		return;

	tgl_mulmatvec(tgl3d->projection, t.vertices[0], out.vertices[0]);
	tgl_mulmatvec(tgl3d->projection, t.vertices[1], out.vertices[1]);
	tgl_mulmatvec(tgl3d->projection, t.vertices[2], out.vertices[2]);

	memcpy(out.intensity, in->intensity, sizeof(uint8_t) * 3);

	if (tgl->settings & TGL_CULL_FACE) {
		TGLVec3 ab, ac, cp;
		tgl_sub3v(out.vertices[1], out.vertices[0], ab);
		tgl_sub3v(out.vertices[2], out.vertices[0], ac);
		tgl_cross(ab, ac, cp);
		if (XOR(tgl3d->settings & TGL_CULL_BIT, signbit(cp[2])))
			return;
	}

	TGLTriangle trig_buffer[127]; /// the size of this buffer assumes a pathological case which is probably impossible
	memcpy(&trig_buffer[0], &out, sizeof(TGLTriangle));
	unsigned buffer_offset = 0;
	unsigned n_cur_stage = 1;
	for (unsigned p = 0; p < 6; p++) {
		unsigned n_next_stage = 0;
		for (unsigned i = 0; i < n_cur_stage; i++)
			n_next_stage += clip_triangle_plane(clip_plane_normals[p], &trig_buffer[i + buffer_offset], &trig_buffer[buffer_offset + n_cur_stage + n_next_stage]);
		buffer_offset += n_cur_stage;
		n_cur_stage = n_next_stage;
	}

	for (unsigned i = 0; i < n_cur_stage; i++) {
		TGLTriangle *trig = &trig_buffer[i + buffer_offset];

		/// INTERMEDIATE SHADER
		if (intermediate_shader)
			intermediate_shader(trig, data);

		/// FRAGMENT SHADER
		( (fill)? tgl_triangle_fill : tgl_triangle )(tgl,
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

static inline void tgl3d_cull_face(TGL *tgl, uint8_t settings)
{
	tgl->tgl3d->settings = (tgl->tgl3d->settings & ~TGL_CULL_BIT) | (XOR(settings & TGL_CULL_FACE_BIT, settings & TGL_WINDING_BIT) ? TGL_CULL_BIT: 0);
}

static inline TGLTransform *tgl3d_get_transform(TGL *tgl)
{
	return &tgl->tgl3d->transform;
}
///////////////////////////////////////////////////////////////////////


/// TermGL General Implementation:
typedef struct Pixel {
	char v_char;
	uint8_t color;
} Pixel;

#define SET_PIXEL_RAW(tgl, x, y, v_char_, color_)\
	do {\
		*(&tgl->frame_buffer[y * tgl->width + x]) = (Pixel) {\
			.v_char = v_char_,\
			.color = color_,\
		};\
	} while (0)

#define SET_PIXEL(tgl, x, y, z, v_char_, color_)\
	do {\
		if (!tgl->z_buffer_enabled) {\
			SET_PIXEL_RAW(tgl, x, y, v_char_, color_);\
		} else if (z >= tgl->z_buffer[y * tgl->width + x]) {\
			SET_PIXEL_RAW(tgl, x, y, v_char_, color_);\
			tgl->z_buffer[y * tgl->width + x] = z;\
		}\
	} while (0)

#define INTENSITY_TO_CHAR(tgl, intensity) tgl->gradient->grad[(tgl->gradient->length * intensity) / 256u]

static const char grad_full_chars[] = " .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
static const Gradient gradient_full = {
	.length = 70,
	.grad = grad_full_chars,
};

static const char grad_min_chars[] = " .:-=+*#%@";
static const Gradient gradient_min = {
	.length = 10,
	.grad = grad_min_chars,
};

static const char *color_codes[] = {
	[TGL_BLACK] =      "\033[0;30m",
	[TGL_RED] =        "\033[0;31m",
	[TGL_GREEN] =      "\033[0;32m",
	[TGL_YELLOW] =     "\033[0;33m",
	[TGL_BLUE] =       "\033[0;34m",
	[TGL_PURPLE] =     "\033[0;35m",
	[TGL_CYAN] =       "\033[0;36m",
	[TGL_WHITE] =      "\033[0;37m",
};

static const char *color_codes_bkg[] = {
	[TGL_BLACK_BKG >> 4] =  "\033[40m",
	[TGL_RED_BKG >> 4] =    "\033[41m",
	[TGL_GREEN_BKG  >> 4] =  "\033[42m",
	[TGL_YELLOW_BKG >> 4] = "\033[43m",
	[TGL_BLUE_BKG >> 4] =   "\033[44m",
	[TGL_PURPLE_BKG >> 4] = "\033[45m",
	[TGL_CYAN_BKG >> 4] =   "\033[46m",
	[TGL_WHITE_BKG >> 4] =  "\033[47m",
};

static inline void itgl_clip(TGL *tgl, int *x, int *y)
{
	*x = MAX(MIN(tgl->max_x, *x), 0);
	*y = MAX(MIN(tgl->max_y, *y), 0);
}

static inline void tgl_clear(TGL *tgl, const uint8_t buffers)
{
	if (buffers & TGL_FRAME_BUFFER) {
		for (unsigned i = 0; i < tgl->frame_size; i++) {
			*(&tgl->frame_buffer[i]) = (Pixel) {
				.v_char = ' ',
				.color = 0x00
			};
		}
	}
	if (buffers & TGL_Z_BUFFER) {
		for (unsigned i = 0; i < tgl->frame_size; i++) {
			tgl->z_buffer[i] = -1.f;
		}
	}
	if (buffers & TGL_OUTPUT_BUFFER) {
		memset(tgl->output_buffer, '\0', tgl->output_buffer_size);
	}
}

static inline TGL *tgl_init(const unsigned width, const unsigned height, const Gradient *gradient)
{
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

static inline void tgl_flush(TGL *tgl)
{
	TGL_CLEAR_SCREEN;
	uint8_t color = 0xFF;
	Pixel *pixel = tgl->frame_buffer;
	bool double_chars = tgl->settings & TGL_DOUBLE_CHARS;

	if (tgl->output_buffer_size) {
		char *output_buffer_loc = tgl->output_buffer;
		for (unsigned row = 0; row < tgl->height; row++) {
			for (unsigned col = 0; col < tgl->width; col++) {
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
		puts(tgl->output_buffer);
	} else {
		for (unsigned row = 0; row < tgl->height; row++) {
			for (unsigned col = 0; col < tgl->width; col++) {
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
	}
	fflush(stdout);
}

static inline void tgl_putchar(TGL *tgl, int x, int y, char c, uint8_t color)
{
	itgl_clip(tgl, &x, &y);
	SET_PIXEL_RAW(tgl, x, y, c, color);
}

static inline void tgl_puts(TGL *tgl, int x, int y, char *str, uint8_t color)
{
	itgl_clip(tgl, &x, &y);
	char *c_ptr = str;
	while (*c_ptr) {
		SET_PIXEL_RAW(tgl, x, y, *c_ptr, color);
		c_ptr++;
	}
}

static inline void tgl_point(TGL *tgl, int x, int y, float z, uint8_t i, uint8_t color)
{
	itgl_clip(tgl, &x, &y);
	SET_PIXEL(tgl, x, y, z, INTENSITY_TO_CHAR(tgl, i), color);
}

/// Bresenham's line algorithm
static inline void tgl_line(TGL *tgl, int x0, int y0, float z0, uint8_t i0, int x1, int y1, float z1, uint8_t i1, const uint8_t color)
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
		for (int x = x0; x <= x1; x++) {
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
		for (int y = y0; y < y1; y++) {
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

static inline void tgl_triangle(TGL *tgl, int x0, int y0, float z0, uint8_t i0, int x1, int y1, float z1, uint8_t i1, int x2, int y2, float z2, int i2, const uint8_t color)
{
	tgl_line(tgl, x0, y0, z0, i0, x1, y1, z1, i1, color);
	tgl_line(tgl, x1, y1, z1, i1, x2, y2, z2, i2, color);
	tgl_line(tgl, x2, y2, z2, i2, x0, y0, z0, i0, color);
}

static inline void itgl_horiz_line(TGL *tgl, int x0, float z0, uint8_t i0, int x1, float z1, uint8_t i1, int y, const uint8_t color)
{
	if (x0 == x1) {
		SET_PIXEL(tgl, x0, y, z0, INTENSITY_TO_CHAR(tgl, i0), color);
	} else {
		int dx = x1 - x0;
		for (int x = x0; x <= x1; x++)
			SET_PIXEL(tgl, x, y,
				((x - x0) * z1 + (x1 - x) * z0) / dx,
				INTENSITY_TO_CHAR(tgl, ((x - x0) * i1 + (x1 - x) * i0) / dx), color);
	}

}

/// Solution based on Bresenham's line algorithm
/// adapted from: https://github.com/OneLoneCoder/videos/blob/master/olcConsoleGameEngine.h
static inline void tgl_triangle_fill(TGL *tgl, int x0, int y0, float z0, uint8_t i0, int x1, int y1, float z1, uint8_t i1, int x2, int y2, float z2, int i2, const uint8_t color)
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

static inline void tgl_enable(TGL *tgl, uint8_t settings)
{
	tgl->settings |= settings;
	if (settings & TGL_Z_BUFFER) {
		tgl->z_buffer_enabled = true;
		tgl->z_buffer = TGL_MALLOC(sizeof(float) * tgl->frame_size);
		tgl_clear(tgl, TGL_Z_BUFFER);
	}
	if (settings & TGL_OUTPUT_BUFFER) {
		tgl->output_buffer_size = 14 * tgl->frame_size + tgl->height + 1;
		tgl->output_buffer = TGL_MALLOC(tgl->output_buffer_size);
		tgl_clear(tgl, TGL_OUTPUT_BUFFER);
	}
}

static inline void tgl_disable(TGL *tgl, uint8_t settings)
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

static inline void tgl_delete(TGL *tgl)
{
	TGL_FREE(tgl->frame_buffer);  tgl->frame_buffer = NULL;
	TGL_FREE(tgl->z_buffer);      tgl->z_buffer = NULL;
	TGL_FREE(tgl->output_buffer); tgl->output_buffer = NULL;
	TGL_FREE(tgl->tgl3d);         tgl->tgl3d = NULL;
	TGL_FREE(tgl);
}
///////////////////////////////////////////////////////////////////////


#ifdef __cplusplus
}
#endif

#endif /** TERMGL_H */
