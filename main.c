
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

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#define SAVE






typedef struct lookup {
	const char* name;
	const char* filename;
	int width;
	int height;
	int comp;
	unsigned char* pixels;
} lookup_t;


int main()
{


	char* array[] = {
	//"Turbo.png",
	//"Cividis.png",
	//  "Hot.png",
//			"Parula.png",
			// "gradientStrong.png",
			// "inferno.png",
			// "magma.png",
			// "plasma.png",
					"viridis.png"
	//				"gradient.png",
					// "burger-king-hex-colors-gradient-background.png",
	//						"sonnenuntergang.png",
							// "google-hex-colors-gradient-background.png"
	};
	int nfiles = sizeof array / sizeof(char*);
	lookup_t* viridis = malloc(nfiles * sizeof(lookup_t));

	for (int kk = 0; kk < nfiles; kk++) {
		viridis[kk].name = array[kk];
		viridis[kk].filename = array[kk];
		char fn[1000];
		strcpy(fn, "");
		strcat(fn, "/home/mathias/Desktop/");
		strcat(fn, viridis[kk].filename);
		fprintf(stdout, "reading lookup image %s .... \n", fn);
		FILE* t = fopen(fn, "r");
		if (!t) { fprintf(stderr, "could not find %s - aborting \n", fn); exit(1); }
		else fclose(t);
		viridis[kk].pixels = stbi_load(fn, &viridis[kk].width, &viridis[kk].height, &viridis[kk].comp, 4);
	}




	int n = 512;
	int nrFrames = 628;
	box* b = box_create(0, 33, 0, 23, 0, 0);
	Timeline* tl = TimelineCreateFromFile("forConv.db", n, b);
	float* tlb = TimelineBlur(tl, n, nrFrames, 0.1, 0.2, 0);
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


