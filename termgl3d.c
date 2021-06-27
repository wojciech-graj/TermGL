#include "termgl3d.h"
#include "termgl_intern.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct TGLTransform {
	TGLMat rotate;
	TGLMat scale;
	TGLMat translate;
	TGLMat result;
} TGLTransform;

typedef struct TGL3D {
	ubyte settings;
	float aspect_ratio;
	float half_width;
	float half_height;
	TGLTransform transform;
	TGLMat projection;
	TGLMat result;
} TGL3D;

#define TGL_CULL_BIT 0x01
#define TGL_CULL_BIT_OFFSET 0

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

__attribute__((const))
float dot3(const TGLVec3 vec1, const TGLVec3 vec2)
{
	return vec1[0] * vec2[0] + vec1[1] * vec2[1] + vec1[2] * vec2[2];
}

__attribute__((const))
float dot43(const float vec1[4], const TGLVec3 vec2)
{
	return vec1[0] * vec2[0] + vec1[1] * vec2[1] + vec1[2] * vec2[2] + vec1[3];
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

void mulmatvec(TGLMat mat, const TGLVec3 vec, TGLVec3 res)
{
	res[0] = dot43(mat[0], vec);
	res[1] = dot43(mat[1], vec);
	res[2] = dot43(mat[2], vec);

	float w = dot43(mat[3], vec);
	if (w != 0.f) {
		w = 1.f / w;
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

	tgl3d->aspect_ratio = tgl->height / (float)tgl->width;
	tgl3d->half_width = tgl->width / 2.f;
	tgl3d->half_height = tgl->height / 2.f;

	tgl->tgl3d = tgl3d;
}

void tgl3d_camera(TGL *tgl, float fov, float near, float far)
{
	TGL3D *tgl3d = tgl->tgl3d;
	float s = 1.f / tanf(fov / 2.f);
	float a = far / (far - near);
	TGLMat projection = {
		{s * tgl3d->aspect_ratio, 0.f, 0.f, 0.f},
		{0.f, s,   0.f,     0.f},
		{0.f, 0.f, a,       1.f},
		{0.f, 0.f, -a * near, 0.f},
	};
	memcpy(tgl->tgl3d->projection, projection, sizeof(TGLMat));
}

void tgl3d_transform_rotate(TGLTransform *transform, float x, float y, float z)
{
	TGLMat rotate = ROTATION_MATRIX(x, y, z);
	memcpy(transform->rotate, rotate, sizeof(TGLMat));
}

void tgl3d_transform_scale(TGLTransform *transform, float x, float y, float z)
{
	TGLMat scale = SCALE_MATRIX(x, y, z);
	memcpy(transform->scale, scale, sizeof(TGLMat));
}

void tgl3d_transform_translate(TGLTransform *transform, float x, float y, float z)
{
	TGLMat translate = TRANSLATE_MATRIX(x, y, z);
	memcpy(transform->translate, translate, sizeof(TGLMat));
}

void tgl3d_transform_update(TGLTransform *transform)
{
	TGLMat temp;
	mulmat(transform->translate, transform->scale, temp);
	mulmat(temp, transform->rotate, transform->result);
}

void tgl3d_projection_update(TGL *tgl)
{
	TGL3D *tgl3d = tgl->tgl3d;
	mulmat(tgl3d->projection, tgl3d->transform.result, tgl3d->result);
}

void tgl3d_transform_apply(TGLTransform *transform, TGLVec3 in[3], TGLVec3 out[3])
{
	mulmatvec(transform->result, in[0], out[0]);
	mulmatvec(transform->result, in[1], out[1]);
	mulmatvec(transform->result, in[2], out[2]);
}

void tgl3d_shader(TGL *tgl, TGLVec3 vertices_in[3], ubyte intensity_in[3], ubyte color, void (*intermediate_shader)(TGLVec3[3], TGLVec3[3], ubyte[3], ubyte[3]))
{
	TGL3D *tgl3d = tgl->tgl3d;

	// VERTEX SHADER
	TGLVec3 vertices_out[3];
	mulmatvec(tgl3d->result, vertices_in[0], vertices_out[0]);
	mulmatvec(tgl3d->result, vertices_in[1], vertices_out[1]);
	mulmatvec(tgl3d->result, vertices_in[2], vertices_out[2]);

	if (tgl->settings & TGL_CULL_FACE) {
		TGLVec3 ab, ac, cp;
		sub3(vertices_out[1], vertices_out[0], ab);
		sub3(vertices_out[2], vertices_out[0], ac);
		cross(ab, ac, cp);
		if (XNOR(tgl3d->settings & TGL_CULL_BIT, signbit(cp[2])))
			return;
	}

	//INTERMEDIATE SHADER
	ubyte intensity_out[3];
	if (intermediate_shader)
		intermediate_shader(vertices_in, vertices_out, intensity_in, intensity_out);
	else
		memcpy(intensity_out, intensity_in, sizeof(ubyte) * 3);

	//FRAGMENT SHADER
	tgl_triangle(tgl,
		MAP_COORD(tgl3d->half_width, vertices_out[0][0]),
		MAP_COORD(tgl3d->half_height, vertices_out[0][1]),
		intensity_out[0],
		MAP_COORD(tgl3d->half_width, vertices_out[1][0]),
		MAP_COORD(tgl3d->half_height, vertices_out[1][1]),
		intensity_out[1],
		MAP_COORD(tgl3d->half_width, vertices_out[2][0]),
		MAP_COORD(tgl3d->half_height, vertices_out[2][1]),
		intensity_out[2],
		color);
}

void tgl3d_cull_face(TGL *tgl, ubyte settings)
{
	tgl->tgl3d->settings = (tgl->tgl3d->settings & ~TGL_CULL_BIT) | XOR(settings & TGL_CULL_FACE_BIT, settings & TGL_WINDING_BIT);
}

TGLTransform *tgl3d_get_transform(TGL *tgl)
{
	return &tgl->tgl3d->transform;
}
