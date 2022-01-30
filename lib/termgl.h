#ifndef TERMGL_H
#define TERMGL_H

#ifdef __cplusplus
extern "C"{
#endif

#define TGL_VERSION_MAJOR 1
#define TGL_VERSION_MINOR 1

#include <stdbool.h>

#if defined(_WIN32) || defined(WIN32)
#define TGL_OS_WINDOWS
#include <windef.h>
#endif

/**
 * Setting MACROs
 */
#define TGL_CLEAR_SCREEN do {puts("\033[1;1H\033[2J");} while (0)
#define TGL_TYPEOF __typeof__
#define TGL_MALLOC malloc
#define TGL_FREE free

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

typedef unsigned char TGLubyte;

typedef struct TGL TGL;
typedef struct TGLGradient {
	unsigned length;
	const char *grad;
} TGLGradient;

extern const TGLGradient gradient_full;
extern const TGLGradient gradient_min;

/**
 * Initializes a TGL struct which must be passed to all functions as context
 * @param gradient: pointer to a gradient struct which holds characters which will be used when rendering. TGLGradients provided by default are gradient_min and gradient_full
 */
TGL *tgl_init(const unsigned width, const unsigned height, const TGLGradient *gradient);

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
void tgl_clear(TGL *tgl, TGLubyte buffers);

/**
 * Enables or disables certain settings
 * @param settings: bitwise combination of settings:
 *   TGL_Z_BUFFER - depth buffer
 *   TGL_DOUBLE_CHARS - square pixels by printing 2 characters per pixel
 *   TGL_CULL_FACE - (3D ONLY) cull specified triangle faces
 *   TGL_OUTPUT_BUFFER - output buffer allowing for just one print to flush. Mush faster on most terminals, but requires a few hundred kilobytes of memory
 */
void tgl_enable(TGL *tgl, TGLubyte settings);
void tgl_disable(TGL *tgl, TGLubyte settings);

/**
 * Various drawing functions
 * @param i: intensity of pixel which will be mapped to character on gradient
 * @param color: bitwise combination of colors defined in above enum. Can use one foreground (TGL_COLOR) and one background (TGL_COLOR_BKG)
 */
void tgl_putchar(TGL *tgl, int x, int y, char c, TGLubyte color);
void tgl_puts(TGL *tgl, int x, int y, char *str, TGLubyte color);
void tgl_point(TGL *tgl, int x, int y, float z, TGLubyte i, TGLubyte color);
void tgl_line(TGL *tgl, int x0, int y0, float z0, TGLubyte i0, int x1, int y1, float z1, TGLubyte i1, TGLubyte color);
void tgl_triangle(TGL *tgl, int x0, int y0, float z0, TGLubyte i0, int x1, int y1, float z1, TGLubyte i1, int x2, int y2, float z2, int i2, TGLubyte color);
void tgl_triangle_fill(TGL *tgl, int x0, int y0, float z0, TGLubyte i0, int x1, int y1, float z1, TGLubyte i1, int x2, int y2, float z2, int i2, TGLubyte color);

#ifdef TERMGL3D

#define TGL_CULL_FACE 0x01

#define TGL_BACK  0x00
#define TGL_FRONT 0x01

#define TGL_CW  0x00
#define TGL_CCW 0x02

#define TGL_ROTATION_MATRIX(x_, y_, z_) {\
		{cosf(z_) * cosf(y_), -sinf(z_) * cosf(x_) + cosf(z_) * sinf(y_) * sinf(x_), sinf(z_) * sinf(x_) + cosf(z_) * sinf(y_) * cosf(x_), 0.f},\
		{sinf(z_) * cosf(y_), cosf(z_) * cosf(x_) + sinf(z_) * sinf(y_) * sinf(x_), -cosf(z_) * sinf(x_) + sinf(z_) * sinf(y_) * cosf(x_), 0.f},\
		{-sinf(y_), cosf(y_) * sinf(x_), cosf(y_) * cosf(x_), 0.f},\
		{0.f, 0.f, 0.f, 1.f},\
	}

#define TGL_SCALE_MATRIX(x_, y_, z_) {\
		{x_,   0.f, 0.f, 0.f},\
		{0.f, y_,   0.f, 0.f},\
		{0.f, 0.f, z_,   0.f},\
		{0.f, 0.f, 0.f, 1.f},\
	}

