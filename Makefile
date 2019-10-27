CC = c99
CFLAGS =  
LIBS = -pthread -lrt

all: server client

server: server.c
	$(CC) -o $@ $^ $(LIBS) $(CFLAGS) 

client: client.c
	$(CC) -o $@ $^ $(LIBS) $(CFLAGS)

clean:
	rm -f server *.o
	rm -f client *.o 
 
.PHONY: clean