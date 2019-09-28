CC = 
CFLAGS = -g -Wall 
LIBS = -lrt -pthread 

all: server client

server: server.c
	$(CC) -o $@ $^

client: client.c
	$(CC) -o $@ $^

clean:
	rm -f server *.o
	rm -f client *.o 
 
.PHONY: clean