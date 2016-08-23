// solve the "Diffusion Minimization on Specific Targets problem"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <float.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include<limits.h>
#include "diffusionMin.h"
#include "util.h"

void ReadGraph(const char * const file_edge, const char * const directory)
{
	/* read the social graph with vertices and the relationship between vertices */
	FILE *fp = read_file(file_edge);
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	bool firstline = true;

	puts("Initial Graph...");
	while((read = getline(&line, &len, fp)) != -1){
		if(firstline){
			firstline = false;
			InitializeVertices(atoi(line));
			continue;
		}
		else{
			StoreRelationship(line);
		}
	}
	free(line);
	puts("	END\n");

	/* read the vertices' feature with XX.feat and XX.featnames files */
	DIR *dp;
	struct dirent *entry;
	struct stat statbuf;
	int length = 0;
	char *filename = NULL;
	const char *dir = directory;

	if((dp = opendir(dir)) == NULL)	err("cannot open directory!\n");
	
	if(chdir(dir) == -1)	err("invalid directory\n");
	
	printf("Read feat and featnames file...\n");
	communityNum = 0;

	bool exists = false;
	featuresName = NULL;				// initialize `featuresName`
	while((entry = readdir(dp)) != NULL){
		lstat(entry->d_name,&statbuf);
		length = strlen(entry->d_name);

		if(strcmp(&entry->d_name[length-5], ".feat") == 0){  		// get the file with XX.feat ending
//			printf("read file : %s\n", entry->d_name);
			filename = malloc((length-4) * sizeof(char));
			strncpy(filename, entry->d_name, length-5);
			filename[length-5] = '\0';
			firstline = true;
			exists = false;
			line = NULL;
			FILE *fp2 = read_file(entry->d_name);
			while((read = getline(&line, &len, fp2)) != -1){
				if(firstline){
					firstline = false;
					if(featuresName){
						for(int i = 0 ; i < totalfeatures ; i++)
							free(featuresName[i]);
						free(featuresName);
					}
					totalfeatures = atoi(line);
//					printf("total features number is %d\n", totalfeatures);
					StoreFeaturesName(filename);				// read XX.featnames to store each node's feature
					continue;
				}
				else{
					bool test = StoreFeatures(line);
					if(test)
						exists = true;
				}
			}
			fclose(fp2);
			if(exists)
				communityNum++;
		}
	}

//	char *filename = NULL, *token1 = NULL, *token2 = NULL;
#ifdef SYN
	communityNum = 0;
	int comTmp = 0;
	char *token1 = NULL, *token2 = NULL;
	// another way to read community
	rewinddir(dp);
	printf("Read communities.txt file...\n");
	while((entry = readdir(dp)) != NULL){
		lstat(entry->d_name,&statbuf);

		// store each node's community. 
		if(strcmp(entry->d_name, "communities.txt") == 0){
			FILE *fc = fopen(entry->d_name, "r");
			while((read = getline(&line, &len, fc)) != -1)
			{
//				printf("%s", line);
				token1 = strtok(line, " ");
				if(*token1 == '#')
					continue;
				else{
					token2 = strtok(NULL, " ");
					comTmp = atoi(token2);
					StoreCommunity(token1, comTmp);
				}
				communityNum = MAX(communityNum, comTmp);
			}
			break;
		}
	}
	communityNum++;
	printf("store finished\n");
	if(chdir("..") == -1)
		err("invalid directory!\n");
#endif

	free(filename);
	closedir(dp);
	fclose(fp);

	return;
}

void StoreCommunity(char *token1, int token2)
{
//	extern struct Vertex **Users;
	int node = atoi(token1) % totalvertices;
	
//	printf("node id : %d\n", node);
	if(Users[node] != NULL){
		Users[node]->community = token2;
//		printf("successful store!\n");
	}
}

/* Create an array of struct Vertex pointer, which points to each user. */
void InitializeVertices(int number)
{
	printf("%d\n", number);
	totalvertices = number;
	Users = malloc(number * sizeof(struct Vertex *));
	for(int i = 0 ; i < number ; i++)
		Users[i] = NULL;
}

