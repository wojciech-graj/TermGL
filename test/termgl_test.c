#include "../src/termgl.h"
#include "../src/termgl3d.h"
#include "../src/termgl_vecmath.h"

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct  STLTriangle {
	float normal[3]; //normal is unreliable so it is not used.
	float vertices[3][3];
	uint16_t attribute_bytes; //attribute bytes is unreliable so it is not used.
} __attribute__ ((packed)) STLTriangle;

void intermediate_shader(TGLTriangle *trig, void *data)
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

int main(void)
{
	TGL *tgl = tgl_init(120, 120, &gradient_min);
	tgl_enable(tgl, TGL_DOUBLE_CHARS);
	tgl3d_init(tgl);

	TGLTriangle *trigs;
	FILE *stl_file = fopen("utah_teapot.stl", "rb");
	assert(stl_file);
	uint32_t n_trigs = stl_load(stl_file, &trigs);
	fclose(stl_file);

	tgl_enable(tgl, TGL_CULL_FACE);
	tgl3d_cull_face(tgl, TGL_BACK | TGL_CCW);

	tgl_enable(tgl, TGL_Z_BUFFER);

	TGLTransform *t = tgl3d_get_transform(tgl);
	tgl3d_transform_scale(t, 0.1f, 0.1f, 0.1f);
	tgl3d_transform_translate(t, 0.f, 0.f, 2.f);
	tgl3d_transform_rotate(t, 2.1f, 0.f, 0.f);
	tgl3d_transform_update(t);

	tgl3d_camera(tgl, 1.57f, 1.f, 5.f);
	tgl3d_projection_update(tgl);

	TGLTransform trans;
	tgl3d_transform_scale(&trans, 1.f, 1.f, 1.f);
	tgl3d_transform_translate(&trans, 0.f, 0.f, 0.f);

	float n = 0;
	while (1) {
		tgl3d_transform_rotate(&trans, 0.f, 0.f, n);
		tgl3d_transform_update(&trans);

		unsigned i;
		for (i = 0; i < n_trigs; i++) {
			TGLTriangle temp;
			tgl3d_transform_apply(&trans, trigs[i].vertices, temp.vertices);
			memcpy(temp.intensity, trigs[i].intensity, 3);
			tgl3d_shader(tgl, &temp, TGL_WHITE, true, &temp, &intermediate_shader);
		}

		tgl_flush(tgl);
		tgl_clear(tgl, TGL_FRAME_BUFFER | TGL_Z_BUFFER);

		n += 0.03f;
		usleep(100000);
	}

	tgl_delete(tgl);
}
