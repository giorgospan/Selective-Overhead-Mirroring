#include <stdio.h>
#include <string.h>/* For strerror */
#include <stdlib.h>/* For exit */
#include <unistd.h>/* For close and read */
#include <pthread.h>/* For threads */
#include <fcntl.h>/* For having access to flags */
#include <sys/stat.h>/* For mkdir */
#include <sys/types.h>/* For mkdir , open*/
#include <netdb.h>/* For gai_strerror */
#include <errno.h>/* For errno in mkpath*/
#include <math.h>/* For sqrt()*/

#include "MirrorServerFunctions.h"
#include "ServerList.h"
#include "AuxList.h"
#include "MiscHeader.h"
#include "Buffer.h"

double find_variance(double av_size,int n)
{
	int i;
	double sum = 0;
	// printf("AverageSize:%.2lf\n",av_size);
	for(i=0;i<n;++i)
	{
		// printf("Size[%d]:%llu\n",i,size_table[i]);
		sum = sum + (size_table[i]-av_size)*(size_table[i]-av_size);
	}
	// printf("Sum:%.2lf\n",sum);
	return sum/n;
}

int mkpath(const char* given_path,int* fd,int type)
{
	size_t len = strlen(given_path);
	char *path;
	char *p;
	errno = 0;

	/* We are gonna use a local variable */
	path = malloc((strlen(given_path)+1) * sizeof(char));
	strcpy(path, given_path);


	/* Iterate the string */
	for (p = path + 1; *p; p++)
		if (*p == '/')
		{
			/* Temporarily truncate */
			*p = '\0';
			/* Might fail if dir already exists */
			if(mkdir(path,DIRPERMS)==-1)
				if(errno!=EEXIST)
					return 1;
				*p = '/';
		}
	switch (type)
	{
		case 0:
			if(mkdir(path,DIRPERMS)==-1)
				if(errno!=EEXIST) return 1;
				else return -1;
			break;
		case 1:
			if (( *fd = open (path,O_CREAT | O_EXCL | O_WRONLY ,FILEPERMS ))== -1)
				if(errno!=EEXIST) return 2;
				else return -1;
	}
	return 0;
}

void communication(int sock)
{
	char rcvbuffer[MSGSIZE];
	struct ServerListNode* temp;
	int i,status,err,nread;

	while(strcmp(rcvbuffer,"END"))
	{
		/* Read Directory or File */
		if(read_data(sock,rcvbuffer,MSGSIZE)==-1)
		{
			perror("MirrorServer reading from Initiator");
			exit(1);
		}
		/* Insert it in our list */
		if(strcmp(rcvbuffer,"END"))
			ServerListInsert(server_list,rcvbuffer);
	}

	/* Create ID table for manager threads */
	if ((manager_ids = malloc(server_list->counter*sizeof(pthread_t))) == NULL )
	{
		perror ("MirrorServer malloc() for manager ids");
		exit (1) ;
	}
	/* Assign each mirror_manager thread a ContentServer to communicate with */
	temp = server_list-> start;
	for(i=0;i<server_list->counter;++i,temp=temp->next)
	{
		if (err = pthread_create(manager_ids+i,NULL,mirror_manager,(void *)temp))
		{
			my_perror("MirrorServer pthread_create()",err);
			exit (1) ;
		}
	}
	/* Wait for managers to exit */
	for(i=0;i<server_list->counter;++i)
	{
		if ( err = pthread_join (*(manager_ids+i), NULL ))
		{
			my_perror("MirrorServer() pthread_join[manager]", err );
			exit (1) ;
		}
	}
	/* Set managers_exited to TRUE */
	managers_exited = 1;
	/* Unblock workers that might be suspended on cond_nonempty */
	pthread_cond_broadcast(&cond_nonempty);

	free(manager_ids);
}

void* mirror_manager(void* arg)
{
	int sock,status,ret,found,nwrite;
	int type;
	char path[PATHSIZE];
	char rcvbuffer[MSGSIZE];
	char message[MSGSIZE];
	struct ListNode* request;
	struct ServerListNode* node = (struct ServerListNode*)arg;

	/* Create connection with ContentServer */
	ret = CreateClientSocket(&sock,&status,node->address,node->port);
	if(ret == 1 || ret==2)
	{
		printf("Could not connect to ContentServer: <%s|%s>\n",node->address,node->port);
		pthread_exit((void*)0);
	}

	/* Do this for each request intended for this ContentServer */
	request = node -> request_list -> start;
	while(request)
	{
		/* ID: <ContentServer Address,ContentServer Port> */
		sprintf(message,"LIST %s %s %d",node->address,node->port,request->type);

		/* Send LIST message to ContentServer */
		if ( write_data(sock ,message,MSGSIZE) == -1)
		{
			perror("MirrorManager writing LIST message");
			exit(1);
		}

		found=0;
		memset(rcvbuffer,0,MSGSIZE);
		/* Read results [one by one] from LIST */
		while(strcmp(rcvbuffer,"END"))
		{
			/* Read incoming path */
			if(read_data(sock,rcvbuffer,MSGSIZE) == -1)
			{
				perror("MirrorManager read()");
				exit(1);
			}
			if(strcmp(rcvbuffer,"END")!=0)
			{
				sscanf(rcvbuffer,"%s %d",path,&type);

				/* Apply filter */
				if(filter(path,request->entity))
				{
					found=1;
					/* Place it in the buffer */
					place(path,node->address,node->port,type,request->type);
				}
			}
			else break;
		}
		if(!found)printf("\n<%s> not found in <%s>\n",request->entity,node->address);

		/* Move to the next request */
		if( !(request = request -> next))
		{
			/* Inform ContentServer that you do not have any other requests */
			if ( write_data(sock ,"END",MSGSIZE) == -1)
			{
				perror("MirrorManager write()");
				exit(1);
			}
		}
		else
		{
			/* Inform ContentServer that you still have more requests */
			if ( write_data(sock ,"NO END",MSGSIZE) == -1)
			{
				perror("MirrorManager write()");
				exit(1);
			}
		}
	}

	pthread_mutex_lock(&devices_done_mtx);
	++numDevicesDone;
	pthread_mutex_unlock(&devices_done_mtx);

	close(sock);
	pthread_exit((void*)0);
}

