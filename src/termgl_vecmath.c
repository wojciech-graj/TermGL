#include "termgl_vecmath.h"

#include <math.h>

__attribute__((const))
float tgl_sqr(const float val)
{
	return val * val;
}

__attribute__((const))
float tgl_mag3(const float vec[3])
{
	return sqrtf(tgl_sqr(vec[0]) + tgl_sqr(vec[1]) + tgl_sqr(vec[2]));
}

__attribute__((const))
float tgl_magsqr3(const float vec[3])
{
	return tgl_sqr(vec[0]) + tgl_sqr(vec[1]) + tgl_sqr(vec[2]);
}

__attribute__((const))
float tgl_dot3(const float vec1[3], const float vec2[3])
{
	return vec1[0] * vec2[0] + vec1[1] * vec2[1] + vec1[2] * vec2[2];
}

__attribute__((const))
float tgl_dot43(const float vec1[4], const float vec2[3])
{
	return vec1[0] * vec2[0] + vec1[1] * vec2[1] + vec1[2] * vec2[2] + vec1[3];
}

void tgl_add3s(const float vec1[3], const float summand, float res[3])
{
	res[0] = vec1[0] + summand;
	res[1] = vec1[1] + summand;
	res[2] = vec1[2] + summand;
}

void tgl_sub3s(const float vec1[3], const float subtrahend, float res[3])
{
	res[0] = vec1[0] - subtrahend;
	res[1] = vec1[1] - subtrahend;
	res[2] = vec1[2] - subtrahend;
}

void tgl_mul3s(const float vec[3], const float mul, float res[3])
{
	res[0] = vec[0] * mul;
	res[1] = vec[1] * mul;
	res[2] = vec[2] * mul;
}

void tgl_add3v(const float vec1[3], const float vec2[3], float res[3])
{
	res[0] = vec1[0] + vec2[0];
	res[1] = vec1[1] + vec2[1];
	res[2] = vec1[2] + vec2[2];
}

void tgl_sub3v(const float vec1[3], const float vec2[3], float res[3])
{
	res[0] = vec1[0] - vec2[0];
	res[1] = vec1[1] - vec2[1];
	res[2] = vec1[2] - vec2[2];
}

void tgl_mul3v(const float vec1[3], const float vec2[3], float res[3])
{
	res[0] = vec1[0] * vec2[0];
	res[1] = vec1[1] * vec2[1];
	res[2] = vec1[2] * vec2[2];
}

void tgl_inv3(const float vec[3], float res[3])
{
	res[0] = 1.f / vec[0];
	res[1] = 1.f / vec[1];
	res[2] = 1.f / vec[2];
}

void tgl_cross(const float vec1[3], const float vec2[3], float res[3])
{
	res[0] = vec1[1] * vec2[2] - vec1[2] * vec2[1];
	res[1] = vec1[2] * vec2[0] - vec1[0] * vec2[2];
	res[2] = vec1[0] * vec2[1] - vec1[1] * vec2[0];
}

void tgl_norm3(float vec[3])
{
	tgl_mul3s(vec, 1.f / tgl_mag3(vec), vec);
}
