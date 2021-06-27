#include "termgl.h"
#include "termgl3d.h"

#include <time.h>

void intermediate_shader(TGLVec3 vertices_in[3], TGLVec3 vertices_out[3], ubyte intensity_in[3], ubyte intensity_out[3])
{
	(void)vertices_in;
	intensity_out[0] = intensity_in[0] / vertices_out[0][2];
	intensity_out[1] = intensity_in[1] / vertices_out[1][2];
	intensity_out[2] = intensity_in[2] / vertices_out[2][2];
}

int main(void)
{
	TGL *tgl = tgl_init(240, 240, &gradient_min);
	tgl_enable(tgl, TGL_DOUBLE_CHARS);
	tgl3d_init(tgl);

	tgl_enable(tgl, TGL_CULL_FACE);
	tgl3d_cull_face(tgl, TGL_BACK | TGL_CW);

	float trigs[12][3][3] = {
		{{0.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {1.f, 1.f, 0.f}},
		{{0.f, 0.f, 0.f}, {1.f, 1.f, 0.f}, {1.f, 0.f, 0.f}},
		{{1.f, 0.f, 0.f}, {1.f, 1.f, 0.f}, {1.f, 1.f, 1.f}},
		{{1.f, 0.f, 0.f}, {1.f, 1.f, 1.f}, {1.f, 0.f, 1.f}},
		{{1.f, 0.f, 1.f}, {1.f, 1.f, 1.f}, {0.f, 1.f, 1.f}},
		{{1.f, 0.f, 1.f}, {0.f, 1.f, 1.f}, {0.f, 0.f, 1.f}},
		{{0.f, 0.f, 1.f}, {0.f, 1.f, 1.f}, {0.f, 1.f, 0.f}},
		{{0.f, 0.f, 1.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 0.f}},
		{{0.f, 1.f, 0.f}, {0.f, 1.f, 1.f}, {1.f, 1.f, 1.f}},
		{{0.f, 1.f, 0.f}, {1.f, 1.f, 1.f}, {1.f, 1.f, 0.f}},
		{{1.f, 0.f, 1.f}, {0.f, 0.f, 1.f}, {0.f, 0.f, 0.f}},
		{{1.f, 0.f, 1.f}, {0.f, 0.f, 0.f}, {1.f, 0.f, 0.f}},
	};

	TGLTransform *t = tgl3d_get_transform(tgl);
	tgl3d_transform_scale(t, 0.6f, 0.6f, 0.6f);
	tgl3d_transform_translate(t, 0.f, 0.f, 2.f);

	tgl3d_camera(tgl, 1.57f, 1.f, 100.f);

	float n = 0;
	while (1) {
		tgl3d_transform_rotate(t, n * .33f, n * .5f, n);
		tgl3d_transform_update(t);
		tgl3d_projection_update(tgl);

		unsigned i;
		for (i = 0; i < 12; i++) {
			tgl3d_shader(tgl, trigs[i], (ubyte[3]){255, 255, 255}, TGL_WHITE, &intermediate_shader);
		}

		tgl_flush(tgl);
		tgl_clear(tgl, TGL_FRAME_BUFFER);

		n += 0.1f;
		usleep(150000);
	}

	tgl_delete(tgl);
}
