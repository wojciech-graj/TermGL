#include "console_game_engine.h"

int main(void)
{
	TGE *tge = tge_init(50, 50, &gradient_min);
	int x, y;
	for (x = 0; x < 50; x++)
		for (y = 0; y < 50; y++)
			tge_point(tge, x, y, x * 5, TGE_WHITE);
	tge_line(tge, 0, 0, 0, 50, 50, 255, TGE_RED);
	tge_flush(tge);
	tge_delete(tge);
}
