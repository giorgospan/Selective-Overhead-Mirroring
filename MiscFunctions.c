#include <stdio.h> 
#include <string.h>
#include <stdlib.h>

/*For sockets*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "MiscHeader.h"

int read_data(int sock,char* buffer,size_t size)
{
	int nread=0;
	/*Clear buffer before start storing bytes in it*/
	memset(buffer,0,sizeof(buffer));
	while ( nread < size ) 
		if ( nread < ( nread+= read (sock , &buffer[nread], size- nread)))
		{
			return -1;
		}
	return nread;
}


int write_data(int sock,char *buffer,size_t size) 
{
	int nwrite=0;
	while ( nwrite < size ) 
		if ( nwrite < ( nwrite+= write(sock , &buffer[nwrite], size - nwrite)))
		{
			return -1;
		}
	return nwrite;
}

int CreateClientSocket(int* sock,int* status,char* address,char* port)
{
	struct addrinfo hints;
	struct addrinfo* result,*p;  /* will point to the result of getaddrinfo() */
	
	memset(&hints, 0, sizeof hints);	 // make sure the struct is empty
    hints.ai_family = PF_INET; 			// IPv4 Protocol
    hints.ai_socktype = SOCK_STREAM;	// TCP Protocol
	
	
	/* Find ContentServer address */
	if ((*status = getaddrinfo(address,port,&hints,&result)) != 0) 
	{
		return 1;
	}
	/* connect to the first available socket*/
    for(p = result; p != NULL; p = p->ai_next) 
	{
        if ((*sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
            continue;
        if (connect(*sock, p->ai_addr, p->ai_addrlen) == -1) 
		{
            close(*sock);
            continue;
        }
        break;
    }
    if (!p) 
	{
        // perror(" mirror-manager could not find an optimal socket");
		// exit(1);
		return 2;
    }
	
	/*  free the linked-list created by getaddrinfo() */
	freeaddrinfo(result);
}

int CreateServerSocket(int* sock,int* status,char* port)
{
	int yes=1;
	struct addrinfo hints;
	struct addrinfo	*result,*p;  /* will point to the result of getaddrinfo() */
	struct sockaddr_storage initiator_address;
	
	memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

	if ((*status = getaddrinfo(NULL,port,&hints,&result)) != 0) return 1;
	
	
	
	/* bind to the optimal socket*/
    for(p = result; p != NULL; p = p->ai_next) 
	{
        if ((*sock = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            continue;
        }

        if (setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1)
			return 2;

        if (bind(*sock, p->ai_addr, p->ai_addrlen) == -1) {
            close(*sock);
            continue;
        }

        break;
    }

    if (!p)  
		return 3;
	
	/*Free the linked-list created by getaddrinfo()*/
	freeaddrinfo(result); 
	
	/*Start listening to this socket */
	if (listen(*sock, BACKLOG) == -1) 
		return 4;
	
	return 0;
}


void remove_slash(char* path)
{
	if(path[strlen(path)-1] == '/')
		path[strlen(path)-1] = '\0';
}