/* Include file.edge to store the users' relationship. */
void StoreRelationship(char *relation)
{
	char *token[3];
	char *p, *pEnd;
	int i = 0, node1 = 0, node2 = 0;

	p = strtok(relation, " ");
	while(p != NULL && i < 3){
		token[i++] = p;
		p = strtok(NULL, " ");
	}
	// test if user id is larger than int_64
	if(strtol(token[0], &pEnd, 10) == LLONG_MAX){
		char *tmp = malloc(8 * sizeof(char));
		strncpy(tmp, token[0], 8);
		node1 = atoi(tmp) % totalvertices;
	}
	else if(strtol(token[1], &pEnd, 10) == LLONG_MAX){
		char *tmp = malloc(8 * sizeof(char));
		strncpy(tmp, token[1], 8);
		node2 = atoi(tmp) % totalvertices;
	}
	else{
		node1 = atoi(token[0]) % totalvertices;
		node2 = atoi(token[1]) % totalvertices;
	}
	int closeness = atoi(token[2]);
	struct Neighbor *current = NULL;

	if(!Users[node1]){
		Users[node1] = malloc(sizeof(struct Vertex));
		Users[node1]->ID = node1;
		Users[node1]->label = NULL;
		Users[node1]->feature = NULL;
		Users[node1]->community = -1;
		Users[node1]->prev = NULL;
		Users[node1]->next = NULL;
	}
	if(!Users[node2]){
		Users[node2] = malloc(sizeof(struct Vertex));
		Users[node2]->ID = node2;
		Users[node2]->label = NULL;
		Users[node2]->feature = NULL;
		Users[node2]->community = -1;
		Users[node2]->prev = NULL;
		Users[node2]->next = NULL;
	}

	struct Neighbor *out_neighbor = malloc(sizeof(struct Neighbor));
	out_neighbor->ID = node2;
	out_neighbor->weight = closeness;
	out_neighbor->next = NULL;

	/* store node1's out neighbors */
	if(Users[node1]->next != NULL){
		current = Users[node1]->next;
		while(current->next != NULL){
			current = current->next;
		}
		current->next = out_neighbor;
	}
	else
		Users[node1]->next = out_neighbor;

	struct Neighbor *in_neighbor = malloc(sizeof(struct Neighbor));
	in_neighbor->ID = node1;
	in_neighbor->weight = closeness;
	in_neighbor->next = NULL;

	/* store node2's in neighbors */
	if(Users[node2]->prev != NULL){
		current = Users[node2]->prev;
		while(current->next != NULL){
			current = current->next;
		}
		current->next = in_neighbor;
	}
	else
		Users[node2]->prev = in_neighbor;

	return;
}

/* Normalize edge weight with w(u,v)/d(v) as the propagation probability. */
void NormalizeEdgeWeight(void)
{
	int i;
	double total = 0.0;
	struct Neighbor *current = NULL;

	printf("Normalize Edge Weight... \n");
	for(i = 0 ; i < totalvertices ; i++){
		if(Users[i] != NULL && Users[i]->prev != NULL){
			current = Users[i]->prev;
			total += current->weight;
			while(current->next != NULL){
				current = current->next;
				total += current->weight;
			}
		}
//		printf("user %d : total weight is %f\n", i, total);

		if(total != 0.0){
			current = Users[i]->prev;									// u->v , only consider v's in-neighbor.
			current->probability = current->weight/total;
			SyncOutNeighbor(current->ID, i, current->probability);		// synchronize out neightbor
//			printf("node %d to node %d probability is %f\n", current->ID, i, current->probability);
			while(current->next != NULL){
				current = current->next;
				current->probability = current->weight/total;
				SyncOutNeighbor(current->ID, i, current->probability);
//				printf("node %d to node %d probability is %f\n", current->ID, i, current->probability);
			}
		}
		total = 0.0;
	}

	return;
}

/* When normalize the edge weight with vertex's in-neighbor,
the in-neighbor's out-neighbor should be synchronize too. */
void SyncOutNeighbor(int u, int v, double probability)
{
	struct Neighbor *current = Users[u]->next;
	while(current != NULL || current->next != NULL){
		if(current->ID == v){
			current->probability = probability;
			break;
		}
		else
			current = current->next;
	}
}

