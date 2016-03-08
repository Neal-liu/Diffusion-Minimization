#include <stdio.h>
#include <float.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "diffusionMin.h"
#include "util.h"

/* Store the "node" influencer into the "UsersLD" tree. */
void StoreInfluencer(int node, int id, double time)
{
	struct Influencer *influencer = malloc(sizeof(struct Influencer));
	struct Influencer *current = NULL;

	influencer->ID = id;
	influencer->time = time;
	influencer->next = NULL;

	if(UsersLD[node]->prev != NULL){
		current = UsersLD[node]->prev;
		while(current->next != NULL)
			current = current->next;
		current->next = influencer;
	}
	else
		UsersLD[node]->prev = influencer;

	return;
}

/* Finding the minimum time path to build each node's local diffusion tree with a threshold bound. */
void FindMTPwithTree(int root, double threshold, int order)
{
	double *dist = malloc(totalvertices * sizeof(double));		// distance from each node to target
	bool *sptSet = malloc(totalvertices * sizeof(bool));		// shortest path tree Set
	int *prev = malloc(totalvertices * sizeof(int));			// the previous node
	int i, src, count, min_index;
	struct Neighbor *current = NULL;
	double minTime;

	/* intialize userLD[root] */
	UsersLD[root] = malloc(sizeof(struct VertexLD));
	UsersLD[root]->ID = root;
	UsersLD[root]->prevPath = malloc(totalvertices * sizeof(int));
	UsersLD[root]->prev = NULL;

	src = root;

	for(i = 0 ; i < totalvertices ; i++){
		dist[i] = DBL_MAX;
		prev[i] = -1;
		sptSet[i] = false;
	}

	dist[src] = 0;											// Distance of source vertex from itself is 0

	for(count = 0 ; count < totalvertices-1 ; count++){

		min_index = minDistance(dist, sptSet);				// Pick the minimum distance from the set of vertices not yet processed

		if(min_index == -1)									// can not find the minimum time vertex, no more visit.
			break;
		sptSet[min_index] = true;
		if(dist[min_index] < threshold && dist[min_index] != 0)
			StoreInfluencer(root, min_index, dist[min_index]);

		/* Update dist value of the adjacent vertices of the picked vertex. */
		if(Users[min_index] != NULL && Users[min_index]->prev != NULL){
			current = Users[min_index]->prev;
			minTime = current->time;
			if(!sptSet[current->ID] && current->time && dist[min_index]!=DBL_MAX && (dist[min_index]+minTime < dist[current->ID]) && (dist[min_index]+minTime < threshold) ){
				dist[current->ID] = dist[min_index]+minTime;
				prev[current->ID] = min_index;
			}
			while(current->next != NULL){
				current = current->next;
				minTime = current->time;
				if(!sptSet[current->ID] && current->time && dist[min_index]!=DBL_MAX && (dist[min_index]+minTime < dist[current->ID]) && (dist[min_index]+minTime < threshold) ){
					dist[current->ID] = dist[min_index]+minTime;
					prev[current->ID] = min_index;
				}
			}
		}
	}

	/* copy the prev[] array to UsersLD array. It will use in block issue. */
	memcpy(UsersLD[root]->prevPath, prev, totalvertices * sizeof(int));

	// copy the dist[] to BoundDist global pointer
	BoundDist[order] = dist;

	/*
		// verify the algorithm is correct!?
		printf("Vertex , Distance(time) from source\n");
		for(i = 0 ; i < totalvertices ; i++){
			printf("%d\t\t%f\n", i, dist[i]);
		}

		printf("Get the path\n");
		for(i = 0 ; i < totalvertices ; i++){
			printf("node %d : ", i);
			printPath(i, UsersLD[root]->prevPath);
			printf("\n");
		}

		printf("root is %d\n", root);
		printf("Local Diffusion Tree :\n");
		if(UsersLD[root]->prev){
			struct Influencer *currents = UsersLD[root]->prev;
			printf("prev id is %d\n", currents->ID);
			printf("\ttime is %f\n", currents->time);

			while(currents->next != NULL && currents->ID != 0){
				currents = currents->next;
				printf("prev id is %d\n", currents->ID);
				printf("\ttime is %f\n", currents->time);
			}
		}
		else
			printf("No local diffusion tree !!\n");

		printf("DONE!\n");
		if(system("read var1") == -1) err("system pause error!\n");
	*/
	free(sptSet);
	free(prev);

}

