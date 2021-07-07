all:
	gcc staticHelpers.c readnumerictable.c box.c blur_plan.c Timeline.c main.c -lm -lfftw3f -lsqlite3 -Ofast
