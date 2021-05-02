src = $(wildcard ./*.c )
dir = $(notdir $(src))
obj = $(patsubst %.c,%.o,$(dir))
exeFile=$(patsubst %.c,%,$(dir))
all:client server $(obj)

server:server.o ftp.o logc.o epoll_reactor.o Socket.o
	gcc -g -o server server.o ftp.o logc.o epoll_reactor.o Socket.o
client:client.o
	gcc -g -o client client.o
%.o:%.c
	gcc -c $^ -o $@

clean:
	-@echo $(src)
	-@echo $(dir)
	-@echo $(obj)
	rm *.o a.out $(exeFile) ftplog.txt