#include <stdio.h>
#include <float.h>
#include <string.h>
#include <stdlib.h>
#include "diffusionMin.h"
#include "util.h"

/* Initialize Communities' structure. */
void InitialCommunities(void)
{
	int i, j;
	extern struct Community **Communities;
	extern int communityNum;

	Communities = malloc(communityNum * sizeof(struct Community *));

	for(i = 0 ; i < communityNum ; i++){
		Communities[i] = malloc(sizeof(struct Community));
		Communities[i]->ID = i;
		Communities[i]->weight = 0.0;
		Communities[i]->degree = 0;
		Communities[i]->central = -1;
		Communities[i]->radius = 0.0;
		Communities[i]->topk = 1;
		Communities[i]->closely = malloc(communityNum * sizeof(int));
		for(j = 0 ; j < communityNum ; j++)
			Communities[i]->closely[j] = -1;
		Communities[i]->merged = false;
		Communities[i]->parent = false;
		Communities[i]->next = NULL;
	}

	return ;
}

/* Calculate the closely connected communities. */
void Closely(void)
{
	int i, count;
	struct Neighbor *current = NULL;
	struct Neighbor_com *current_com = NULL;
	double average, tmp;
	extern struct Community **Communities;
	extern int communityNum;

	/* count the edge weight and edge degree between communities */
	for(i = 0 ; i < totalvertices ; i++){
		if(Users[i] != NULL && Users[i]->community != -1){
			current = Users[i]->next;
			while(current != NULL && Users[current->ID]->community != -1){
				
				if(Users[i]->community == Users[current->ID]->community){
					current = current->next;
					continue;
				}
//				printf("welcome %d to %d\n", i, current->ID);
//				printf("user %d community is %d\n", i, Users[i]->community);
				Communities[Users[i]->community]->weight += current->weight;
				Communities[Users[i]->community]->degree += 1;

				current_com = Communities[Users[i]->community]->next;
				if(current_com == NULL) {
					struct Neighbor_com *com = malloc(sizeof(struct Neighbor_com));
					com->ID = Users[current->ID]->community;
					com->weight += current->weight;
					com->degree += 1;
					Communities[Users[i]->community]->next = com;
				}
				else {
					while(current_com != NULL){
						if(current_com->ID == Users[current->ID]->community){
							current_com->weight += current->weight;
							current_com->degree += 1;
							break;
						}

						if(current_com->next == NULL){
							struct Neighbor_com *out_neighbor = malloc(sizeof(struct Neighbor_com));
							out_neighbor->ID = Users[current->ID]->community;
							out_neighbor->weight += current->weight;
							out_neighbor->degree += 1;
							current_com->next = out_neighbor;
							break;
						}

						current_com = current_com->next;
					}
				}
				current = current->next;
			}
		}
	}
/*
	// print communities information
	for(i = 0 ; i < communityNum ; i++){
		printf("Community %d :\n total weight : %lf\n total degree : %d\n", i, Communities[i]->weight, Communities[i]->degree);
		current_com = Communities[i]->next;
		while(current_com != NULL){
			printf("\t To community %d : total weight : %lf, total degree : %d\n", current_com->ID, current_com->weight, current_com->degree);
			current_com = current_com->next;
		}

	}
*/
	/* calculate the closely connected communities */
	for(i = 0 ; i < communityNum ; i++){
		if(Communities[i]->degree == 0){
			printf("%d : No closely connected communities.\n", i);
			continue;
		}
		current_com = Communities[i]->next;
		average = Communities[i]->weight/Communities[i]->degree;
		count = 0;
		printf("%d : average is %lf\n", i, average);
		while(current_com != NULL){
			tmp = current_com->weight/current_com->degree;
			if(average <= tmp){
				while(Communities[i]->closely[count] != -1)
					count++;
				Communities[i]->closely[count] = current_com->ID;
				printf("\t closely community : %d\n", current_com->ID);
			}
			current_com = current_com->next;
		}

	}

	return ;
}

