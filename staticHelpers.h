#pragma once
#include <stdio.h>
#include <stdbool.h>
#include "box.h"

#define MIN(a, b) (a > b ? b : a)
#define MAX(a, b) (a > b ? a : b)
#define CLAMP(v, lo, hi) MAX(lo, MIN(hi, v))


float getBwCorrection();
void error_callback(int error, const char *description);
int* triangle(int n);
void drawTextureNoClip(int *trianglestrip, float** A, const int* n, int blend);
void drawVertices0(box* bglBox, float* allV, int nVert);
void markPoint(box* b, float x, float y);
float mean(float* array, int n);
int positive_modulo(int i, int n);
bool bwChanged();
void bwAdjusted();
