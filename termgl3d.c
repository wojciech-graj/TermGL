#include "termgl3d.h"
#include "termgl_intern.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct TGL3D {
	ubyte settings;
	float width;
	float height;
	float half_width;
	float half_height;
	TGLMat rot_mat;
	TGLMat scale_mat;
	TGLMat trans_mat;
	TGLMat proj_mat;
	TGLMat mat;
} TGL3D;

#define TGL_FACE_BIT 0x01

#define MAP_COORD(half, val) ((val * half) + half)

__attribute__((const))
float dot3(const TGLVec3 vec1, const TGLVec3 vec2)
{
	return vec1[0] * vec2[0] + vec1[1] * vec2[1] + vec1[2] * vec2[2];
}

__attribute__((const))
float dot4(const TGLVec3 vec1, const TGLVec3 vec2)
{
	return vec1[0] * vec2[0] + vec1[1] * vec2[1] + vec1[2] * vec2[2] + vec1[3] * vec2[3];
}

void sub3(const TGLVec3 vec1, const TGLVec3 vec2, TGLVec3 res)
{
	res[0] = vec1[0] - vec2[0];
	res[1] = vec1[1] - vec2[1];
	res[2] = vec1[2] - vec2[2];
}

void cross(const TGLVec3 vec1, const TGLVec3 vec2, TGLVec3 result)
{
	result[0] = vec1[1] * vec2[2] - vec1[2] * vec2[1];
	result[1] = vec1[2] * vec2[0] - vec1[0] * vec2[2];
	result[2] = vec1[0] * vec2[1] - vec1[1] * vec2[0];
}

void mulmatvec(TGLMat mat, const float vec[4], float res[4])
{
	res[0] = dot4(mat[0], vec);
	res[1] = dot4(mat[1], vec);
	res[2] = dot4(mat[2], vec);
	res[3] = dot4(mat[3], vec);

	if (res[3] != 0.f) {
		float w = 1.f / res[3];
		res[0] *= w;
		res[1] *= w;
		res[2] *= w;
	}
}

void mulmat(TGLMat mat1, TGLMat mat2, TGLMat res)
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

void tgl3d_init(TGL *tgl)
{
	TGL3D *tgl3d = malloc(sizeof(TGL3D));

	tgl3d->width = tgl->width;
	tgl3d->height = tgl->height;
	tgl3d->half_width = tgl3d->width / 2.f;
	tgl3d->half_height = tgl3d->height / 2.f;

	tgl->tgl3d = tgl3d;
}

void tgl3d_camera(TGL *tgl, float fov, float near, float far)
{
	TGL3D *tgl3d = tgl->tgl3d;
	float s = 1.f / tanf(fov / 2.f);
	float a = far / (far - near);
	TGLMat proj_mat = {
		{s * tgl3d->height / tgl3d->width, 0.f, 0.f, 0.f},
		{0.f, s,   0.f,     0.f},
		{0.f, 0.f, a,       1.f},
		{0.f, 0.f, -a * near, 0.f},
	};
	memcpy(tgl->tgl3d->proj_mat, proj_mat, sizeof(TGLMat));
}

void tgl3d_rotate(TGL *tgl, float x, float y, float z)
{
	TGLMat rot_mat = ROTATION_MATRIX(x, y, z);
	memcpy(tgl->tgl3d->rot_mat, rot_mat, sizeof(TGLMat));
}

void tgl3d_scale(TGL *tgl, float x, float y, float z)
{
	TGLMat scale_mat = SCALE_MATRIX(x, y, z);
	memcpy(tgl->tgl3d->scale_mat, scale_mat, sizeof(TGLMat));
}

void tgl3d_translate(TGL *tgl, float x, float y, float z)
{
	TGLMat trans_mat = TRANSLATE_MATRIX(x, y, z);
	memcpy(tgl->tgl3d->trans_mat, trans_mat, sizeof(TGLMat));
}

void tgl3d_calculate_matrix(TGL *tgl)
{
	TGL3D *tgl3d = tgl->tgl3d;
	TGLMat res;
	//mulmat(tgl3d->rot_mat, tgl3d->scale_mat, tgl3d->mat);
	//mulmat(tgl3d->mat, tgl3d->trans_mat, res);
	//memcpy(tgl3d->mat, res, sizeof(TGLMat));
	//mulmat(res, tgl3d->proj_mat, tgl3d->mat);
	mulmat(tgl3d->proj_mat, tgl3d->trans_mat, tgl3d->mat);
	mulmat(tgl3d->mat, tgl3d->scale_mat, res);
	mulmat(res, tgl3d->rot_mat, tgl3d->mat);
}

void tgl3d_triangle(TGL *tgl, TGLVec3 points[3], ubyte intensity[3], const ubyte color)
{
	TGL3D *tgl3d = tgl->tgl3d;
	float p[3][4];

	mulmatvec(tgl3d->mat, (float[4]){points[0][0], points[0][1], points[0][2], 1.f}, p[0]);
	mulmatvec(tgl3d->mat, (float[4]){points[1][0], points[1][1], points[1][2], 1.f}, p[1]);
	mulmatvec(tgl3d->mat, (float[4]){points[2][0], points[2][1], points[2][2], 1.f}, p[2]);

	if (tgl->settings & TGL_CULL_FACE) {
		TGLVec3 ab, ac, cp;
		sub3(p[1], p[0], ab);
		sub3(p[2], p[0], ac);
		cross(ab, ac, cp);
		if ((bool)signbit(cp[2]) == (bool)(tgl3d->settings & TGL_FACE_BIT))
			return;
	}

	tgl_triangle(tgl,
		MAP_COORD(tgl3d->half_width, p[0][0]),
		MAP_COORD(tgl3d->half_height, p[0][1]),
		intensity[0] / p[0][2],
		MAP_COORD(tgl3d->half_width, p[1][0]),
		MAP_COORD(tgl3d->half_height, p[1][1]),
		intensity[1] / p[1][2],
		MAP_COORD(tgl3d->half_width, p[2][0]),
		MAP_COORD(tgl3d->half_height, p[2][1]),
		intensity[2] / p[2][2],
		color);
}

void tgl3d_cull_face(TGL *tgl, ubyte settings)
{
	tgl->tgl3d->settings = (tgl->tgl3d->settings & ~TGL_FACE_BIT) | settings;
}
