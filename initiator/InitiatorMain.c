#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*for getaddrinfo() and sockets*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


#include "InitiatorFunctions.h"
#include "MiscHeader.h"


int main(int argc,char* argv[])
{

	int i,sock,status,ret;
	int arg1,arg2,arg3 ;
	char* serveraddress;
	char* port;
	char* list;
	arg1 = arg2 = arg3 = 0;

	for(i=1;i<argc;++i)
	{
		if(!strcmp(argv[i],"-n"))
		{
			serveraddress = malloc((strlen(argv[i+1])+1)* sizeof(char)   );
			strcpy(serveraddress,argv[i+1]);
			arg1 = 1;
		}
		else if(!strcmp(argv[i],"-p"))
		{
			port = malloc((strlen(argv[i+1])+1)* sizeof(char)   );
			strcpy(port,argv[i+1]);
			arg2 = 1;
		}
		else if(!strcmp(argv[i],"-s"))
		{
			list = malloc((strlen(argv[i+1])+1)* sizeof(char)   );
			strcpy(list,argv[i+1]);
			arg3 = 1;
		}

	}
	/*Sanity check*/
	if(!arg1 || !arg2 || !arg3)
	{
		fprintf(stderr,"Usage: ./MirrorInitiator -n <MirrorServerAddress> -p <MirrorServerPort>"
"-s <ContentServerAddress1:ContentServerPort1:dirorfile1:delay1,"
"ContentServerAddress2:ContentServerPort2:dirorfile2:delay2, ...>\n\n");
		exit(1);
	}


	printf("Arguments\n");
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	printf("Address	                    :%s\n",serveraddress);
	printf("Port	                      :%s\n",port);
	printf("Directories to be mirrored	:%s\n\n\n",list);
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

	/*Create connection with MirrorServer*/
	ret = CreateClientSocket(&sock,&status,serveraddress,port);
	if(ret == 1)
	{
		fprintf(stderr, "Initiator getaddrinfo():%s\n", gai_strerror(status));
		exit(1);
	}
	else if(ret == 2)
	{
		perror("Initiator could not find an optimal socket");
		exit(1);
	}

	/*Start communication Initiator <-----> MirrorServer */
	communication(sock,list);

	free(serveraddress);
	free(list);
	free(port);
	close(sock);

	printf("Initiator is exiting....\n");
	exit(0);
}