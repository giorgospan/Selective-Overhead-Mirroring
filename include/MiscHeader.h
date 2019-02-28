#ifndef MISC_HEADER_H
#define MISC_HEADER_H

/*My perror for thread functions*/
#define my_perror(s,e) fprintf(stderr,"%s: %s\n",s,strerror(e))

#define BACKLOG 10
#define MSGSIZE 200
#define NAMESIZE 100
#define PORTSIZE 10
#define PATHSIZE 200
#define ADDRESSSIZE 100
#define BUFFERSIZE 10

#define DIRPERMS 0755/* permissions for directories*/
#define FILEPERMS 0777/*permissions for files*/

/*Reads until MSGSIZE bytes have been received*/
int read_data(int,char*,size_t);

/*Guarantees that a given number of bytes have been sent*/
int write_data(int,char*,size_t);

/*Creates a socket and starts listening to it*/
int CreateServerSocket(int*,int*,char*);

/*Creates a socket and initiates connection with the given server*/
int CreateClientSocket(int*,int*,char* ,char*);

/*Removes trailing slash if needed*/
void remove_slash(char*);

#endif