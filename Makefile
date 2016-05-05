CFLAGS = -Wall -Werror -std=gnu99 -O3 -g
CC = gcc
TOPK = 1
BIN = diffusion1
TARGET = diffusion
COM = no

all: $(BIN)

diffusion1: diffusionMin.c LDTree.c community.c
	@echo "make baseline algorithm!"
ifeq ($(COM), no)
	$(CC) $(CFLAGS) -DBASE -o $(TARGET) $^
else ifeq ($(COM), yes)
	$(CC) $(CFLAGS) -DBASE -DSYN -o $(TARGET) $^
else
	exit 1;
endif

diffusion2: diffusionMin.c LDTree.c community.c
	@echo "make LDT algorithm!"
ifeq ($(COM), no)
	$(CC) $(CFLAGS) -DLDT -o $(TARGET) $^
else ifeq ($(COM), yes)
	$(CC) $(CFLAGS) -DLDT -DSYN -o $(TARGET) $^
else
	exit 1;
endif

diffusion3: diffusionMin.c LDTree.c community.c
	@echo "make community-based algorithm!"
ifeq ($(COM), no)
	$(CC) $(CFLAGS) -DCOM -o $(TARGET) $^
else ifeq ($(COM), yes)
	$(CC) $(CFLAGS) -DCOM -DSYN -o $(TARGET) $^
else
	exit 1;
endif

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
	./diffusion ./realData/facebook/facebookWithEdge.edge ./realData/facebook 10;
#	for j in 1 2 3 4 5 ; do \
		for i in 1 2 3 4 5 10 20 30 40 50 60 80 ; do \
			./diffusion ./realData/facebook/facebookWithEdge.edge ./realData/facebook $$i ; \
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
	./diffusion ./realData/epinions/epinionsWithEdge.edge ./realData/epinions 10 ;
#	for j in 1 2 3 4 5 ; do \
		for i in 10 12 14 16 18 20 22 24 ; do \
			./diffusion ./realData/epinions/epinionsWithEdge.edge ./realData/epinions $$i ; \
			mv realData/result experiements/epinions/community/google/$$i ; \
		done \
	done

wiki:
#	./diffusion ./realData/wikiVote/wikiVoteWithEdge.edge ./realData/wikiVote 10 ;
	for i in 1 2 3 4 5 ; do \
		for j in 20 22 24 26 28 30 32 ; do \
			./diffusion ./realData/wikiVote/wikiVoteWithEdge.edge ./realData/wikiVote $$j ; \
			mv realData/wikiVote/result experiements/wikiVote/community/google/$$j.$$i ; \
		done \
	done

1000:
#	for i in 1 5 10 20 30 40 50 ; do 
	for i in 2; do \
		for j in 1 2 3 4 5 ; do \
			./diffusion ./syntheticData/1000/graphWithEdge.edge ./syntheticData/1000 $$i ; \
			mv syntheticData/result experiements/syntheticData/1000/LDT/$$i.$$j ; \
		done \
	done
#mv syntheticData/result experiements/syntheticData/1000/community/$$i.$$j ;

5000:
#	for i in 1 5 10 20 30 40 50 ; do 
	for i in 2 ; do \
		for j in 1 2 3 4 5 ; do \
			./diffusion ./syntheticData/5000/graphWithEdge.edge ./syntheticData/5000 $$i ; \
			mv syntheticData/result experiements/syntheticData/5000/LDT/$$i.$$j ; \
		done \
	done

10000:
	for i in 1 5 10 20 30 40 50 ; do \
		for j in 1 ; do \
			./diffusion ./syntheticData/10000/graphWithEdge.edge ./syntheticData/10000 $$i ; \
			mv syntheticData/result experiements/syntheticData/10000/community/$$i.$$j ; \
		done \
	done

40000:
	for i in 10 ; do \
		for j in 2 3 4 5 ; do \
			./diffusion ./syntheticData/40000/graphWithEdge.edge ./syntheticData/40000 $$i ; \
			mv syntheticData/result experiements/syntheticData/40000/baseline/$$i.$$j ; \
		done \
	done


60000:
	./diffusion ./syntheticData/60000/graphWithEdge.edge ./syntheticData/60000 10 ;

80000:
	./diffusion ./syntheticData/80000/graphWithEdge.edge ./syntheticData/80000 10 ;

100000:
	for i in 10 ; do \
		for j in 1 ; do \
			./diffusion ./syntheticData/100000/graphWithEdge.edge ./syntheticData/100000 $$i ; \
			mv syntheticData/result experiements/syntheticData/100000/LDT/$$i.$$j ; \
		done \
	done

clean:
	rm -f diffusion
#	rm -f *.o *.elf *.bin *.list
