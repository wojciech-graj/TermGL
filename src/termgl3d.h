#ifndef TERMGL3D_H
#define TERMGL3D_H

/**
 * 3D library. Handles 3D Transformations and 3D rendering.
 */

#include "termgl.h"

#include <stdbool.h>

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
	ubyte intensity[3];
} TGLTriangle;

#define TGL_CULL_FACE 0x01

#define TGL_BACK  0x00
#define TGL_FRONT 0x01

#define TGL_CW  0x00
#define TGL_CCW 0x02

/**
 * Initializes 3D component of TermGL
 * @param tgl : a TGL context previously created using tgl_init()
 */
void tgl3d_init(TGL *tgl);

/**
 * Sets the camera's perspective projection matrix
 * @param fov : field of view angle in radians
 * @param near : distance to near clipping plane
 * @param far : distance to far clipping plane
 */
void tgl3d_camera(TGL *tgl, float fov, float near, float far);

/**
 * Gets the camera's TGLTransform transformation matrices which can be operated on using tgl3d_transform_... functions
 */
TGLTransform *tgl3d_get_transform(TGL *tgl);

/**
 * Sets which face should be culled. Requires tgl_enable(TGL_CULL_FACE) to be run before faces will be culled
 * @param settings : bitwise combination of :
 *   TGL_BACK OR TGL_FRONT - face to cull
 *   TGL_CW OR TGL_CCW - winding order of triangles
 */
void tgl3d_cull_face(TGL *tgl, ubyte settings);

/**
 * Renders triangle onto framebuffer
 * @param intermediate_shader : (allow NULL) pointer to a shader function which is executed after vertex shader (projection and clipping) and before fragment shader (drawing onto framebuffer). Parameters are a projected triangle from vertex shader, and optional data. See termgl_test.c for example
 * @param data : (allow NULL) data which is passed to intermediate_shader
 */
void tgl3d_shader(TGL *tgl, TGLTriangle *in, ubyte color, bool fill, void *data, void (*intermediate_shader)(TGLTriangle*, void*));

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

#endif
