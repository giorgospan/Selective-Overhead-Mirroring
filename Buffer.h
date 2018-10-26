#ifndef BUFFER_H
#define BUFFER_H

/*mutex for buffer*/
extern pthread_mutex_t buffer_mtx ;

/*mutex for bytes_transferred*/
extern pthread_mutex_t bytes_mtx ;

/*mutex for files_transferred*/
extern pthread_mutex_t files_mtx ;

/*mutex for dirs_transferred*/
extern pthread_mutex_t dirs_mtx ;

/*mutex for numDevicesDone*/
extern pthread_mutex_t devices_done_mtx ;

/*mutex for size_table*/
extern pthread_mutex_t size_table_mtx ;

/*Anchor for worker threads*/
extern pthread_cond_t cond_nonempty ;

/*Anchor for mirror-manager threads*/
extern pthread_cond_t cond_nonfull ;


struct buffer_entry
{
	char* address;
	char* port;
	char* dirorfilename;
	int type;
	int delay;
};

typedef struct buffer
{
	struct buffer_entry* data;
	int count;
	int start;
	int end;
	
}buffer_t;


/*Our bounded buffer*/
extern buffer_t buffer;


int place(char*,char*,char*,int,int);

int obtain(char* ,char* ,char* ,int* ,int*);

#endif