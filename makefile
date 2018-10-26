INIT_OBJS 		= InitiatorMain.o InitiatorFunctions.o MiscFunctions.o
MIRROR_OBJS 	= MirrorServerMain.o MirrorServerFunctions.o ServerList.o AuxList.o MiscFunctions.o Buffer.o
CONTENT_OBJS	= ContentServerMain.o ContentServerFunctions.o ServerList.o AuxList.o MiscFunctions.o 

INIT_SOURCE		= InitiatorMain.c InitiatorFunctions.c MiscFunctions.c 
MIRROR_SOURCE	= MirrorServerMain.c MirrorServerFunctions.c ServerList.c AuxList.c MiscFunctions.c Buffer.c
CONTENT_SOURCE	= ContentServerMain.c ContentServerFunctions.c ServerList.c AuxList.c MiscFunctions.c

HEADER  = InitiatorFunctions.h MirrorServerFunctions.h ContentServerFunctions.h MiscHeader.h AuxList.h ServerList.h Buffer.h

INIT_OUT	= MirrorInitiator 
MIRROR_OUT 	= MirrorServer
CONTENT_OUT = ContentServer

CC	= gcc
FLAGS   = -g -c 

###################  For my testing  ##################################

# run_init: $(INIT_OUT)
	# ./$(INIT_OUT) -n linux03.di.uoa.gr -s linux02.di.uoa.gr:9002:Content02:25 -p 9002
	
	
# run_mirror:$(MIRROR_OUT)
	# ./$(MIRROR_OUT) -p 9002 -m Finals -w 10
	
# run_content:$(CONTENT_OUT)
	# ./$(CONTENT_OUT) -d Content02 -p 9002
	
	
#####################################################################


########## Executables ##############

$(INIT_OUT): clean_init $(INIT_OBJS)
	$(CC) -o $@ -g $(INIT_OBJS) 

$(MIRROR_OUT):clean_mirror $(MIRROR_OBJS)
	$(CC) -o $@ -g -pthread $(MIRROR_OBJS) 
	
$(CONTENT_OUT):clean_content $(CONTENT_OBJS)
	$(CC)  -o $@ -g -pthread $(CONTENT_OBJS) 


############## Initiator Objs ##############

InitiatorMain.o:InitiatorMain.c
	$(CC) $(FLAGS) InitiatorMain.c

InitiatorFunctions.o:InitiatorFunctions.c
	$(CC) $(FLAGS) InitiatorFunctions.c
	
############## MirrorServer Objs ##############

MirrorServerMain.o:MirrorServerMain.c
	$(CC) $(FLAGS) MirrorServerMain.c

MirrorServerFunctions.o:MirrorServerFunctions.c
	$(CC) $(FLAGS) MirrorServerFunctions.c
	

############## ContentServer Objs ##############

ContentServerMain.o:ContentServerMain.c
	$(CC) $(FLAGS) ContentServerMain.c

ContentServerFunctions.o:ContentServerFunctions.c
	$(CC) $(FLAGS) ContentServerFunctions.c


############ Auxilary Objs ##############
AuxList.o:AuxList.c
	$(CC) $(FLAGS) AuxList.c

ServerList.o:ServerList.c
	$(CC) $(FLAGS) ServerList.c
	
Buffer.o:Buffer.c
	$(CC) $(FLAGS) Buffer.c
	
############### house cleaning ##################
clean_mirror:
	rm -f $(MIRROR_OBJS) $(MIRROR_OUT)
	
clean_content:
	rm -f $(CONTENT_OBJS) $(CONTENT_OUT)
	
clean_init:
	rm -f $(INIT_OBJS) $(INIT_OUT)
	
##################### accounting ######################
count:
	wc $(INIT_SOURCE) $(CONTENT_SOURCE) $(MIRROR_SOURCE) $(HEADER)
