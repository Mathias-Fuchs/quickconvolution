
![Blurring and thresholding for image processing](hania.gif)

# Quickconvolution
This is a single file C library called blur_plan.h for image blurring using convolution with a Gaussian kernel, implemented 
The complexity is that of the Fast Fourier transform, namely O(n log n) where n is the number of pixels of the image.
This is almost the same complexity as that of a single pass over the file.
The naive implementation of a Gaussian stencil convolution is O(n^2).

# Usage
```C
#include "blur_plan.h"
#define BLUR_PLAN_IMPLEMENTATION

// prepare for blurring images of size 128 x 128 with a stencil size of 5%, takes a little time to warm up
blur_in_place_plan* bipp = create_blur_plan_quadratic_2d(128, 0.05);

// get the raw pointer to the image date
float* bpd = blur_plan_data(bipp);

while (true) {
// this loop is extremely fast and can be a hot path
// .... fill the image
	bpd[0] = somepixelvalue;
    bpd[1] = anotherpixelvalue;
// ...

// do the blurring
	blur_plan_execute(bipp);

// do something with the blurred image
    float firstpixel = bpd[0];
// ...
}

// free the memory
free_blur_plan(bipp);
```

As usual with single file header libraries, include the header file blur_plan.h wherever you want to use the declaration of the API functions. Once and only once the symbol
```
#define BLUR_PLAN_IMPLEMENTATION
```
needs to be defined in a compilation unit that will provide the definition of the API functions.
Make sure your compiler can include and link to fftw3f. On Debian-based Linux distros,
```bash
sudo apt install libfftw3-dev
```
and then link with -lfftw3f.
On Windows and with Visual Studio, it is advisable to use CMake as explained below.

# Example 
This repository contains a simple example in the file blurimage.c. It produces images which concatenate to the gif above.

# Building the example blurimage executable with gcc on  linux
```
gcc blurimage.c -lfftw3 -Ofast -o blurimage
```

# Building the example with CMake on Windows
... requires the presence of fftw. Download from ftp://ftp.fftw.org/pub/fftw/fftw-3.3.5-dll64.zip and unpack to a directory of your choice. Open the file CMakeLists.txt with an editor and insert the directory where you placed it to all places that mention fftw.
Open the repository folder in Visual Studio and switch to CMake Targets view in the solution explorer. Set the executable blurimage to the startup item and click the green arrow.

# API reference 
```C
blur_in_place_plan *create_blur_plan_quadratic_2d(int sidelength, float bw);
```
Create a 2d blur_plan with quadratic sidelength sidelength and bandwidth bw, specificed in fraction of the image size, for instance bw==0.1 leads to a Gaussian stencil size of one tenth the image width._

```C
float* blur_plan_data;
```
Get the raw data of the image before and after the convolution.

```C
blur_in_place_plan* duplicate_blur_plan_2d(blur_in_place_plan* src);
```
Deep-copy the blur plan.

```C
blur_in_place_plan *create_blur_plan_1d(int n, float bw);
```
Create a 1d blur_plan.

```C
void blur_plan_update_bandwidth_1d(blur_in_place_plan* k, float bw);
```
Change the bandwidth of the 1d blur_plan.

```C
void blur_plan_update_bandwidth_quadratic_2d(blur_in_place_plan* k, float bw);
```
Change the bandwidth of the 2d blur_plan.

```C
void blur_plan_execute(blur_in_place_plan *bp);
```
Do it, perform the convolution. A pointer to the raw pixel data of the convolved image can be obtained by blur_plan_data.

```C
void free_blur_plan(blur_in_place_plan *bp)
```
Clean up to avoid memory leaks.

# More info and other projects
https://mathiasfuchs.de/mfem.html

# License
A very restrictive GPLv3. Drop me a line for other options.

# Emscripten build and inclusion in a website
I have an emscripten build, drop me a line for more info.