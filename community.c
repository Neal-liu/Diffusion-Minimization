#include <stdio.h>
#include <float.h>
#include <string.h>
#include <stdlib.h>
#include "diffusionMin.h"

/* Initialize Communities' structure. */
void InitialCommunities(void)
{
	int i, j;
	extern struct Community **Communities;
	extern int communityNum;

	for(i = 0 ; i < communityNum ; i++){
		Communities[i] = malloc(sizeof(struct Community));
		Communities[i]->ID = i;
		Communities[i]->weight = 0.0;
		Communities[i]->degree = 0;
		Communities[i]->central = -1;
		Communities[i]->radius = 0.0;
		Communities[i]->closely = malloc(communityNum * sizeof(int));
		for(j = 0 ; j < communityNum ; j++)
			Communities[i]->closely[j] = -1;
		Communities[i]->next = NULL;
	}

	return ;
}

/* Calculate the closely connected communities. */
void Closely(void)
{
	int i, j, count;
	struct Neighbor *current = NULL;
	struct Neighbor_com *current_com = NULL;
	double average, tmp;
	extern struct Community **Communities;
	extern int communityNum;

	// Initialize the communities' structure
	Communities = malloc(communityNum * sizeof(struct Community *));
	InitialCommunities();

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
		if(Communities[i]->degree == 0)
			continue;
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

bool isIncludeCom(int node, int *comMembers)
{
	int count = 0;
	while(comMembers[count] != -1){
		if(node == comMembers[count])
			return true;
		count++;
	}
	return false;
}

/* Find the central node from this community with brute force algorithm. */
double BruteForce(int *comMembers, int number, int comIndex1, int comIndex2)
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
			if(Users[j] != NULL && isIncludeCom(Users[j]->ID, comMembers)){
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
	if(comIndex2 == -1){
		Communities[comIndex1]->central = comMembers[central];
		Communities[comIndex1]->radius = maxTime[0];
	}
	
	return maxTime[0];
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
	double tmp;
	double *sortRadius = malloc(communityNum * sizeof(double));
	int i, j, itmp;
	int *indexRadius = malloc(communityNum * sizeof(int));

	for(i = 0 ; i < communityNum ; i++){
		sortRadius[i] = Communities[i]->radius;
		indexRadius[i] = i;
	}

	// Sorted the communities' radius
	SortComRadius(sortRadius, indexRadius);
/*
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
*/
	printf("minimum radius community is %d : %lf\n", indexRadius[0], sortRadius[0]);
	printf("maxmum radius community is %d : %lf\n", indexRadius[communityNum-1], sortRadius[communityNum-1]);

	// Not Yet Finished...
	int *mergedMembers;
	int mergeCount, mergeNumbers, min1, min2;
	double mergeTime;
//	while(communityNum > seedNumber){
	while(1){
		min1 = indexRadius[0];
		min2 = Communities[indexRadius[0]]->closely[0];
		printf("min1 %d min2 %d\n", min1, min2);
		mergeNumbers = eachComNumber[min1] + eachComNumber[min2];
		mergedMembers = malloc(mergeNumbers * sizeof(int));
		mergeCount = 0;
		for(i = 0 ; comMember[min1][i] != -1 ; i++)
			mergedMembers[mergeCount++] = comMember[min1][i];
		for(i = 0 ; comMember[min2][i] != -1 ; i++)
			mergedMembers[mergeCount++] = comMember[min2][i];
//		for(i = 0 ; i < mergeNumbers ; i++)
//			printf("%d ", mergedMembers[i]);

		mergeTime = BruteForce(mergedMembers, mergeNumbers, min1, min2);
		printf("\nmerge Time : %lf\n", mergeTime);
		system("read var1");
		if(mergeTime <= sortRadius[communityNum-1]){
			// need recontruct the communities and sort the radius again
			continue;
		}
		else
			break;
	}
}

void Community_based(int targetCount)
{
	Closely();
	CalculateCentral();
	CommunityMerge();
}