/* Add the node id into candidates, candidates must be unique. */
int AddCandidate(int *candidates, int id)
{
	int i;
	for(i = 0 ; i < totalvertices ; i++){
		if(candidates[i] == id)
			break;
		else if(candidates[i] == -1){
			candidates[i] = id;
			break;
		}
	}

	return i+1;				// return number of candidates
}

/* Choose the targets and its local diffusion tree as the candidates. */
int ChooseCandidates(int targetCount, int *candidates)
{
	int i;
	struct Influencer *current = NULL;
	int candidatesNum = 0, candidatesTmp;

	memset(candidates, -1, totalvertices * sizeof(int));
	memcpy(candidates, targetUsers, targetCount * sizeof(int));
//	for(i = 0 ; i < totalvertices ; i++){
//		printf("%d ", candidates[i]);
//	}

	candidatesNum = targetCount;
	printf("\n");

	for(i = 0 ; i < targetCount ; i++){
		current = UsersLD[targetUsers[i]]->prev;
		if(current != NULL){
			printf("\ntarget %d is %d\n", i, targetUsers[i]);
			printf("its tree is : %d ", current->ID);
			candidatesTmp = AddCandidate(candidates, current->ID);
			candidatesNum = MAX(candidatesNum, candidatesTmp);

			while(current->next != NULL){
				current = current->next;
				printf("%d ", current->ID);
				candidatesTmp = AddCandidate(candidates, current->ID);
				candidatesNum = MAX(candidatesNum, candidatesTmp);
			}

		}
		else
			continue;
	}

	return candidatesNum;
}

/* Choose candidates with sorted list. */
int ChooseCandidatesWithSL(int targetCount, int *candidates)
{
	int i, j;
	int level = 0;
	struct Influencer *current = NULL;
	int candidatesNum = 0, candidatesTmp;
	int *hashTable = malloc(totalvertices * sizeof(int));

	memset(hashTable, 0, totalvertices * sizeof(int));
	memset(candidates, -1, totalvertices * sizeof(int));
	memcpy(candidates, targetUsers, targetCount * sizeof(int));
	candidatesNum = targetCount;

	for(i = 0 ; i < totalvertices ; i++){
		for(j = 0 ; j < targetCount ; j++){
			level = 0;
			current = UsersLD[targetUsers[j]]->prev;
			while(current != NULL && level != i){
				current = current->next;
				level++;
			}
			if(current == NULL)
				continue;
			else if(level == i){
				candidatesTmp = AddCandidate(candidates, current->ID);
				candidatesNum = MAX(candidatesNum, candidatesTmp);
				printf("Add %d as candidate!\n", current->ID);
				hashTable[current->ID]++;
				if(hashTable[current->ID] == targetCount)
					return candidatesNum;
			}
		}
	}

	free(hashTable);
	printf("No seed can reach to all targets.\n");
	return 0;
}


/* Check prevID is included in seedSet or not. */
bool isIncludeSeeds(int prevID, int *seedSet)
{
	int i;

	for(i = 0 ; i < seedNumber ; i++){
		if(prevID == seedSet[i])
			return true;
	}

	return false;
}

/* Recursively check the previous id. */
bool MeetSeed(int targetID, int candidateID, int *seedSet)
{
	int prevID = UsersLD[targetID]->prevPath[candidateID];
	if(prevID != -1){
		if(isIncludeSeeds(prevID, seedSet))
			return true;
		MeetSeed(targetID, prevID, seedSet);
	}
	else
		return false;

	return false;
}

/* Check the candidate go through the target will meet seeds or not. */
bool isBlock(int targetID, int candidateID, int *seedSet)
{

//	printf("\ncandidate %d \n", candidateID);

	bool match = MeetSeed(targetID, candidateID, seedSet);
	if(match){
//		printf("\nBlock!!\n");
		return true;
	}
	else
		return false;

}

