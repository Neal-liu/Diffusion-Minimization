#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<float.h>
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
		printf("%s", line);
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
		printf("%s", line);
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
	printf("%d\n", number);
	totalvertices = number;
	Users = malloc(number * sizeof(struct Vertex *));
	return;
}

/* include file.edge to store the users' relationship. */
void StoreRelationship(char *relation)
{
	int node1 = atoi(relation);
	int node2 = atoi(relation + 2);
	int closeness = atoi(relation + 4);
	struct Neighbor *current = NULL;

	if(!Users[node1]){
		Users[node1] = malloc(sizeof(struct Vertex));
		Users[node1]->ID = node1;
		Users[node1]->prev = NULL;
		Users[node1]->next = NULL;
//		printf("create node %d\n", node1);
	}
	if(!Users[node2]){
		Users[node2] = malloc(sizeof(struct Vertex));
		Users[node2]->ID = node2;
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
			printf("weight is %f\n", current->probability);
			while(current->next != NULL){
				current = current->next;
				current->probability = current->weight/total;
				SyncOutNeighbor(current->ID, i, current->probability);
				printf("weight is %f\n", current->probability);
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

/* store every vertex's feature in their struct */
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

	printf("Users %d feature : \n\t", node);
	for(i = 0 ; i < totalfeatures ; i++){
		printf("%d ", Users[node]->label[i]);
	}
	printf("\n");
	return;
}

/* compute the diffusion time with 1/probability * 1/weight. */
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

/* find minimum time path using dijkstra's algorithm. */
void FindMTP(void)
{
	double dist[totalvertices];
	bool sptSet[totalvertices];		// shortest path tree Set.
	int prev[totalvertices];
	int i, src, count;
	int min_index;
	struct Neighbor *current = NULL;

	srand(time(NULL));
	src = rand()%totalvertices;
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
		if(Users[min_index] != NULL && Users[min_index]->next != NULL){
			current = Users[min_index]->next;
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
	printf("Vertex		Distance from source\n");
	for(i = 0 ; i < totalvertices ; i++)
		printf("%d\t\t%f\n", i, dist[i]);

	printf("Get the path\n");
	for(i = 0 ; i < totalvertices ; i++){
		printf("node %d : ", i);
		printPath(i, prev);
		printf("\n");
	}

	return;
}

/* On query processing, split the query and get the target features. */
void QueryProcessing(void)
{
	int seednum;	// number of influential nodes
	char *target_labels;	// total target labels
	char *label;	// single label
	char *saveptr;	// used in strtok_r() in order to maintain context between successive calls that parse the same string.
	int targetFeature[totalfeatures];
	int index;

	printf("Input k : \n");
//	scanf("%d", &seednum);
	printf("Input specific targets using attributes with blank to separate : \n");
	printf("(for example : basketball curry ...)\n");
//	scanf("%s", labels);

	seednum = 12;
	target_labels = "basketball curry Taipei";
	printf("k is %d\nlabels are %s\n", seednum, target_labels);

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
	return;
}

int CompareFeatures(char *label)
{
	int i;

	if(!FeaturesName){
		StoreFeaturesName();
	}	

	for(i = 0 ; i < totalfeatures ; i++){
		if(strcmp(label, FeaturesName[i]) == 0)
			return i;
	}

	return -1;	
}

void StoreFeaturesName(void)
{
	FILE *fp = fopen("synthetic.featnames", "r");
	char *line = NULL;
	char *token;
	size_t len = 0;
	ssize_t read;
	int count = 0;

	FeaturesName = malloc(totalfeatures * sizeof(char *));

	while((read = getline(&line, &len, fp)) != -1){
		token = strtok(line, " ");
		if(token != NULL)
			token = strtok(NULL, "\n");

		FeaturesName[count] = malloc((strlen(token)+1) * sizeof(char));
		strcpy(FeaturesName[count], token);
//		FeaturesName[count] = token;
		printf("%d %s", strlen(token), FeaturesName[count]);
		count++;
	}

/*
	printf("print again\n");
	int i;
	for(i = 0 ; i < totalfeatures ; i++)
		printf("%s test ", FeaturesName[i]);
*/
	printf("\n");

	return;
}

void RecalProbability(void)
{

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
	
	ReadGraph();
	NormalizeEdgeWeight();
	QueryProcessing();
	RecalProbability();
//	DiffusionTime();
//	FindMTP();
//	printf("\n");
//	printGraph();

	return 0;
}
