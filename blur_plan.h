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
blur_in_place_plan* create_blur_plan_1d(int n, float bw);
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

	int sidelength = 4;
	blur_in_place_plan* bp2d = create_blur_plan_quadratic_2d(sidelength, 0.3);
	float* data2d;  blur_plan_get_data(bp2d, &data2d);

	data2d[0] = 1.f;
	blur_plan_execute(bp2d);
	for (int i = 0; i < sidelength; i++) {
		for (int j = 0; j < sidelength; j++) {
			printf("%0f ", data2d[i + sidelength * j]);
		}
		printf("\n");
	}
	free_blur_plan(bp2d);
	free(data2d);
	exit(0);
}

blur_plan_in_place* bp = create_blur_plan_quadratic_2d(128, 1. / 20.);
float* data; blur_plan_get_data(bp, &data);
data[10 + n * 12] = 1.f;
blur_plan_execute(bp2d);
// do sth with the *data
// ...
free_blur_plan(bp2d);
//etc.
#endif


#ifdef BLUR_PLAN_IMPLEMENTATION

#include <stdlib.h>
#include <stdio.h>
#include <fftw3.h>
#include <math.h>
#include <string.h>

#ifndef __EMSCRIPTEN__

#define FFTCOMPLEX fftwf_complex
#define FFTMALLOC fftwf_malloc
#define FFTPLANR2C2D fftwf_plan_dft_r2c_2d
#define FFTPLANC2R2D fftwf_plan_dft_c2r_2d
#define FFTPLANR2C1D fftwf_plan_dft_r2c_1d
#define FFTPLANC2R1D fftwf_plan_dft_c2r_1d
#define FFTEXECUTE fftwf_execute
#define FFTDESTROY fftwf_destroy_plan

#else

#define FFTCOMPLEX fftw_complex
#define FFTMALLOC fftw_malloc
#define FFTPLANR2C2D fftw_plan_dft_r2c_2d
#define FFTPLANC2R2D fftw_plan_dft_c2r_2d
#define FFTPLANR2C1D fftw_plan_dft_r2c_1d
#define FFTPLANC2R1D fftw_plan_dft_c2r_1d
#define FFTEXECUTE fftw_execute
#define FFTDESTROY fftw_destroy_plan

#endif

// todo: declare struct in header file but define it here so that it becomes opaque
blur_in_place_plan* create_blur_plan_quadratic_2d(int sidelength, float bw)
{
	blur_in_place_plan* k = malloc(sizeof(blur_in_place_plan));
	if (k == NULL) {
		fprintf(stderr, "Out of memory");
		exit(1);
	}
	k->rank = 2;
	k->sizes = malloc(k->rank * sizeof(int));

	k->sizes[0] = sidelength;
	k->sizes[1] = sidelength;
	k->dataLength = 1;
	for (int r = 0; r < k->rank; r++) k->dataLength *= k->sizes[r];

	k->bandwidth = bw;
	k->data = FFTMALLOC(k->dataLength * sizeof(float));
	k->frequencies = FFTMALLOC(sizeof(FFTCOMPLEX) * k->dataLength);
	// fftw_set_timelimit(1.5);
	k->planForward = FFTPLANR2C2D(sidelength, sidelength, k->data, k->frequencies, FFTW_PATIENT);
	// fprintf(stdout, fftw_sprint_plan(k->planForward));
	k->planBackward = FFTPLANC2R2D(sidelength, sidelength, k->frequencies, k->data, FFTW_PATIENT);
	// fprintf(stdout, fftw_sprint_plan(k->planForward));
	// is no input to an fft but might still benefit from alignment
	k->multiplier = FFTMALLOC(sizeof(FFTCOMPLEX) * k->dataLength);

	float sumKernel = 0.f;
	for (int i = 0; i < sidelength; i++) {
		for (int j = 0; j < sidelength; j++)
		{
			int n = sidelength;
			float distCorner1Sq = ((float)(i * i + j * j)) / (float)n / (float)n;
			float distCorner2Sq = ((float)((n - i) * (n - i) + j * j)) / (float)n / (float)n;
			float distCorner3Sq = ((float)(i * i + (n - j) * (n - j))) / (float)n / (float)n;
			float distCorner4Sq = ((float)((n - i) * (n - i) + (n - j) * (n - j))) / (float)n / (float)n;

			float value = expf(-distCorner1Sq / bw / bw) + expf(-distCorner2Sq / bw / bw) + expf(-distCorner3Sq / bw / bw) + expf(-distCorner4Sq / bw / bw);
			k->data[i + n * j] = value;
			sumKernel += value;
		}
	}
	// renormalize the kernel so it has sum 1
	for (int i = 0; i < k->dataLength; i++) {
		k->data[i] /= sumKernel;
	}
	// printf("\n");
	FFTEXECUTE(k->planForward);

	// unfortunately, this copying doesn't seem to work with memcpy ... need to find the reason. So, let's do it literally.
	for (int j = 0; j < k->dataLength; j++) {
		k->multiplier[j][0] = k->frequencies[j][0];
		k->multiplier[j][1] = k->frequencies[j][1];
	}


	/* for (int i = 0; i < sidelength; i++) { */
	/*   for (int j = 0; j < sidelength; j++) { */
	/*     printf("%f %f    ", k->multiplier[i + sidelength * j][0], k->multiplier[i + sidelength * j][1]); */
	/*   } */
	/*   printf("\n"); */
	/* } */

	for (int j = 0; j < k->dataLength; j++) k->data[j] = 0.f;
	return k;
}

