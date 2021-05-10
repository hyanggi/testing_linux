CC = gcc
CPPFLAGS = -D_DEFAULT_SOURCE
CFLAGS = -std=c17 -pedantic -Wall -Wconversion -Wsign-conversion -Wsign-compare

all: client server

client: client.c common.c common.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $(filter %.c,$^)

server: server.c common.c common.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $(filter %.c,$^)

clean:
	$(RM) client server

.PHONY: all clean
