CC = gcc
CPPFLAGS = -D_DEFAULT_SOURCE
CFLAGS = -std=c17 -pedantic -Wall -Wsign-compare

all: transmitter receiver

transmitter: transmitter.c common.c common.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $(filter %.c,$^)

receiver: receiver.c common.c common.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $(filter %.c,$^)

clean:
	$(RM) transmitter receiver

.PHONY: all clean
