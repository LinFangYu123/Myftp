all:client.o server.o logc.o
	gcc -o server server.o logc.o
	gcc -o client client.o
client.o:
	gcc -c client.c -o client.o 
server.o:
	gcc -c server.c -o server.o 
logc.o:
	gcc -c logc.c -o logc.o
clean:
	rm client server
	rm *o
