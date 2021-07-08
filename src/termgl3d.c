#include "termgl3d.h"
#include "termgl_intern.h"
#include "termgl_vecmath.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct TGL3D {
	ubyte settings;
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

const TGLVec3 clip_plane_normals[6] = {
	[CLIP_NEAR] = {0.f, 0.f, -1.f},
	[CLIP_FAR] = {0.f, 0.f, 1.f},
	[CLIP_LEFT] = {1.f, 0.f, 0.f},
	[CLIP_RIGHT] = {-1.f, 0.f, 0.f},
	[CLIP_BOTTOM] = {0.f, 1.f, 0.f},
	[CLIP_TOP] = {0.f, -1.f, 0.f},
};

void tgl_mulmatvec(TGLMat mat, const TGLVec3 vec, TGLVec3 res)
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

void tgl_mulmat(TGLMat mat1, TGLMat mat2, TGLMat res)
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

float tgl_distance_point_plane(const TGLVec3 normal, TGLVec3 point)
{
	return tgl_dot3(normal, point) + 1.f;
}

void tgl3d_init(TGL *tgl)
{
	TGL3D *tgl3d = TGL_MALLOC(sizeof(TGL3D));

	tgl3d->aspect_ratio = tgl->height / (float)tgl->width;
	tgl3d->half_width = tgl->width / 2.f;
	tgl3d->half_height = tgl->height / 2.f;

	tgl->tgl3d = tgl3d;
}

void tgl3d_camera(TGL *tgl, float fov, float near, float far)
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
	tgl_mulmat(transform->translate, transform->scale, temp);
	tgl_mulmat(temp, transform->rotate, transform->result);
}

void tgl3d_transform_apply(TGLTransform *transform, TGLVec3 in[3], TGLVec3 out[3])
{
	tgl_mulmatvec(transform->result, in[0], out[0]);
	tgl_mulmatvec(transform->result, in[1], out[1]);
	tgl_mulmatvec(transform->result, in[2], out[2]);
}

float tgl_line_intersect_plane(const TGLVec3 normal, TGLVec3 start, TGLVec3 end, TGLVec3 point)
{
	TGLVec3 line_vec;
	tgl_sub3v(start, end, line_vec);
	float distance = -(tgl_dot3(normal, start) + 1.f) / (tgl_dot3(normal, line_vec));
	tgl_mul3s(line_vec, distance, point);
	tgl_add3v(point, start, point);
	return distance;
}

unsigned clip_triangle_plane(const TGLVec3 normal, TGLTriangle *in, TGLTriangle out[2])
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
		//TODO: fix this
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

void tgl3d_shader(TGL *tgl, TGLTriangle *in, ubyte color, bool fill, void *data, void (*intermediate_shader)(TGLTriangle*, void*))
{
	TGL3D *tgl3d = tgl->tgl3d;

	// VERTEX SHADER
	TGLTriangle t, out;

	tgl_mulmatvec(tgl3d->transform.result, in->vertices[0], t.vertices[0]);
	tgl_mulmatvec(tgl3d->transform.result, in->vertices[1], t.vertices[1]);
	tgl_mulmatvec(tgl3d->transform.result, in->vertices[2], t.vertices[2]);

	//TODO: clipping with the near plane should be done before projection, but this solution works well enough for now
	if (t.vertices[0][2] < 1e-6f
		|| t.vertices[1][2] < 1e-6f
		|| t.vertices[2][2] < 1e-6f)
		return;

	tgl_mulmatvec(tgl3d->projection, t.vertices[0], out.vertices[0]);
	tgl_mulmatvec(tgl3d->projection, t.vertices[1], out.vertices[1]);
	tgl_mulmatvec(tgl3d->projection, t.vertices[2], out.vertices[2]);

	memcpy(out.intensity, in->intensity, sizeof(ubyte) * 3);

	if (tgl->settings & TGL_CULL_FACE) {
		TGLVec3 ab, ac, cp;
		tgl_sub3v(out.vertices[1], out.vertices[0], ab);
		tgl_sub3v(out.vertices[2], out.vertices[0], ac);
		tgl_cross(ab, ac, cp);
		if (XOR(tgl3d->settings & TGL_CULL_BIT, signbit(cp[2])))
			return;
	}

	TGLTriangle trig_buffer[127]; //the size of this buffer assumes a pathological case which is probably impossible
	memcpy(&trig_buffer[0], &out, sizeof(TGLTriangle));
	unsigned buffer_offset = 0;
	unsigned n_cur_stage = 1;
	unsigned p, i;
	for (p = 0; p < 6; p++) {
		unsigned n_next_stage = 0;
		for (i = 0; i < n_cur_stage; i++)
			n_next_stage += clip_triangle_plane(clip_plane_normals[p], &trig_buffer[i + buffer_offset], &trig_buffer[buffer_offset + n_cur_stage + n_next_stage]);
		buffer_offset += n_cur_stage;
		n_cur_stage = n_next_stage;
	}

	for (i = 0; i < n_cur_stage; i++) {
		TGLTriangle *trig = &trig_buffer[i + buffer_offset];

		//INTERMEDIATE SHADER
		if (intermediate_shader)
			intermediate_shader(trig, data);

		//FRAGMENT SHADER
		if (fill)
			tgl_triangle_fill(tgl,
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
		else
			tgl_triangle(tgl,
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

void tgl3d_cull_face(TGL *tgl, ubyte settings)
{
	tgl->tgl3d->settings = (tgl->tgl3d->settings & ~TGL_CULL_BIT) | (XOR(settings & TGL_CULL_FACE_BIT, settings & TGL_WINDING_BIT) ? TGL_CULL_BIT : 0);
}

TGLTransform *tgl3d_get_transform(TGL *tgl)
{
	return &tgl->tgl3d->transform;
}
