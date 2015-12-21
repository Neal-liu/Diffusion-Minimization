#ifndef __DIFFUSIONLIB_H__
#define __DIFFUSIONLIB_H__

#include<stdio.h>
#include<stdbool.h>

#define MAX(X, Y) ( X > Y ? X : Y)
#define MIN(X, Y) ( X < Y ? X : Y)

/* Each vertex's structure */
struct Vertex {
	int ID;
	int *label;
	struct Neighbor *prev;
	struct Neighbor *next;
};

/* Each vertex's neighbor, no matter in-neighbor or out-neighbor */
struct Neighbor {
	int ID;
	double weight;
	double probability;
	double time;
	struct Neighbor *next;
};

/* Each vertex's local diffusion tree */
struct VertexLD {
	int ID;
	int *prevPath;
	struct Influencer *prev;
};

/* Each vertex's influencer, including in-neighbors and in-neighbors' neighbor, ... */
struct Influencer {
	int ID;
	double time;
	struct Influencer *next;
};

struct Vertex **Users;						// Store the network graph
struct VertexLD **UsersLD;					// Store the local diffusion tree

void ReadGraph(char *);						// Read social networks graph
void InitializeVertices(int);				// Create an array of struct Vertex pointer, which points to each user
void StoreRelationship(char *);				// Include "file.edge" to store the users' relationship
void NormalizeEdgeWeight(void);				// Normalize edge weight with w(u,v)/d(v) as the propagation probability
void StoreFeatures(char *);					// Store every vertex's feature in their struct
void SyncOutNeighbor(int ,int, double);
void SyncInNeighbor(int ,int, double);
void DiffusionTime(void);					// Compute the diffusion time with 1/probability * 1/weight
double *FindMTP(int, double *);				// Find minimum time path using dijkstra's algorithm
void printGraph(void);

void QueryProcessing(void);					// Processing the query set, including k influential nodes and targets features
int CompareFeatures(char *);				// Compare which users matches or not to choose targets
void StoreFeaturesName(void);				// Initialize the feature array 
void ReNormalizeEdgeProbability(void);		// Re normalize edge probability
void SyncInNeighborWithPro(int, int, double);	

void StoreInfluencer(int, int, double);		// Store each node's influencer into LD tree
void FindMTPwithTree(int, double);			// Finding the minimum time path to build each node's local diffusion tree with a threshold bound
int AddCandidate(int *, int);				// union the LD tree as the candidates
int ChooseCandidates(int, int *);			// select the targets' LD tree as the candidates
void LD_Tree(int);							// second algorithm to find out the k influential nodes with LD tree approach

char *dataset;								// name of the dataset
int totalvertices;							// number of total users
int totalfeatures;							// number of total features
int seedNumber;								// number of influential nodes
int *targetFeature;							// target feature with 0 and 1 to represent
char **featuresName;						// store total features' name by scanning "file.featname"
int *targetUsers;							// store target users whose features have include all query features


#endif

