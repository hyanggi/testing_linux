CC = gcc
CSTD = -std=c17
WARN = -pedantic -Wall -Wconversion -Wsign-conversion -Wsign-compare
LDLIBS =

src := $(wildcard *.c)
bin = $(src:%.c=bin/%)

all: $(bin)

# --------- General rules -----------

bin/%: %.c
	$(CC) $(WARN) $(CSTD) $^ $(LDLIBS) -o $@

# --------- Directories -------------

bin:
	mkdir -p $@

$(bin): | bin

# ----------------------------------

clean:
	$(RM) -r bin

.PHONY: all clean
