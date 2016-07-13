#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <time.h>
#include "diffusionMin.h"
#include "util.h"

int *comb;
int *optComb;
int targetNumber;
double totalDiffusionT = DBL_MAX;


void getDiffusionT(int *result)
{
	int i, j;
	double diffusionTime = 0.0;

	/* create a 2D array distToTargets[eachTarget][eachNode] */
	double **distToTargets = malloc(targetNumber * sizeof(double *));
	double *targetsReceive = malloc(targetNumber * sizeof(double));		// the minimum time to arrive each target

	/*	initialize targets array, distToTargets array, firstRound array, eachReduce array. */
	for(i = 0 ; i < targetNumber ; i++){
		targetsReceive[i] = DBL_MAX;
		distToTargets[i] = malloc(totalvertices * sizeof(double));
		for(j = 0 ; j < totalvertices ; j++){
			distToTargets[i][j] = -1;
		}
	}

	
	for(int i = 0 ; i < targetNumber ; i++){
			distToTargets[i] = FindMTP(targetUsers[i], distToTargets[i]);	// calculate the time of each node to the target
		for(int j = 0 ; j < seedNumber ; j++){			

//			printf("node %d to node %d : %f\n", result[j], targetUsers[i], distToTargets[i][result[j]]);
			targetsReceive[i] = MIN(targetsReceive[i], distToTargets[i][result[j]]);
		}
	}

	for(int i = 0 ; i < targetNumber ; i++){
		diffusionTime = MAX(diffusionTime, targetsReceive[i]);
	}	
//	printf("The diffusion time is %lf\n", diffusionTime);

	if(diffusionTime < totalDiffusionT){
		totalDiffusionT = diffusionTime;
		for(int i = 0 ; i < seedNumber ; i++){
			optComb[i] = result[i];
		}
	}

//	getchar();
	for(i = 0 ; i < targetNumber ; i++)
		free(distToTargets[i]);
	free(distToTargets);
	free(targetsReceive);
}


void listComb(int total, int len, int startPosition, int *result)
{
	if(len == 0){
//		for(int i = 0 ; i < seedNumber ; i++)
//			printf("%d ", result[i]);
		
		getDiffusionT(result);
//		puts("\n");
		return;
	}
	for(int i = startPosition ; i<= total-len ; i++){
		result[seedNumber - len] = i;
		listComb(total, len-1, i+1, result);
	}

}


void Optimal(int targetCount)
{
//	double diffusionTime = 0.0, time_spent;
//	extern clock_t begin, end;

	targetNumber = targetCount;
	comb = malloc(seedNumber * sizeof(int));
	optComb = malloc(seedNumber * sizeof(int));

	listComb(totalvertices, seedNumber, 0, comb);

//	end = clock();
//	time_spent = (double)(end-begin) / CLOCKS_PER_SEC;
//	fprintf(f, "execution time : %f\n", time_spent);

	printf("targets are : \n");
	for(int i = 0 ; i < targetCount ; i++)
		printf("%d ", targetUsers[i]);
	printf("\nnumber of targets : %d\n", targetCount);
	puts("Final seeds are : \n");
	for(int i = 0 ; i < seedNumber ; i++){
		printf("%d ", optComb[i]);
	}
	printf("\nDiffusion Time is %lf\n", totalDiffusionT);
}
