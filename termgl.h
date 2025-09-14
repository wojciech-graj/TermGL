/*
 * Copyright (c) 2021-2025 Wojciech Graj
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
#define TGL_VERSION_MINOR 6

#include <stdbool.h>
#include <stdint.h>

#if defined(_WIN32) || defined(WIN32)
#define TGL_OS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

enum /* indexed colors */ {
	TGL_BLACK = 0x00,
	TGL_RED = 0x01,
	TGL_GREEN = 0x02,
	TGL_YELLOW = 0x03,
	TGL_BLUE = 0x04,
	TGL_PURPLE = 0x05,
	TGL_CYAN = 0x06,
	TGL_WHITE = 0x07,
	TGL_HIGH_INTENSITY = 0x08,
};

enum /* TGLFmt flags */ {
	TGL_NONE = 0x00,
	TGL_RGB24 = 0x01,
	/* only checked in fg */
	TGL_BOLD = 0x10, /* Often equivalent to TGL_HIGH_INTENSITY */
	TGL_UNDERLINE = 0x20,
};

enum {
	/* buffers */
	TGL_FRAME_BUFFER = 0x01,
	TGL_OUTPUT_BUFFER = 0x02,
	TGL_Z_BUFFER = 0x04,
	/* settings */
	TGL_DOUBLE_WIDTH = 0x08,
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
 * Vertex data passed into 2D drawing functions
 */
typedef struct TGLVert {
	int x;
	int y;
	float z; /**< depth */
	uint8_t u; /**< u passed into TGLPixelShader */
	uint8_t v; /**< v passed into TGLPixelShader */
} TGLVert;

typedef struct TGLRGB {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} TGLRGB;

typedef union TGLFmtColor {
	TGLRGB rgb;
	uint8_t indexed;
} TGLFmtColor;

typedef struct TGLFmt {
	uint8_t flags;
	union TGLFmtColor color;
} TGLFmt;

typedef struct TGLPixFmt {
	TGLFmt fg;
	TGLFmt bkg;
} TGLPixFmt;

/**
 * @param fg: TGLFmt
 * @param [bkg]: TGLFmt
 */
#define TGL_PIXFMT(...)                                                                            \
	TGL_GET_MACRO4(__VA_ARGS__, _4, _3, TGL_PIXFMT2, TGL_PIXFMT1, UNUSED)                      \
	(__VA_ARGS__)

/**
 * @param color: uint8_t
 * @param [flags]: uint8_t
 */
#define TGL_IDX(...)                                                                               \
	TGL_GET_MACRO4(__VA_ARGS__, _4, _3, TGL_IDX2, TGL_IDX1, UNUSED)                            \
	(__VA_ARGS__)

/**
 * @param r: uint8_t
 * @param g: uint8_t
 * @param b: uint8_t
 * @param [flags]: uint8_t
 */
#define TGL_RGB(...)                                                                               \
	TGL_GET_MACRO4(__VA_ARGS__, TGL_RGB4, TGL_RGB3, _2, _1, UNUSED)                            \
	(__VA_ARGS__)

/**
 * Pixel shader that is called for each pixel in draw functions
 */
typedef void TGLPixelShader(uint8_t u, uint8_t v, TGLPixFmt *color, char *c, const void *data);

#ifndef TERMGL_MINIMAL

/**
 * Gradient of characters from dark to bright
 */
typedef struct TGLGradient {
	unsigned length;
	const char *grad;
} TGLGradient;

typedef struct TGLPixelShaderSimple {
	TGLPixFmt color;
	const TGLGradient *grad;
} TGLPixelShaderSimple;

typedef struct TGLPixelShaderTexture {
	uint8_t width;
	uint8_t height;
	const char *chars;
	const TGLPixFmt *colors;
} TGLPixelShaderTexture;

extern const TGLGradient tgl_gradient_full;
extern const TGLGradient tgl_gradient_min;

/**
 * Pixel shader that maps u+v onto a TGLGradient and has fixed color
 * @param data (TGLPixelShaderSimple *)
 */
void tgl_pixel_shader_simple(uint8_t u, uint8_t v, TGLPixFmt *color, char *c, const void *data);

/**
 * Pixel shader that maps (u,v) onto a texture
 * @param data (TGLPixelShaderTexture *)
 */
void tgl_pixel_shader_texture(uint8_t u, uint8_t v, TGLPixFmt *color, char *c, const void *data);

/**
 * Gets a gradient's character corresponding to an intensity (i.e. u or v value)
 */
char tgl_grad_char(const TGLGradient *grad, uint8_t intensity);

#endif /* ~TERMGL_MINIMAL */

/**
 * Initializes the terminal emulator.
 * Must be called at the start of the program before any other TermGL functions
 * @return: 0 on success, -1 on failure
 * On failure, errno is set to value specified by:
 *   UNIX: CANNOT FAIL
 *   Windows: https://docs.microsoft.com/en-us/windows/win32/debug/system-error-codes
 */
int tgl_boot(void);

/**
 * Initializes a TGL struct which must be passed to all functions as context
 * @param gradient: pointer to a gradient struct which holds characters which will be used when rendering. TGLGradients provided by default are gradient_min and gradient_full
 * @return: pointer to a TGL struct, NULL on failure
 * On failure, errno is set to value specified by: https://www.man7.org/linux/man-pages/man3/malloc.3.html#ERRORS
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
 *   TGL_OUTPUT_BUFFER - output buffer
 */
void tgl_clear(TGL *tgl, uint8_t buffers);

/**
 * Clears the screen
 * @return 0 on success, -1 on failure
 * On failure, errno is set to value specified by: https://man7.org/linux/man-pages/man3/fputc.3p.html#ERRORS
 */
int tgl_clear_screen(void);

/**
 * Enables or disables certain settings
 * @param settings: bitwise combination of settings:
 *   TGL_Z_BUFFER - depth buffer
 *   TGL_DOUBLE_WIDTH - display characters at double their standard widths (Limited support from terminal emulators. Should work on Windows Terminal, XTerm, and Konsole)
 *   TGL_DOUBLE_CHARS - square pixels by printing 2 characters per pixel
 *   TGL_CULL_FACE - (3D ONLY) cull specified triangle faces
 *   TGL_OUTPUT_BUFFER - output buffer allowing for just one print to flush. Much faster on most terminals, but requires a few hundred kilobytes of memory
 *   TGL_PROGRESSIVE - Over-write previous frame. Eliminates strobing but requires call to tgl_clear_screen before drawing smaller image and after resizing terminal if terminal size was smaller than frame size
 * @return 0 on success, -1 on failure
 * On failure, errno is set to value specified by: https://www.man7.org/linux/man-pages/man3/malloc.3.html#ERRORS
 */
int tgl_enable(TGL *tgl, uint8_t settings);
void tgl_disable(TGL *tgl, uint8_t settings);

/**
 * Printing functions similar to those provided by stdio.h
 */
void tgl_putchar(TGL *tgl, int x, int y, char c, TGLPixFmt color);
void tgl_puts(TGL *tgl, int x, int y, const char *str, TGLPixFmt color);

/**
 * Drawing functions
 */
void tgl_point(TGL *tgl, TGLVert v0, TGLPixelShader *t, const void *data);
void tgl_line(TGL *tgl, TGLVert v0, TGLVert v1, TGLPixelShader *t, const void *data);
void tgl_triangle(
	TGL *tgl, TGLVert v0, TGLVert v1, TGLVert v2, TGLPixelShader *t, const void *data);
void tgl_triangle_fill(
	TGL *tgl, TGLVert v0, TGLVert v1, TGLVert v2, TGLPixelShader *t, const void *data);

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
void tgl_vertex_shader_simple(const TGLVec3 vert, TGLVec4 out, const void *data);

/**
 * Transformation matrix generation functions
 */
void tgl_camera(TGLMat camera, int width, int height, float fov, float near_val, float far_val);
void tgl_rotate(TGLMat rotate, float x, float y, float z);
void tgl_scale(TGLMat scale, float x, float y, float z);
void tgl_translate(TGLMat translate, float x, float y, float z);

float tgl_sqr(float val);
float tgl_mag3(const float vec[3]);
float tgl_magsqr3(const float vec[3]);
float tgl_dot3(const float vec1[3], const float vec2[3]);
float tgl_dot4(const float vec1[4], const float vec2[4]);
float tgl_dot43(const float vec1[4], const float vec2[3]);

void tgl_add3s(const float vec1[3], float summand, float res[3]);
void tgl_sub3s(const float vec1[3], float subtrahend, float res[3]);

void tgl_add3v(const float vec1[3], const float vec2[3], float res[3]);
void tgl_mul3v(const float vec1[3], const float vec2[3], float res[3]);
void tgl_inv3(const float vec[3], float res[3]);

void tgl_norm3(float vec[3]);

void tgl_mulmatvec(const TGLMat mat, const TGLVec3 vec, TGLVec4 res);
void tgl_mulmat(const TGLMat mat1, const TGLMat mat2, TGLMat res);

#endif /* ~TERMGL_MINIMAL */

void tgl_mul3s(const float vec[3], float mul, float res[3]);
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
void tgl_triangle_3d(TGL *tgl, const TGLTriangle in, const uint8_t (*uv)[2], bool fill,
	TGLVertexShader *vert_shader, const void *vert_data, TGLPixelShader *frag_shader,
	const void *frag_data);

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

enum /* mouse button */ {
	TGL_MOUSE_UNKNOWN = 0x01, /* Assume button state didn't change */
	TGL_MOUSE_RELEASE = 0x02, /* At least 1 button released. It is unknown which one */
	TGL_MOUSE_1 = 0x04, /* Left */
	TGL_MOUSE_2 = 0x08, /* Right */
	TGL_MOUSE_3 = 0x10, /* Middle */
	TGL_MOUSE_WHEEL_OR_MOVEMENT = 0x20,
};

typedef struct TGLMouseEvent {
	/**
	 * ONE of TGL_MOUSE_UNKNOWN, TGL_MOUSE_RELEASE, TGL_MOUSE_(1/2/3)
	 *   optionally OR'd with TGL_MOUSE_WHEEL_OR_MOVEMENT
	 */
	uint8_t button;
	uint8_t x; /* Coordinates might have constant offset */
	uint8_t y;
} TGLMouseEvent;

/**
 * Reads up to count bytes from raw terminal input into buf and optionally reads count_events mouse events
 * If mouse tracking is enabled but event_buf==NULL, buf may contain Xterm control sequences
 * If mouse tracking is enabled, ensure sizeof(buf) >= count_events * 6
 * @param event_buf: Pass NULL if mouse tracking is disabled
 * @param count_events: Length of event_buf
 * @param count_read_events: Gets set to number of mouse events read
 * @return number of bytes read on success, negative value on failure
 * On failure, errno is set to value specified by:
 *   UNIX:
 *     -1: https://man7.org/linux/man-pages/man2/read.2.html#ERRORS
 *     -2: https://www.man7.org/linux/man-pages/man3/tcgetattr.3p.html#ERRORS
 *     -3: https://www.man7.org/linux/man-pages/man3/tcsetattr.3p.html#ERRORS
 *     -4: https://www.man7.org/linux/man-pages/man3/tcflush.3p.html#ERRORS
 *   Windows: https://docs.microsoft.com/en-us/windows/win32/debug/system-error-codes
 */
TGL_SSIZE_T tglutil_read(char *buf, size_t count, TGLMouseEvent *event_buf, size_t count_events,
	size_t *count_read_events);

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

/**
 * Attempts to set window title
 * @return 0 on lack of errors, -1 on failure
 * On failure, errno is set to value specified by: https://man7.org/linux/man-pages/man3/fputc.3p.html#ERRORS
 */
int tglutil_set_window_title(const char *title);

/**
 * Sets if stdin input is displayed / echoed
 * Set to false when using mouse tracking
 * @return 0 on success, negative value on failure
 * On failure, errno is set to value specified by:
 *   UNIX:
 *     -1: https://www.man7.org/linux/man-pages/man3/tcgetattr.3p.html#ERRORS
 *     -2: https://www.man7.org/linux/man-pages/man3/tcsetattr.3p.html#ERRORS
 *   Windows: https://docs.microsoft.com/en-us/windows/win32/debug/system-error-codes
 */
int tglutil_set_echo_input(bool enabled);

/**
 * Sets if mouse is tracked
 * It is recommended to call tglutil_set_echo_input(false) when using mouse tracking
 * @return 0 on success, negative value on failure
 * On failure, errno is set to value specified by:
 *   UNIX:
 *     -1: https://www.man7.org/linux/man-pages/man3/fputc.3p.html#ERRORS
 *     -2: https://www.man7.org/linux/man-pages/man3/fflush.3p.html#ERRORS
 *   Windows: https://docs.microsoft.com/en-us/windows/win32/debug/system-error-codes
 */
int tglutil_set_mouse_tracking_enabled(bool enabled);

#endif /* TERMGLUTIL */

/**
 * FOR INTERNAL USE ONLY
 */
#define TGL_GET_MACRO4(_1, _2, _3, _4, NAME, ...) NAME
#define TGL_PIXFMT1(fg_)                                                                           \
	((TGLPixFmt){                                                                              \
		.fg = (fg_),                                                                       \
		.bkg = { 0 },                                                                      \
	})
#define TGL_PIXFMT2(fg_, bkg_)                                                                     \
	((TGLPixFmt){                                                                              \
		.fg = (fg_),                                                                       \
		.bkg = (bkg_),                                                                     \
	})
#define TGL_IDX1(color_)                                                                           \
	((TGLFmt){                                                                                 \
		.color.indexed = (color_),                                                         \
	})
#define TGL_IDX2(color_, flags_)                                                                   \
	((TGLFmt){                                                                                 \
		.flags = (flags_),                                                                 \
		.color.indexed = (color_),                                                         \
	})
#define TGL_RGB3(r_, g_, b_)                                                                       \
	((TGLFmt){ .flags = TGL_RGB24,                                                             \
		.color.rgb = (TGLRGB){                                                             \
			.r = (r_),                                                                 \
			.g = (g_),                                                                 \
			.b = (b_),                                                                 \
		} })
#define TGL_RGB4(r_, g_, b_, flags_)                                                               \
	((TGLFmt){ .flags = (flags_) | TGL_RGB24,                                                  \
		.color.rgb = (TGLRGB){                                                             \
			.r = (r_),                                                                 \
			.g = (g_),                                                                 \
			.b = (b_),                                                                 \
		} })

#ifdef __cplusplus
}
#endif

#endif /* TERMGL_H */