void* worker(void* arg)
{
	char address[ADDRESSSIZE];
	char port[PORTSIZE];
	char dirorfilename[PATHSIZE];
	char message[MSGSIZE];
	char path[PATHSIZE];
	char rcvbuffer[PATHSIZE];
	int type,delay;
	int current_file_size;
	int fd,sock,status,ret,nwrite,nread;

	/*There are still running managers */
	/*   OR 	*/
	/*All managers have exited but there are still items in the buffer */
	while(!managers_exited || buffer.count)
	{
		/*Consume from buffer*/
		ret = obtain(dirorfilename,address,port,&type,&delay);
		/*Make sure something has been consumed*/
		if(!ret)break;

		/* Call mkpath */
		snprintf(path,sizeof(path)-1,"%s/%s_%s/%s",dirname,address,port,dirorfilename);
		ret = mkpath(path,&fd,type);
		if(ret  == 1)
		{
			printf("Could not create <%s>\n",path);
			continue;
		}
		else if(ret == 2)
		{
			printf("Could not create open <%s>\n",path);
			continue;
		}
		else if(ret==-1)
		{
			continue;
		}

		/*A file has been obtained*/
		if(type)
		{
			/*Create connection with ContentServer*/
			ret = CreateClientSocket(&sock,&status,address,port);
			if(ret == 1)
			{
				fprintf(stderr, "Worker getaddrinfo():%s\n", gai_strerror(status));
				exit(1);
			}
			else if(ret == 2)
			{
				printf("Worker could not connect to ContentServer: <%s|%s>\n\n",address,port);
			}

			/*Send FETCH message to ContentServer */
			sprintf(message,"FETCH %s %d",dirorfilename,delay);
			if ( write_data(sock ,message,MSGSIZE) == -1)
			{
				perror("Worker writing FETCH message");
				exit(1);
			}

			/*Make sure file is still in ContentServer*/
			if( read_data(sock,rcvbuffer,MSGSIZE)==-1)
			{
				perror("Worker reading answer for FETCH message");
				exit(1);
			}
			if(!strcmp(rcvbuffer,"EXISTS"))
			{
				current_file_size = 0;
				/*Read from socket - Write all bytes read into the file*/
				memset(message,0,MSGSIZE);
				while( (nread = read(sock,message,MSGSIZE) )>0)
				{
					/*write_data() guarantees that nread bytes are written in the file*/
					nwrite=write_data(fd,message,nread);
					current_file_size+=nwrite;

					pthread_mutex_lock(&bytes_mtx);
					bytes_transferred+=nwrite;
					pthread_mutex_unlock(&bytes_mtx);
				}
				pthread_mutex_lock(&files_mtx);
				/*Check if realloc is needed for size_table*/
				if(files_transferred+1 == count)
				{
					long long* temp = realloc(size_table,2*count*sizeof(long long));
					if(!temp)
					{
						perror("MirrorServer realloc()");
						exit(1);
					}
					count = 2*count;
					size_table = temp;
				}
				/*Insert size of current file in size_table*/
				size_table[files_transferred] = current_file_size;
				/*Increase number of files transferred*/
				files_transferred++;
				pthread_mutex_unlock(&files_mtx);

			}
			else printf("Could not fetch <%s>\n",dirorfilename);
			close(fd);
			close(sock);
		}
		else
		{
			pthread_mutex_lock(&dirs_mtx);
			dirs_transferred++;
			pthread_mutex_unlock(&dirs_mtx);
		}
	}
	pthread_exit((void*)0);
}

int filter(char* contentpath,char* initpath)
{

	char* ptr;
	if(ptr = strstr(contentpath,initpath))
	{
		/*Check if contentpath ends with initpath*/
		if(strlen(ptr) == strlen(initpath))
			return 1;
		/*Check if a slash follows initpath in contentpath string*/
		else return (ptr[strlen(initpath)] == '/');
	}
	return 0;
}