/* When calculate the diffusion time with vertex's out-neighbor,
the out-neighbor's in-neighbor should be synchronize too. */
void SyncInNeighbor(int u, int v, double time)
{
	struct Neighbor *current = Users[v]->prev;
	while(current != NULL || current->next != NULL){
		if(current->ID == u){
			current->time = time;
			break;
		}
		else
			current = current->next;
	}
}

/* Store every vertex's feature in their struct. */
bool StoreFeatures(char *features)
{
	int i, count = 0;
	char *featurestmp = malloc((strlen(features)+1) * sizeof(char));

	strncpy(featurestmp, features, strlen(features));
	featurestmp[strlen(features)] = '\0';
	featurestmp = strtok(featurestmp, " ");

	int node = atoi(featurestmp) % totalvertices;
	if(Users[node] == NULL)
		return false;

	if(Users[node]->label == NULL && Users[node]->feature == NULL){
		Users[node]->label = malloc(totalfeatures * sizeof(int));
		Users[node]->feature = malloc(totalfeatures * sizeof(char *));
		for(int i = 0 ; i < totalfeatures ; i++)
			Users[node]->feature[i] = NULL;
	}
	else{
//		printf("duplicate features !!\n");
		return false;
	}

	#ifndef SYN
		Users[node]->community = communityNum;
	#endif

	for(i = 1 ; i <= totalfeatures ; i++){
		featurestmp = strtok(NULL, " ");
		if(featurestmp == NULL)
			break;
		else
			Users[node]->label[i-1] = atoi(featurestmp);

		if(Users[node]->label[i-1] == 1){
			Users[node]->feature[count] = malloc((strlen(featuresName[i-1])+1) * sizeof(char));
			strncpy(Users[node]->feature[count], featuresName[i-1], strlen(featuresName[i-1]));
			Users[node]->feature[count][strlen(featuresName[i-1])] = '\0';
			count++;
		}
	}
	/*  // Print each user's feature.
		printf("Users %d feature : \n\t", node);
		for(i = 0 ; i < totalfeatures ; i++){
			printf("%d ", Users[node]->label[i]);
		}
		printf("\n");
	*/
	return true;
}

/* Compute the diffusion time with 1/probability * 1/weight. */
void DiffusionTime(void)
{
	int i;
	struct Neighbor *current = NULL;

	for(i = 0 ; i < totalvertices ; i++){
		if(Users[i] != NULL && Users[i]->next != NULL){
			current = Users[i]->next;
			current->time = (1/current->probability) * (1/current->weight);
			SyncInNeighbor(i, current->ID, current->time);		// synchronize in neightbor
			while(current->next != NULL){
				current = current->next;
				current->time = (1/current->probability) * (1/current->weight);
				SyncInNeighbor(i, current->ID, current->time);
			}
		}
	}
}

/* Pick the minimum distance from the set of vertices not yet processed. */
int minDistance(double dist[], bool sptSet[])
{
	double min = DBL_MAX;
	int min_index, i;

	min_index = -1;
	for(i = 0 ; i < totalvertices ; i++){
		if(sptSet[i] == false && dist[i]<min){
			min = dist[i];
			min_index = i;
		}
	}

	return min_index;
}

void printPath(int dest, int prev[])
{
	if(prev[dest] != -1)
		printPath(prev[dest], prev);
//	printf("%d ", dest);
}

