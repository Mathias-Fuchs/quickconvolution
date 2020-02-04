#include "box.h"
#include "staticHelpers.h"
#include <stdio.h>
#include <math.h>
box* box_create(
		float lx,
		float ux,
		float ly,
		float uy,
		float lz,
		float uz) {
  box* b = malloc(sizeof(box));
  b->lx =  lx;
  b->ux =  ux;
  b->ly =  ly;
  b->uy =  uy;
  b->lz =  lz;
  b->uz =  uz;
  return b;
}

box* box_duplicate(box* temp) {
	box* b = malloc(sizeof(box));
	b->lx = temp->lx;
	b->ux = temp->ux;
	b->ly = temp->ly;
	b->uy = temp->uy;
	b->lz = temp->lz;
	b->uz = temp->uz;
	return b;
}




box* box_create_from_data3d(float* a, int nrow) {
  box* b = malloc(sizeof(box));

  float lx=a[0];
  float ux =a[0];
  float ly=a[1];
  float uy =a[1];
  float lz=a[2];
  float uz =a[2];

  for (int x = 0; x < nrow; x++)
    {
      float xx=a[3*x+0];
      float yy=a[3*x+1];
      float zz=a[3*x+2];
      if (lx > xx) lx=xx;
      if (ux < xx) ux=xx;
      if (ly > yy) ly=yy;
      if (uy < yy) uy=yy;
      if (lz > zz) lz=zz;
      if (uz < zz) uz=zz;
    }

  float ex=ux-lx;
  float ey=uy-ly;
  float ez=uz-lz;
  float max=ex;
  
  if (ey > max) max=ey;
  if (ez > max) max=ez;

  float mx=(ux+lx)/2.f;
  float my=(uy+ly)/2.f;
  float mz=(uz+lz)/2.f;
  
  b->lx = mx-max/2.f;
  b->ux = mx+max/2.f;
  b->ly = my-max/2.f;
  b->uy = my+max/2.f;
  b->lz = mz-max/2.f+max/2.f;
  b->uz = mz+max/2.f+max/2.f;

  return b;
}

void box_print(box* b) {
  printf("%f %f, %f %f, %f %f\n", b->lx, b->ux,b->ly,b->uy,b->lz,b->uz);
}

box* box_create_from_data2d(float* a, int nrow) {
  box* b = malloc(sizeof(box));
  float lx=a[0];
  float ux =a[0];
  float ly=a[1];
  float uy =a[1];

  for (int x = 0; x < nrow; x++)
    {
      float xx=a[2*x+0];
      float yy=a[2*x+1];
      if (lx > xx) lx=xx;
      if (ux < xx) ux=xx;
      if (ly > yy) ly=yy;
      if (uy < yy) uy=yy;
    }
  
  float ex = ux - lx;
  float ey = uy - ly;
  float max = ex;

  if (ey > max) max = ey;

  float mx = (ux + lx) / 2.f;
  float my = (uy + ly) / 2.f;

  b->lx = mx - max / 2.f;
  b->ux = mx + max / 2.f;
  b->ly = my - max / 2.f;
  b->uy = my + max / 2.f;

  return b;
}

void box_flip_along_y(box* b, float* x) {
  *x = b->lx + b->ux - *x;
}

void box_flip_along_x(box* b, float* y) {
  *y = b->ly + b->uy - *y;
}



void box_stretch_x(box*b, float* x, float alpha) {
	float m = (b->ux + b->lx) / 2.f;
	*x = m + alpha * (*x - m);
}

void box_stretch_y(box*b, float* y, float alpha) {
	float m = (b->uy + b->ly) / 2.f;
	*y = m + alpha * (*y - m);
}

void box_shift_x_by_factor(box* b, float* x, float g) {
  (*x) += g * (b->ux - b->lx);
}

void box_shift_y_by_factor(box* b, float* y, float g) {
  (*y) += g * (b->uy - b->ly);
}




