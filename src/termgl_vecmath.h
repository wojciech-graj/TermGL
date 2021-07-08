#ifndef TERMGL_VECMATH_H
#define TERMGL_VECMATH_H

/**
 * Vector math library
 */

float tgl_sqr(const float val);
float tgl_mag3(const float vec[3]);
float tgl_magsqr3(const float vec[3]);
float tgl_dot3(const float vec1[3], const float vec2[3]);
float tgl_dot43(const float vec1[4], const float vec2[3]);

void tgl_add3s(const float vec1[3], const float summand, float res[3]);
void tgl_sub3s(const float vec1[3], const float subtrahend, float res[3]);
void tgl_mul3s(const float vec[3], const float mul, float res[3]);

void tgl_add3v(const float vec1[3], const float vec2[3], float res[3]);
void tgl_sub3v(const float vec1[3], const float vec2[3], float res[3]);
void tgl_mul3v(const float vec1[3], const float vec2[3], float res[3]);
void tgl_inv3(const float vec[3], float res[3]);

void tgl_cross(const float vec1[3], const float vec2[3], float res[3]);

void tgl_norm3(float vec[3]);

#endif
