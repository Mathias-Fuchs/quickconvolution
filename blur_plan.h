#ifndef BLURPLAN
#define BLURPLAN
#include <fftw3.h>


#ifndef __EMSCRIPTEN__

#define FFTCOMPLEX fftwf_complex
#define FFTPLAN fftwf_plan

#else

#define FFTCOMPLEX fftw_complex
#define FFTPLAN fftw_plan

#endif




// define it in the c file
typedef struct {
  int rank;
  int* sizes;
  int dataLength;
  float bandwidth;
  float* data;
  //  fftw_plan jjs;
  FFTPLAN planForward;
  FFTCOMPLEX* frequencies;
  FFTCOMPLEX* multiplier;
  FFTPLAN planBackward;
} blur_in_place_plan;

blur_in_place_plan* create_blur_plan_quadratic_2d(int n, float bw);
blur_in_place_plan *create_blur_plan_1d(int n, float bw);
void blur_plan_update_bandwidth_quadratic_2d(blur_in_place_plan* k, float bw);
void blur_plan_update_bandwidth_1d(blur_in_place_plan* k, float bw);
void blur_plan_execute(blur_in_place_plan* bp);
float* blur_plan_data(blur_in_place_plan* p);
blur_in_place_plan* duplicate_blur_plan_2d(blur_in_place_plan* src);
void free_blur_plan(blur_in_place_plan* bp);

/* usage: */

/* float* data; */
/* blur_plan_in_place* bp = create_blur_plan_quadratic_2d(128, 1./20.); */
/* blur_plan_get_data(bp, &data); */
/* data[10 + n * 12] = 1.f; */
/* blur2d(bp); */

#endif


#ifdef USAGEEXAMPLE
int main(void)
{
  float* data;
  blur_in_place_plan* bp = create_blur_plan_1d(10, 1.3); 
  blur_plan_get_data(bp, &data);

  data[4] = 1.f;

  blur_plan_execute(bp);
  for (int j = 0; j < 10; j++) {
    printf("%0f ", data[j]);
  }
  printf("\n");
  free_blur_plan(bp);
  free(data);
  
  int sidelength=4;
  blur_in_place_plan* bp2d = create_blur_plan_quadratic_2d(sidelength, 0.3);
  float* data2d;  blur_plan_get_data(bp2d, &data2d);
  
  data2d[0] = 1.f;
  blur_plan_execute(bp2d);
  for (int i = 0; i < sidelength; i++) {
    for (int j = 0; j < sidelength; j++) {
      printf("%0f ", data2d[i + sidelength*j]);
    }
    printf("\n");
  }
  free_blur_plan(bp2d);
  free(data2d);
  exit(0);
}

blur_plan_in_place* bp = create_blur_plan_quadratic_2d(128, 1./20.);
float* data; blur_plan_get_data(bp, &data);
data[10 + n * 12] = 1.f;
blur_plan_execute(bp2d);
// do sth with the *data
// ...
free_blur_plan(bp2d);
//etc.
#endif