float* blur_plan_data(blur_in_place_plan* p) { return p->data; }

blur_in_place_plan* duplicate_blur_plan_2d(blur_in_place_plan* src) {
	blur_in_place_plan* k = (blur_in_place_plan*)malloc(sizeof(blur_in_place_plan));
	k->rank = src->rank;
	k->sizes = (int*)malloc(k->rank * sizeof(int));
	memcpy(k->sizes, src->sizes, k->rank);
	k->dataLength = src->dataLength;
	k->bandwidth = src->bandwidth;
	k->data = (float*)FFTMALLOC(k->dataLength * sizeof(float));

	k->frequencies = (FFTCOMPLEX*)FFTMALLOC(sizeof(FFTCOMPLEX) * k->dataLength);
	k->planForward = FFTPLANR2C2D(k->sizes[0], k->sizes[1], k->data, k->frequencies, FFTW_MEASURE);
	k->planBackward = FFTPLANC2R2D(k->sizes[0], k->sizes[1], k->frequencies, k->data, FFTW_MEASURE);
	k->multiplier = (FFTCOMPLEX*)FFTMALLOC(sizeof(FFTCOMPLEX) * k->dataLength);

	for (int i = 0; i < k->dataLength; i++) {
		// find out why I can't memcpy here   // memcpy(k->multiplier, src->multiplier, k->dataLength);
		k->multiplier[i][0] = src->multiplier[i][0];
		k->multiplier[i][1] = src->multiplier[i][1];
	}
	for (int j = 0; j < k->dataLength; j++) k->data[j] = 0.f;
	return k;
}

blur_in_place_plan* create_blur_plan_1d(int n, float bw)
{
	blur_in_place_plan* k = (blur_in_place_plan*)malloc(sizeof(blur_in_place_plan));
	k->rank = 1;
	k->sizes = (int*)malloc(k->rank * sizeof(int));

	k->sizes[0] = n;
	k->dataLength = 1;
	for (int r = 0; r < k->rank; r++) k->dataLength *= k->sizes[r];

	k->bandwidth = bw;
	k->data = (float*)FFTMALLOC(n * sizeof(float));
	k->frequencies = (FFTCOMPLEX*)FFTMALLOC(sizeof(FFTCOMPLEX) * n);
	k->planForward = FFTPLANR2C1D(n, k->data, k->frequencies, FFTW_MEASURE);
	k->planBackward = FFTPLANC2R1D(n, k->frequencies, k->data, FFTW_MEASURE);
	// is no input to an fft but might still benefit from alignment
	k->multiplier = (FFTCOMPLEX*)FFTMALLOC(n * sizeof(FFTCOMPLEX));
	float sumKernel = 0.f;

	// todo: write an interface that replaces this function with a user-provided callback
	for (int i = 0; i < n; i++)
	{
		float distCorner1Sq = ((float)(i * i)) / (float)n / (float)n;
		float distCorner2Sq = (float)((n - i) * (n - i)) / (float)n / (float)n;
		float value = expf(-distCorner1Sq / bw / bw) + expf(-distCorner2Sq / bw / bw);
		k->data[i] = value;
		sumKernel += value;
	}

	// renormalize the kernel so it has sum 1
	for (int i = 0; i < n; i++) k->data[i] /= sumKernel;
	FFTEXECUTE(k->planForward);
	for (int j = 0; j < n; j++) {
		k->multiplier[j][0] = k->frequencies[j][0];
		k->multiplier[j][1] = k->frequencies[j][1];
		// printf("the kernel fft is %f %f\n", k->multiplier[j][0] , k->multiplier[j][1]); 
	}
	// memcpy(k->multiplier, k->frequencies, n * sizeof(FFTCOMPLEX));
	//  memset(k->data, 0.f, n); doesn't work, unfortunately
	for (int j = 0; j < n; j++) k->data[j] = 0.f;

	return k;
}

