#ifndef TERMGL3D_H
#define TERMGL3D_H

#include "termgl.h"

typedef float TGLMat[4][4];
typedef float TGLVec3[3];
typedef struct TGLTransform TGLTransform;

#define TGL_CULL_FACE 0x01

#define TGL_BACK  0x00
#define TGL_FRONT 0x01

#define TGL_CW  0x00
#define TGL_CCW 0x02

void tgl3d_transform_rotate(TGLTransform *transform, float x, float y, float z);
void tgl3d_transform_scale(TGLTransform *transform, float x, float y, float z);
void tgl3d_transform_translate(TGLTransform *transform, float x, float y, float z);
void tgl3d_transform_update(TGLTransform *transform);
void tgl3d_transform_apply(TGLTransform *transform, TGLVec3 in[3], TGLVec3 out[3]);

void tgl3d_init(TGL *tgl);
void tgl3d_camera(TGL *tgl, float fov, float near, float far);
TGLTransform *tgl3d_get_transform(TGL *tgl);
void tgl3d_projection_update(TGL *tgl);

void tgl3d_cull_face(TGL *tgl, ubyte settings);

void tgl3d_shader(TGL *tgl, TGLVec3 in[3], ubyte intensity_in[3], ubyte color, void (*intermediate_shader)(TGLVec3[3], TGLVec3[3], ubyte[3], ubyte[3]));

#endif
