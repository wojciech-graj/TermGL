#include "termgl.h"
#include "termgl3d.h"

#include <time.h>

void intermediate_shader(TGLTriangle *in, TGLTriangle *out)
{
	out->intensity[0] /= (-out->vertices[0][2] + 3.f);
	out->intensity[1] /= (-out->vertices[1][2] + 3.f);
	out->intensity[2] /= (-out->vertices[2][2] + 3.f);
}

int main(void)
{
	TGL *tgl = tgl_init(240, 240, &gradient_min);
	tgl_enable(tgl, TGL_DOUBLE_CHARS);
	tgl3d_init(tgl);

	tgl_enable(tgl, TGL_CULL_FACE);
	tgl3d_cull_face(tgl, TGL_BACK | TGL_CW);

	TGLTriangle trigs[12] = {
		{{{0.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {1.f, 1.f, 0.f}},{255, 255, 255}},
		{{{0.f, 0.f, 0.f}, {1.f, 1.f, 0.f}, {1.f, 0.f, 0.f}},{255, 255, 255}},
		{{{1.f, 0.f, 0.f}, {1.f, 1.f, 0.f}, {1.f, 1.f, 1.f}},{255, 255, 255}},
		{{{1.f, 0.f, 0.f}, {1.f, 1.f, 1.f}, {1.f, 0.f, 1.f}},{255, 255, 255}},
		{{{1.f, 0.f, 1.f}, {1.f, 1.f, 1.f}, {0.f, 1.f, 1.f}},{255, 255, 255}},
		{{{1.f, 0.f, 1.f}, {0.f, 1.f, 1.f}, {0.f, 0.f, 1.f}},{255, 255, 255}},
		{{{0.f, 0.f, 1.f}, {0.f, 1.f, 1.f}, {0.f, 1.f, 0.f}},{255, 255, 255}},
		{{{0.f, 0.f, 1.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 0.f}},{255, 255, 255}},
		{{{0.f, 1.f, 0.f}, {0.f, 1.f, 1.f}, {1.f, 1.f, 1.f}},{255, 255, 255}},
		{{{0.f, 1.f, 0.f}, {1.f, 1.f, 1.f}, {1.f, 1.f, 0.f}},{255, 255, 255}},
		{{{1.f, 0.f, 1.f}, {0.f, 0.f, 1.f}, {0.f, 0.f, 0.f}},{255, 255, 255}},
		{{{1.f, 0.f, 1.f}, {0.f, 0.f, 0.f}, {1.f, 0.f, 0.f}},{255, 255, 255}},
	};

	TGLTransform *t = tgl3d_get_transform(tgl);
	tgl3d_transform_scale(t, 0.8f, 0.8f, 0.8f);
	tgl3d_transform_translate(t, 0.f, 0.f, 2.f);

	tgl3d_camera(tgl, 1.57f, 1.f, 5.f);

	float n = 0;
	while (1) {
		tgl3d_transform_rotate(t, n * 0.33f, n * 0.5f, n);
		tgl3d_transform_update(t);
		tgl3d_projection_update(tgl);

		unsigned i;
		for (i = 0; i < 12; i++) {
			tgl3d_shader(tgl, &trigs[i], TGL_WHITE, true, &intermediate_shader);
		}

		tgl_flush(tgl);
		tgl_clear(tgl, TGL_FRAME_BUFFER);

		n += 0.1f;
		usleep(150000);
	}

	tgl_delete(tgl);
}
