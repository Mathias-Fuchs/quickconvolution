#include <sqlite3.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <fftw3.h>
#include "Timeline.h"
#include "box.h"
#include "staticHelpers.h"
#include "blur_plan.h"
#include "readnumerictable.h"


static int callback1(void* count, int argc, char** argv, char** azColName) {
	int* c = count;
	*c = atoi(argv[0]);
	return 0;
}

Timeline* TimelineCreateFromWhenIJVal(box* b, char* filename, char* tablename, int n) {
	char m[500];

	sqlite3* db;
	sqlite3_stmt* stmt;

	int kk = sqlite3_open_v2(filename, &db, SQLITE_OPEN_READONLY, NULL);

	if (db == NULL || kk != SQLITE_OK)
	{
		printf("Failed to open DB ");
		printf(filename);
		printf(" with error code %i \n", kk);
		exit(1);
	}

	// get that info from the info table .... later.
	int nx = 156;
	int ny = 173;
	int ndistinct;
	strcpy(m, "select count(distinct timepoint) from ");
	strcat(m, tablename);
	// const char* weekend = " where strftime(\"%w\", timepoint, \"unixepoch\", \"localtime\")==\"0\" or strftime(\"%w\", timepoint, \"unixepoch\", \"localtime\")==\"6\"";
	const char* weekday = ""; // " where timepoint > 1518652800 and timepoint < 1524096000 and strftime(\"%w\", timepoint, \"unixepoch\", \"localtime\")!=\"0\" and strftime(\"%w\", timepoint, \"unixepoch\", \"localtime\")!=\"6\"";
	strcat(m, weekday);
	char* zErrMsg;
	int rc = sqlite3_exec(db, m, callback1, &ndistinct, &zErrMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}

	Timeline* self = malloc(sizeof(Timeline));
	TimeHeatmapObservation** thos = malloc(ndistinct * sizeof(TimeHeatmapObservation*));
	for (int h = 0; h < ndistinct; h++) {
		TimeHeatmapObservation* tho = malloc(sizeof(TimeHeatmapObservation));
		tho->nx = nx;
		tho->ny = ny;
		tho->data = calloc(n * n, sizeof(int));
		if (!tho->data) {
			fprintf(stderr, "out of memory\n"); exit(1);
		}
		thos[h] = tho;
	}


	strcpy(m, "select distinct timepoint, datetime(timepoint, 'unixepoch', 'localtime') from ");
	strcat(m, tablename);
	strcat(m, weekday);
	sqlite3_prepare_v2(db, m, -1, &stmt, NULL);
	int k = 0;
	while (sqlite3_step(stmt) != SQLITE_DONE) {
		thos[k]->time = sqlite3_column_int(stmt, 0);
		const char* ts = (const char*)sqlite3_column_text(stmt, 1);
		strncpy(thos[k++]->timeStr, ts, 20);
	}
	strcpy(m, "select timepoint, i, j, value from ");
	strcat(m, tablename);
	strcat(m, weekday);
	sqlite3_prepare_v2(db, m, -1, &stmt, NULL);
	k = 0;
	int jjj = 0;
	box* texturebox = box_create(-10, 160, -10, 160, 0, 0);
	// 155, 172
	while (sqlite3_step(stmt) != SQLITE_DONE) {
		jjj++;
		int timepoint = sqlite3_column_int(stmt, 0);
		int i = sqlite3_column_int(stmt, 1);
		int j = sqlite3_column_int(stmt, 2);
		int val = sqlite3_column_int(stmt, 3);
		int k = 0;
		// find the right k for that timepoint
		while (thos[k]->time != timepoint) { k++; };
		//		fprintf(stdout, "I m getting in the %i th line for timepoint %i up to timepoint counter %i \n", jjj, timepoint, k);
		//		fprintf(stdout, "timestring %s\n", thos[k]->timeStr);
		int m = MAX(nx, ny);
		int aa, bb;

		float x, y;
		// convert the heatmap coordinates to world coordinates

		x = 3.f + (float)j / 172.0 * (17.f - 3.f);
		y = 3.f + (float)i / 155.0 * (17.f - 3.f);
		/*
		// box_flip_along_x(textureBox, &y);
		// box_flip_along_y(textureBox, &x);

		// convert these world coordinates to texture coordinates
		box_stretch_y(b, &y, 1.38f);
		box_stretch_x(b, &x, 1.2f);
		*/
		box_world_texture(b, n, x, y, &aa, &bb);
		thos[k]->data[bb + n * aa] += val;
	}
	sqlite3_finalize(stmt);

	printf("read all data in\n");
	sqlite3_close(db);

	self->nrObservations = ndistinct;
	self->heatmaps = thos;

	return self;

}

