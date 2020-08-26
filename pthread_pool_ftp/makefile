all:client.o server.o logc.o ThreadPool.o Socket.o ftp.o
	gcc -o server server.o logc.o ThreadPool.o Socket.o ftp.o -lpthread
	gcc -o client client.o
client.o:
	gcc -c client.c -o client.o 
ftp.o:
	gcc -c ftp.c -o ftp.o 
Socket.o:
	gcc -c Socket.c -o Socket.o 
ThreadPool.o:
	gcc -c ThreadPool.c -o ThreadPool.o 
server.o:
	gcc -c server.c -o server.o 
logc.o:
	gcc -c logc.c -o logc.o
clean:
	rm client server
	rm *o
