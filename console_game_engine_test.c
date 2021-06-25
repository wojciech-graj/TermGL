#include "console_game_engine.h"

int main(void)
{
	TGE *tge = tge_init(50, 50, &gradient_min);
	tge_begin(tge, TGE_POINTS);
	unsigned x, y;
	for (x = 0; x < 50; x++)
		for (y = 0; y < 50; y++)
			tge_vertex(tge, x, y, x * 5, y / 7u);
	tge_end(tge);
	tge_flush(tge);
	tge_delete(tge);
}
