CFLAGS = -Wall

client: client.o
	gcc $(CFLAGS) client.o -o client
client.o: common.h MyString.h handler.h client.c
	gcc -c $(CFLAGS) client.c
clean:
	rm -f client.o client
