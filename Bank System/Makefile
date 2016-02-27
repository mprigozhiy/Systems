COMPILER = gcc
CCFLAGS  = -Wall
all: client server

client: client.c 
	$(COMPILER) -o client client.c -lpthread

server: server.c
	$(COMPILER) -o server server.c -lpthread -lm

clean:
	rm -rf client server *.o
