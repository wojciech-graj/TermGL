#include "termgl.h"

int main(void)
{
	TGL *tgl = tgl_init(50, 30, &gradient_min);
	tgl_triangle_fill(tgl, 45, 27, 0, 28, 2, 255, 3, 2, 127, TGL_WHITE);
	tgl_flush(tgl);
	tgl_delete(tgl);
}