/* Find minimum time path using dijkstra's algorithm. */
double *FindMTP(int root, double *dist)
{
	bool *sptSet = malloc(totalvertices * sizeof(bool));		// shortest path tree Set
	int *prev = malloc(totalvertices * sizeof(int));			// the previous node
	int i, src, count;
	int min_index;
	struct Neighbor *current = NULL;

	src = root;

	for(i = 0 ; i < totalvertices ; i++){
		dist[i] = DBL_MAX;
		prev[i] = -1;
		sptSet[i] = false;
	}

	dist[src] = 0;		// Distance of source vertex from itself is 0

	for(count = 0 ; count < totalvertices-1 ; count++){

		/* Pick the minimum distance from the set of vertices not yet processed. */
		min_index = minDistance(dist, sptSet);
		if(min_index == -1)
			break;
		sptSet[min_index] = true;

		/* Update dist value of the adjacent vertices of the picked vertex. */
		if(Users[min_index] != NULL && Users[min_index]->prev != NULL){
			current = Users[min_index]->prev;
			if(!sptSet[current->ID] && current->time && dist[min_index]!=DBL_MAX && (dist[min_index]+current->time < dist[current->ID]) ){
				dist[current->ID] = dist[min_index]+current->time;
				prev[current->ID] = min_index;
			}
			while(current->next != NULL){
				current = current->next;
				if(!sptSet[current->ID] && current->time && dist[min_index]!=DBL_MAX && (dist[min_index]+current->time < dist[current->ID]) ){
					dist[current->ID] = dist[min_index]+current->time;
					prev[current->ID] = min_index;
				}
			}
		}
	}

	/* verify the algorithm is correct!? */
//	printf("Vertex		Distance(time) from source\n");
//	for(i = 0 ; i < totalvertices ; i++)
//		printf("%d\t\t%f\n", i, dist[i]);

//	printf("Get the path\n");
//	for(i = 0 ; i < totalvertices ; i++){
//		printf("node %d : ", i);
//		printPath(i, prev);
//		printf("\n");
//	}

	free(sptSet);
	free(prev);
	return dist;
}

/* On query processing, split the query, get the k influential nodes and the target features. */
void QueryProcessing(char *number)
{
	char *target_labels;	// total target labels

	printf("Input k : \n");
//	scanf("%d", &seedNumber);
	printf("Input specific targets using attributes with blank to separate : \n");
	printf("(for example : Basketball Curry ...)\n");
//	scanf("%s", labels);	// use target_labels to replace it temporarily

	seedNumber = atoi(number);

	target_labels = "13";
//	target_labels = "ff";
//	target_labels = "google";
//	target_labels = "basketball";
	printf("k is %d\nlabels are %s\n", seedNumber, target_labels);

	targetFeature = malloc((strlen(target_labels)+1) * sizeof(char));
	strncpy(targetFeature, target_labels, strlen(target_labels));
	targetFeature[strlen(target_labels)] = '\0';
	printf("target Features are : %s", targetFeature);

	printf("\n\n");
	return;
}

/* Compare the "label" is matched with targetFeature or not, and return true if matches. */
bool CompareFeatures(char *label)
{
	char *token = NULL;
	char *tmp = malloc((strlen(targetFeature)+1) * sizeof(char));
	strncpy(tmp, targetFeature, strlen(targetFeature));
	tmp[strlen(targetFeature)] = '\0';

	while((token = strtok_r(tmp, " ", &tmp)) != NULL){
		if(strcasecmp(label, token) == 0){
			return true;
		}
	}

	return false;
}

/* The FeaturesName array is corrensponding to each "file.featnames" */
void StoreFeaturesName(char *file_featnames)
{
	file_featnames = realloc(file_featnames, (strlen(file_featnames)+11) * sizeof(char));
	strcat(file_featnames, ".featnames");

	extern char **featuresName;
	FILE *fp = read_file(file_featnames);
	char *line1 = NULL;
	size_t len1 = 0;
	ssize_t read;
	int count = 0;

	featuresName = malloc((totalfeatures) * sizeof(char *));

	while((read = getline(&line1, &len1, fp)) != -1){

		char *token = strtok(line1, " ");
		if(token != NULL)
			token = strtok(NULL, "\n");

//		printf("token : %s %d\n", token, count);
		if(token == NULL){
			token = malloc(5 * sizeof(char));
			strncpy(token, "NULL", 4);
			token[4] = '\0';
		}

		featuresName[count] = malloc((strlen(token)+1) * sizeof(char));
		strncpy(featuresName[count], token, strlen(token));
		featuresName[count][strlen(token)] = '\0';
//		printf("\n%zu %s\n", strlen(token), featuresName[count]);
//		printf("store feature : %s \n", featuresName[count]);

		if(!allFeatures){
			allFeatures = malloc((strlen(token)+2) * sizeof(char));
			strncpy(allFeatures, token, strlen(token));
			allFeatures[strlen(token)] = ' ';
			allFeatures[strlen(token)+1] = '\0';
		}
		else{
			allFeatures = realloc(allFeatures, (strlen(allFeatures)+strlen(token)+2) * sizeof(char));

			strcat(allFeatures, token);
			strcat(allFeatures, " ");
		}

//		printf("all Features : %s\n", allFeatures);
		count++;

	}

	free(line1);
	fclose(fp);
	
	return;
}

