CFLAGS = -Wall -Werror -std=gnu99 -O3 -g
TOPK = 20
BIN = synthetic2

#LDFLAGS = `pkg-config --libs --cflags libpng`
#PNGLIBS := $(shell pkg-config libpng --libs)

all: $(BIN)

synthetic:
	gcc $(CFLAGS) -o diffusion diffusionMin.c LDTree.c
	./diffusion ./syntheticData/small/synthetic.edge ~/graph-tool/syntheticData/small 2

synthetic2:
	gcc $(CFLAGS) -o diffusion diffusionMin.c LDTree.c
	./diffusion ./syntheticData/synthetic2.edge ~/graph-tool/syntheticData 10

twitter:
	gcc $(CFLAGS) -o diffusion diffusionMin.c LDTree.c
	./diffusion ./realData/twitter/twitterWithEdge.edge ~/graph-tool/realData/twitter $(TOPK)
	