void FindSeeds(int targetCount, int *candidates, int candidatesNum)
{
	int i, j, top1;
	int topk = seedNumber;
	int *seedSet = malloc(seedNumber * sizeof(int));
	bool best = true;
	double diffusionTime, time_spent;
	extern clock_t begin, end;

	double **distToTargets = malloc(targetCount * sizeof(double *));	// create a 2D array distToTargets[eachTarget][totalvertices]
	double *firstRound = malloc(candidatesNum * sizeof(double));		// store max time of each candidate to targets
	double *targets = malloc(targetCount * sizeof(double));				// the minimum time to arrive each target
	double *eachReduce = malloc(candidatesNum * sizeof(double));		// store each candidate to target causes the diffusion time less

	/*	initialize targets array, distToTargets array, firstRound array, eachReduce array and seedSet */
	for(i = 0 ; i < targetCount ; i++){
		targets[i] = DBL_MAX;
		distToTargets[i] = malloc(totalvertices * sizeof(double));
		for(j = 0 ; j < totalvertices ; j++)
			distToTargets[i][j] = -1;
	}
	for(i = 0 ; i < candidatesNum ; i++){
		firstRound[i] = -1;
		eachReduce[i] = 0;
	}
	memset(seedSet, -1, seedNumber * sizeof(int));

	FILE *f = write_file("result");
	if(f == NULL) err("Couldn't open file.\n");
	fprintf(f, "number of candidates : %d\n", candidatesNum);
	fprintf(f, "number of targets : %d\n", targetCount);

	/* Get the first seed. Then get the next seed by recording the marginal gain of each candidate. */
	int count = 0;
	while(topk > 0){
		InitializeEachReduce(eachReduce, candidatesNum);
		best = true;														// check the diffusion time has improved or not

		for(i = 0 ; i < targetCount ; i++){
			distToTargets[i] = BoundDist[i];								// the time of each node to the target
			for(j = 0 ; j < candidatesNum ; j++){
				if(count == 0){
					firstRound[j] = MAX(firstRound[j], distToTargets[i][candidates[j]]);
//					printf("node %d to node %d : %f\n", candidates[j], targetUsers[i], distToTargets[i][candidates[j]]);
					best = false;
				}
				/* check the candidate is blocked by seed or not. */
				else if(isBlock(targetUsers[i], candidates[j], seedSet)){
					continue;
				}
				/* if node has contribution of diffusion time, then store it into "eachReduce" to get the most one. */
				else if(!isInclude(candidates[j], seedSet) && distToTargets[i][candidates[j]]<targets[i]){
					eachReduce[j] += targets[i]-distToTargets[i][candidates[j]];
					best = false;
//					printf("\ncandidates %d has contribution!\n\n", candidates[j]);
				}
			}
		}

		// if there is no node has contribution , then quit the algorithm.
		if(best){
			printf("No more seeds !!\nIt's the shortest diffusion time !!\n");
			break;
		}
		if(count == 0)
			top1 = BubbleSort(firstRound, false, candidatesNum);
		else
			top1 = BubbleSort(eachReduce, true, candidatesNum);

		top1 = candidates[top1];
		seedSet[count++] = top1;											// store top1 to seed set
		diffusionTime = 0.0;
//		printf("top %d is %d\n", count, top1);
		for(i = 0 ; i < targetCount ; i++){
			if(distToTargets[i][top1] < targets[i])
				targets[i] = distToTargets[i][top1];
			diffusionTime = MAX(diffusionTime, targets[i]);
//			printf("top %d to node %d is : %f\n\n", count, targetUsers[i], targets[i]);
		}

		topk--;

		fprintf(f, "top%d", count);
		fprintf(f, "\nseed set are : \n\t");
		for(i = 0 ; i < count ; i++)
			fprintf(f, "%d ", seedSet[i]);
		fprintf(f, "\nDiffusion Time is %lf\n", diffusionTime);

		end = clock();
		time_spent = (double)(end-begin) / CLOCKS_PER_SEC;
		fprintf(f, "execution time : %f\n", time_spent);
	}

	printf("targets are : \n");
	for(i = 0 ; i < targetCount ; i++)
		printf("%d ", targetUsers[i]);
	printf("\nseed set are : \n");
	for(i = 0 ; i < count ; i++)
		printf("%d ", seedSet[i]);
	printf("\nDiffusion Time is %lf\n", diffusionTime);

	fclose(f);
}

void LD_Tree(int targetCount)
{
	int i = 0;
//    double threshold = 1.0;
	double threshold = 234;
	int *candidates = malloc(totalvertices * sizeof(int));
	int candidatesNum;

	/* initialize vertex with local diffusion tree */
	UsersLD = malloc(totalvertices * sizeof(struct VertexLD *));
	BoundDist = malloc(targetCount * sizeof(double *));

	while(targetUsers[i] != -1){
		BoundDist[i] = malloc(totalvertices * sizeof(double));
		FindMTPwithTree(targetUsers[i], threshold, i);
		i++;
	}

	/* union the LD tree as the candidates */
	candidatesNum = ChooseCandidatesWithSL(targetCount, candidates);
	if(candidatesNum == 0)
		candidatesNum = ChooseCandidates(targetCount, candidates);

	printf("\nnumbers of candidates : %d\n", candidatesNum);
	for(i = 0 ; i < totalvertices ; i++){
		if(candidates[i] != -1)
			printf("%d ", candidates[i]);
	}

	FindSeeds(targetCount, candidates, candidatesNum);
}


