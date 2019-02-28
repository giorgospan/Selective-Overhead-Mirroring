#include <stdio.h>
#include <string.h>    /* For strerror strcpy*/
#include <stdlib.h>    /* For exit  malloc  free   */
#include <pthread.h>   /* For threads  */


#include "MiscHeader.h"
#include "MirrorServerFunctions.h"
#include "Buffer.h"


int place(char* dirorfilename,char* address,char* port,int type,int delay)
{
	/*Lock Buffer mutex*/
	pthread_mutex_lock(&buffer_mtx);
	/*Suspend yourself while buffer is full*/
	while(buffer.count >= BUFFERSIZE)
	{
		pthread_cond_wait(&cond_nonfull,& buffer_mtx );
	}

	/*Move to next cell*/
	buffer.end = (buffer.end + 1) % BUFFERSIZE ;

	/*Copy data to the buffer cell */
	buffer.data[buffer.end].address = malloc((strlen(address)+1)*sizeof(char)) ;
	strcpy(buffer.data[buffer.end].address,address);

	buffer.data[buffer.end].port = malloc((strlen(port)+1)*sizeof(char)) ;
	strcpy(buffer.data[buffer.end].port,port);

	buffer.data[buffer.end].dirorfilename = malloc((strlen(dirorfilename)+1)*sizeof(char)) ;
	strcpy(buffer.data[buffer.end].dirorfilename,dirorfilename);

	buffer.data[buffer.end].type = type;
	buffer.data[buffer.end].delay = delay;

	/*Increase counter of elements in buffer*/
	buffer.count ++;

	/*Wake up a suspended worker*/
	pthread_cond_signal (& cond_nonempty );

	/*Unlock Buffer mutex*/
	pthread_mutex_unlock(&buffer_mtx);
}

int obtain(char* dirorfilename,char* address,char* port,int* type,int* delay)
{
	/*Lock Buffer mutex*/
	pthread_mutex_lock(&buffer_mtx);

	/*Suspend yourself while buffer is empty*/
	/* AND */
	/*Managers are still active*/
	while(buffer.count <= 0 && !managers_exited)
	{
		pthread_cond_wait(&cond_nonempty,&buffer_mtx );
	}


	/*Make sure buffer cell is not empty*/
	/*Copy data from the buffer cell (free() pointers and set them to NULL)*/
	if(buffer.data[buffer.start].address)
	{
		strcpy(address,buffer.data[buffer.start].address);
		free(buffer.data[buffer.start].address);
		buffer.data[buffer.start].address = NULL;

		strcpy(port,buffer.data[buffer.start].port);
		free(buffer.data[buffer.start].port);
		buffer.data[buffer.start].port = NULL;


		strcpy(dirorfilename,buffer.data[buffer.start].dirorfilename);
		free(buffer.data[buffer.start].dirorfilename);
		buffer.data[buffer.start].dirorfilename = NULL;

		*type = buffer.data[buffer.start].type;
		*delay = buffer.data[buffer.start].delay;

		buffer.start = (buffer.start + 1) % BUFFERSIZE ;
		buffer.count --;

		/*Wake up a sleeping manager*/
		pthread_cond_signal (& cond_nonfull );

		/*Unlock Buffer mutex*/
		pthread_mutex_unlock(&buffer_mtx);
		return 1;
	}
	else
	{
		/*Unlock Buffer mutex*/
		pthread_mutex_unlock(&buffer_mtx);
		return 0;
	}
}





