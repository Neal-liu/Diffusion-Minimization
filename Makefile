CFLAGS = -Wall -Werror -std=gnu99 -O3 -g
#LDFLAGS = `pkg-config --libs --cflags libpng`
#PNGLIBS := $(shell pkg-config libpng --libs)

all:
	gcc diffusionMin.c
	./a.out 
