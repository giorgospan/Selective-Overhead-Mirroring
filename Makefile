# source files
INIT_SOURCE	     = $(wildcard ./initiator/*.c)
MIRROR_SOURCE	   = $(wildcard ./mirror_server/*.c)
CONTENT_SOURCE	 = $(wildcard ./content_server/*.c)
COMMON_SOURCE    = $(wildcard ./common/*.c)

# object files
INIT_OBJS 	     = $(INIT_SOURCE:%.c=%.o)
MIRROR_OBJS 	   = $(MIRROR_SOURCE:%.c=%.o)
CONTENT_OBJS	   = $(CONTENT_SOURCE:%.c=%.o)
COMMON_OBJS      = $(COMMON_SOURCE:%.c=%.o)

# headers
HEADERS	       	= $(wildcard ./include/*.h)

# build directory
BUILDDIR = ./build

# build files
INIT_OUT	       = MirrorInitiator
MIRROR_OUT 	     = MirrorServer
CONTENT_OUT 	   = ContentServer

CC		           = gcc
CFLAGS		       = -I./include -g

############## Initiator ##############
init:$(BUILDDIR)/$(INIT_OUT)
$(BUILDDIR)/$(INIT_OUT): $(INIT_OBJS) $(COMMON_OBJS) | $(BUILDDIR)
	$(CC) -o $@ $^

$(INIT_OBJS):./initiator/%.o : ./initiator/%.c
		$(CC) $(CFLAGS) -c $< -o $@

############## MirrorServer ##############
mirror:$(BUILDDIR)/$(MIRROR_OUT)
$(BUILDDIR)/$(MIRROR_OUT):$(MIRROR_OBJS) $(COMMON_OBJS) | $(BUILDDIR)
	$(CC) -o $@  -pthread $^

$(MIRROR_OBJS):./mirror_server/%.o : ./mirror_server/%.c
			$(CC) $(CFLAGS) -c $< -o $@


############## ContentServer ##############
content:$(BUILDDIR)/$(CONTENT_OUT)
$(BUILDDIR)/$(CONTENT_OUT):$(CONTENT_OBJS) $(COMMON_OBJS) | $(BUILDDIR)
	$(CC) -o $@  -pthread $^

$(CONTENT_OBJS):./content_server/%.o : ./content_server/%.c
			$(CC) $(CFLAGS) -c $< -o $@


############## common object files ##############
$(COMMON_OBJS):./common/%.o : ./common/%.c
	$(CC) $(CFLAGS) -c $< -o $@


# this rule will get triggered in case build dir is not created yet
$(BUILDDIR):
		@mkdir $@

############### house cleaning ##################

.PHONY:clean_all
clean_all:clean_init clean_mirror clean_content

.PHONY:clean_init
clean_init:
	@rm -f $(INIT_OBJS) $(COMMON_OBJS) $(BUILDDIR)/$(INIT_OUT)

.PHONY:clean_mirror
clean_mirror:
	@rm -f $(MIRROR_OBJS) $(COMMON_OBJS) $(BUILDDIR)/$(MIRROR_OUT)

.PHONY:clean_content
clean_content:
	@rm -f $(CONTENT_OBJS) $(COMMON_OBJS) $(BUILDDIR)/$(CONTENT_OUT)


##################### accounting ######################
.PHONY:count
count:
	wc $(INIT_SOURCE) $(CONTENT_SOURCE) $(MIRROR_SOURCE) $(COMMON_SOURCE) $(HEADERS)