bool isIncludeCom(int node, int *comMembers, int number)
{
	for(int i = 0 ; i < number ; i++){
		if(node == comMembers[i])
			return true;
	}
	return false;
}

/* Find the central node from this community with brute force algorithm. */
struct Central_Info BruteForce(int *comMembers, int number, int comIndex1, int comIndex2)
{
	extern struct Community **Communities;
	int i, j, central, count;
	// Store each node to each node's distance in this community
	double **distToTargets = malloc(number * sizeof(double *));
	double *maxTime = malloc(number * sizeof(double));
	int *maxTimeID = malloc(number * sizeof(int));

	for(i = 0 ; i < number ; i++){
		distToTargets[i] = malloc(totalvertices * sizeof(double));
		maxTime[i] = -1;
		for(j = 0 ; j < totalvertices ; j++)
			distToTargets[i][j] = -1;
	}

	for(i = 0 ; i < number ; i++){
		distToTargets[i] = FindMTP(comMembers[i], distToTargets[i]);
		count = 0;
		for(j = 0 ; j < totalvertices ; j++){
			if(Users[j] != NULL && isIncludeCom(Users[j]->ID, comMembers, number)){
				maxTime[count] = MAX(maxTime[count], distToTargets[i][j]);
				maxTimeID[count] = Users[j]->ID;
				count++;
//				printf("dist To Target : %d to %d : %lf\n", Users[j]->ID, comMembers[i], distToTargets[i][j]);
			}
		}
	}

	central = BubbleSort(maxTime, false, number);
	printf("\ncentral node is %d : %lf\n", maxTimeID[central], maxTime[0]);
//	printf("central node is %d : %lf\n", comMembers[central], maxTime[0]);

	// calculate only one community's central node
	if(comIndex2 == -1){
		Communities[comIndex1]->central = comMembers[central];
		Communities[comIndex1]->radius = maxTime[0];
	}
	else if(comIndex2 == -2){
//		central = BubbleSort2();
		printf("\ncentral node is %d : %lf\n", maxTimeID[central], maxTime[1]);
		struct Central_Info info = {maxTimeID[central], maxTime[1]}; 
		return info;
	}

	struct Central_Info info = {maxTimeID[central], maxTime[0]}; 

	return info;
}

void CalculateCentral(void)
{
	extern struct Community **Communities;
	extern int communityNum;
	extern int **comMember;
	extern int *eachComNumber;
	int i, j, count;
	comMember = malloc(communityNum * sizeof(int *));
	eachComNumber = malloc(communityNum * sizeof(int));

	// declaration of comMember
	for(i = 0 ; i < communityNum ; i++){
		comMember[i] = malloc(totalvertices * sizeof(int));
		for(j = 0 ; j < totalvertices ; j++)
			comMember[i][j] = -1;
	}

	// find members for each community
	for(i = 0 ; i < communityNum ; i++){
		count = 0;
		printf("community : %d\n", i);
		for(j = 0 ; j < totalvertices ; j++){
			if(Users[j] != NULL && Users[j]->community == i){
				comMember[i][count] = Users[j]->ID;
				printf("member : %d\n", Users[j]->ID);
				count++;
			}
		}
		eachComNumber[i] = count;
	}

	for(i = 0 ; i < communityNum ; i++)
		BruteForce(comMember[i], eachComNumber[i], i, -1);
	
}

/* Sorted the communities' radius. */
void SortComRadius(double *sortRadius, int *indexRadius)
{
	extern int communityNum;
	double tmp;
	int i, j, itmp;

	for(i = 0 ; i < communityNum-1 ; i++){
		for(j = 0 ; j < communityNum-1 ; j++){
			if(sortRadius[j] > sortRadius[j+1]){
				tmp = sortRadius[j];			// value exchange
				sortRadius[j] = sortRadius[j+1];
				sortRadius[j+1] = tmp;

				itmp = indexRadius[j];			// index exchange
				indexRadius[j] = indexRadius[j+1];
				indexRadius[j+1] = itmp;
			}
		}
	}
}

