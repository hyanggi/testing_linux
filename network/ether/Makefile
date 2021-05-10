CC = gcc
CPPFLAGS = -D_DEFAULT_SOURCE
CFLAGS = -std=c17 -pedantic -Wall -Wconversion -Wsign-conversion -Wsign-compare

all: sender receiver

sender: sender.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^

receiver: receiver.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^

clean:
	$(RM) sender receiver

.PHONY: all clean
