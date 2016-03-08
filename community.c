#include <stdio.h>
#include <float.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
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
		Communities[i]->topk = 0;
		Communities[i]->closely = malloc(communityNum * sizeof(int));
		for(j = 0 ; j < communityNum ; j++)
			Communities[i]->closely[j] = -1;
		Communities[i]->merged = false;
		Communities[i]->parent = false;
		Communities[i]->next = NULL;
	}
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

	puts("Calculate the closely connected communities...\n");
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
				if(current_com == NULL){
					struct Neighbor_com *com = malloc(sizeof(struct Neighbor_com));
					com->ID = Users[current->ID]->community;
					com->weight += current->weight;
					com->degree += 1;
					Communities[Users[i]->community]->next = com;
				}
				else{
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

/* "node" is include community or not. */
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
	int central = 0, count, seedCount = 0, topk = comIndex2;
	double **distToTargets = malloc(number * sizeof(double *));		// Store each node to each node's distance in this community
	double *maxTime = malloc(number * sizeof(double));				// is the same as firstRound in Baseline Algorithm
	double *eachReduce = malloc(number * sizeof(double));
	double *targets = malloc(number * sizeof(double));				// the minimum time to arrive each target
	int *maxTimeID = malloc(number * sizeof(int));
	int *seedSet = malloc(seedNumber * sizeof(int));
	bool best;
	double diffusionTime = 0.0;

	for(int i = 0 ; i < number ; i++){
		targets[i] = DBL_MAX;
		distToTargets[i] = malloc(totalvertices * sizeof(double));
		maxTime[i] = -1;
		eachReduce[i] = 0;
		for(int j = 0 ; j < totalvertices ; j++)
			distToTargets[i][j] = -1;
	}
	memset(seedSet, -1, seedNumber * sizeof(int));

	while(topk < 0 || topk > 0){
		InitializeEachReduce(eachReduce, number);
		best = true;

		for(int i = 0 ; i < number ; i++){
			distToTargets[i] = FindMTP(comMembers[i], distToTargets[i]);
			count = 0;
			for(int j = 0 ; j < totalvertices ; j++){
				if(Users[j] != NULL && isIncludeCom(Users[j]->ID, comMembers, number)){
					if(seedCount == 0){
						maxTime[count] = MAX(maxTime[count], distToTargets[i][j]);
						maxTimeID[count] = Users[j]->ID;
						count++;
						best = false;
					}
					else if(!isInclude(Users[j]->ID, seedSet) && distToTargets[i][j] < targets[i]){
//						printf("I'm here %d, j = %d\n", Users[j]->ID, j);
						eachReduce[count] += targets[i] - distToTargets[i][j];
						maxTimeID[count] = Users[j]->ID;
						count++;
						best = false;
					}
//					printf("dist To Target : %d to %d : %lf\n", Users[j]->ID, comMembers[i], distToTargets[i][j]);
				}
			}
		}

		// if there is no node has contribution , then quit the algorithm.
		if(best){
			printf("No more seeds !!\nIt's the shortest diffusion time !!\n");
			break;
		}
		if(seedCount == 0)
			central = BubbleSort(maxTime, false, number);
		else
			central = BubbleSort(eachReduce, true, number);

		seedSet[seedCount++] = maxTimeID[central];
		diffusionTime = 0.0;
//		printf("top %d is %d\n", seedCount, maxTimeID[central]);
		for(int i = 0 ; i < number ; i++){
			for(int j = 0 ; j < totalvertices ; j++){
				if(Users[j] != NULL && Users[j]->ID == maxTimeID[central]){
//					printf("node %d to node %d is %lf\n", Users[j]->ID, comMembers[i], distToTargets[i][j]);
					if(distToTargets[i][j] < targets[i])
						targets[i] = distToTargets[i][j];

					diffusionTime = MAX(diffusionTime, targets[i]);
					break;
				}
			}
		}
//		printf("\ncentral node is %d : %lf\n", maxTimeID[central], diffusionTime);
		topk++;

		// Just Do once time.
		if(topk > 0)
			break;
	}

	// calculate only one community's central node
	if(comIndex2 == -1){
		Communities[comIndex1]->central = comMembers[central];
		Communities[comIndex1]->radius = maxTime[0];
	}

	struct Central_Info info;
	if(comIndex2 > -2){
		info.central = maxTimeID[central];
		info.radius = maxTime[0];
	}
	else{
		info.central = maxTimeID[central];
		info.radius = diffusionTime;
	}

	free(distToTargets);
	free(maxTime);
	free(eachReduce);
	free(maxTimeID);
	free(seedSet);
	return info;
}

