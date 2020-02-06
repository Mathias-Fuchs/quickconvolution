
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "Timeline.h"
#include "staticHelpers.h"
#include "blur_plan.h"
#include "box.h"
#include "readnumerictable.h"
#include <stdbool.h>


#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#define SAVE
int main()
{
	int n = 512;
	int nrFrames = 628;
	box* b = box_create(0, 100, 0, 100, 0, 0);
	Timeline* tl = TimelineCreateFromFile("forConv.db", n, b);
	float* tlb = TimelineBlur(tl, n, nrFrames, 0.021, 0.12, 0);
	char filename[200];
	unsigned char* buf = malloc(3 * n * n * sizeof(unsigned char));

	float max = 0;
	for (int i = 0; i < n * n * nrFrames; i++) if (tlb[i] > max) max = tlb[i];
	float min = 1e9;
	for (int i = 0; i < n * n * nrFrames; i++) if (tlb[i] < min) min = tlb[i];

	for (int framecounter = 0; framecounter < nrFrames; framecounter++) {
		double sum = 0;
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < n; j++) {
				float v = (tlb[framecounter * n * n + j * n + i] - min) / (max - min);
				float red = (expf(v - 1.f) - expf(-1.f));
				float green = (sinf(v * v) * cosf(v * v));
				float blue = v;

				float cutoff = v * 10.0 - (int)(v * 10.0);

				float valred = sqrt(sqrt(cutoff * cutoff * cutoff * red)) * 255.0f;
				float valgreen = sqrt(sqrt(cutoff * cutoff * green)) * 255.0f;
				float valblue = sqrt(sqrt(cutoff * blue)) * 255.0f;
			

				if (valred > 255.0f) valred = 255.0f;
				if (valgreen > 255.0f) valgreen = 255.0f;
				if (valblue > 255.0f) valblue = 255.0f;
				buf[3 * ((n - i - 1) * n + j)] = (unsigned char)valred;
				buf[3 * ((n - i - 1) * n + j) + 1] = (unsigned char)valgreen;
				buf[3 * ((n - i - 1) * n + j) + 2] = (unsigned char)valblue;
			}
		}
#ifdef SAVE
		unsigned char* last_row = buf + 3 * (n * (n - 1));
		snprintf(filename, sizeof(char) * 200, "Screenshot%05i.png", framecounter);
		if (!stbi_write_png(filename, n, n, 3, last_row, -3 * n)) {
			fprintf(stdout, "error in stb");
		}
#endif
	}
	free(buf);
	TimelineFree(tl, nrFrames);
	return 0;

}


