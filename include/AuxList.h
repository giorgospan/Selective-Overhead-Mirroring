#ifndef AUX_LIST_H
#define AUX_LIST_H

struct ListNode
{
	char* entity;
	int type;  /*dirorfilename OR delay time*/
	struct ListNode* next;
};

struct List
{
	struct ListNode* start;
	int counter;
};

void ListCreate(struct List**);

void ListInsert(struct List*,char*,int);


void ListPrint(struct List*);

void ListDestroy(struct List*);


#endif