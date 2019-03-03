#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "AuxList.h"
#include "ServerList.h"
#include "MiscHeader.h"


void ServerListCreate(struct ServerList** list)
{
	if( (*list = malloc(sizeof(struct ServerList)))==NULL)
	{
		perror("MirrorServer ServerListCreate()");
		exit(EXIT_FAILURE);
	}

	(*list) -> start = NULL;
	(*list) -> counter = 0;
}


void ServerListInsert(struct ServerList* list,char* data)
{
	struct ServerListNode* new;
	struct ServerListNode* current;
	char address[NAMESIZE];
	char port[PORTSIZE];
	char dirorfile[PATHSIZE];
	int delay;
	int ret;
	ret = sscanf(data,"%[^:] : %[^:] : %[^:] : %d",address,port,dirorfile,&delay);
	if(ret != 4)
	{
		fprintf(stderr,"Incorrect request: \"%s\"\nMoving on to the next one...\n\n",data);
		// exit(EXIT_FAILURE);
		return;
	}

	/*Check if server is already in ServerList*/
	current = list->start;
	while(current)
	{
		if(!strcmp(current->address,address) && !strcmp(current->port,port))
		{
			ListInsert(current->request_list,dirorfile,delay);
			return;
		}
		current = current -> next;
	}
	/*Create a new node*/
	if( (new = malloc(sizeof(struct ServerListNode)))==NULL)
	{
		perror("MirrorServer ListInsert()");
		exit(EXIT_FAILURE);
	}

	/*Copy address*/
	new->address = malloc( (strlen(address)+1)*sizeof(char) );
	strcpy(new->address,address);

	/*Copy port*/
	new->port = malloc( (strlen(port)+1)*sizeof(char) );
	strcpy(new->port,port);

	/*Create a request list for this server and insert dirorfile to this list */
	ListCreate(&new->request_list);
	ListInsert(new->request_list,dirorfile,delay);

	/*Attach the new node to the list*/
	new->next = list->start;
	list->start = new;

	/*Increase number of elements*/
	list->counter ++ ;
}

void ServerListPrint(struct ServerList* list)
{
	struct ServerListNode* current = list->start;
	if(!list->counter)printf("ServerList is empty\n");

	while(current)
	{
		printf("Server:%s | Port:%s\n",current->address,current->port);

		printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
		ListPrint(current->request_list);
		printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

		current=current->next;
	}
}

void ServerListPrintInner(struct ServerListNode* node)
{
	printf("Address:%s\n",node->address);
	printf("Port:%s\n",node->port);

	printf("--------------------------------------\n");
	ListPrint(node->request_list);
	printf("--------------------------------------\n");

}


void ServerListDestroy(struct ServerList* list)
{

	struct ServerListNode* current;
	struct ServerListNode* temp;
	current = list->start;

	/*Loop until we've reached end of list*/
	while(current)
	{
		ListDestroy(current->request_list);
		free(current->address);
		free(current->port);
		temp = current;
		current = current -> next;
		free(temp);
	}
	/*Free the main list node*/
	free(list);
}
