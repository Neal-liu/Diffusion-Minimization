CFLAGS = -Wall -Werror -std=gnu99 -O3 -g
CC = gcc
TOPK = 1
BIN = diffusion1
TARGET = diffusion

all: $(BIN)

diffusion1: diffusionMin.c LDTree.c community.c
	$(CC) $(CFLAGS) -DBASE -o $(TARGET) $^

diffusion2: diffusionMin.c LDTree.c community.c
	$(CC) $(CFLAGS) -DLDT -o $(TARGET) $^

diffusion3: diffusionMin.c LDTree.c community.c
	$(CC) $(CFLAGS) -DCOM -o $(TARGET) $^

syntheticsmall:
	./diffusion ./syntheticData/small/synthetic.edge ./syntheticData/small 1

synthetic2:
	./diffusion ./syntheticData/synthetic2.edge ./syntheticData 1

twitter:
	./diffusion ./realData/twitter/twitterWithEdge.edge ./realData/twitter 10 ;
#	for i in 1 2 3 4 5 10 20 30 40 50 60 80 100 150 200 300 400 500 ; do \
		./diffusion ./realData/twitter/twitterWithEdge.edge ~/graph-tool/realData/twitter $$i ; \
		mv realData/result experiements/twitter/baseline/youtube/$$i ; \
	done
	
facebook:
	./diffusion ./realData/facebook/facebookWithEdge.edge ./realData/facebook 10 ;
#	for j in 1 ; do \
		for i in 300 400 500 ; do \
			./diffusion ./realData/facebook/facebookWithEdge.edge ~/graph-tool/realData/facebook $$i ; \
			mv realData/result experiements/facebook/community/feature0/$$i.$$j ; \
		done \
	done
	
gplus:
	./diffusion ./realData/gplus/gplus/gplusWithEdge.edge ./realData/gplus/gplus 1
#	for i in 1 2 3 4 5 10 20 30 40 50 60 80 100 ; do
#	for i in 1 ; do \
#		./diffusion ./realData/gplus/gplus/gplusWithEdge.edge ~/graph-tool/realData/gplus/gplus $$i ; \
#		mv realData/result experiements/gplus/baseline/$$i ; \
#	done

epinions:
		for i in 20 40 60 80 100 150 200; do \
			./diffusion ./realData/epinions/epinionsWithEdge.edge ./realData/epinions $$i ; \
			mv realData/result experiements/epinions/community/google/$$i ; \
		done

wiki:
	./diffusion ./realData/wikiVote/wikiVoteWithEdge.edge ./realData/wikiVote 10 ;
#	for j in 10 20 30 40 50 60 80 100 150 200 ; do \
		./diffusion ./realData/wikiVote/wikiVoteWithEdge.edge ~/graph-tool/realData/wikiVote $$j ; \
		mv realData/result experiements/wikiVote/communtiy/youtube/$$j ; \
	done 

clean:
	rm -f diffusion
#	rm -f *.o *.elf *.bin *.list
