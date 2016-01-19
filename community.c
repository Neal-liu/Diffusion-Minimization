#include <stdio.h>
#include <float.h>
#include <string.h>
#include <stdlib.h>
#include "diffusionMin.h"

/* Initialize Communities' structure. */
void InitialCommunities()
{
	int i, j;
	for(i = 0 ; i < communityNum ; i++){
		Communities[i] = malloc(sizeof(struct Community));
		Communities[i]->ID = i;
		Communities[i]->weight = 0.0;
		Communities[i]->degree = 0;
		Communities[i]->central = -1;
		Communities[i]->closely = malloc(communityNum * sizeof(int));
		for(j = 0 ; j < communityNum ; j++)
			Communities[i]->closely[j] = -1;
		Communities[i]->next = NULL;
	}

	return ;
}

/* Calculate the closely connected communities. */
void closely()
{
	int i, j, count;
	struct Neighbor *current = NULL;
	struct Neighbor_com *current_com = NULL;
	double average, tmp;

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

	// print communities information
	for(i = 0 ; i < communityNum ; i++){
		printf("Community %d :\n total weight : %lf\n total degree : %d\n", i, Communities[i]->weight, Communities[i]->degree);
		current_com = Communities[i]->next;
		while(current_com != NULL){
			printf("\t To community %d : total weight : %lf, total degree : %d\n", current_com->ID, current_com->weight, current_com->degree);
			current_com = current_com->next;
		}

	}

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
			if(average >= tmp){
				while(Communities[i]->closely[count] == -1)
					count++;
				Communities[i]->closely[count] = current_com->ID;
				printf("\t closely community : %d\n", current_com->ID);
			}
			current_com = current_com->next;
		}

	}
	

	return ;
}


void Community_based(int targetCount)
{
	closely();

}
