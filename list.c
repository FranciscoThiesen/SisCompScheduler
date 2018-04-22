#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

struct List {
	struct Node *first, *last;
}

struct Node {
  int data;
  struct Node *next;
};

void initList() {
	struct List* list = (struct List*) malloc(sizeof(struct List));
	list->first = list->last = NULL;
	return List;
}

void push(struct List* list, int new_data) {
    struct Node* new_node = (struct Node*) malloc(sizeof(struct Node));
	new_node->data = new_data;  
    new_node->next = NULL;
  	list->last->next = new_node;
  	list->last = new_node;
}

int pop(struct List* list) {
	int data = list->node->first;
	list->node->first = list->node->first->next;
	return data;
}
