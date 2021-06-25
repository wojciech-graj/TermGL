#ifndef CONSOLE_GAME_ENGINE_H
#define CONSOLE_GAME_ENGINE_H

#define CLEAR_SCREEN puts("\033[1;1H\033[2J")

typedef unsigned char ubyte;

typedef struct TGE TGE;
typedef struct Gradient Gradient;

extern const Gradient gradient_full;
extern const Gradient gradient_min;

enum colors {
	TGE_BLACK = 0,
	TGE_RED,
	TGE_GREEN,
	TGE_YELLOW,
	TGE_BLUE,
	TGE_PURPLE,
	TGE_CYAN,
	TGE_WHITE,
	TGE_BLACK_BKG,
	TGE_RED_BKG,
	TGE_GREEN_BKG,
	TGE_YELLOW_BKG,
	TGE_BLUE_BKG,
	TGE_PURPLE_BKG,
	TGE_CYAN_BKG,
	TGE_WHITE_BKG,
};

enum draw_modes {
	TGE_LINE = 0,
	TGE_FILL = 1,
};

enum primitives {
	TGE_POINTS = 1,
};

TGE *tge_init(unsigned width, unsigned height, const Gradient *gradient);
void tge_delete(TGE *tge);
void tge_flush(TGE *tge);
void tge_begin(TGE *tge, unsigned primitive);
void tge_end(TGE *tge);
void tge_draw_mode(TGE *tge, unsigned draw_mode);
void tge_vertex(TGE *tge, unsigned x, unsigned y, ubyte intensity, ubyte color);

#endif