void CommunityMerge(void)
{
	extern struct Community **Communities;
	extern int communityNum;
	extern int *eachComNumber;
	extern int **comMember;
	double maxRadius;
	int i, mergeCount, mergeNumbers, min1, min2, count = 0;
//	int communityNum;
	struct Central_Info info;
	struct Community_Merge *current;
	bool first = true;

//	while(communityNum > seedNumber){
	while(1){
		// put values in sortRadius and indexRadius array , and SortComRadius in here.
		double *sortRadius = malloc(communityNum * sizeof(double));
		int *indexRadius = malloc(communityNum * sizeof(int));

		for(i = 0 ; i < communityNum ; i++){
			if(!Communities[i]->merged)
				sortRadius[i] = Communities[i]->radius;
			else
				sortRadius[i] = -1;
			indexRadius[i] = i;
		}
		// Sorted the communities' radius
		SortComRadius(sortRadius, indexRadius);

		while(sortRadius[count] == -1)
			count++;
		printf("minimum radius community is %d : %lf\n", indexRadius[count], sortRadius[count]);
		printf("maxmum radius community is %d : %lf\n", indexRadius[communityNum-1], sortRadius[communityNum-1]);


		min1 = indexRadius[count];
		min2 = Communities[indexRadius[count]]->closely[0];
		printf("min1 %d min2 %d\n", min1, min2);
		mergeNumbers = eachComNumber[min1] + eachComNumber[min2];
		int *mergedMembers = malloc(mergeNumbers * sizeof(int));
		mergeCount = 0;

		for(i = 0 ; comMember[min1][i] != -1 ; i++)
			mergedMembers[mergeCount++] = comMember[min1][i];
		for(i = 0 ; comMember[min2][i] != -1 ; i++)
			mergedMembers[mergeCount++] = comMember[min2][i];
		
//		printf("merged members : \n\t");
//		for(i = 0 ; i < mergeNumbers ; i++)
//			printf("%d ", mergedMembers[i]);

		info = BruteForce(mergedMembers, mergeNumbers, min1, min2);
		printf("\nmerge Time : %lf\n", info.radius);
		if(system("read var1") == -1) err("system pause error!\n");

		if(first){
			first = false;
			maxRadius = sortRadius[communityNum-1];
		}
		// if merged radius <= max R(C), then combined these two communities.
		if(info.radius < maxRadius){
			Communities[min1]->merged = true;
			Communities[min2]->merged = true;
			
			// need recontruct the communities and sort the radius again
			if(CommunityMerged == NULL){			// first one communities merged
				CommunityMerged = malloc(sizeof(struct Community_Merge));
				CommunityMerged->ID = communityNum;
				CommunityMerged->central = info.central;
				CommunityMerged->radius = info.radius;
				CommunityMerged->child = malloc(2 * sizeof(int));
				CommunityMerged->child[0] = min1;
				CommunityMerged->child[1] = min2;
				CommunityMerged->next = NULL;
				current = CommunityMerged;
			}
			else{
				current = CommunityMerged;
				while(current->next != NULL)
					current = current->next;
				// new a communitymerged struct and assign it to current next
				struct Community_Merge *n = malloc(sizeof(struct Community_Merge));
				n->ID = current->ID + 1;
				n->central = info.central;
				n->radius = info.radius;
				n->child = malloc(2 * sizeof(int));
				n->child[0] = min1;
				n->child[1] = min2;
				n->next = NULL;

				current->next = n;
				current = current->next;
			}

			// remalloc eachComNumber , add mergedCommunity to "Communities" , update communityNum , remalloc comMember
			communityNum++;
			eachComNumber = realloc(eachComNumber, communityNum * sizeof(int));
			eachComNumber[communityNum-1] = mergeCount;
			UpdateCommunities(communityNum, current);
			UpdatecomMember(mergedMembers, mergeNumbers);

			free(sortRadius);
			free(indexRadius);
			free(mergedMembers);
			continue;
		}
		else
			break;
	}
}