Timeline* TimelineCreateFromCumulativePositions(char* filename, char* tablename, int nColumns, int n, box* b) {

	Timeline* self = malloc(sizeof(Timeline));

	int nrow;
	float* d = readnumerictable(filename, tablename, nColumns, &nrow);

	// just one timepoint
	int ndistinct = 1;
	TimeHeatmapObservation* tho = malloc(sizeof(TimeHeatmapObservation));
	tho->nx = n;
	tho->ny = n;
	tho->time = 0;
	sprintf(tho->timeStr, "%s", "genericTimepoint");

	tho->data = calloc(tho->nx * tho->ny, sizeof(int));
	for (int i = 0; i < nrow; i++) {
		float x = d[nColumns * i];
		float y = d[nColumns * i + 1];
		int i, j;
		box_world_texture(b, n, x, y, &i, &j);
		tho->data[j + n * i] += 1;
	}

	free(d);
	self->nrObservations = ndistinct;
	self->heatmaps = malloc(ndistinct * sizeof(TimeHeatmapObservation*));
	(self->heatmaps)[0] = tho;
	return self;
}

Timeline* TimelineCreateFromFile(char* filename, int n, box* b) {
	char m[500];

	Timeline* self = malloc(sizeof(Timeline));

	sqlite3* db;
	sqlite3_stmt* stmt;

	sqlite3_open_v2(filename, &db, SQLITE_OPEN_READONLY, NULL);

	if (db == NULL)
	{
		printf("Failed to open DB ");
		printf(filename);
		printf("\n");
		exit(1);
	}

	int ndistinct;
	strcpy(m, "select count(DISTINCT t) from main;");
	char* zErrMsg;
	int rc = sqlite3_exec(db, m, callback1, &ndistinct, &zErrMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}


	strcpy(m, "select max(t) from main;");
	int tmax;
	rc = sqlite3_exec(db, m, callback1, &tmax, &zErrMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}



	// now we know how many timepoints there are in the data;
	TimeHeatmapObservation** thos = malloc(ndistinct * sizeof(TimeHeatmapObservation*));
	for (int h = 0; h < ndistinct; h++) {
		TimeHeatmapObservation* tho = malloc(sizeof(TimeHeatmapObservation));
		tho->nx = n;
		tho->ny = n;
		tho->data = calloc(tho->nx * tho->ny, sizeof(int));

		if (!tho->data) exit(1);
		thos[h] = tho;
	}

	// for this 
	// create table timepoints as select distinct(updated) from allData;
	// create table timepointsWithId as select rowid, updated from timepoints;

	strcpy(m, "select x, y, t from main;");

	sqlite3_prepare_v2(db, m, -1, &stmt, NULL);

	int i = 0;
	while (sqlite3_step(stmt) != SQLITE_DONE) {
		float x = (float)sqlite3_column_double(stmt, 0);
		float y = (float)sqlite3_column_double(stmt, 1);
		float t = (float)sqlite3_column_double(stmt, 2);
		int i, j;
		box_world_texture(b, n, x, y, &i, &j);
		int tresc = (int)((double)t / (double)tmax * (double)(ndistinct - 1));
		thos[tresc]->data[j + n * i] += 1;
		thos[tresc]->time = tresc;
	}
	sqlite3_finalize(stmt);

	printf("read all data in\n");
	sqlite3_close(db);
	self->nrObservations = ndistinct;
	self->heatmaps = thos;
	return self;
}

// array: an array of ints, containing the timeline in a flat format. Each entry represents the number of counts of particles in that bin at that timepoint.
// n: the sidelength of the data grid.
// nrFrames: the number of frames, or number of timepoints.
// so, the total length of the array has to be n*n*nrFrames
// b: pointer to the box describing the bounding box of the data.
Timeline* TimelineCreateFromArray(int* array, int n, int nrFrames) {
	Timeline* self = malloc(sizeof(Timeline));
	int ndistinct = nrFrames;
	// now we know how many timepoints there are in the data;
	TimeHeatmapObservation** thos = malloc(ndistinct * sizeof(TimeHeatmapObservation*));
	for (int h = 0; h < ndistinct; h++) {
		TimeHeatmapObservation* tho = malloc(sizeof(TimeHeatmapObservation));
		tho->nx = n;
		tho->ny = n;
		tho->data = calloc(n*n, sizeof(int));
		// the h-th timepoint starts at memory location array + h * n * n, and occupies n * n * sizeof(int) bytes.
		// we can copy them linearly over.
		memcpy(tho->data, array + h * n * n, n * n * sizeof(int));
		thos[h] = tho;
	}
	self->nrObservations = ndistinct;
	self->heatmaps = thos;
	return self;
}

