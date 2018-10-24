CC = g++
CFLAGS = -g -Wall -ansi -pedantic

all: myshell myls

myshell: myshell.c 
	$(CC) $(CFLAGS) myshell.c -o myshell

myls: myls.c 
	$(CC) $(CFLAGS) myls.c -o myls

clean: 
	$(RM) myshell myls