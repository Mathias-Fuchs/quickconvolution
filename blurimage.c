#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#ifdef _WIN32
#define STBI_WINDOWS_UTF8
#define _USE_MATH_DEFINES
#endif
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include <math.h>

#define BLUR_PLAN_IMPLEMENTATION
#include "blur_plan.h"

int main(int argc, char** argv)
{
	const char* filename = "hania.png";
	FILE* f = fopen(filename, "r");
	if (f == NULL) {
		fprintf(stderr, "Can't find input file %s. Aborting.\n", filename);
		return 1;
	}
	fclose(f);
	int width, height, comp;

	unsigned char* pix = stbi_load(filename, &width, &height, &comp, 1);
	fprintf(stdout, "width %i, height %i, comp %i\n", width, height, comp);
	int sl = width > height ? width : height;

	unsigned char* pixsq = malloc(sl * sl);

	memset(pixsq, 255, sl * sl);

	for (int i = 0; i < sl; i++)
		for (int j = 0; j < sl; j++)
		{
			if (j > height)
				continue;
			pixsq[i + sl * j] = pix[i + width * j];
		}
	stbi_image_free(pix);
	blur_in_place_plan* bipp = create_blur_plan_quadratic_2d(sl, 0.008);
	float* bpd = blur_plan_data(bipp);

	int countbright = 0;
	for (int i = 0; i < sl; i++)
		for (int j = 0; j < sl; j++)
		{
			unsigned char r = pixsq[i + j * sl];
			float bright = (float)r / 255;
			bpd[i + sl * j] = bright > 0.5 ? 1 : 0;
			if (bright > 0.5)
				countbright++;
		}
	free(pixsq);
	float percbright = (float)countbright / width / height * 100.0;

	fprintf(stdout, "%i bright out of %i, Percentage = %.2f%%\n", countbright, width * height, percbright);

	blur_plan_execute(bipp);
	unsigned char* res = malloc(sl * sl);
	if (res == NULL) {
		fprintf(stderr, "Ran out of memory.");
		exit(1);
	}
	for (int k = 0; k < 90; k++)
	{
		double angle = (double)k / 90 * 2 * M_PI;
		float thresh = 0.7 + (float)k / 300 * sin(angle) * sin(angle);
		for (int i = 0; i < width; i++)
			for (int j = 0; j < width; j++)
			{
				float blurred = bpd[i + sl * j];
				res[i + sl * j] = blurred > thresh ? 255 : 0;
			}

		char buffer[5000];
		snprintf(buffer, 5000, "res_%05i.png", k);
		stbi_write_png(buffer, sl, height, 1, res, sl);
	}

	free(res);
	free_blur_plan(bipp);

	return 0;
}
