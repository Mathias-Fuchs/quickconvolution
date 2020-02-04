#ifndef BOX
#define BOX
#include <stdlib.h>

typedef struct {
  float lx;
  float ux;
  float ly;
  float uy;
  float lz;
  float uz;
} box;

box* box_create(
		float lx,
		float ux,
		float ly,
		float uy,
		float lz,
		float uz);
box* box_duplicate(box* temp);
box* box_create_from_data3d(float* a, int nrow);
box* box_create_from_data2d(float* a, int nrow);
void box_print(box* b);
void box_flip_along_x(box* b, float* y);
void box_flip_along_y(box* b, float* x);
void box_free(box* b);
extern void box_world_gl(box* b, float worldx, float worldy, float worldz, float* x, float*y, float* z);
extern void box_world_texture(box* b, int n, float worldx, float worldy, int* i, int* j);
extern void box_gl_world(box* b, float glx, float gly, float glz, float* worldx, float* worldy, float* worldz);
extern void box_gl_texture(int n, float glx, float gly, float glz, int* i, int* j);
extern void box_texture_gl(int n, int i, int j, float* glx, float* gly);
extern void box_texture_world(box* b, int n, int i, int j, float* worldx, float* worldy);
extern void box_map_one_into_another(box* b1, box* b2, float x, float y, float z, float* xw, float* yw, float* zw);
extern void box_stretch_x(box*b, float* x, float alpha);
extern void box_stretch_y(box*b, float* y, float alpha);

extern void box_shift_x_by_factor(box* b, float* x, float g);
extern void box_shift_y_by_factor(box* b, float* y, float g);

void box_mean(box* b, float* x, float*y, float* z);
void boxEnlarge(box* dest, box* summand);
#endif
