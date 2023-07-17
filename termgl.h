/*
 * Copyright (c) 2021-2023 Wojciech Graj
 *
 * Licensed under the MIT license: https://opensource.org/licenses/MIT
 * Permission is granted to use, copy, modify, and redistribute the work.
 * Full license information available in the project LICENSE file.
 **/

#ifndef TERMGL_H
#define TERMGL_H

#ifdef __cplusplus
extern "C" {
#endif

#define TGL_VERSION_MAJOR 1
#define TGL_VERSION_MINOR 3

#include <stdbool.h>
#include <stdint.h>

#if defined(_WIN32) || defined(WIN32)
#define TGL_OS_WINDOWS
#include <windef.h>
#endif

enum /* colors */ {
	/* text colors */
	TGL_BLACK = 0x00,
	TGL_RED = 0x01,
	TGL_GREEN = 0x02,
	TGL_YELLOW = 0x03,
	TGL_BLUE = 0x04,
	TGL_PURPLE = 0x05,
	TGL_CYAN = 0x06,
	TGL_WHITE = 0x07,
	/* highlight colors */
	TGL_BLACK_BKG = 0x00,
	TGL_RED_BKG = 0x10,
	TGL_GREEN_BKG = 0x20,
	TGL_YELLOW_BKG = 0x30,
	TGL_BLUE_BKG = 0x40,
	TGL_PURPLE_BKG = 0x50,
	TGL_CYAN_BKG = 0x60,
	TGL_WHITE_BKG = 0x70,
	/* modifiers */
	TGL_HIGH_INTENSITY = 0x08,
	TGL_HIGH_INTENSITY_BKG = 0x80,
	TGL_BOLD = 0x100, /* Often equivalent to TGL_HIGH_INTENSITY */
	TGL_UNDERLINE = 0x200,
};

enum {
	/* buffers */
	TGL_FRAME_BUFFER = 0x01,
	TGL_OUTPUT_BUFFER = 0x02,
	TGL_Z_BUFFER = 0x04,
	/* settings */
	TGL_DOUBLE_CHARS = 0x10,
	TGL_PROGRESSIVE = 0x20,
#ifdef TERMGL3D
	TGL_CULL_FACE = 0x40,
	/* internal - DO NOT USE */
	TGL_CULL_BIT = 0x80,
#endif
};

/**
 * Rendering context that is passed to most functions
 */
typedef struct TGL TGL;

/**
 * Pixel shader that is called for each pixel in draw functions
 */
typedef void TGLPixelShader(uint8_t u, uint8_t v, uint16_t *color, char *c, const void *data);

/**
 * Vertex data passed into 2D drawing functions
 */
typedef struct TGLVert {
	int x;
	int y;
	float z; /**< depth */
	uint8_t u; /**< u passed into TGLPixelShader */
	uint8_t v; /**< v passed into TGLPixelShader */
} TGLVert;

#ifndef TERMGL_MINIMAL

/**
 * Gradient of characters from dark to bright
 */
typedef struct TGLGradient {
	unsigned length;
	const char *grad;
} TGLGradient;

typedef struct TGLPixelShaderSimple {
	uint16_t color;
	const TGLGradient *grad;
} TGLPixelShaderSimple;

typedef struct TGLPixelShaderTexture {
	uint8_t width;
	uint8_t height;
	const char *chars;
	const uint16_t *colors;
} TGLPixelShaderTexture;

extern const TGLGradient gradient_full;
extern const TGLGradient gradient_min;

/**
 * Pixel shader that selects the char from a TGLGradient and has fixed color
 * @param data (TGLPixelShaderSimple *)
 */
void tgl_pixel_shader_simple(uint8_t u, uint8_t v, uint16_t *color, char *c, const void *data);

/**
 * Pixel shader that maps u+v onto a TGLGradient
 * @param data (TGLPixelShaderTexture *)
 */
void tgl_pixel_shader_texture(uint8_t u, uint8_t v, uint16_t *color, char *c, const void *data);

/**
 * Gets a gradient's character corresponding to an intensity (i.e. u or v value)
 */
char tgl_grad_char(const TGLGradient *grad, uint8_t intensity);

#endif /* ~TERMGL_MINIMAL */

/**
 * Initializes a TGL struct which must be passed to all functions as context
 * @param gradient: pointer to a gradient struct which holds characters which will be used when rendering. TGLGradients provided by default are gradient_min and gradient_full
 * @return: pointer to a TGL struct, NULL on failure
 * On failure, errno is set to value specified by:
 *   UNIX and Windows: https://www.man7.org/linux/man-pages/man3/malloc.3.html#ERRORS
 *   Windows: https://docs.microsoft.com/en-us/windows/win32/debug/system-error-codes
 */
TGL *tgl_init(unsigned width, unsigned height);

/**
 * Frees a TGL context
 */
void tgl_delete(TGL *tgl);

/**
 * Prints frame buffer to terminal
 * @return 0 on success, -1 on failure
 * On failure, errno is set to value specified by: https://man7.org/linux/man-pages/man3/fputc.3p.html#ERRORS
 */
int tgl_flush(TGL *tgl);

/**
 * Clears buffers
 * @param buffers: bitwise combination of buffers:
 *   TGL_FRAME_BUFFER - frame buffer
 *   TGL_Z_BUFFER - depth buffer
 */
void tgl_clear(TGL *tgl, uint8_t buffers);

/**
 * Clears the screen
 */
void tgl_clear_screen(void);

/**
 * Enables or disables certain settings
 * @param settings: bitwise combination of settings:
 *   TGL_Z_BUFFER - depth buffer
 *   TGL_DOUBLE_CHARS - square pixels by printing 2 characters per pixel
 *   TGL_CULL_FACE - (3D ONLY) cull specified triangle faces
 *   TGL_OUTPUT_BUFFER - output buffer allowing for just one print to flush. Mush faster on most terminals, but requires a few hundred kilobytes of memory
 *   TGL_PROGRESSIVE - Over-write previous frame. Eliminates strobing but requires call to tgl_clear_screen before drawing smaller image and after resizing terminal if terminal size was smaller than frame size
 * @return 0 on success, -1 on failure
 * On failure, errno is set to value specified by: https://www.man7.org/linux/man-pages/man3/malloc.3.html#ERRORS
 */
int tgl_enable(TGL *tgl, uint8_t settings);
void tgl_disable(TGL *tgl, uint8_t settings);

/**
 * Printing functions similar to those provided by stdio.h
 */
void tgl_putchar(TGL *tgl, int x, int y, char c, uint16_t color);
void tgl_puts(TGL *tgl, int x, int y, const char *str, uint16_t color);

/**
 * Drawing functions
 */
void tgl_point(TGL *tgl, TGLVert v0, TGLPixelShader *t, const void *data);
void tgl_line(TGL *tgl, TGLVert v0, TGLVert v1, TGLPixelShader *t, const void *data);
void tgl_triangle(TGL *tgl, TGLVert v0, TGLVert v1, TGLVert v2, TGLPixelShader *t, const void *data);
void tgl_triangle_fill(TGL *tgl, TGLVert v0, TGLVert v1, TGLVert v2, TGLPixelShader *t, const void *data);

#ifdef TERMGL3D

enum /* faces */ {
	TGL_BACK = 0x00,
	TGL_FRONT = 0x01,
};

enum /* winding */ {
	TGL_CW = 0x00,
	TGL_CCW = 0x02,
};

typedef float TGLMat[4][4];
typedef float TGLVec3[3];
typedef float TGLVec4[4];
typedef TGLVec3 TGLTriangle[3];

/**
 * Vertex shader that should transform an input vertex into Clip Space
 */
typedef void TGLVertexShader(const TGLVec3 in, TGLVec4 out, const void *data);

#ifndef TERMGL_MINIMAL
typedef struct TGLVertexShaderSimple {
	TGLMat mat;
} TGLVertexShaderSimple;

/**
 * Vertex shader that outputs the input vertex multiplied by a matrix
 * @param data (TGLVertexShaderSimple *)
 */
void tgl3d_vertex_shader_simple(const TGLVec3 vert, TGLVec4 out, const void *data);

/**
 * Transformation matrix generation functions
 */
void tgl_camera(TGLMat camera, int width, int height, float fov, float near_val, float far_val);
void tgl_rotate(TGLMat rotate, float x, float y, float z);
void tgl_scale(TGLMat scale, float x, float y, float z);
void tgl_translate(TGLMat translate, float x, float y, float z);

float tgl_sqr(const float val);
float tgl_mag3(const float vec[3]);
float tgl_magsqr3(const float vec[3]);
float tgl_dot3(const float vec1[3], const float vec2[3]);
float tgl_dot4(const float vec1[4], const float vec2[4]);
float tgl_dot43(const float vec1[4], const float vec2[3]);

void tgl_add3s(const float vec1[3], const float summand, float res[3]);
void tgl_sub3s(const float vec1[3], const float subtrahend, float res[3]);

void tgl_add3v(const float vec1[3], const float vec2[3], float res[3]);
void tgl_mul3v(const float vec1[3], const float vec2[3], float res[3]);
void tgl_inv3(const float vec[3], float res[3]);

void tgl_norm3(float vec[3]);

void tgl_mulmatvec(const TGLMat mat, const TGLVec3 vec, TGLVec4 res);
void tgl_mulmat(const TGLMat mat1, const TGLMat mat2, TGLMat res);

#endif /* ~TERMGL_MINIMAL */

void tgl_mul3s(const float vec[3], const float mul, float res[3]);
void tgl_sub3v(const float vec1[3], const float vec2[3], float res[3]);
void tgl_cross(const float vec1[3], const float vec2[3], float res[3]);

/**
 * Sets which face should be culled. Requires tgl_enable(TGL_CULL_FACE) for faces to be culled
 * @param settings: bitwise combination of:
 *   TGL_BACK OR TGL_FRONT - face to cull
 *   TGL_CW OR TGL_CCW - winding order of triangles
 */
void tgl_cull_face(TGL *tgl, uint8_t settings);

/**
 * Renders triangle onto framebuffer
 */
void tgl_triangle_3d(TGL *tgl, const TGLTriangle in, const uint8_t (*uv)[2], bool fill, TGLVertexShader *vert_shader, const void *vert_data, TGLPixelShader *frag_shader, const void *frag_data);

#endif /* TERMGL3D */

#ifdef TERMGLUTIL

#include <stddef.h>

#ifdef __unix__
#include <unistd.h>
#define TGL_SSIZE_T ssize_t
#elif defined(TGL_OS_WINDOWS)
#define TGL_SSIZE_T SSIZE_T
#else
#error "TermGLUtil is only supported on *NIX and Windows."
#endif

/**
 * Reads up to count bytes from raw terminal input into buf
 * @return number of bytes read on success, negative value on failure
 * On failure, errno is set to value specified by:
 *   UNIX:
 *     -1: https://man7.org/linux/man-pages/man2/read.2.html#ERRORS
 *     -2: https://www.man7.org/linux/man-pages/man3/tcgetattr.3p.html#ERRORS
 *     -3: https://www.man7.org/linux/man-pages/man3/tcsetattr.3p.html#ERRORS
 *     -4: https://www.man7.org/linux/man-pages/man3/tcflush.3p.html#ERRORS
 *   Windows: https://docs.microsoft.com/en-us/windows/win32/debug/system-error-codes
 */
TGL_SSIZE_T tglutil_read(char *buf, size_t count);

/**
 * Stores number of console columns and rows in *col and *row respectively
 * @param screen_buffer: true for size of screen buffer, false for size of window. On UNIX, value is ignored and assumed true.
 * @return 0 on success, -1 on failure
 * On failure, errno is set to value specified by:
 *   UNIX: https://man7.org/linux/man-pages/man2/ioctl.2.html#ERRORS
 *   Windows: https://docs.microsoft.com/en-us/windows/win32/debug/system-error-codes
 */
int tglutil_get_console_size(unsigned *col, unsigned *row, bool screen_buffer);

/**
 * Sets console size
 * Only changes printable area and will not change window size if new size is larger than window
 * @return 0 on success, -1 on failure
 * On failure, errno is set to value specified by:
 *   UNIX: https://man7.org/linux/man-pages/man2/ioctl.2.html#ERRORS
 *   Windows: https://docs.microsoft.com/en-us/windows/win32/debug/system-error-codes
 */
int tglutil_set_console_size(unsigned col, unsigned row);

#endif /* TERMGLUTIL */

#ifdef __cplusplus
}
#endif

#endif /* TERMGL_H */
