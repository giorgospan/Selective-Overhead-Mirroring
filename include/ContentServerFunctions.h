#ifndef CONTENT_FUNCTIONS_H
#define CONTENT_FUNCTIONS_H

#define MORE_THREADS 2
#include "MiscHeader.h"

/*Contains everything available in this ContentServer*/
extern struct List* contentlist;

/*Argument for thread function*/
struct argument
{
	char rcvbuffer[MSGSIZE];
	int sock;
	int id;
};

/*Inserts in contentlist every available entity*/
void content(const char *);

/*Thread function [calls either list or fetch]*/
void* thread_f(void*);

/*Serves LIST requests*/
void list(int);

/*Serves FETCH requests*/
void fetch(int,char*);

#endif