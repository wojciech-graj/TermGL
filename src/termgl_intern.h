#ifndef TERMGL_INTERN_H
#define TERMGL_INTERN_H

/**
 * Internal TermGL header. Not for external use.
 */

#include <stdbool.h>

typedef struct Pixel Pixel;
typedef struct TGL3D TGL3D;
typedef struct TGL {
	unsigned width;
	unsigned height;
	int max_x;
	int max_y;
	unsigned frame_size;
	Pixel *frame_buffer;
	float *z_buffer;
	bool z_buffer_enabled;
	ubyte settings;
	const Gradient *gradient;
	TGL3D *tgl3d;
} TGL;

#define SWAP(a, b) do {TGL_TYPEOF(a) temp = a; a = b; b = temp;} while(0)
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define XOR(a, b) (((bool)(a))!=((bool)(b)))

#endif
