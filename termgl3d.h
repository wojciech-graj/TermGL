#ifndef TERMGL3D_H
#define TERMGL3D_H

#include "termgl.h"

typedef float TGLMat[4][4];
typedef float TGLVec3[3];

#define TGL_CULL_FACE 0x01

#define TGL_BACK 0x00
#define TGL_FRONT 0x01

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

void tgl3d_init(TGL *tgl);
void tgl3d_camera(TGL *tgl, float fov, float near, float far);
void tgl3d_rotate(TGL *tgl, float x, float y, float z);
void tgl3d_scale(TGL *tgl, float x, float y, float z);
void tgl3d_translate(TGL *tgl, float x, float y, float z);
void tgl3d_calculate_matrix(TGL *tgl);
void tgl3d_point(TGL *tgl, float x, float y, float z, ubyte i, ubyte color);
void tgl3d_triangle(TGL *tgl, TGLVec3 points[3], ubyte intensity[3], const ubyte color);
void tgl3d_cull_face(TGL *tgl, ubyte settings);

#endif
