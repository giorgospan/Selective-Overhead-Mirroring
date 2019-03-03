#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "InitiatorFunctions.h"
#include "MiscHeader.h"

void communication(int socket,char* list)
{
	/*Send requests to MirrorServer*/
	char* request;
	char sendbuffer[MSGSIZE];
	memset(sendbuffer,0,MSGSIZE);
	char rcvbuffer[MSGSIZE];
	int end = 0;

	request = strtok (list,",");
    while (request != NULL)
	{
		strcpy(sendbuffer,request);
		remove_slash(sendbuffer);
		if ( write_data(socket ,sendbuffer,MSGSIZE) == -1)
		{
			perror("Initiator write()");
			exit(EXIT_FAILURE);
		}
		request = strtok (NULL, ",");
    }
	/*Inform MirrorServer that you have no more requests*/
	if ( write_data(socket,"END",MSGSIZE) == -1)
	{
		perror("Initiator writing END");
		exit(EXIT_FAILURE);
	}
	/*Wait for statistics*/
	if ( read_data(socket,rcvbuffer,MSGSIZE) == -1)
	{
		perror("Initiator reading stats");
		exit(EXIT_FAILURE);
	}

	printf("Statistics:\n");
	printf("=============================================================\n");
	printf("%s\n\n",rcvbuffer);
}




