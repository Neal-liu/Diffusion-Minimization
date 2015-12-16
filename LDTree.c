#include <stdio.h>
#include <float.h>
#include <stdlib.h>
#include "diffusionMin.h"

/* Store the "node" influencer into the "UsersLD" tree. */
void StoreInfluencer(int node, int id, double time)
{
	struct Influencer *influencer = malloc(sizeof(struct Influencer));
	struct Influencer *current = NULL;

	influencer->ID = id;
	influencer->time = time;
	influencer->next = NULL;
	
//	printf("id is %d\n", UsersLD[node]->ID);
//	printf("test\n%d\t%f\n", influencer->ID, influencer->time);
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
void FindMTPwithTree(int root, double threshold)
{
	double dist[totalvertices];									// distance from each node to target.
	bool sptSet[totalvertices];									// shortest path tree Set.
	int prev[totalvertices];
	int i, src, count;
	int min_index;
	struct Neighbor *current = NULL;

	/* intialize userLD[root] */
	UsersLD[root] = malloc(sizeof(struct VertexLD));
	UsersLD[root]->ID = root;
	UsersLD[root]->prev = NULL;

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
		if(min_index == -1)										// can not find the minimum time vertex, no more visit.
			break;
		sptSet[min_index] = true;

		/* Update dist value of the adjacent vertices of the picked vertex. */
		if(Users[min_index] != NULL && Users[min_index]->prev != NULL){
			current = Users[min_index]->prev;
			double minTime = 1 * 1/current->weight; 			// set propagation probability to 1 as the fastest diffusion time.
//			printf("minTime is %f\n", minTime);
//			printf("min index is %d\n", min_index);
//			printf("current id is %d\n", current->ID);
			if(!sptSet[current->ID] && current->time && dist[min_index]!=DBL_MAX && (dist[min_index]+minTime < dist[current->ID]) && (dist[min_index]+minTime < threshold) ){
				dist[current->ID] = dist[min_index]+minTime;
				prev[current->ID] = min_index;
			}	
			while(current->next != NULL){
				current = current->next;
				minTime = 1 * 1/current->weight;
				if(!sptSet[current->ID] && current->time && dist[min_index]!=DBL_MAX && (dist[min_index]+minTime < dist[current->ID]) && (dist[min_index]+minTime < threshold) ){
					dist[current->ID] = dist[min_index]+minTime;
					prev[current->ID] = min_index;
				}	
			}
		}
	}

	/* verify the algorithm is correct!? and store the root's influencer into local diffusion tree. */
	printf("Vertex		Distance(time) from source\n");
	for(i = 0 ; i < totalvertices ; i++){
		if(dist[i] < threshold && dist[i] != 0)
			StoreInfluencer(root, i, dist[i]);
		printf("%d\t\t%f\n", i, dist[i]);
	}

	printf("Get the path\n");
	for(i = 0 ; i < totalvertices ; i++){
		printf("node %d : ", i);
		printPath(i, prev);
		printf("\n");
	}

	printf("Local Diffusion Tree :\n");
	if(UsersLD[root]->prev){
		struct Influencer *currents = UsersLD[root]->prev;
		printf("prev id is %d\n", currents->ID);
		printf("\ttime is %f\n", currents->time);

		while(currents->next != NULL){
			currents = currents->next;
			printf("prev id is %d\n", currents->ID);
			printf("\ttime is %f\n", currents->time);
		}
	}
	else
		printf("No local diffusion tree !!\n");

	printf("DONE!\n");
	return;
}

void LD_Tree(void)
{
	int i;
	double threshold = 100;	

	/* initialize vertex with local diffusion tree */	
	UsersLD = malloc(totalvertices * sizeof(struct VertexLD *));

	for(i = 0 ; i < totalvertices ; i++){
//		FindMTPwithTree(3, threshold);
		FindMTPwithTree(i, threshold);
	}



	return;
}


