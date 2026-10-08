CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -D_POSIX_C_SOURCE=200809L

PARENT_SRCS = parent.c parent_input.c pipe_io.c
CHILD_SRCS  = child.c sum.c pipe_io.c

all: parent child

parent: $(PARENT_SRCS) status.h parent_input.h pipe_io.h
	$(CC) $(CFLAGS) -o parent $(PARENT_SRCS)

child: $(CHILD_SRCS) status.h sum.h pipe_io.h
	$(CC) $(CFLAGS) -o child $(CHILD_SRCS)

clean:
	rm -f parent child *.o

.PHONY: all clean