/* When the query is comming, the diffsion probability should be recalculated
because of users' spontaneity. And record the target node at the same time.*/
int RecalProbability(void)
{
	int i, j, targetNum = 0, targetCount = 0;
	int intersections = 0;
	int unions = 0;
	struct Neighbor *current = NULL;
	bool voluntary = false;

	printf("Re calculate edge probability... \n");

	targetUsers = malloc(totalvertices * sizeof(int));
	for(int i = 0 ; i < totalvertices ; i++)
		targetUsers[i] = -1;

	char *tmp = malloc((strlen(targetFeature)+1) * sizeof(char));
	strncpy(tmp, targetFeature, strlen(targetFeature)*sizeof(char));
//	tmp[strlen(targetFeature)] = '\0';
	char *label = strtok(tmp, " ");				// calculate the total target features' number
	while(label != NULL){
		targetNum++;
		label = strtok(NULL, " ");
	}

	for(i = 0 ; i < totalvertices ; i++){
		if(Users[i] != NULL){
//			printf("Test user : %d\n", i);
			intersections = 0;
			unions = 0;
			j = 0;
			if(Users[i]->label != NULL){
				while(Users[i]->feature[j] != NULL){
					printf("%s %d\n", Users[i]->feature[j], j);
					if(CompareFeatures(Users[i]->feature[j])){
						intersections++;
						unions++;
					}
					else
						unions++;
					j++;
				}
			}
			/* if user's intersections is included whole query features, the user will be target. */
			if(intersections == targetNum){
				targetUsers[targetCount++] = i;
				printf("\nuser %d is target !!\n", i);
			}
			if(unions != 0){
//				printf("\nintersections:%d unions:%d -> %f\n", intersections, unions, (double)intersections/unions);
				if((double)intersections/unions > 0){
					voluntary = true;
				}
				else{
					voluntary = false;
//					printf("No need to update !\n");
				}

				if(Users[i]->next)
					current = Users[i]->next;
				else
					current = NULL;
				while(current != NULL && voluntary){
					printf("Previous : user %d to %d: %f\n", i, current->ID, current->probability);
					/* 1*propagation probability + self sontaniously * propagation probability */
					current->probability = 1*current->probability + ((double)intersections/unions)*current->probability;
					printf("\tuser %d to %d: %f\n", i, current->ID, current->probability);
					SyncInNeighborWithPro(i, current->ID, current->probability);
					current = current->next;
				}
			}
			else
				printf("No Features !!\n");
		}
	}

	if(targetCount == 0)
		err("Targets are not found !!\n");

	return targetCount;
}

/* When calculate the propagation probability with vertex's out-neighbor,
the out-neighbor's in-neighbor should be synchronize too. */
void SyncInNeighborWithPro(int u, int v, double probability)
{
	struct Neighbor *current = Users[v]->prev;
	while(current != NULL){
		if(current->ID == u){
			current->probability = probability;
			break;
		}
		else
			current = current->next;
	}
}

/* Re-Normalize edge probability with p(u,v)/total(p(v)). */
void ReNormalizeEdgeProbability(void)
{
	int i;
	double total = 0.0;
	struct Neighbor *current = NULL;

	printf("Re Normalize edge probability : \n");
	for(i = 0 ; i < totalvertices ; i++){
		if(Users[i] != NULL && Users[i]->prev != NULL){
			current = Users[i]->prev;
			total += current->probability;

			while(current->next != NULL){
				current = current->next;
				total += current->probability;
			}
		}

//		printf("user %d : total probability is %f\n", i, total);

		if(total != 0.0){
			current = Users[i]->prev;									// u->v , only consider v's in-neighbor.
			current->probability = current->probability/total;
			SyncOutNeighbor(current->ID, i, current->probability);		// synchronize out neightbor
//			printf("\tuser %d to %d -> probability is %f\n", current->ID, i, current->probability);
			while(current->next != NULL){
				current = current->next;
				current->probability = current->probability/total;
				SyncOutNeighbor(current->ID, i, current->probability);
//				printf("\tuser %d to %d -> probability is %f\n", current->ID, i, current->probability);
			}
		}

		total = 0.0;
	}

	return;
}

