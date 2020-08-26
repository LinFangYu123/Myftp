#include "ftp.h"
#include "logc.h"
#include "Socket.h"
#include "ThreadPool.h"
#define SIZE_BUF 1024

void *task(void *arg);
struct socksmg{
	struct sockaddr_in sock;
	int fd;
};
int main(){
	
	log_create("ftplog.txt");
	pthread_pool *pool = NULL;
	pool = (pthread_pool *)malloc(sizeof(pthread_pool));
	int ret = create_thread_pool(pool,5,3,4);
	if(ret != 0){
		thread_pool_destroy(pool);
		printf("%d\n",ret);
		return -1;
	}
	
	int sfd = initTcpSocket(8888,NULL);	
	
	//printf("Server run...waiting client connect\n");

	while(1){
		struct sockaddr_in clie ;										//定义客户端的地址族
		socklen_t clie_len = sizeof(struct sockaddr_in);	
		int cfd = accept(sfd,(struct sockaddr *)&clie,&clie_len);	//接收客户端的连接请求
		if(cfd == -1){
			if(errno == EINTR){
				continue;
			}
			else{
				log_write("accept err");
				break;
			}
		}
		while(get_current_thread_task_num(pool) >= get_max_thread_num(pool)){
			usleep(500);
		}
		log_write("IP：%s,PORT:%d CONNECT!\n",\
			inet_ntoa(clie.sin_addr),ntohs(clie.sin_port));
		struct socksmg *msg = (struct socksmg *)malloc(sizeof(struct socksmg));
		msg->fd = cfd;
		msg->sock = clie;
		thread_pool_add_task(pool,task,(void *)msg);
	}
}

void *task(void *arg){
	struct socksmg *msg = (struct socksmg *)arg;
	int cfd = msg->fd;
	struct sockaddr_in clie = msg->sock;
	while(1){
		char buf[SIZE_BUF];
		memset(buf,0,sizeof(buf));
		int len = recv(cfd,buf,sizeof(buf),0);
		if(len < -1){
			log_write("read err\n");
			continue;
		}
		else if(len == 0){
			log_write("IP:%s PORT:%d DISCONNECT!\n",\
				inet_ntoa(clie.sin_addr),ntohs(clie.sin_port));
			close(cfd);
			return NULL;
		}
		else{
			log_write("IP:%s PORT:%d %s",\
				inet_ntoa(clie.sin_addr),ntohs(clie.sin_port),buf);
			if(strstr(buf,"ls") == buf){
				Dirlist(cfd);
			}
			else if(strstr(buf,"cd") == buf){
				char *tok;
				tok = strtok(buf," ");
				tok = strtok(NULL," \n");
				Dircd(tok,cfd,clie);
			}
			else if(strstr(buf,"pwd") == buf){
				memset(buf,0,sizeof(buf));
				getcwd(buf,sizeof(buf));
				send(cfd,buf,sizeof(buf),0);
			}
			else if(strstr(buf,"put") == buf){
				char *tok;
				tok = strtok(buf," ");
				tok = strtok(NULL," \n");
				if(put(cfd,tok,clie)!=0){
					return NULL;
				}
			}
			else if(strstr(buf,"get") == buf){
				char *tok;
				tok = strtok(buf," ");
				tok = strtok(NULL," \n");
				if(get(cfd,tok,clie)!=0){
					return NULL;
				}
			}
			else if(strstr(buf,"quit") == buf){
				log_write("IP:%s PORT:%d DISCONNECT!\n",\
					inet_ntoa(clie.sin_addr),ntohs(clie.sin_port));
				return NULL;
			}
		}
	}
}