#include "staticHelpers.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

int hold = 0;
static int xTextureCorrection = 0.f;// 121;
static int yTextureCorrection = 0.f;// -128;
const static float zTextureCorrection = 0.4; // 0.4f;// 0.16f;
static float bwCorrection = 0.f;
bool _bwChanged = false;
static int isolines = 0;

bool _3d = false;


#ifdef USESHADER
static int programID;
#endif

#define MIN(a, b) (a > b ? b : a)
#define MAX(a, b) (a > b ? a : b)
#define CLAMP(v, lo, hi) MAX(lo, MIN(hi, v))

static inline void minmaxF(float* b, int l, float* min, float* max)
{
	float m = b[0];
	float mm = b[0];
	for (int x = 0; x < l; x++)
	{
		if (m > b[x]) m = b[x];
		if (mm < b[x]) mm = b[x];
	}
	*min = m;
	*max = mm;
}

static inline void Zebra(float ints, float* r) {
	*r = (float)(((int)(5.f * ints)) % 2);
}

static inline float LinearStep(float x, float start)
{
	float width = 0.2f;
	if (x < start)
		return 0.0f;
	else if (x > start + width)
		return 1;
	else
		return (x - start) / width;
}

static inline float GetRedValue(float intensity)
{
	return intensity * intensity * intensity;
}

static inline float GetGreenValue(float intensity)
{
	return intensity * intensity;
	//	return LinearStep(intensity, 0.6f);
}

static inline float GetBlueValue(float intensity)
{
	return  intensity;
	//	return LinearStep(intensity, 0.0f) - LinearStep(intensity, 0.4f) + LinearStep(intensity, 0.8f);
}

void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

float getBwCorrection() {
	return bwCorrection;
}

bool bwChanged() {
	return _bwChanged;
}

void bwAdjusted() {
	_bwChanged = false;
}

int* triangle(int n) {

	// generate the triangle sequence
	int cols = n;
	int rows = n;
	int RCvertices = 2 * cols * (rows - 1);
	int TSvertices = 2 * cols * (rows - 1) + 2 * (rows - 2);
	int numVertices = TSvertices;
	int j = 0;
	int i;
	int* trianglestrip = malloc(sizeof(int) * numVertices);

	for (i = 1; i <= RCvertices; i += 2)
	{
		trianglestrip[j] = (1 + i) / 2;
		trianglestrip[j + 1] = (cols * 2 + i + 1) / 2;
		if (trianglestrip[j + 1] % cols == 0)
		{
			if (trianglestrip[j + 1] != cols && trianglestrip[j + 1] != cols * rows)
			{
				trianglestrip[j + 2] = trianglestrip[j + 1];
				trianglestrip[j + 3] = (1 + i + 2) / 2;
				j += 2;
			}
		}
		j += 2;
	}
	return trianglestrip;
}

float mean(float* array, int n) {
	/* // to be replaced with moving average calculation */
	float sum = 0.f;
	for (int loop = 0; loop < n; loop++) sum += array[loop];
	return (float)sum / n;
}

int positive_modulo(int i, int n) {
	return (i % n + n) % n;
}

