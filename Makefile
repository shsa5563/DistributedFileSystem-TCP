#Makefile
CC = gcc
INCLUDE = /usr/lib
LIBS = -lpthread -lrt -lssl -lcrypto 
OBJS = 
HEADERS= .
all:  dfs dfc

dfc:
	$(CC) -g -o dfc  util.c DFC.c -I$(HEADERS) $(CFLAGS) $(LIBS) 

dfs:
	$(CC) -o dfs util.c DFS.c -I$(HEADERS) $(CFLAGS) $(LIBS)
clean:
	rm -f  dfc dfs 

