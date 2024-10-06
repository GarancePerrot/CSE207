CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS =

all: server client

server: server.o
	$(CC) $(LDFLAGS) -o server server.o

client: client.o
	$(CC) $(LDFLAGS) -o client client.o

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

client.o: client.c
	$(CC) $(CFLAGS) -c client.c

clean:
	rm -f server client *.o