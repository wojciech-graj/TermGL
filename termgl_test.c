#include "termgl.h"
#include "termgl3d.h"

#include <time.h>

int main(void)
{
	TGL *tgl = tgl_init(240, 240, &gradient_min);
	//tgl_triangle_fill(tgl, 45, 27, 0, 28, 2, 255, 3, 2, 127, TGL_WHITE);
	tgl_enable(tgl, TGL_DOUBLE_CHARS);
	tgl3d_init(tgl);
	tgl_enable(tgl, TGL_CULL_FACE);
	tgl3d_cull_face(tgl, TGL_BACK);

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

	float n = 0;
	while (1) {
		tgl3d_camera(tgl, 1.57f, 1.f, 100.f);
		tgl3d_rotate(tgl, n / 2.f, 0.f, n);
		tgl3d_scale(tgl, 0.6f, 0.6f, 0.6f);
		tgl3d_translate(tgl, 0.f, 0.f, 2.f);
		tgl3d_calculate_matrix(tgl);

		unsigned i;
		for (i = 0; i < 12; i++) {
			tgl3d_triangle(tgl, trigs[i], (ubyte[3]){255, 255, 255}, TGL_WHITE);
		}

		tgl_flush(tgl);
		tgl_clear(tgl, TGL_FRAME_BUFFER);

		n += 0.1f;
		usleep(100000);
	}

	tgl_delete(tgl);
}
