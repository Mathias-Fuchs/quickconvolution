#pragma once
#include "box.h"


typedef struct {
    int time;
    int nx;
    int ny;
    int* data;
    char timeStr[20];
} TimeHeatmapObservation;


typedef struct {
  int nrObservations;
  TimeHeatmapObservation** heatmaps;
} Timeline;

Timeline* TimelineCreateFromFile(char * filename, int n, box* b);
Timeline* TimelineCreateFromCumulativePositions(char * filename, char* tablename, int nColumns, int n, box* b);
Timeline* TimelineCreateFromWhenIJVal(box* b, char* filename, char* tablename, int n);
int TimelineIndex(Timeline* tl, int f, int nrTextureFrames);
float* TimelineBlur(Timeline* tl, int n, int nrTextureFrames, float bw2d, float bw1d, int cumulative);
void TimelineWriteToFile(char* filename, float* data3d, int n, int nrTextureFrames);
float* TimelineReadFromFile(char* filename, int n, int* nrTextureFrames);
