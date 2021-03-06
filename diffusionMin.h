#ifndef __DIFFUSIONLIB_H__
#define __DIFFUSIONLIB_H__

#include<stdio.h>
#include<stdbool.h>

#define MAX(X, Y) ( X > Y ? X : Y)
#define MIN(X, Y) ( X < Y ? X : Y)

/* Each vertex's structure */
struct Vertex
{
	int ID;
	int *label;
	char **feature;
	int community;
	struct Neighbor *prev;
	struct Neighbor *next;
};

/* Each vertex's neighbor, no matter in-neighbor or out-neighbor */
struct Neighbor
{
	int ID;
	double weight;
	double probability;
	double time;
	struct Neighbor *next;
};

/* Each vertex's local diffusion tree */
struct VertexLD
{
	int ID;
	int *prevPath;
	struct Influencer *prev;
};

/* Each vertex's influencer, including in-neighbors and in-neighbors' neighbor, ... */
struct Influencer
{
	int ID;
	double time;
	struct Influencer *next;
};

/* Each community's structure */
struct Community
{
	int ID;
	double weight;
	int degree;
	int central;
	double radius;
	int *closely;
	bool merged;
	bool parent;
	int topk;
	struct Neighbor_com *next;
};

/* Each community's out-neighbor */
struct Neighbor_com
{
	int ID;
	double weight;
	int degree;
	struct Neighbor_com *next;
};

/* Store which communities are merged */
struct Community_Merge
{
	int ID;
	int central;
	double radius;
	int *child;
	struct Community_Merge *next;
};

/* Merged Communities some information, used by return multiple value */
struct Central_Info
{
	int central;
	double radius;
};

struct Vertex **Users;						// Store the network graph
struct VertexLD **UsersLD;					// Store the local diffusion tree
struct Community **Communities;				// Store the comunities' graph
struct Community_Merge *CommunityMerged;	// Store which communities are merged

void ReadGraph(const char *, const char *);	// Read social networks graph
void StoreCommunity(char *, int);			// Store each node's community
void InitializeVertices(int);				// Create an array of struct Vertex pointer, which points to each user
void StoreRelationship(char *);				// Include "file.edge" to store the users' relationship
void NormalizeEdgeWeight(void);				// Normalize edge weight with w(u,v)/d(v) as the propagation probability
bool StoreFeatures(char *);					// Store every vertex's feature in their struct
void SyncOutNeighbor(int ,int, double);
void SyncInNeighbor(int ,int, double);
void DiffusionTime(void);					// Compute the diffusion time with 1/probability * 1/weight
int minDistance(double *, bool *);			// Pick the minimum distance from the set of vertices not yet processed
double *FindMTP(int, double *);				// Find minimum time path using dijkstra's algorithm
void printGraph(void);

void QueryProcessing(char *);				// Processing the query set, including k influential nodes and targets features
bool CompareFeatures(char *);				// Compare the label is matched with targetFeature or not
void StoreFeaturesName(char *);				// Initialize the feature array
void ReNormalizeEdgeProbability(void);		// Re normalize edge probability
void SyncInNeighborWithPro(int, int, double);
int BubbleSort(double *, bool, int);		// Sort the distToTarget array using bubble sort.
bool isInclude(int, int *);					// "node" is include seedset or not
double *InitializeEachReduce(double *, int);

void StoreInfluencer(int, int, double);		// Store each node's influencer into LD tree
void FindMTPwithTree(int, double, int);		// Finding the minimum time path to build each node's local diffusion tree with a threshold bound
int AddCandidate(int *, int);				// union the LD tree as the candidates
int ChooseCandidates(int, int *);			// select the targets' LD tree as the candidates
void LD_Tree(int);							// second algorithm to find out the k influential nodes with LD tree approach
void Community_based(int);
void UpdateCommunities(int, struct Community_Merge *);
void UpdatecomMember(int *, int);
void DeleteChildUpdate(struct Community_Merge *);

void Optimal(int);							// optimal method

char *dataset;								// name of the dataset
char *directory;							// name of the dataset directory
int totalvertices;							// numbers of total users
int totalfeatures;							// numbers of total features
int seedNumber;								// numbers of influential nodes
//int *targetFeature;							// target feature with 0 and 1 to represent
char *targetFeature;						// target feature with features name represent
char **featuresName;						// store total features' name by scanning "file.featname"
int *targetUsers;							// store target users whose features have include all query features
char *allFeatures;							// store all Features
double **BoundDist;							// store the MTP tree with the bound distance
/********************************************/
int communityNum;							// numbers of communities
int **comMember;							// store each community's members
int *eachComNumber;							// store each community's number
clock_t begin, end;							// calcuate the execution time of the program

#endif

