#ifndef CONSOLE_GAME_ENGINE_H
#define CONSOLE_GAME_ENGINE_H

#define CLEAR_SCREEN puts("\033[1;1H\033[2J")

typedef unsigned char ubyte;

typedef struct TGE TGE;
typedef struct Gradient Gradient;

extern const Gradient gradient_full;
extern const Gradient gradient_min;

enum colors {
	TGE_BLACK = 0x00,
	TGE_RED = 0x01,
	TGE_GREEN = 0x02,
	TGE_YELLOW = 0x03,
	TGE_BLUE = 0x04,
	TGE_PURPLE = 0x05,
	TGE_CYAN = 0x06,
	TGE_WHITE = 0x07,
	TGE_BLACK_BKG = 0x00,
	TGE_RED_BKG = 0x10,
	TGE_GREEN_BKG = 0x20,
	TGE_YELLOW_BKG = 0x30,
	TGE_BLUE_BKG = 0x40,
	TGE_PURPLE_BKG = 0x50,
	TGE_CYAN_BKG = 0x60,
	TGE_WHITE_BKG = 0x70,
};

enum draw_modes {
	TGE_LINE = 0,
	TGE_FILL = 1,
};

TGE *tge_init(unsigned width, unsigned height, const Gradient *gradient);
void tge_delete(TGE *tge);
void tge_flush(TGE *tge);
void tge_point(TGE *tge, int x, int y, ubyte i, ubyte color);
void tge_line(TGE *tge, int x0, int y0, ubyte i0, int x1, int y1, ubyte i1, ubyte color);

#endif
