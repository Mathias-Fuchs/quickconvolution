all:
	gcc staticHelpers.c readnumerictable.c box.c blur_plan.c Timeline.c main.c -lm -lfftw3f -lsqlite3 -Ofast

linux:
	gcc blur_plan.c blurimage.c -lm -lfftw3f -Ofast

mingw:
	gcc blur_plan.c blurimage.c -I ../../libraries/fftw335WIN32/ -L ../../libraries/fftw335WIN32/ -lfftw3f-3 -Ofast
