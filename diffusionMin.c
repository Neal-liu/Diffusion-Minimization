#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <float.h>
#include <string.h>
#include "diffusionMin.h"

void ReadGraph(void)
{
	FILE *fp = fopen("synthetic.edge", "r");
	FILE *fp2 = fopen("synthetic.feat", "r");
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	bool firstline = true;

	while((read = getline(&line, &len, fp)) != -1){
//		printf("%s", line);
		if(firstline){
			firstline = false;
			InitializeVertices(atoi(line));
			continue;
		}
		else {
			StoreRelationship(line);
		}
	}

	firstline = true;
	while((read = getline(&line, &len, fp2)) != -1){
//		printf("%s", line);
		if(firstline){
			firstline = false;
			totalfeatures = atoi(line);
			printf("total features number is %d\n", totalfeatures);
			continue;
		}
		else {
			StoreFeatures(line);
		}
	}

	fclose(fp);
	fclose(fp2);
	return;
}

/* Create an array of struct Vertex pointer, which points to each user. */
void InitializeVertices(int number)
{
//	printf("%d\n", number);
	totalvertices = number;
	Users = malloc(number * sizeof(struct Vertex *));
	return;
}

/* Include file.edge to store the users' relationship. */
void StoreRelationship(char *relation)
{
	int node1 = atoi(relation);
	int node2 = atoi(relation + 2);
	int closeness = atoi(relation + 4);
	struct Neighbor *current = NULL;

	if(!Users[node1]){
		Users[node1] = malloc(sizeof(struct Vertex));
		Users[node1]->ID = node1;
//		Users[node1]->update = false;
		Users[node1]->prev = NULL;
		Users[node1]->next = NULL;
//		printf("create node %d\n", node1);
	}
	if(!Users[node2]){
		Users[node2] = malloc(sizeof(struct Vertex));
		Users[node2]->ID = node2;
//		Users[node2]->update = false;
		Users[node2]->prev = NULL;
		Users[node2]->next = NULL;
//		printf("create node %d\n", node2);
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

	printf("Normalize Edge Weight : \n");
	for(i = 0 ; i < totalvertices ; i++){
		if(Users[i] != NULL && Users[i]->prev != NULL){
			current = Users[i]->prev;
			total += current->weight;
//			printf("weight is %f\n", current->weight);
			while(current->next != NULL){
				current = current->next;
				total += current->weight;
			}
		}

		printf("user %d : total weight is %f\n", i, total);

		if(total != 0.0){
			current = Users[i]->prev;		// u->v , only consider v's in-neighbor.
			current->probability = current->weight/total;
			SyncOutNeighbor(current->ID, i, current->probability);		// synchronize out neightbor
			printf("node %d to node %d probability is %f\n", current->ID, i, current->probability);
			while(current->next != NULL){
				current = current->next;
				current->probability = current->weight/total;
				SyncOutNeighbor(current->ID, i, current->probability);
				printf("node %d to node %d probability is %f\n", current->ID, i, current->probability);
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
void StoreFeatures(char *features)
{
	int i, value;
	int node = atoi(features);

	if(!Users[node]->label){
		Users[node]->label = malloc(totalfeatures * sizeof(int));
	}
	else{
		printf("duplicate features !!\n");
		return;
	}

	for(i = 1 ; i <= totalfeatures ; i++){
		Users[node]->label[i-1] = atoi(features + i*2);
	}

/*  // Print each user's feature.
	printf("Users %d feature : \n\t", node);
	for(i = 0 ; i < totalfeatures ; i++){
		printf("%d ", Users[node]->label[i]);
	}
	printf("\n");
*/
	return;
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

	return;
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
	printf("%d ", dest);
}

/* Find minimum time path using dijkstra's algorithm. */
double *FindMTP(int root, double *dist)
{
//	double dist[totalvertices];		// distance from each node to target.
	bool sptSet[totalvertices];		// shortest path tree Set.
	int prev[totalvertices];
	int i, src, count;
	int min_index;
	struct Neighbor *current = NULL;

//	srand(time(NULL));
//	src = rand()%totalvertices;
	src = root;
	printf("src is %d\n", src);

	for(i = 0 ; i < totalvertices ; i++){
		dist[i] = DBL_MAX;
		prev[i] = -1;
		sptSet[i] = false;
	}

	dist[src] = 0;		// Distance of source vertex from itself is 0

	for(count = 0 ; count < totalvertices-1 ; count++){

		/* Pick the minimum distance from the set of vertices not yet processed. */
		min_index = minDistance(dist, sptSet);
		printf("min index is %d\n", min_index);
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
	printf("Vertex		Distance(time) from source\n");
	for(i = 0 ; i < totalvertices ; i++)
		printf("%d\t\t%f\n", i, dist[i]);

	printf("Get the path\n");
	for(i = 0 ; i < totalvertices ; i++){
		printf("node %d : ", i);
		printPath(i, prev);
		printf("\n");
	}

	return dist;
}

/* On query processing, split the query and get the target features. */
void QueryProcessing(void)
{
	char *target_labels;	// total target labels
	char *label;	// single label
	char *saveptr;	// used in strtok_r() in order to maintain context between successive calls that parse the same string.
	int index, i;

	targetFeature = malloc(totalfeatures * sizeof(int));
	
	for(i = 0 ; i < totalfeatures ; i++)		// Initialize the array 
		targetFeature[i] = 0;
	printf("Input k : \n");
//	scanf("%d", &seedNumber);
	printf("Input specific targets using attributes with blank to separate : \n");
	printf("(for example : basketball curry ...)\n");
//	scanf("%s", labels);

	seedNumber = 4;
	target_labels = "basketball curry Taipei";
	printf("k is %d\nlabels are %s\n", seedNumber, target_labels);

	char *labels = strdup(target_labels);		// maybe target_labels return a pointer to a read-only char array, strdup is one of the solution.

	label = strtok_r(labels, " ", &saveptr);
	while(label != NULL){
		printf("%s ", label);
		index = CompareFeatures(label);
		if(index == -1){		// If the label doesn't feat any other features.
			printf("No such features on our users.\n");
			exit(1);
		}
		targetFeature[index] = 1;
//		printf("index is %d\n", index);
		label = strtok_r(NULL, " ", &saveptr);
	}

/*	Above code could rewrite as follows : 
	while((label = strtok_r(labels, " ", &labels))){
		printf("%s ", label);
		...
	}
*/
	printf("\n");
	for(i = 0 ; i < totalfeatures ; i++)
		printf("%d ", targetFeature[i]);

	printf("\n\n");
	return;
}

/* Compare the features are matched or not, and return the "file.featnames"'s index. */
int CompareFeatures(char *label)
{
	int i;

	if(!featuresName){
		StoreFeaturesName();
	}	

	for(i = 0 ; i < totalfeatures ; i++){
		if(strcmp(label, featuresName[i]) == 0)
			return i;
	}

	return -1;	
}

/* Initialize the FeaturesName array , and store the value in it. */
void StoreFeaturesName(void)
{
	FILE *fp = fopen("synthetic.featnames", "r");
	char *line = NULL;
	char *token;
	size_t len = 0;
	ssize_t read;
	int count = 0;

	featuresName = malloc(totalfeatures * sizeof(char *));

	while((read = getline(&line, &len, fp)) != -1){
		token = strtok(line, " ");
		if(token != NULL)
			token = strtok(NULL, "\n");

		featuresName[count] = malloc((strlen(token)+1) * sizeof(char));
		strcpy(featuresName[count], token);
//		printf("%d %s", strlen(token), featuresName[count]);
		count++;
	}

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

	printf("Re calculate edge probability : \n");

	if(!targetUsers)
		targetUsers = malloc(totalvertices * sizeof(int));
	

	for(i = 0 ; i < totalfeatures ; i++){				// calculate the total target features' number
		if(targetFeature[i] == 1)
			targetNum++;
	}

	for(i = 0 ; i < totalvertices ; i++){
		if(Users[i] != NULL){
			intersections = 0;
			unions = 0;
			targetUsers[i] = -1;
			for(j = 0 ; j < totalfeatures ; j++){		// calculate the intersections and unions of each user's feature
//				printf("%d ", Users[i]->label[j]);	
				if(Users[i]->label[j] == targetFeature[j] && targetFeature[j] == 1)
					intersections++;
				if(Users[i]->label[j] == 1 || targetFeature[j] == 1)
					unions++;
			}

			/* if user's intersections is included whole query features, the user will be target. */
			if(intersections == targetNum){
				targetUsers[targetCount++] = i;
				printf("\nuser %d is target !!\n", i);
			}

			printf("\n");
			printf("intersections:%d unions:%d -> %f\n", intersections, unions, (double)intersections/unions);
			if((double)intersections/unions > 0)
				voluntary = true;
			else{
				voluntary = false;
				printf("No need to update !\n");
			}

			current = Users[i]->next;
			while(current != NULL && voluntary){
				printf("Previous : user %d to %d: %f\n", i, current->ID, current->probability);
				/* 1*propagation probability + self sontaniously * propagation probability */
				current->probability = 1*current->probability + ((double)intersections/unions)*current->probability;
				printf("\tuser %d to %d: %f\n", i, current->ID, current->probability);
				SyncInNeighborWithPro(i, current->ID, current->probability);
				current = current->next;
			}
		}
	}

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

		printf("user %d : total probability is %f\n", i, total);

		if(total != 0.0){
			current = Users[i]->prev;		// u->v , only consider v's in-neighbor.
			current->probability = current->probability/total;
			SyncOutNeighbor(current->ID, i, current->probability);		// synchronize out neightbor
			printf("\tuser %d to %d -> probability is %f\n", current->ID, i, current->probability);
			while(current->next != NULL){
				current = current->next;
				current->probability = current->probability/total;
				SyncOutNeighbor(current->ID, i, current->probability);
				printf("\tuser %d to %d -> probability is %f\n", current->ID, i, current->probability);
			}
		}

		total = 0.0;
	}

	return;
}

/* Sort the distToTarget array using bubble sort. "end" is true means get 
the last one index, it also means get the maximum value index, vice versa. */
int BubbleSort(double *distToTarget, bool end)
{
	int i, j, itmp;
	double tmp;
	int index[totalvertices];

	for(i = 0 ; i < totalvertices ; i++)
		index[i] = i;

	for(i = 0 ; i < totalvertices-1 ; i++){
		for(j = 0 ; j < totalvertices-i-1 ; j++){
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

	for(i = 0 ; i < totalvertices ; i++)
		printf("index %d is %f\n", index[i], distToTarget[i]);

	if(end)
		return index[totalvertices-1];
	else
		return index[0];
}

/* "node" is include seedset or not */
bool isInclude(int node ,int *seedSet){
	int i;
	for(i = 0 ; i < seedNumber ; i++){
		if(seedSet[i] == node)
			return true;
	}
	return false;
}

double *InitializeEachReduce(double *eachReduce)
{
	int i;
	for(i = 0 ; i < totalvertices ; i++)
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

	/* create a 2D array distToTargets[eachTarget][eachNode] */
	double **distToTargets = malloc(targetCount * sizeof(double *));
	double *firstRound = malloc(totalvertices * sizeof(double));		// store max time of each node to targets
	double *targets = malloc(targetCount * sizeof(double));				// the minimum time to arrive each target

	/* store each vertex to target causes the diffusion time less */
	double *eachReduce = malloc(totalvertices * sizeof(double));		

	/*	initialize targets array, distToTargets array, firstRound array, eachReduce array. */
	for(i = 0 ; i < targetCount ; i++){
		targets[i] = DBL_MAX;

		if(!distToTargets[i])
			distToTargets[i] = malloc(totalvertices * sizeof(double));
		for(j = 0 ; j < totalvertices ; j++)
			distToTargets[i][j] = -1;
	}
	for(i = 0 ; i < totalvertices ; i++){
		firstRound[i] = -1;
		eachReduce[i] = 0;
	}

	/* Get the first seed. Then get the next seed by recording the marginal gain of each node. */
	int count = 0;
	while(topk > 0){
		InitializeEachReduce(eachReduce);
		best = true;														// check the diffusion time has improved or not

		for(i = 0 ; i < targetCount ; i++){
			distToTargets[i] = FindMTP(targetUsers[i], distToTargets[i]);	// calculate the time of each node to the target
			for(j = 0 ; j < totalvertices ; j++){
				if(count == 0){
					firstRound[j] = MAX(firstRound[j], distToTargets[i][j]);
					printf("node %d to node %d : %f\n", j, targetUsers[i], distToTargets[i][j]);
					best = false;
				}
				/* if node has contribution of diffusion time, then store it into "eachReduce" to get the most one. */
				else if(!isInclude(j, seedSet) && distToTargets[i][j]<targets[i]) {
					eachReduce[j] += targets[i]-distToTargets[i][j];		
					best = false;
				}
			}
		}

		if(best){															// if there is no node has contribution , then quit the algorithm.
			printf("No more seeds !!\nIt's the shortest diffusion time !!\n");
			break;
		}
		if(count == 0)
			top1 = BubbleSort(firstRound, false);
		else
			top1 = BubbleSort(eachReduce, true);
			
		seedSet[count++] = top1;											// store top1 to seed set
		printf("top %d is %d\n", count, top1);
		for(i = 0 ; i < targetCount ; i++){
			if(distToTargets[i][top1] < targets[i])
				targets[i] = distToTargets[i][top1];
			printf("top %d to node %d is : %f\n\n", count, targetUsers[i], targets[i]);
		}

		topk--;
	}

	printf("seed set are : \n");
	for(i = 0 ; i < count ; i++)
		printf("%d ", seedSet[i]);
	printf("\n");

	return;
}


/* Show useful message to confirm it. */
void printGraph(void)
{
	int i;
	struct Neighbor *current = NULL;

	for(i = 0 ; i < totalvertices ; i++){
		if(Users[i] != NULL){
			printf("node %d next: \n", Users[i]->ID);
			if(Users[i]->next != NULL){
				current = Users[i]->next;
				printf("\tnode %d\n", current->ID);
				printf("\tprobability %f\n", current->probability);
				printf("\tdiffusion time %f\n", current->time);
				while(current->next != NULL){
					current = current->next;
					printf("\tnode %d\n", current->ID);
					printf("\tprobability %f\n", current->probability);
					printf("\tdiffusion time %f\n", current->time);
				}
			}
			printf("prev: \n");
			if(Users[i]->prev != NULL){
				current = Users[i]->prev;
				printf("\tnode %d\n", current->ID);
				printf("\tprobability %f\n", current->probability);
				printf("\tdiffusion time %f\n", current->time);
				while(current->next != NULL){
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

int main(int argc, char **argv)
{
	int targetCount;

	ReadGraph();
	NormalizeEdgeWeight();

//	QueryProcessing();
//	targetCount = RecalProbability();
//	ReNormalizeEdgeProbability();
	DiffusionTime();

//	printf("\nPrint Graph Information : \n");
//	printGraph();

//	Baseline(targetCount);

	LD_Tree();		// build LD tree before query processing.

//	double dist[totalvertices];
//	FindMTP(2, dist);

	return 0;
}
