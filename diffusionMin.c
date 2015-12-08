#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>

struct Vertex {
	int ID;
	int label[10];
	struct Neighbor *prev;
	struct Neighbor *next;
};

struct Neighbor {
	int ID;
	double weight;
	struct Neighbor *next;
};

struct Vertex **Users;
void InitializeVertices(int);
void StoreRelationship(char *);
int totalvertices;

void readGraph()
{
	FILE *fp = fopen("synthetic.edge", "r");
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

void StoreRelationship(char *relation)
{
	int node1 = atoi(relation);
	int node2 = atoi(relation + 2);
	int closeness = atoi(relation + 4);
	struct Neighbor *current = NULL;

	if(!Users[node1]){
		Users[node1] = malloc(sizeof(struct Vertex));
		Users[node1]->ID = node1;
//		printf("create node %d\n", node1);
	}
	if(!Users[node2]){
		Users[node2] = malloc(sizeof(struct Vertex));
		Users[node2]->ID = node2;
//		printf("create node %d\n", node2);
	}

	struct Neighbor *neighbor = malloc(sizeof(struct Neighbor));
	neighbor->ID = node2;
	neighbor->weight = closeness;
	neighbor->next = NULL;

	if(Users[node1]->next != NULL){
		current = Users[node1]->next;
//		printf("%d\n", current->ID);
		while(current->next != NULL){
			current = current->next;
//			printf("%d\n", current->ID);
		}
		current->next = neighbor;
	}
	else
		Users[node1]->next = neighbor;

	return;
}

void printGraph()
{
	int i;
	struct Neighbor *current = NULL;

	for(i = 0 ; i < totalvertices ; i++){
		if(Users[i] != NULL){
			printf("node %d : \n", Users[i]->ID);
			if(Users[i]->next != NULL){
				current = Users[i]->next;
				printf("\tnode %d\n", current->ID);
				while(current->next != NULL){
					current = current->next;
					printf("\tnode %d\n", current->ID);
				}
			}
		}
	}
	return;
}

int main(int argc, char **argv)
{
	
	readGraph();
	printf("\n");
	printGraph();

	return 0;
}