void UpdateCommunities(int communityNum, struct Community_Merge *current)
{
	extern struct Community **Communities;
	int count = 0;

	Communities = realloc(Communities, communityNum * sizeof(struct Community *));
	Communities[communityNum-1] = malloc(sizeof(struct Community));
	Communities[communityNum-1]->ID = current->ID;
	Communities[communityNum-1]->weight = Communities[current->child[0]]->weight + Communities[current->child[1]]->weight;
	Communities[communityNum-1]->degree = Communities[current->child[0]]->degree + Communities[current->child[1]]->degree;
	Communities[communityNum-1]->central = current->central;
	Communities[communityNum-1]->radius = current->radius;
	Communities[communityNum-1]->closely = malloc(communityNum * sizeof(int));
	for(int j = 0 ; j < communityNum ; j++)
		Communities[communityNum-1]->closely[j] = -1;
	Communities[communityNum-1]->merged = false;
	Communities[communityNum-1]->parent = true;
	Communities[communityNum-1]->next = NULL;

	// merged Communtity's neighbors are its children's neighbor (only two)
	struct Neighbor_com *merged = Communities[communityNum-1]->next;
	bool found = false;

	for(int i = 0 ; i < 2 ; i++){
		struct Neighbor_com *cur = Communities[current->child[i]]->next;
		while(cur != NULL){
			struct Neighbor_com *out = malloc(sizeof(struct Neighbor_com));
			out->ID = cur->ID;
			out->weight = cur->weight;
			out->degree = cur->degree;
			out->next = NULL;

			found = false;
			merged = Communities[communityNum-1]->next;
			if(merged == NULL){
				Communities[communityNum-1]->next = out;
			}
			else{
				if(merged->ID == cur->ID){
					cur = cur->next;
					continue;
				}
				while(merged->next != NULL){
					merged = merged->next;
					if(merged->ID == cur->ID){
						found = true;
						break;
					}
				}
				if(found){
					cur = cur->next;
					continue;
				}
				else
					merged->next = out;
			}
			cur = cur->next;
		}
	}

	// update closely. Need update all communities because the merged communities could effect other communities.
	// update communities->next, too.
	double average , tmp;

	// delete merged children from other communities->next
	DeleteChildUpdate(current);

	for(int i = 0 ; i < communityNum ; i++){
		if(Communities[i]->merged){
			printf("%d : I have been merged, not my business.\n", i);
			continue;
		}
		else if(Communities[i]->degree == 0){
			printf("%d : No closely connected communities.\n", i);
			continue;
		}
		merged = Communities[i]->next;
		average = Communities[i]->weight/Communities[i]->degree;
		count = 0;
		printf("%d : average is %lf\n", i, average);
		memset(Communities[i]->closely, -1, sizeof(*Communities[i]->closely));
		while(merged != NULL){
			if(!Communities[merged->ID]->merged){
				tmp = merged->weight/merged->degree;
				if(average <= tmp){
					while(Communities[i]->closely[count] != -1)
						count++;
					Communities[i]->closely[count] = merged->ID;
					printf("\t closely community : %d\n", merged->ID);
				}
			}
			merged = merged->next;
		}
	}
}

