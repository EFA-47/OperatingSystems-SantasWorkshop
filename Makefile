CC = gcc
CFLAGS = -g -Wall -std=c99 -m64

all:
	$(CC) $(CFLAGS) -o project_2 project_2.c

clean:
	rm -f project_2