void TimelineFree(Timeline* tl, int nrFrames) {
	for (int h = 0; h < nrFrames; h++)
		free(tl->heatmaps[h]->data);
	free(tl->heatmaps);
	free(tl);
}

int TimelineIndex(Timeline* tl, int f, int nrTextureFrames) {
	// as we go through all texture frames, we walk through all available observations
	int p = (int)(((float)f / (float)nrTextureFrames) * (float)(tl->nrObservations));
	p = positive_modulo(p, tl->nrObservations);
	return p;
}

void printStats(int* A, int n) {
	float mean = 0;
	for (int i = 0; i < n * n; i++) mean += (float) A[i] / n / n;
	int max = 0;
	for (int i = 0; i < n * n; i++) if (A[i] > max) max = A[i];
	int min = 1e9;
	for (int i = 0; i < n * n; i++) if (A[i] < min) min = A[i];
	fprintf(stdout, "mean %f, min %i, max %i\n", mean, min, max);
}

void printStatsF(float* A, int n) {
	float mean = 0;
	for (int i = 0; i < n * n; i++) mean += (float)A[i] / n / n;
	float max = 0;
	for (int i = 0; i < n * n; i++) if (A[i] > max) max = A[i];
	float min = 1e9;
	for (int i = 0; i < n * n; i++) if (A[i] < min) min = A[i];
	fprintf(stdout, "mean %f, min %f, max %f\n", mean, min, max);
}


float* TimelineBlur(Timeline* tl, int n, int nrTextureFrames, float bw2d, float bw1d, int cumulative) {
	TimeHeatmapObservation** thos = tl->heatmaps;

	float* data3d = (float*)fftwf_malloc(nrTextureFrames * n * n * sizeof(float));
	if (data3d == NULL) exit(1);
	for (int h = 0; h < nrTextureFrames * n * n; h++) data3d[h] = 0.f;
	for (int f = 0; f < nrTextureFrames; f++)
	{

		int p = TimelineIndex(tl, f, nrTextureFrames);
		TimeHeatmapObservation* tho = thos[p];
		// printStats(tho->data, n);
		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < n; j++)
			{
				int entry = tho->data[j + n * i];
				data3d[j + n * (i + n * f)] = (float)entry;
			}
		}
	}
	blur_in_place_plan* bp2d = create_blur_plan_quadratic_2d(n, bw2d);
	float* A = blur_plan_data(bp2d);

	for (int f = 0; f < nrTextureFrames; f++) {
		for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) A[j + n * i] = data3d[j + n * (i + n * f)];
		blur_plan_execute(bp2d);
//		printStatsF(A, n);
		for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) data3d[j + n * (i + n * f)] = A[j + n * i];
	}
	free_blur_plan(bp2d);

	blur_in_place_plan* bp1d = create_blur_plan_1d(nrTextureFrames, bw1d);
	float* B = blur_plan_data(bp1d);
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			for (int f = 0; f < nrTextureFrames; f++) B[f] = data3d[j + n * (i + n * f)];
			blur_plan_execute(bp1d);
			for (int f = 0; f < nrTextureFrames; f++) data3d[j + n * (i + n * f)] = B[f];
		}
	}
	free_blur_plan(bp1d);

	return data3d;
}


void TimelineWriteToFile(char* filename, float* data3d, int n, int nrTextureFrames) {
	FILE* fp = fopen(filename, "w+");
	fwrite(data3d, sizeof(float), nrTextureFrames * n * n, fp);
	fclose(fp);
}

float* TimelineReadFromFile(char* filename, int n, int* nrTextureFrames) {
	FILE* fp = fopen(filename, "r");
	fseek(fp, 0L, SEEK_END);
	int sz = ftell(fp);
	rewind(fp);
	*nrTextureFrames = sz / n / n / sizeof(float);
	float* data3d = malloc(sz);
	fread(data3d, sizeof(float), sz / sizeof(float), fp);
	fclose(fp);
	return data3d;
}