void DeleteChildUpdate(struct Community_Merge *cur)
{
	int child1 = cur->child[0];
	int child2 = cur->child[1];
	bool update = false;	

	for(int i = 0 ; i < communityNum-1 ; i++){
		struct Neighbor_com *current = Communities[i]->next;
		struct Neighbor_com *previous = NULL;
		update = false;

		while(current != NULL){
			if(current->ID == child1 || current->ID == child2){
				update = true;
				if(previous == NULL){
					current = current->next;
					Communities[i]->next = current;
				}
				else{
					previous->next = current->next;
					current = previous->next;
				}
			}
			else{
				previous = current;
				current = current->next;
			}
		}

		if(update){
			struct Neighbor_com *new = malloc(sizeof(struct Neighbor_com));
			new->ID = Communities[communityNum-1]->ID;
			new->weight = Communities[communityNum-1]->weight;
			new->degree = Communities[communityNum-1]->degree;
			new->next = NULL;

			current = Communities[i]->next;
			if(current == NULL){
				Communities[i]->next = new;
			}
			else{
				while(current->next != NULL)
					current = current->next;
				current->next = new;
			}
		}
	}

/*
	for(int i = 0 ; i < communityNum ; i++){
		struct Neighbor_com *current = Communities[i]->next;
		printf("community %d :\n", i);
		while(current != NULL){
			printf("next is %d\n", current->ID);
			current = current->next;
		}
	}
*/
}

void UpdatecomMember(int *mergedMembers, int mergeNumbers)
{
	extern int **comMember;
	
	comMember = realloc(comMember, communityNum * sizeof(int *));
	comMember[communityNum-1] = malloc(totalvertices * sizeof(int));
	for(int i = 0 ; i < totalvertices ; i++)	// initialize
		comMember[communityNum-1][i] = -1;
	for(int i = 0 ; i < mergeNumbers ; i++){
		comMember[communityNum-1][i] = mergedMembers[i];
//		printf("each comMember : %d\n", comMember[communityNum-1][i]);
	}
}

void PickSeeds(void)
{
	extern struct Community_Merge *CommunityMerged;
	extern struct Community **Communities;
	struct Central_Info info;
	double *sortRadius = malloc(communityNum * sizeof(double));
	int *indexRadius = malloc(communityNum * sizeof(int));
	int seedCount = 0, maxRadiusIndex;
	int *seedSet = malloc(seedNumber * sizeof(int));

	memset(seedSet, -1, sizeof(*seedSet));
	memset(sortRadius, -1, sizeof(*sortRadius));
	
	for(int i = 0 ; i < communityNum ; i++){
		if(!Communities[i]->merged){
			seedSet[seedCount++] = Communities[i]->central;
			sortRadius[i] = Communities[i]->radius;
		}
		else
			sortRadius[i] = -1;			
		indexRadius[i] = i;
	}

	while(seedCount < seedNumber){

		// Sorted the communities' radius
		SortComRadius(sortRadius, indexRadius);
		printf("maxmum radius community is %d : %lf\n", indexRadius[communityNum-1], sortRadius[communityNum-1]);
		maxRadiusIndex = indexRadius[communityNum-1];

		// pick its children's central nodes
		if(Communities[maxRadiusIndex]->parent){
			struct Community_Merge *current = CommunityMerged;
			while(current != NULL && current->ID != Communities[maxRadiusIndex]->ID){
				current = current->next;
			}
			seedSet[seedCount++] = Communities[current->child[0]]->central;
			Communities[current->child[0]]->merged = false;
			seedSet[seedCount++] = Communities[current->child[1]]->central;
			Communities[current->child[1]]->merged = false;
			
		}
		// Not Yet Finished
		else{
			// this community maybe pick more than 2 seeds
			int topk = -(Communities[maxRadiusIndex]->topk+1);
			info = BruteForce(comMember[maxRadiusIndex], eachComNumber[maxRadiusIndex], maxRadiusIndex, topk);
			seedSet[seedCount++] = info.central;
			// update communitiy's radius
			Communities[maxRadiusIndex]->radius = info.radius;
		}

		for(int i = 0 ; i < communityNum ; i++){
			if(!Communities[i]->merged)
				sortRadius[i] = Communities[i]->radius;
			else
				sortRadius[i] = -1;			
			indexRadius[i] = i;
		}
	}
}

void Community_based(int targetCount)
{
	// Initialize the communities' structure
	InitialCommunities();
	Closely();
	CalculateCentral();
	CommunityMerge();
	PickSeeds();
	// pick seeds and calculate diffusion time.
}
