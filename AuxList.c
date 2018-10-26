#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include "AuxList.h"

void ListCreate(struct List** list)
{
	
	if( (*list = malloc(sizeof(struct List)))==NULL)
	{
		perror("ContentServer ListCreate()");
		exit(1);
	}
	
	(*list) -> start = NULL;
	(*list) -> counter = 0;
}




void ListInsert(struct List* list,char* payload,int type)
{
	struct ListNode* new;
	/*Create a new node*/
	if( (new = malloc(sizeof(struct ListNode)))==NULL)
	{
		perror("ContentServer ListInsert()");
		exit(1);
	}
	/* Add payload to the newly created node */
	new->entity = malloc( (strlen(payload)+1)*sizeof(char) );
	strcpy(new->entity,payload);
	new->type = type;
	
	/*Attach the new node to the list*/
	new->next = list->start;
	list->start = new;
	
	/*Increase number of elements*/
	list->counter ++ ;
}

void ListPrint(struct List* list)
{
	struct ListNode* current = list->start;
	if(!list->counter)printf("List is empty\n");
	
	while(current)
	{
		printf("Entity		:%s\n",current->entity);
		printf("Type or Delay	:%d\n",current->type);
		current=current->next;
	}
}




void ListDestroy(struct List* list)
{
	
	struct ListNode* current;
	struct ListNode* temp;
	current = list->start;
	
	/*Loop until we've reached end of list*/
	while(current)
	{
		temp = current;
		current = current -> next;
		free(temp);
	}
	
	/*Free the main list node*/
	free(list);
}