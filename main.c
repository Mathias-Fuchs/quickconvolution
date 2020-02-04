
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
	box* b = box_create(0, 100, 0, 100, 0, 0);
	Timeline* tl = TimelineCreateFromFile("forConv.db", n, b);
	float* tlb = TimelineBlur(tl, n, 628, 0.021, 0.12, 0);
	int* trianglestrip = triangle(n);
	char filename[200];
	unsigned char* buf = malloc(3 * n * n * sizeof(unsigned char));

	float max = 0;
	for (int i = 0; i < n * n * 628; i++) if (tlb[i] > max) max = tlb[i];
	float min = 1e9;
	for (int i = 0; i < n * n * 628; i++) if (tlb[i] < min) min = tlb[i];

	for (int framecounter = 0; framecounter < 628; framecounter++) {
		double sum = 0;
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < n; j++) {
				float v = (tlb[framecounter * n * n + j * n + i] - min) / (max - min);
				float red = (expf(v - 1.f) - expf(-1.f));
				float green = (sinf(v * v) * cosf(v * v));
				float blue = v;


				float valred =  red * 255.0f;
				float valgreen = green * 255.0f;
				float valblue = blue * 255.0f;
				
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
	return 0;

}

