#ifndef __DIFFUSIONLIB_H__
#define __DIFFUSIONLIB_H__

#include<stdio.h>
#include<stdbool.h>

struct Vertex {
	int ID;
	int *label;
	bool update;
	struct Neighbor *prev;
	struct Neighbor *next;
};

struct Neighbor {
	int ID;
	double weight;
	double probability;
	double time;
	struct Neighbor *next;
};

struct Vertex **Users;

void ReadGraph(void);						// Read social networks graph
void InitializeVertices(int);				// Create an array of struct Vertex pointer, which points to each user
void StoreRelationship(char *);				// Include "file.edge" to store the users' relationship
void NormalizeEdgeWeight(void);				// Normalize edge weight with w(u,v)/d(v) as the propagation probability
void StoreFeatures(char *);					// Store every vertex's feature in their struct
void SyncOutNeighbor(int ,int, double);
void SyncInNeighbor(int ,int, double);
void DiffusionTime(void);					// Compute the diffusion time with 1/probability * 1/weight
void FindMTP(void);							// Find minimum time path using dijkstra's algorithm
void printGraph(void);

void QueryProcessing(void);
int CompareFeatures(char *);
void ReNormalizeEdgeProbability(void);
void SyncInNeighborWithPro(int, int, double);

int totalvertices;							// number of total users
int totalfeatures;							// number of total features
int seedNumber;								// number of influential nodes
int *targetFeature;							// target feature with 0 and 1 to represent
char **FeaturesName;						

#endif
