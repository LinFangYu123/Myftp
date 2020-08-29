#include "ftp.h"
#include "logc.h"
#include <sys/epoll.h>


#define SIZE_BUF 1024
struct my_event{
	int fd;
	uint32_t events;
	void (*callback)(void *arg);
	void *arg;
	int status;
	struct sockaddr_in sock;
};

struct my_event my_ev[3000+1];

int main(){
	
	log_create("ftplog.txt");

	int sfd = socket(AF_INET,SOCK_STREAM,0);	
	//获得服务端套接字的描述符
	struct sockaddr_in serv;										//定义服务端的地址族
	serv.sin_addr.s_addr = htonl(INADDR_ANY);						//获得服务器的ip地址
	serv.sin_port = htons(8888);									//对外监听的端口
	serv.sin_family = AF_INET;										//设置IPv4协议簇
											
	int opt = 1;												
    setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));		//设置端口复用
	
	if(bind(sfd,(struct sockaddr *)&serv,sizeof(serv))!=0){			//将地址族中的特定地址赋给socket
		log_write("bind err\n");
		exit(1);
	}
	
	if(listen(sfd,128)!=0){											//开始监听客户端的连接请求
		log_write("listen err\n");
		exit(1);
	}
	//printf("Server run...waiting client connect\n");
	
	struct sockaddr_in clie;										//定义客户端的地址族
	socklen_t clie_len = sizeof(clie);	
	
	int epfd = epoll_create(3000);
	struct epoll_event all[3000];					
	struct epoll_event ev;
	

	bzero(&ev,sizeof(ev));
	bzero(&my_ev[3000],sizeof(my_ev[3000]));
	my_ev[3000].fd = sfd;
	my_ev[3000].sock = serv;
	my_ev[3000].status = 1;
	ev.events = my_ev[3000].events = EPOLLIN;
	ev.data.ptr = (void *)&my_ev[3000];
	epoll_ctl(epfd,EPOLL_CTL_ADD,sfd,&ev);
	
	while(1){
		int ret = epoll_wait(epfd,all,3000,-1);
		if(ret<0){
			perror("epoll_wait");
			if(errno == EINTR){
				continue;
			}
			break;
		}
		int i;
		for(i = 0;i<ret;i++){
			struct my_event *m = (struct my_event *)all[i].data.ptr;
			
			
			if(m->fd == sfd){
				int cfd = accept(sfd,(struct sockaddr *)&clie,&clie_len);	//接收客户端的连接请求
				if(cfd == -1){
					if(errno == EWOULDBLOCK||errno == EINTR){
						continue;
					}
					else{
						log_write("accept err");
						break;
					}
				}
				int i;
				for(i = 0;i<3000;i++ ){
					if(my_ev[i].status == 0)
						break;
				}
				struct epoll_event evv;
				bzero(&ev,sizeof(evv));
				bzero(&my_ev[i],sizeof(my_ev[i]));
				my_ev[i].fd = cfd;
				my_ev[i].sock = clie;
				my_ev[i].status = 1;
				evv.events = my_ev[i].events = EPOLLIN;
				evv.data.ptr = (void *)&my_ev[i];
				epoll_ctl(epfd,EPOLL_CTL_ADD,cfd,&evv);
				
				log_write("IP:%s PORT:%d CONNECT!\n",\
					inet_ntoa(clie.sin_addr),ntohs(clie.sin_port));
			}
			else{
				char buf[SIZE_BUF];
				memset(buf,0,sizeof(buf));
				int len = recv(m->fd,buf,sizeof(buf),0);
				if(len < -1){
					log_write("read err\n");
					continue;
				}
				else if(len == 0){
					log_write("IP:%s PORT:%d DISCONNECT!\n",\
						inet_ntoa(m->sock.sin_addr),ntohs(m->sock.sin_port));
					close(m->fd);
					epoll_ctl(epfd,EPOLL_CTL_DEL,m->fd,NULL);
					m->status = 0;
					all[i].data.ptr = NULL;
				}
				else{
					log_write("IP:%s PORT:%d %s",\
					inet_ntoa(m->sock.sin_addr),ntohs(m->sock.sin_port),buf);
					if(strstr(buf,"ls") == buf){
						Dirlist(m->fd);
					}
					else if(strstr(buf,"cd") == buf){
						char *tok;
						tok = strtok(buf," ");
						tok = strtok(NULL," \n");
						Dircd(tok,m->fd,m->sock);
					}
					else if(strstr(buf,"put") == buf){
						char *tok;
						tok = strtok(buf," ");
						tok = strtok(NULL," \n");
						if(put(m->fd,tok,m->sock)!=0){
							epoll_ctl(epfd,EPOLL_CTL_DEL,m->fd,NULL);
							close(m->fd);
							m->status = 0;
							all[i].data.ptr = NULL;
						}
					}
					else if(strstr(buf,"get") == buf){
						char *tok;
						tok = strtok(buf," ");
						tok = strtok(NULL," \n");
						if(get(m->fd,tok,m->sock)!=0){
							epoll_ctl(epfd,EPOLL_CTL_DEL,m->fd,NULL);
							close(m->fd);
							m->status = 0;
							all[i].data.ptr = NULL;
						}
					}
					else if(strstr(buf,"pwd") == buf){
						memset(buf,0,sizeof(buf));
						getcwd(buf,sizeof(buf));
						send(m->fd,buf,sizeof(buf),0);
					}
					else if(strstr(buf,"quit") == buf){
						log_write("IP:%s PORT:%d DISCONNECT!\n",\
							inet_ntoa(m->sock.sin_addr),ntohs(m->sock.sin_port));
						epoll_ctl(epfd,EPOLL_CTL_DEL,m->fd,NULL);
						close(m->fd);
						m->status = 0;
						all[i].data.ptr = NULL;
					}
				}	
			}
		}
	}
}
