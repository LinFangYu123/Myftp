#include "ftp.h"
#include "logc.h"
#include "epoll_reactor.h"
#include <sys/epoll.h>
#include <string.h>



struct my_event my_ev[3000+1];

void acceptconn(void *arg);

void recvdata(void *arg);

void senddata(void *arg);

int epfd;

int main(){
	
	log_create("ftplog.txt");

	int sfd = Socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK,0);	
	//获得服务端套接字的描述符
	struct sockaddr_in serv;										//定义服务端的地址族
	serv.sin_addr.s_addr = htonl(INADDR_ANY);						//获得服务器的ip地址
	serv.sin_port = htons(8888);									//对外监听的端口
	serv.sin_family = AF_INET;										//设置IPv4协议簇
											
	int opt = 1;												
    setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));		//设置端口复用
	
	Bind(sfd,(struct sockaddr *)&serv,sizeof(serv));				//将地址族中的特定地址赋给socket

	Listen(sfd,128);												//开始监听客户端的连接请求
	
	epfd = epoll_create(3000+1);
	struct epoll_event all[3000+1];					

	bzero(&my_ev[3000],sizeof(my_ev[3000]));
	my_ev[3000].sock = serv;
	my_ev[3000].len = 0;

	if(eventset(&my_ev[3000],sfd,acceptconn,&my_ev[3000],NULL)!=0){
		perr_exit("eventset");
	}
	
	if(eventadd(epfd,EPOLLIN,&my_ev[3000])!=0){
		perr_exit("eventset");
	}
	printf("main aaa\n");
	while(1){
		int ret = epoll_wait(epfd,all,3000+1,-1);
		if(ret<0){
			perror("epoll_wait");
			if(errno == EINTR){
				continue;
			}
			break;
		}
		int i;
		for(i = 0;i<ret;i++){
			struct my_event *ev = (struct my_event *)all[i].data.ptr;
			if((ev->events & EPOLLIN))
            {
            	printf("EPOLLIN\n");
                ev->callback(ev->arg);
            }
            //如果监听的是写事件，并返回的是写事件
            if( (ev->events & EPOLLOUT))
            {
            	printf("EPOLLOUT\n");
                ev->callback(ev->arg);
            }
		}
	}
}

void acceptconn(void *arg){
	printf("acceptconn aaa\n");
	struct my_event *ev = (struct my_event *)arg;
 	struct sockaddr_in clie;										//定义客户端的地址族
	socklen_t clie_len = sizeof(clie);
	printf("acceptconn bbb\n");
	int cfd = Accept(ev->fd, (struct sockaddr *)&clie, &clie_len);	//接收客户端的连接请求
	int i;
	for(i = 0;i<3000;i++){
		if(my_ev[i].status == 0)
			break;
	}
	// 超出连接数上限
	if(i == 3000) {  							
		Close(cfd);
    	return ;
    }


	my_ev[i].sock = clie;
	my_ev[i].len = 0;
	
	//找到合适的节点之后，将其添加到监听树中，并监听读事件
	eventset(&my_ev[i], cfd, recvdata, &my_ev[i], NULL); 
    eventadd(epfd, EPOLLIN, &my_ev[i]);
    
    log_write("IP:%s PORT:%d CONNECT!\n",\
			inet_ntoa(clie.sin_addr),ntohs(clie.sin_port));
			printf("acceptconn eee\n");
    return ;
}

void recvdata(void *arg){
	
	struct my_event *ev = (struct my_event *)arg;

	ev->len = recv(ev->fd,ev->buf,sizeof(ev->buf),0);

	if(ev->len == -1){
		printf("recvdata ppp\n");
		log_write("recv err\n");
		return;
	}
	else if(ev->len == 0){
		printf("recvdata =0\n");
		log_write("IP:%s PORT:%d DISCONNECT!\n",\
			inet_ntoa(ev->sock.sin_addr),ntohs(ev->sock.sin_port));
		eventdel(epfd,ev);
		return;
	}

	eventset(ev, ev->fd, senddata, ev, NULL);   //设置该fd对应的回调函数为senddata    
    eventmod(epfd, EPOLLOUT, ev);
  
    return;
}

void senddata(void *arg){
	
	struct my_event *ev = (struct my_event *)arg;
	
	log_write("IP:%s PORT:%d %s",\
		inet_ntoa(ev->sock.sin_addr),ntohs(ev->sock.sin_port),ev->buf);
	
	if(strstr(ev->buf,"ls") == ev->buf){
		printf("ls\n");
		Dirlist(ev->fd);
	}
	else if(strstr(ev->buf,"cd") == ev->buf){
		printf("cd\n");
		char *tok;
		tok = strtok(ev->buf," ");
		tok = strtok(NULL," \n");
		Dircd(tok,ev->fd,ev->sock);
	}
	else if(strstr(ev->buf,"put") == ev->buf){
		printf("put\n");
		char *tok;
		tok = strtok(ev->buf," ");
		tok = strtok(NULL," \n");
		if(put(ev->fd,tok,ev->sock)!=0){
			eventdel(epfd,ev);
			return ;
		}
	}
	else if(strstr(ev->buf,"get") == ev->buf){
		printf("get\n");
		char *tok;
		tok = strtok(ev->buf," ");
		tok = strtok(NULL," \n");
		if(get(ev->fd,tok,ev->sock)!=0){
			eventdel(epfd,ev);
			return;
		}
	}
	else if(strstr(ev->buf,"pwd") == ev->buf){
		printf("pwd\n");
		memset(ev->buf,0,sizeof(ev->buf));
		getcwd(ev->buf,sizeof(ev->buf));
		Send(ev->fd,ev->buf,sizeof(ev->buf),0);
	}
	else if(strstr(ev->buf,"quit") == ev->buf){
		printf("quit\n");
		log_write("IP:%s PORT:%d DISCONNECT!\n",\
			inet_ntoa(ev->sock.sin_addr),ntohs(ev->sock.sin_port));
		eventdel(epfd,ev);
		return;
	}
	printf("pppp\n");
	eventset(ev, ev->fd, recvdata, ev,NULL); //找到合适的节点之后，将其添加到监听树中，并监听读事件
    eventmod(epfd, EPOLLIN, ev);
}