void blur_plan_update_bandwidth_1d(blur_in_place_plan* k, float bw) {
	k->bandwidth = bw;
	float sumKernel = 0.f;
	int n = k->sizes[0];
	for (int i = 0; i < n; i++)
	{
		float distCorner1Sq = ((float)(i * i)) / (float)n / (float)n;
		float distCorner2Sq = (float)((n - i) * (n - i)) / (float)n / (float)n;
		float value = exp(-distCorner1Sq / bw / bw) + exp(-distCorner2Sq / bw / bw);
		k->data[i] = value;
		sumKernel += value;
	}
	for (int i = 0; i < n; i++) k->data[i] /= sumKernel;
	FFTEXECUTE(k->planForward);
	for (int j = 0; j < n; j++) {
		k->multiplier[j][0] = k->frequencies[j][0];
		k->multiplier[j][1] = k->frequencies[j][1];
	}
	for (int j = 0; j < n; j++) k->data[j] = 0.f;
}

void blur_plan_update_bandwidth_quadratic_2d(blur_in_place_plan* k, float bw) {
	k->bandwidth = bw;
	float sumKernel = 0.f;
	int sidelength = k->sizes[0];
	for (int i = 0; i < sidelength; i++) {
		for (int j = 0; j < sidelength; j++)
		{
			int n = sidelength;
			float distCorner1Sq = ((float)(i * i + j * j)) / (float)n / (float)n;
			float distCorner2Sq = ((float)((n - i) * (n - i) + j * j)) / (float)n / (float)n;
			float distCorner3Sq = ((float)(i * i + (n - j) * (n - j))) / (float)n / (float)n;
			float distCorner4Sq = ((float)((n - i) * (n - i) + (n - j) * (n - j))) / (float)n / (float)n;
			float value = exp(-distCorner1Sq / bw / bw) + exp(-distCorner2Sq / bw / bw) + exp(-distCorner3Sq / bw / bw) + exp(-distCorner4Sq / bw / bw);
			k->data[i + n * j] = value;
			sumKernel += value;
		}
	}
	// renormalize the kernel so it has sum 1
	for (int i = 0; i < k->dataLength; i++) {
		k->data[i] /= sumKernel;
	}
	FFTEXECUTE(k->planForward);
	for (int j = 0; j < k->dataLength; j++) {
		k->multiplier[j][0] = k->frequencies[j][0];
		k->multiplier[j][1] = k->frequencies[j][1];
	}
	for (int j = 0; j < k->dataLength; j++) k->data[j] = 0.f;
}

void blur_plan_execute(blur_in_place_plan* bp)
{
	FFTEXECUTE(bp->planForward);

	FFTCOMPLEX* aa = bp->frequencies;

	// why can't one use the built-in multiplication?
	// also, if it's blurring, the imaginary part is zero.
	for (int i = 0; i < bp->dataLength; i++) {
		float* a = aa[i];
		float* b = bp->multiplier[i];
		float re = a[0] * b[0] - a[1] * b[1];
		float im = a[0] * b[1] + a[1] * b[0];

		bp->frequencies[i][0] = re;
		bp->frequencies[i][1] = im;
	}

	// now, the multiplication is correct only up to n/2 the rest is zero but that's ok, fftw doesn't need the others
	FFTEXECUTE(bp->planBackward);

	// up to a cyclic shuffle, it's the same as
	// rev(Re(fft((fft(k) * fft(sapply(0:9,function(i) exp(-i^2/1.3/1.3/10/10) + exp(-(10-i)^2/1.3/1.3/10/10)))))))) for instance


	// now, divide by the sum of the kernel and n so that the sum of the input data is preserved
	float scalingFactor = (float)bp->dataLength;
	for (int i = 0; i < bp->dataLength; i++) {
		bp->data[i] /= scalingFactor;
	}

	/* // it's still reversed so we turn it around */
	/* for(int i=0;i<bp->dataLength/2;i++) */
	/*   { */
	/*       float temp=bp->data[i]; */
	/*       bp->data[i]=bp->data[bp->dataLength-1-i]; */
	/*       bp->data[bp->dataLength-1-i]=temp; */
	/*   } */

}

void free_blur_plan(blur_in_place_plan* bp)
{
	FFTDESTROY(bp->planBackward);
	FFTDESTROY(bp->planForward);

	fftwf_free(bp->frequencies);
	fftwf_free(bp->multiplier);
	fftwf_free(bp->data);

	free(bp->sizes);
	free(bp);
}

#endif // BLUR_PLAN_IMPLEMENTATION