/* Sort the distToTarget array using bubble sort. "end" is true means get
the last one index, it also means get the maximum value index, vice versa. */
int BubbleSort(double *distToTarget, bool end, int size)
{
	int i, j, itmp;
	double tmp;
	int index[size];

	for(i = 0 ; i < size ; i++)
		index[i] = i;

	for(i = 0 ; i < size-1 ; i++){
		for(j = 0 ; j < size-i-1 ; j++){
			if(distToTarget[j] > distToTarget[j+1]){
				tmp = distToTarget[j];		// value exchange
				distToTarget[j] = distToTarget[j+1];
				distToTarget[j+1] = tmp;

				itmp = index[j];			// index exchange
				index[j] = index[j+1];
				index[j+1] = itmp;
			}
		}
	}

	if(end)
		return index[size-1];
	else
		return index[0];
}

/* "node" is include seedset or not */
bool isInclude(int node ,int *seedSet)
{
	int i;
	for(i = 0 ; i < seedNumber ; i++){
		if(seedSet[i] == node)
			return true;
	}
	return false;
}

double *InitializeEachReduce(double *eachReduce, int number)
{
	int i;
	for(i = 0 ; i < number ; i++)
		eachReduce[i] = 0;
	return eachReduce;
}

/* Baseline algorithm for finding top k users causing the minimum diffusion time.
Use Greedy to get the maximum marginal gain. */
void Baseline(int targetCount)
{
	int i, j, top1;
	int topk = seedNumber;
	int seedSet[seedNumber];
	bool best = true;
	double diffusionTime = 0.0, time_spent;
	extern clock_t begin, end;

	/* create a 2D array distToTargets[eachTarget][eachNode] */
	double **distToTargets = malloc(targetCount * sizeof(double *));
	double *firstRound = malloc(totalvertices * sizeof(double));		// store max time of each node to targets
	double *targets = malloc(targetCount * sizeof(double));				// the minimum time to arrive each target

	/* store each vertex to target causes the diffusion time less */
	double *eachReduce = malloc(totalvertices * sizeof(double));

	/*	initialize targets array, distToTargets array, firstRound array, eachReduce array. */
	for(i = 0 ; i < targetCount ; i++){
		targets[i] = DBL_MAX;
		distToTargets[i] = malloc(totalvertices * sizeof(double));
		for(j = 0 ; j < totalvertices ; j++){
			distToTargets[i][j] = -1;
		}
	}
	for(i = 0 ; i < totalvertices ; i++){
		firstRound[i] = -1;
		eachReduce[i] = 0;
	}
	memset(seedSet, -1, seedNumber * sizeof(int));

	FILE *f = write_file("result");
	if(f == NULL) err("Couldn't open file.\n");
	fprintf(f, "number of targets : %d\n", targetCount);

	/* Get the first seed. Then get the next seed by recording the marginal gain of each node. */
	int count = 0;
	while(topk > 0){
		InitializeEachReduce(eachReduce, totalvertices);
		best = true;														// check the diffusion time has improved or not

		for(i = 0 ; i < targetCount ; i++){
			distToTargets[i] = FindMTP(targetUsers[i], distToTargets[i]);	// calculate the time of each node to the target
			for(j = 0 ; j < totalvertices ; j++){
				if(count == 0){
					firstRound[j] = MAX(firstRound[j], distToTargets[i][j]);
//					printf("node %d to node %d : %f\n", j, targetUsers[i], distToTargets[i][j]);
					best = false;
				}
				/* if node has contribution of diffusion time, then store it into "eachReduce" to get the most one. */
				else if(!isInclude(j, seedSet) && distToTargets[i][j]<targets[i]){
					eachReduce[j] += targets[i]-distToTargets[i][j];
					best = false;
				}
			}
		}
		// if there is no node has contribution , then quit the algorithm.
		if(best){
			printf("No more seeds !!\nIt's the shortest diffusion time !!\n");
			break;
		}
		if(count == 0)
			top1 = BubbleSort(firstRound, false, totalvertices);
		else
			top1 = BubbleSort(eachReduce, true, totalvertices);

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
	printf("\nnumber of targets : %d\n", targetCount);
	printf("\nseed set are : \n");
	for(i = 0 ; i < count ; i++)
		printf("%d ", seedSet[i]);
	printf("\nDiffusion Time is %lf\n", diffusionTime);

	fclose(f);
	free(distToTargets);
	free(firstRound);
	free(targets);
	free(eachReduce);
	return;
}


/* Show useful message to confirm it. */
void printGraph(void)
{
	int i;
	struct Neighbor *current = NULL;

	for(i = 0 ; i < totalvertices ; i++)
	{
		if(Users[i] != NULL)
		{
			printf("node %d next: \n", Users[i]->ID);
			if(Users[i]->next != NULL)
			{
				current = Users[i]->next;
				printf("\tnode %d\n", current->ID);
				printf("\tprobability %f\n", current->probability);
				printf("\tdiffusion time %f\n", current->time);
				while(current->next != NULL)
				{
					current = current->next;
					printf("\tnode %d\n", current->ID);
					printf("\tprobability %f\n", current->probability);
					printf("\tdiffusion time %f\n", current->time);
				}
			}
			printf("prev: \n");
			if(Users[i]->prev != NULL)
			{
				current = Users[i]->prev;
				printf("\tnode %d\n", current->ID);
				printf("\tprobability %f\n", current->probability);
				printf("\tdiffusion time %f\n", current->time);
				while(current->next != NULL)
				{
					current = current->next;
					printf("\tnode %d\n", current->ID);
					printf("\tprobability %f\n", current->probability);
					printf("\tdiffusion time %f\n", current->time);
				}
			}
		}
	}
	return;
}

void CalMinPro(void)
{
	double minimum = DBL_MAX;
	double maximum = 0.0;
	struct Neighbor *current = NULL;
	
	for(int i = 0 ; i < totalvertices && Users[i] != NULL ; i++){
		if(Users[i]->next != NULL){
			current = Users[i]->next;
			maximum = MAX(current->probability, maximum);
			minimum = MIN(current->probability, minimum);
		}
	}
	printf("minimum prbability is : %lf\n", minimum);
	printf("maximum prbability is : %lf\n", maximum);
	if(system("read var1") != -1)	err("system error!!\n");
}

int main(int argc, char **argv)
{
	int targetCount;
	extern clock_t begin, end;
	double time_spent;

	dataset = malloc((strlen(argv[1])+1) * sizeof(char));
	directory = malloc((strlen(argv[2])+1) * sizeof(char));
	strncpy(dataset, argv[1], strlen(argv[1]));
	strncpy(directory, argv[2], strlen(argv[2]));

	ReadGraph(dataset, directory);
	NormalizeEdgeWeight();

	begin = clock();
	
	QueryProcessing(argv[3]);
	targetCount = RecalProbability();

	printf("target number : %d\n", targetCount);
	puts("targets are : \n");
	for(int i = 0 ; i < targetCount ; i++){
		printf("%d ", targetUsers[i]);
	}
//	getchar();

	ReNormalizeEdgeProbability();
	DiffusionTime();

//	printf("\ncalcuate minimum probability:\n");
//	CalMinPro();
//	printf("\nPrint Graph Information : \n");
//	printGraph();

	#ifdef BASE
		printf("Run Baseline Algorithm!\n");
		Baseline(targetCount);
	#endif
	#ifdef LDT
		printf("Run LD Tree Algorithm!\n");
		LD_Tree(targetCount);
	#endif
	#ifdef COM
		printf("Run Community-based Algorithm\n");
		Community_based(targetCount);
	#endif
	#ifdef OPT
		printf("Run Optimal\n");
		Optimal(targetCount);
	#endif

	end = clock();
	time_spent = (double)(end-begin) / CLOCKS_PER_SEC;
	printf("execution time : %f\n", time_spent);

	return 0;
}
