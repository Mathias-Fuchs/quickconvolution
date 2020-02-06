#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#ifdef _WIN32
#define STBI_WINDOWS_UTF8
#endif
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include <unistd.h>
#include <math.h>
#include "blur_plan.h"

int main(int argc, char **argv)
{
    /*    if (argc < 2) return 1;
    const char *filename = argv[1];
    */
    const char *filename = "hania.png";
    if (access(filename, F_OK) != 0)
        return 1;
    int width, height, comp;

    unsigned char *pix = stbi_load(filename, &width, &height, &comp, 1);
    fprintf(stdout, "width %i, height %i, comp %i\n", width, height, comp);
    int sl = width > height ? width : height;

    unsigned char *pixsq = malloc(sl * sl);

    memset(pixsq, 255, sl * sl);

    for (int i = 0; i < sl; i++)
        for (int j = 0; j < sl; j++)
        {
            if (j > height)
                continue;
            pixsq[i + sl * j] = pix[i + width * j];
        }

    blur_in_place_plan *bipp = create_blur_plan_quadratic_2d(sl, 0.008);
    float *bpd = blur_plan_data(bipp);

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

    float percbright = (float)countbright / width / height * 100.0;

    fprintf(stdout, "%i bright out of %i, Percentage = %.2f%%\n", countbright, width * height, percbright);
    stbi_image_free(pix);

    blur_plan_execute(bipp);
    unsigned char *res = malloc(sl * sl);
    for (int k = 0; k < 90; k++)
    {
        double angle = (double) k / 90 * 2 * M_PI;
        float thresh = 0.7 + (float) k / 300 * sin(angle) * sin(angle);
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