#define TGL_TRANSLATE_MATRIX(x_, y_, z_) {\
		{1.f, 0.f, 0.f, x_},\
		{0.f, 1.f, 0.f, y_},\
		{0.f, 0.f, 1.f, z_},\
		{0.f, 0.f, 0.f, 1.f},\
	}

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
	TGLubyte intensity[3];
} TGLTriangle;

float tgl_sqr(const float val);
float tgl_mag3(const float vec[3]);
float tgl_magsqr3(const float vec[3]);
float tgl_dot3(const float vec1[3], const float vec2[3]);
float tgl_dot43(const float vec1[4], const float vec2[3]);

void tgl_add3s(const float vec1[3], const float summand, float res[3]);
void tgl_sub3s(const float vec1[3], const float subtrahend, float res[3]);
void tgl_mul3s(const float vec[3], const float mul, float res[3]);

void tgl_add3v(const float vec1[3], const float vec2[3], float res[3]);
void tgl_sub3v(const float vec1[3], const float vec2[3], float res[3]);
void tgl_mul3v(const float vec1[3], const float vec2[3], float res[3]);
void tgl_inv3(const float vec[3], float res[3]);

void tgl_cross(const float vec1[3], const float vec2[3], float res[3]);

void tgl_norm3(float vec[3]);

/**
 * Initializes 3D component of TermGL
 * @param tgl: a TGL context previously created using tgl_init()
 */
void tgl3d_init(TGL *tgl);

/**
 * Sets the camera's perspective projection matrix
 * @param fov: field of view angle in radians
 * @param near_val: distance to near clipping plane
 * @param far_val: distance to far clipping plane
 */
void tgl3d_camera(TGL *tgl, float fov, float near_val, float far_val);

/**
 * Gets the camera's TGLTransform transformation matrices which can be operated on using tgl3d_transform_... functions
 */
TGLTransform *tgl3d_get_transform(TGL *tgl);

/**
 * Sets which face should be culled. Requires tgl_enable(TGL_CULL_FACE) to be run before faces will be culled
 * @param settings: bitwise combination of:
 *   TGL_BACK OR TGL_FRONT - face to cull
 *   TGL_CW OR TGL_CCW - winding order of triangles
 */
void tgl3d_cull_face(TGL *tgl, TGLubyte settings);

/**
 * Renders triangle onto framebuffer
 * @param intermediate_shader: (allow NULL) pointer to a shader function which is executed after vertex shader (projection and clipping) and before fragment shader (drawing onto framebuffer). Parameters are a projected triangle from vertex shader, and optional data. See termgl_test.c for example
 * @param data: (allow NULL) data which is passed to intermediate_shader
 */
void tgl3d_shader(TGL *tgl, TGLTriangle *in, TGLubyte color, bool fill, void *data, void (*intermediate_shader)(TGLTriangle*, void*));

/**
 * Various functions to edit TGLTransform matrices
 */
void tgl3d_transform_rotate(TGLTransform *transform, float x, float y, float z);
void tgl3d_transform_scale(TGLTransform *transform, float x, float y, float z);
void tgl3d_transform_translate(TGLTransform *transform, float x, float y, float z);

/**
 * Updates TGLTransform after any contained matrices were changed by above functions
 */
void tgl3d_transform_update(TGLTransform *transform);

/**
 * Applies TGLTransform to array of 3 3D points, typically triangle vertices
 */
void tgl3d_transform_apply(TGLTransform *transform, TGLVec3 in[3], TGLVec3 out[3]);

#endif /* TERMGL3D */

#ifdef TERMGLUTIL

#include <stddef.h>

#ifdef __unix__
#include <unistd.h>
#define TGL_SSIZE_T ssize_t
#elif defined(TGL_OS_WINDOWS)
#define TGL_SSIZE_T SSIZE_T
#else
#error "TermGLUtil is only supported on UNIX and Windows."
#endif

/**
 * Reads up to count bytes from raw terminal input into buf
 */
TGL_SSIZE_T tglutil_read(char *buf, size_t count);

#endif /* TERMGLUTIL */

#ifdef __cplusplus
}
#endif

#endif /* TERMGL_H */
