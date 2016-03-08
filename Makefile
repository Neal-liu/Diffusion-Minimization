CFLAGS = -Wall -Werror -std=gnu99 -O3 -g
CC = gcc
TOPK = 1
BIN = diffusion

#LDFLAGS = `pkg-config --libs --cflags libpng`
#PNGLIBS := $(shell pkg-config libpng --libs)

all: $(BIN)

diffusion: diffusionMin.c LDTree.c community.c
	$(CC) $(CFLAGS) -o $@ $^

synthetic:
	./diffusion ./syntheticData/small/synthetic.edge ~/graph-tool/syntheticData/small 1

synthetic2:
	./diffusion ./syntheticData/synthetic2.edge ~/graph-tool/syntheticData 1

twitter:
	for i in 1 2 3 4 5 10 20 30 40 50 60 80 100 150 200 300 400 500 ; do \
		./diffusion ./realData/twitter/twitterWithEdge.edge ~/graph-tool/realData/twitter $$i ; \
		mv realData/result experiements/twitter/baseline/youtube/$$i ; \
	done
	
facebook:
	for i in 300 ; do \
		./diffusion ./realData/facebook/facebookWithEdge.edge ~/graph-tool/realData/facebook $$i ; \
		mv realData/result experiements/facebook/community/$$i.2 ; \
	done
	
gplus:
	./diffusion ./realData/gplus/gplus/gplusWithEdge.edge ~/graph-tool/realData/gplus/gplus 1
#	for i in 1 2 3 4 5 10 20 30 40 50 60 80 100 ; do
#	for i in 1 ; do \
#		./diffusion ./realData/gplus/gplus/gplusWithEdge.edge ~/graph-tool/realData/gplus/gplus $$i ; \
#		mv realData/result experiements/gplus/baseline/$$i ; \
#	done

epinions:
	for i in 1 2 3 4 5 10 20 30 40 50 60 80 100 ; do \
		./diffusion ./realData/epinions/epinionsWithEdge.edge ~/graph-tool/realData/epinions $$i ; \
		mv realData/result experiements/epinions/baseline/youtube/$$i ; \
	done

wiki:
	for j in 10 20 30 40 50 60 70 80 90 100 150 200 250 300 350 400 450 500 ; do \
		./diffusion ./realData/wikiVote/wikiVoteWithEdge.edge ~/graph-tool/realData/wikiVote $$j ; \
		mv realData/result experiements/wikiVote/community/$$j ; \
	done

clean:
	rm -f diffusion
#	rm -f *.o *.elf *.bin *.list
