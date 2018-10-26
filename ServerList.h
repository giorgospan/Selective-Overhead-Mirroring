#ifndef SERVER_LIST_H
#define SERVER_LIST_H


struct ServerListNode
{
	char* address;
	char* port;
	struct List* request_list;
	struct ServerListNode* next;
};

struct ServerList
{
	struct ServerListNode* start;
	int counter;
};

void ServerListCreate(struct ServerList**);

void ServerListInsert(struct ServerList*,char*);

/*Just for debugging purposes*/
void ServerListPrintInner(struct ServerListNode*);
void ServerListPrint(struct ServerList*);

void ServerListDestroy(struct ServerList*);


#endif