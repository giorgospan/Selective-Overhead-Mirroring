#include <stdio.h>
#include <string.h> /* For strerror */
#include <stdlib.h> /* For exit */
#include <unistd.h> /* For close,read,write */
#include <pthread.h> /* pthread functions */

#include <fcntl.h> /* For having access to flags  */

/*for readdir opendir*/
#include <sys/types.h>
#include <dirent.h>

#include "ContentServerFunctions.h"
#include "MiscHeader.h"
#include "AuxList.h"

void content(const char *name)
{
	char path[1024];
	int len;
    DIR* dir;
    struct dirent* entry;
	int type;

	/* Open Directory given */
    if (!(dir = opendir(name)))
	{
		perror("ContentServer opening directory");
        exit(1);
	}

    while (entry = readdir(dir))
	{
		len = snprintf(path, sizeof(path)-1, "%s/%s", name, entry->d_name);
		path[len] = 0;
		type=1;
        if (entry->d_type == DT_DIR)
		{
			type=0;
            if (!strcmp(entry->d_name, ".") ||  !strcmp(entry->d_name, "..") )
                continue;
            content(path);
        }

		/*Insert entry at list */
		ListInsert(contentlist,path,type);
    }
    closedir(dir);
}


void* thread_f(void* argument)
{
	char operation[MSGSIZE];
	char termsignal[MSGSIZE];
	char rcvbuffer[MSGSIZE];
	struct argument* arg = (struct argument*)argument;
	int sock = arg->sock;
	strcpy(rcvbuffer,arg->rcvbuffer);

	// printf("%d-th Thread[%u] with socket:%d\n",arg->id,(unsigned)pthread_self(),sock);

	/*Check operation*/
	sscanf(rcvbuffer,"%s",operation);
	if(!strcmp(operation,"LIST"))
	{
		/*call list for each requested dirorfile*/
		while(1)
		{
			list(sock);

			/*Read termination signal written by mirror-manager thread*/
			if( read_data(sock,termsignal,MSGSIZE)==-1)
			{
				perror("ContentServer reading termisignal from mirror-manager");
				exit(1);
			}
			if(!strcmp(termsignal,"NO END"))
			{
				char newbuffer[MSGSIZE];
				/*Read LIST <ID> <delay>*/
				if( read_data(sock,newbuffer,MSGSIZE)==-1)
				{
					perror("ContentServer reading new LIST <ID> <delay>");
					exit(1);
				}
			}
			else break;
		}
	}
	else
	{
		fetch(sock,rcvbuffer);
	}
	close(sock);
	free(argument);
	pthread_exit((void*)0);
}

void list(int newsock)
{
	char sendbuffer[MSGSIZE];
	memset(sendbuffer,0,MSGSIZE);
	struct ListNode* current;
	int nwrite;
	current = contentlist -> start;
	while(current)
	{
		/*Send every available entity in from your contentlist*/
		sprintf(sendbuffer,"%s %d",current->entity,current->type);
		if( write_data(newsock,sendbuffer,MSGSIZE)==-1)
		{
			perror("ContentServer write() during LIST");
			exit(1);
		}
		current = current -> next;
	}
	/*Tell MirrorServer that you've sent everything in your contentlist*/
	if( write_data(newsock,"END",MSGSIZE)==-1)
	{
		perror("ContentServer write() during LIST");
		exit(1);
	}
}

void fetch(int sock,char* rcvbuffer)
{
	int delay,fd;
	int nread,newsock;
	char databuffer[MSGSIZE];
	char path_to_file[PATHSIZE];
	char fetchrequest[MSGSIZE];

	/*Use local variables*/
	newsock=sock;
	strcpy(fetchrequest,rcvbuffer);

	sscanf(fetchrequest,"%*s %s %d",path_to_file,&delay);

	/*Inform Worker thread that the requested file is still here*/
	if ((fd = open (path_to_file,O_RDONLY))== -1)
	{
		if( write_data(newsock,"NOT EXIST",MSGSIZE)==-1)
		{
			perror("ContentServer write() NOT EXISTS during FETCH");
			exit(1);
		}
	}
	else
	{
		if( write_data(newsock,"EXISTS",MSGSIZE)==-1)
		{
			perror("ContentServer write() EXISTS during FETCH");
			exit(1);
		}
	}

	/*Delay for a couple of seconds*/
	sleep(delay);

	/*Read from file - write all bytes read to the socket*/
	while( (nread=read(fd,databuffer,MSGSIZE)) >0)
	{
		/*write_data() guarantees that nread bytes are written in the socket*/
		if( write_data(newsock,databuffer,nread)==-1)
		{
			perror("ContentServer write() FILEDATA during FETCH");
			exit(1);
		}
	}
	close(fd);
}
