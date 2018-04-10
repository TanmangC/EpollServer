.SUFFIXES:.c .o
CC=gcc
SRCCLIENT=epoll_client.c\
	common.c
SRCSERVER=epoll_server.c\
	common.c
OBJCLIENT=$(SRCCLIENT:.c=.o)
OBJSERVER=$(SRCSERVER:.c=.o)
CLIENT=EpollClient
SERVER=EpollServer

start:$(OBJCLIENT) $(OBJSERVER)
	$(CC) -o $(CLIENT) $(OBJCLIENT)
	$(CC) -o $(SERVER) $(OBJSERVER)
	@echo "-------ok-----------"
.c.o:
	$(CC) -Wall -g -o $@ -c $<
clean:
	rm -f $(OBJCLIENT)
	rm -f $(CLIENT)
	rm -f $(OBJSERVER)
	rm -f $(SERVER)