/* Calculate each community's central. */
void CalculateCentral(void)
{
	extern struct Community **Communities;
	extern int communityNum;
	extern int **comMember;
	extern int *eachComNumber;
	int i, j, count;
	comMember = malloc(communityNum * sizeof(int *));
	eachComNumber = malloc(communityNum * sizeof(int));

	puts("Calculate each community's central node...\n");
	// declaration of comMember
	for(i = 0 ; i < communityNum ; i++){
		comMember[i] = malloc(totalvertices * sizeof(int));
		for(j = 0 ; j < totalvertices ; j++)
			comMember[i][j] = -1;
	}

	// find members for each community
	for(i = 0 ; i < communityNum ; i++){
		count = 0;
		for(j = 0 ; j < totalvertices ; j++){
			if(Users[j] != NULL && Users[j]->community == i){
				comMember[i][count] = Users[j]->ID;
//				printf("member : %d\n", Users[j]->ID);
				count++;
			}
		}
		eachComNumber[i] = count;
		printf("community %d : %d\n", i, eachComNumber[i]);
	}

	for(i = 0 ; i < communityNum ; i++)
		BruteForce(comMember[i], eachComNumber[i], i, -1);

}

/* Sorted the communities' radius. */
void SortComRadius(double *sortRadius, int *indexRadius)
{
	extern int communityNum;
	double tmp;
	int itmp;

	for(int i = 0 ; i < communityNum-1 ; i++){
		for(int j = 0 ; j < communityNum-1 ; j++){
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

/* Pick the minimum diffusion radius community as minCom, and choose its closely connected
 * community as minCom2. Try to merge these two communities and compare to the maximum dif-
 * -fusion radius community(max R(C)), if it's smaller than max R(C), than combined it. */
void CommunityMerge(void)
{
	extern struct Community **Communities;
	extern int communityNum;
	extern int *eachComNumber;
	extern int **comMember;
	double maxRadius;
	int i, mergeCount, mergeNumbers, min1, min2, count = 0;
	struct Central_Info info;
	struct Community_Merge *current;
	bool first = true;

	puts("Access community merged...\n");
//	if(system("read var1") == -1) err("system pause error!!\n");
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
		int j = 0;
		while(min2 == -1){
			j++;
			min1 = indexRadius[count+j];
			min2 = Communities[indexRadius[count+j]]->closely[0];
			if((count+j) == (communityNum-1))
				return;
		}
		printf("min1 %d min2 %d\n", min1, min2);
//		if(system("read var1") == -1) err("system error!!\n");

		mergeNumbers = eachComNumber[min1] + eachComNumber[min2];
		int *mergedMembers = malloc(mergeNumbers * sizeof(int));
		mergeCount = 0;

		for(i = 0 ; comMember[min1][i] != -1 ; i++)
			mergedMembers[mergeCount++] = comMember[min1][i];
		for(i = 0 ; comMember[min2][i] != -1 ; i++)
			mergedMembers[mergeCount++] = comMember[min2][i];

		info = BruteForce(mergedMembers, mergeNumbers, min1, min2);
		printf("\nmerge Time : %lf\n", info.radius);
//		if(system("read var1") == -1) err("system pause error!\n");

		if(first){
			first = false;
			maxRadius = sortRadius[communityNum-1];
		}
		// if merged radius <= max R(C), then combined these two communities.
		if(info.radius <= maxRadius && info.radius != DBL_MAX){
			printf("Start merging...\n");
			Communities[min1]->merged = true;
			Communities[min2]->merged = true;

			// need recontruct the communities and sort the radius again
			if(CommunityMerged == NULL){  			// first one communities merged
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

/* Need reconstruct the "Communities", and update all informations including id,
 * weight, degree, central, radius, closely, merged, parent, next, ...*/
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
	Communities[communityNum-1]->topk = 0;
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
		memset(Communities[i]->closely, -1, communityNum * sizeof(int));
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

/* The merged communities will disapear. So other communities' information
 * should delete them. */
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

/* Reconstruct "comMember" by using realloc. */
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

/* Pick k nodes as seeds from maximum diffusion radius community.
 * There have two cases. (1).Ci is a merged community, so separate them as
 * two children and pick their central as seeds.(2).Ci is a original community,
 * so using BruteForce to pick the next seed. */
int *PickSeeds(void)
{
	extern struct Community_Merge *CommunityMerged;
	extern struct Community **Communities;
	extern int seedNumber;
	extern int communityNum;
	struct Central_Info info;
	double *sortRadius = malloc(communityNum * sizeof(double));
	int *indexRadius = malloc(communityNum * sizeof(int));
	int seedCount = 0, maxRadiusIndex;
	int *seedSet = malloc(seedNumber * sizeof(int));

	memset(seedSet, -1, seedNumber * sizeof(int));
	memset(sortRadius, -1, communityNum * sizeof(double));

	for(int i = 0 ; i < communityNum ; i++){
		if(!Communities[i]->merged){
			seedSet[seedCount++] = Communities[i]->central;
			sortRadius[i] = Communities[i]->radius;
			printf("pick %d as seeds -> %lf\n", Communities[i]->central, Communities[i]->radius);
		}
		else
			sortRadius[i] = -1;
		indexRadius[i] = i;
	}

	printf("Start picking seeds...\n");
	if(seedCount >= seedNumber){
		printf("Need more than %d seeds.\n", seedCount);
		seedNumber = seedCount;
	}
	while(seedCount < seedNumber){
		// Sorted the communities' radius
		SortComRadius(sortRadius, indexRadius);
		printf("maxmum radius community is %d : %lf\n", indexRadius[communityNum-1], sortRadius[communityNum-1]);
		maxRadiusIndex = indexRadius[communityNum-1];

		if(Communities[maxRadiusIndex]->radius == 0)
			break;

		if(Communities[maxRadiusIndex]->topk == eachComNumber[maxRadiusIndex]){
			printf("Already pick all nodes as seeds.\n");
			break;
		}
		// pick its children's central nodes
		else if(Communities[maxRadiusIndex]->parent){
			printf("I'm parents, seperate me.\n");
			struct Community_Merge *current = CommunityMerged;
			while(current != NULL && current->ID != Communities[maxRadiusIndex]->ID){
				current = current->next;
			}
			// delete original central node from seedSet
			for(int i = 0 ; i < seedCount ; i++){
				if(seedSet[i] == current->ID)
					seedSet[i] = -1;
				seedCount--;
			}
			seedSet[seedCount++] = Communities[current->child[0]]->central;
			seedSet[seedCount++] = Communities[current->child[1]]->central;
			Communities[current->child[0]]->merged = false;
			Communities[current->child[1]]->merged = false;
			Communities[current->child[0]]->topk = 1;
			Communities[current->child[1]]->topk = 1;
			printf("pick %d as seeds.\n", Communities[current->child[0]]->central);
			printf("pick %d as seeds.\n", Communities[current->child[1]]->central);

			Communities[maxRadiusIndex]->radius = 0;
		}
		else{
			// this community maybe pick more than 2 seeds
			Communities[maxRadiusIndex]->topk += 1;
			int topk = -(Communities[maxRadiusIndex]->topk);
			if(topk == -1){
				info.central = Communities[maxRadiusIndex]->central;
			}
			else{
				info = BruteForce(comMember[maxRadiusIndex], eachComNumber[maxRadiusIndex], maxRadiusIndex, topk);
				// update community's radius
				Communities[maxRadiusIndex]->radius = info.radius;
			}
			seedSet[seedCount++] = info.central;
			printf("pick %d as seeds.\n", info.central);
		}

		for(int i = 0 ; i < communityNum ; i++){
			if(!Communities[i]->merged)
				sortRadius[i] = Communities[i]->radius;
			else
				sortRadius[i] = -1;
			indexRadius[i] = i;
		}
//		if(system("read var1") == -1) err("system pause error!\n");
	}
	printf("Seedset are : \n\t");
	for(int i = 0 ; i < seedCount ; i++){
		printf("%d ", seedSet[i]);
	}
	puts("\n");

	return seedSet;
}

/* Calculate the diffusion time from seeds to targets. */
double DiffusionFromSeeds(int *seedSet, int targetCount)
{
	double **distToTargets = malloc(targetCount * sizeof(double *));
	double *minTime = malloc(targetCount * sizeof(double));
	double diffusionTime = 0;

	for(int i = 0 ; i < targetCount ; i++){
		distToTargets[i] = malloc(totalvertices * sizeof(double));
		minTime[i] = DBL_MAX;
		for(int j = 0 ; j < totalvertices ; j++){
			distToTargets[i][j] = -1;
		}
	}

	for(int i = 0 ; i < targetCount ; i++){
		distToTargets[i] = FindMTP(targetUsers[i], distToTargets[i]);
		for(int j = 0 ; j < totalvertices ; j++){
			if(Users[j] != NULL && isInclude(Users[j]->ID, seedSet)){
				minTime[i] = MIN(minTime[i], distToTargets[i][j]);
//				printf("user %d to target %d ; %lf\n", Users[j]->ID, targetUsers[i], distToTargets[i][j]);
			}
		}
	}

	for(int i = 0 ; i < targetCount ; i++){
		diffusionTime = MAX(diffusionTime, minTime[i]);
	}
//	printf("Total diffusionTime is %lf\n", diffusionTime);
	free(distToTargets);
	free(minTime);;
	return diffusionTime;
}

void Community_based(int targetCount)
{
	int *seedSet = malloc(seedNumber * sizeof(int));
	memset(seedSet, -1, seedNumber * sizeof(int));
	double time_spent;
	extern clock_t begin, end;

	// Initialize the communities' structure
	InitialCommunities();
	Closely();
	CalculateCentral();
	CommunityMerge();
	// pick seeds and calculate diffusion time.
	seedSet = PickSeeds();
	double time = DiffusionFromSeeds(seedSet, targetCount);

	printf("targets are : \n");
	for(int i = 0 ; i < targetCount ; i++)
		printf("%d ", targetUsers[i]);
	FILE *f = write_file("result");
	if(f == NULL) err("Couldn't open file.\n");
	fprintf(f, "numbers of targets : %d\n", targetCount);
	printf("\nnumbers of targets : %d\n", targetCount);
	printf("numbers of seeds : %d\n", seedNumber);
	fprintf(f, "numbers of seeds : %d\n", seedNumber);
	printf("Seedset are : \n\t");
	fprintf(f, "Seedset are : \n\t");
	for(int i = 0 ; i < seedNumber ; i++){
		printf("%d ", seedSet[i]);
		fprintf(f, "%d ", seedSet[i]);
	}
	printf("\nTotal diffusion time is %lf\n", time);
	fprintf(f, "\nTotal diffusion time is %lf\n", time);
	end = clock();
	time_spent = (double)(end-begin) / CLOCKS_PER_SEC;
	fprintf(f, "execution time : %f\n", time_spent);
	fclose(f);
}