// maps the world coordinates to other world coordinates such that the boxes correspond to each other.
void box_map_one_into_another(box* b1, box* b2, float x, float y, float z, float* xw, float* yw, float* zw) {
	*xw = b2->lx + (b2->ux - b2->lx) * (x - b1->lx) / (b1->ux - b1->lx);
	*yw = b2->ly + (b2->uy - b2->ly) * (y - b1->ly) / (b1->uy - b1->ly);
	*zw = b2->lz + (b2->uz - b2->lz) * (z - b1->lz) / (b1->uz - b1->lz);
}

void box_free(box* b) {free(b);}

/*
  im create polygon wird ein Pixel i, j im outline image mit einem Punkt
  box_texture_world(self->box, self->sidelength, i, j, &p.x, &p.y);
  der Walls-Daten verglichen.

  in der oc walls wird ein Punkt der Wall direkt mithilfe von
  box_texture(wallsBox, n, x, y, &i0, &j0);
  in einen Index umgewandelt.

Ein Punkt mit x-Koordinate x kriegt den Index
2.f * (worldx - b->lx) / (b->ux - b->lx) - 1.f
*/

inline void box_world_gl(box* b, float worldx, float worldy, float worldz, float* x, float*y, float* z) {
  *x = 2.f * (worldx - b->lx) / (b->ux - b->lx) - 1.f;
  *y = 2.f * (worldy - b->ly) / (b->uy - b->ly) - 1.f;
  *z = 2.f * (worldz - b->lz) / (b->uz - b->lz) - 1.f;
}
inline void box_world_texture(box* b, int n, float worldx, float worldy, int* i, int* j) {
  *i = (int) ((float) n * (worldx - b->lx) / (b->ux - b->lx));
  *j = (int) ((float) n * (worldy - b->ly) / (b->uy - b->ly));
  *i = positive_modulo(*i, n);
  *j = positive_modulo(*j, n);
};
inline void box_gl_world(box* b, float glx, float gly, float glz, float* worldx, float* worldy, float* worldz){
  *worldx = b->lx + (b->ux - b->lx) * (glx + 1.f) / 2.f;
  *worldy = b->ly + (b->uy - b->ly) * (gly + 1.f) / 2.f;
  *worldz = b->lz + (b->uz - b->lz) * (glz + 1.f) / 2.f;
};
inline void box_gl_texture(int n, float glx, float gly, float glz, int* i, int* j){
  *i = (int) ((float) n * (glx + 1.f) / 2.f);
  *j = (int) ((float) n * (gly + 1.f) / 2.f);
  *i = positive_modulo(*i, n);
  *j = positive_modulo(*j, n);
};
inline void box_texture_gl(int n, int i, int j, float* glx, float* gly) {
  i = positive_modulo(i, n);
  j = positive_modulo(j, n);
  *glx = 2.f * (float) i / (float) n - 1.f;
  *gly = 2.f * (float) j / (float) n - 1.f;
}
inline void box_texture_world(box* b, int n, int i, int j, float* worldx, float* worldy) {
  *worldx = b->lx + (b->ux - b->lx) * (float) i / (float) n;
  *worldy = b->ly + (b->uy - b->ly) * (float) j / (float) n;
}
void box_mean(box* b, float* x, float*y, float* z) {
  *x = (b->ux + b->lx) / 2.f;
  *y = (b->uy + b->ly) / 2.f;
  *z = (b->uz + b->lz) / 2.f;
}

void boxEnlarge(box* dest, box* summand) {
	dest->lx = MIN(dest->lx, summand->lx);
	dest->ux = MAX(dest->ux, summand->ux);

	dest->ly = MIN(dest->ly, summand->ly);
	dest->uy = MAX(dest->uy, summand->uy);

	dest->lz = MIN(dest->lz, summand->lz);
	dest->uz = MAX(dest->uz, summand->uz);


}
