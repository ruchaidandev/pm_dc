CC = c99 
CFLAGS = -Wall 
LDFLAGS = 

all: server client

server: server.c

client: client.c

clean:
	rm -f server *.o
	rm -f client *.o 
 
.PHONY: clean