CC=gcc
CFLAGS=-Wall -g -pthread
SRCS=filesync.c inotify_utils.c event_queue.c file_queue.c
OBJS=filesync.o inotify_utils.o event_queue.o file_queue.o

.c.o:
	$(CC) $(CFLAGS) -c $<

all: filesync

filesync: $(OBJS)
	$(CC) $(CFLAGS) inotify_utils.o filesync.o event_queue.o file_queue.o -o filesync

clean:
	rm -f $(OBJS) *.bak filesync
