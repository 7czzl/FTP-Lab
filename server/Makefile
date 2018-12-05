CFLAGS = -Wall

server: server.o
	gcc $(CFLAGS) server.o -o server
server.o: common.h MyString.h handler.h server.c
	gcc -c $(CFLAGS) server.c
clean:
	rm -f server.o server
