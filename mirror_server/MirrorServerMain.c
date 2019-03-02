#include <stdio.h>
#include <string.h>/* For strerror strcat strcpy */
#include <stdlib.h>/* For exit malloc free   */
#include <pthread.h>/* For threads  */
#include <errno.h>/* For errno detection in mkdir */
#include <unistd.h>/* For close and read */

/*For sockets*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "MirrorServerFunctions.h"
#include "ServerList.h"
#include "MiscHeader.h"
#include "Buffer.h"

struct ServerList* server_list;
long long* size_table;
int count;
buffer_t buffer;
pthread_t* manager_ids;


long long bytes_transferred 	   = 0;
int files_transferred 			     = 0;
int dirs_transferred			       = 0;
int numDevicesDone 				       = 0;
int managers_exited 			       = 0;
char* dirname;


pthread_mutex_t buffer_mtx 	     = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t bytes_mtx 	     = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t files_mtx 	     = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t dirs_mtx 	       = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t devices_done_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t size_table_mtx 	 = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t cond_nonempty 	   = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_nonfull 	   = PTHREAD_COND_INITIALIZER;

int main(int argc,char* argv[])
{

	int i,err,ret,status;
	int arg1,arg2,arg3;
	int threadnum;
	int sock,newsock;
	double av_size,var;
	char* port;
	pthread_t* worker_ids;

	/*Gonna use this for accept*/
	struct sockaddr_storage initiator_address;
	socklen_t initiator_length;

	arg1 = arg2 = arg3 = 0;
	for(i=1;i<argc;++i)
	{

		if(!strcmp(argv[i],"-p"))
		{
			port = malloc((strlen(argv[i+1])+1)* sizeof(char));
			strcpy(port,argv[i+1]);
			arg1 = 1;
		}
		else if(!strcmp(argv[i],"-m"))
		{
			dirname  = malloc((strlen(argv[i+1])+1)* sizeof(char));
			strcpy(dirname,argv[i+1]);
			arg2 = 1;
		}
		else if(!strcmp(argv[i],"-w"))
		{
			threadnum = atoi(argv[i+1]);
			arg3 = 1;
		}
	}
	/*Sanity check*/
	if(!arg1 || !arg2 || !arg3)
	{
		fprintf(stderr,"Usage: ./MirrorServer -p <port> -m <dirname> -w <threadnum>\n\n");
		exit(1);
	}

	printf("\n=============================================================\n");
	printf("%-20s%s\n","Mirror Port:",port);
	printf("%-20s%d\n","Mirror Threadnum:",threadnum);
	printf("%-20s%s\n","Mirror Dest Dir:",dirname);
	printf("=============================================================\n\n");

	count      = 2;
	size_table = malloc(2*sizeof(long long));

	/*Create directory for results*/
	remove_slash(dirname);

	/*Call mkpath */
	ret = mkpath(dirname,NULL,0);
	switch (ret)
	{
		case -1:
			printf("Note: Destination directory already exists\n\n");
			break;
		case 1:
		case 2:
			perror("MirrorServer making dirname");
			exit(1);
	}

	/* Create list with requests for each device */
	ServerListCreate(&server_list);

	/* Create ID table for worker threads */
	if ((worker_ids = malloc(threadnum*sizeof(pthread_t))) == NULL )
	{
		perror("MirrorServer malloc() for worker ids");
		exit(1);
	}

	/*Create threadnum workers*/
	for(i=0;i<threadnum;++i)
	{
		if (err = pthread_create(worker_ids+i,NULL,worker,NULL))
		{
			my_perror("MirrorServer pthread_create()",err);
			exit(1);
		}
	}

	/*Initialize our buffer*/
	buffer.data = malloc(BUFFERSIZE*sizeof(struct buffer_entry));
	for(i=0;i<BUFFERSIZE;++i)
	{
		buffer.data[i].address       = NULL;
		buffer.data[i].port          = NULL;
		buffer.data[i].dirorfilename = NULL;
	}
	buffer.start = 0;
	buffer.end   = -1;
	buffer.count = 0;

	/*Create socket and start listening to it*/
	ret = CreateServerSocket(&sock,&status,port);
	if(ret == 1)
	{
		fprintf(stderr, "MirrorServer getaddrinfo():%s\n", gai_strerror(status));
		exit(1);
	}
	else if(ret == 2)
	{
		perror("MirrorServer: setsockopt()");
		exit(1);
	}
	else if(ret == 3)
	{
		perror("MirrorServer could not find an optimal socket");
		exit(1);
	}
	else if(ret == 4)
	{
		perror("MirrorServer listen()");
        exit(1);
	}

	/* Wait for Initiator to connect */
  initiator_length = sizeof initiator_address;
	if( (newsock = accept(sock, (struct sockaddr *)&initiator_address, &initiator_length)) == -1)
	{
		perror("MirrorServer accept()");
		exit(1);
	}

	/*Start communication with Initiator using the newsock*/
	communication(newsock);

	/*Wait for workers to exit*/
	for(i=0;i<threadnum;++i)
	{
		if ( err = pthread_join (*(worker_ids+i), NULL ))
		{
			my_perror("MirrorServer() pthread_join [worker]", err );
			exit (1) ;
		}
	}

	/*Send back statistics to Initiator*/
	char statistics[MSGSIZE];
	memset(statistics,0,MSGSIZE);
	char bytes[MSGSIZE];
	char files[MSGSIZE];
	char dirs[MSGSIZE];
	char average[MSGSIZE];
	char variance[MSGSIZE];

	av_size = (double)bytes_transferred / files_transferred;
	var     = find_variance(av_size,files_transferred);

	snprintf(bytes,sizeof(bytes)-1,"%-30s%llu","Bytes transferred:",bytes_transferred);
	snprintf(files,sizeof(bytes)-1,"%-30s%d","Files transferred:",files_transferred);
	snprintf(dirs,sizeof(bytes)-1,"%-30s%d","Directories transferred:",dirs_transferred);
	snprintf(average,sizeof(average)-1,"%-30s%.2lf bytes","Average file size:",av_size);
	snprintf(variance,sizeof(variance)-1,"%-30s%.2lf bytes","Size variance:",var);


	snprintf(statistics,sizeof(statistics)-1,"%s\n%s\n%s\n%s\n%s",bytes,files,dirs,average,variance);

	if ( write_data(newsock,statistics,MSGSIZE) == -1)
	{
		perror("MirrorServer write to Initiator");
		exit(1);
	}
	ServerListDestroy(server_list);

	if(pthread_cond_destroy(&cond_nonempty))
	{
		perror("cond_nonempty");
		exit(1);
	}
	if(pthread_cond_destroy(&cond_nonfull))
	{
		perror("cond_nonfull");
		exit(1);
	}

	pthread_mutex_destroy(&buffer_mtx);
	pthread_mutex_destroy(&files_mtx);
	pthread_mutex_destroy(&bytes_mtx);
	pthread_mutex_destroy(&devices_done_mtx);

	close(sock);
	close(newsock);
	free(port);
	free(buffer.data);
	free(worker_ids);
	free(dirname);
	free(size_table);

	printf("\n==========================\n");
	printf("MirrorServer is exiting...");
	printf("\n==========================\n");
	exit(0);
}
