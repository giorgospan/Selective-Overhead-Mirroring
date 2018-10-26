#ifndef FUNCTIONS_H
#define FUNCTIONS_H

extern pthread_t* manager_ids;
extern long long bytes_transferred; 
extern int files_transferred;
extern int dirs_transferred;

/*Gonna use count for reallocing size_table*/
extern int count;

/*File size might be up to a couple of Megabytes*/
extern long long* size_table;

/* Number of devices done */
extern int numDevicesDone;

/*TRUE iff all MirrorManagerThreads have exited*/
extern int managers_exited ;

/* Directory under which all mirrorred data is stored*/
extern char* dirname;

/* List structure with devices requested [requestlist for each device]*/
extern struct ServerList* server_list;


/*Communication with initiator*/
/*Creates mirror_managers and workers*/
void communication(int);


/*Function for mirror_manager threads (producers)*/
void* mirror_manager(void*);

/*Function for worker threads (consumers)*/
void* worker(void*);

/*Filters results sent by ContentServer*/
int filter(char*,char*);


/*Creates all directories in the given path*/
/*Creates and opens file if path ends with a file*/
int mkpath(const char*,int*,int);

/*Calculates variance*/
double find_variance(double,int);

#endif