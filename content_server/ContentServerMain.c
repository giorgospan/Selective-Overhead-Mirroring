#include <stdio.h>
#include <string.h> /* For strerror */
#include <stdlib.h> /* For exit,malloc,realloc */
#include <pthread.h> /* pthread functions */
#include <unistd.h> /* For close,read,write */
#include <errno.h>

/*For sockets*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "ContentServerFunctions.h"
#include "MiscHeader.h"
#include "AuxList.h"

struct List* contentlist;

int main(int argc ,char* argv[])
{
	pthread_t* ids;
	int count      = 2; 	/*size of thread_id array*/
	int numthreads = 0; /*number of threads so far*/
	int arg1,arg2;
	int i,err,ret,status,sock,newsock;
	char* port;
	char* pathname;
	char rcvbuffer[MSGSIZE];

	/*Gonna use this for accept*/
	socklen_t length;
	struct sockaddr_storage mirrorserver_address;

	arg1 = arg2 = 0;
	for(i=1;i<argc;++i)
	{

		if(!strcmp(argv[i],"-p"))
		{
			port = malloc((strlen(argv[i+1])+1)* sizeof(char)   );
			strcpy(port,argv[i+1]);
			arg1 = 1;
		}
		else if(!strcmp(argv[i],"-d"))
		{
			pathname  = malloc((strlen(argv[i+1])+1)* sizeof(char)   );
			strcpy(pathname,argv[i+1]);
			arg2 = 1;
		}
	}
	/*Sanity check*/
	if(!arg1 || !arg2)
	{
		fprintf(stderr,"Usage: ./ContentServer -p <port> -d <dirorfilename>\n\n");
		exit(1);
	}
	printf("=============================================================\n");
	printf("%-20s%s\n","Port:",port);
	printf("%-20s%s\n","Content Directory:",pathname);
	printf("=============================================================\n\n");

	/*Remove trailing slash from given path*/
	remove_slash(pathname);

	/* Create a list with my available content */
	ListCreate(&contentlist);
	content(pathname);
	ids = malloc(2*sizeof(pthread_t));

	/* Create socket and start listening to it */
	ret = CreateServerSocket(&sock,&status,port);
	if(ret == 1)
	{
		fprintf(stderr, "ContentServer getaddrinfo():%s\n", gai_strerror(status));
		exit(1);
	}
	else if(ret == 2)
	{
		perror("ContentServer: setsockopt()");
		exit(1);
	}
	else if(ret == 3)
	{
		perror("ContentServer could not find an optimal socket");
		exit(1);
	}
	else if(ret == 4)
	{
		perror("ContentServer listen()");
        	exit(1);
	}

	// printf("Passive socket		:%d\n\n",sock);

	/* Loops forever waiting for clients[mirror-managers or workers] to serve */
	while(1)
	{
		length = sizeof mirrorserver_address;

		if( (newsock = accept(sock, (struct sockaddr *)&mirrorserver_address, &length)) == -1)
		{
			perror("ContentServer accept()");
			exit(1);
		}


		/* Read LIST or FETCH */
		if(read_data(newsock,rcvbuffer,MSGSIZE)==-1)
		{
			perror("ContentServer read()");
			exit(1);
		}


		/* Check if realloc is needed for thread_id array */
		if(numthreads+1 == count)
		{
			pthread_t* temp = realloc(ids,2*count*sizeof(pthread_t));
			if(!temp)
			{
				perror("ContentServer realloc()");
				exit(1);
			}
			count = 2*count;
			ids = temp;
		}

		/* This will be passed to thread's functions */
		struct argument arg;
		strcpy(arg.rcvbuffer,rcvbuffer);
		arg.sock = newsock;

		/* Create a new thread to serve the incoming request */
		if (err = pthread_create(ids+numthreads,NULL,thread_f,(void *)&arg))
		{
			my_perror("MirrorServer pthread_create()",err);
			exit (1) ;
		}

		++numthreads;
	}

	/* Wait for threads to exit */
	for(i=0;i<numthreads;++i)
	{
		if ( err = pthread_join (*(ids+i), NULL ))
		{
			my_perror("ContentServer pthread_join", err );
			exit (1) ;
		}
	}
	/* Destroy content list */
	ListDestroy(contentlist);
	/* Free malloc()-ed pointer */
	free(port);
	free(pathname);
	close(sock);

	printf("ContentServer is exiting...\n");
	exit(0);
}
