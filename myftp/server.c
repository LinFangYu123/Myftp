<<<<<<< HEAD
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include "logc.h"

#define SIZE_BUF 1024

void Dirlist(int fd);

void Dircd(char *path,int fd,struct sockaddr_in clie);

void get(int cfd,char *filename,struct sockaddr_in clie);

void put(int cfd,char *filename,struct sockaddr_in clie);

=======

#include "logc.h"
#include "ftp.h"

#define SIZE_BUF 1024

>>>>>>> FTP
void catch_sig(int num){
	pid_t wpid;
   	if((wpid = waitpid(0,NULL,WNOHANG)) == -1)
   		log_write("waitpid err");
}

int main(){
	
	struct sigaction act;
	log_create("ftplog.txt");
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	act.sa_handler = catch_sig;
	sigaction(SIGCHLD, &act,NULL);
	
	pid_t childpid;
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
	
	while(1){
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
		log_write("IP：%s,PORT:%d CONNECT!\n",\
			inet_ntoa(clie.sin_addr),ntohs(clie.sin_port));
			
		if((childpid = fork()) == 0){
			close(sfd);
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
					exit(0);
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
					else if(strstr(buf,"put") == buf){
						char *tok;
						tok = strtok(buf," ");
						tok = strtok(NULL," \n");
						put(cfd,tok,clie);
					}
					else if(strstr(buf,"get") == buf){
						char *tok;
						tok = strtok(buf," ");
						tok = strtok(NULL," \n");
						get(cfd,tok,clie);
					}
					else if(strstr(buf,"pwd") == buf){
						memset(buf,0,sizeof(buf));
						getcwd(buf,sizeof(buf));
						send(cfd,buf,sizeof(buf),0);
					}
				}
			}
		}
		else{
			close(cfd);
		}
	}
	if(childpid > 0){
		close(sfd);
		log_close();
	}
		
}

<<<<<<< HEAD
void Dirlist(int fd){
	FILE *in;
	char filename[SIZE_BUF];
	if((in = popen("ls", "r"))!=NULL){
		while(fgets(filename, sizeof(filename), in)!=NULL){
			send(fd,filename,sizeof(filename),0);
		}
	}
	send(fd,"end\n",sizeof(filename),0);
	fclose(in);
}

void Dircd(char *path,int fd,struct sockaddr_in clie){
	if(chdir(path)<0){
		log_write("IP:%s PORT:%d CHANGE FILE DIRECTORY FAIL!\n",\
				inet_ntoa(clie.sin_addr),ntohs(clie.sin_port));
		send(fd,"SERVER:CHANGE FILE DIRECTORY FAIL!",\
			SIZE_BUF,0);
	}
	else{ 
		log_write("IP:%s PORT:%d CHANGE FILE DIRECTORY SUCCEED!\n",\
				inet_ntoa(clie.sin_addr),ntohs(clie.sin_port));	
		send(fd,"SERVER:CHANGE FILE DIRECTORY SUCCEED!",\
			SIZE_BUF,0);
	}
	send(fd,"end\n",SIZE_BUF,0);
}

void get(int cfd,char *filename,struct sockaddr_in clie){
	int len;
	char buf[SIZE_BUF];
	FILE* fd ;
	if((fd = fopen(filename,"r"))!=NULL){	
		send(cfd,"OK",strlen("OK"),0);
		memset(buf,0,sizeof(buf));
		if(len = recv(cfd,buf,sizeof(buf),0)>0){
			if(strstr(buf,"FAIL")==buf){
				log_write("IP:%s PORT:%d CLIENT:%s\n",\
					inet_ntoa(clie.sin_addr),ntohs(clie.sin_port),buf);
			}
			else{
				char *filebuf;
				
				fseek(fd,0,SEEK_END);
				int filesize = ftell(fd);
				rewind(fd);
				
				filebuf = (char *)malloc(filesize);
				memset(buf,0,sizeof(buf));
				sprintf(buf,"%d",filesize);
				send(cfd,buf,sizeof(buf),0);
				
				fread(filebuf,sizeof(char),filesize,fd);
				send(cfd,filebuf,filesize,0);
				free(filebuf);
			}
		}
		else if(len <0){
			fclose(fd);
			close(cfd);
			log_write("IP:%s PORT:%d Error:recv!",\
					inet_ntoa(clie.sin_addr),ntohs(clie.sin_port));
			exit(1);				
		}
		else if(len == 0){
			
			log_write("IP:%s PORT:%d DISCONNECT！",\
					inet_ntoa(clie.sin_addr),ntohs(clie.sin_port));
			fclose(fd);
			close(cfd);
			exit(0);
		}
		fclose(fd);
	}
	else{
		send(cfd,"FILE IS NOT EXIST!",\
				SIZE_BUF,0);
	}
}

void put(int cfd,char *filename,struct sockaddr_in clie){
	char buf[SIZE_BUF];
	int len;
	memset(buf,0,sizeof(buf));
	if(len = recv(cfd,buf,sizeof(buf),0)>0){
		if(strstr(buf,"NOT") == buf){
			log_write("IP:%s,PORT:%d CLIENT:%s\n",\
					inet_ntoa(clie.sin_addr),ntohs(clie.sin_port),buf);
		}
		else{
			FILE *fd;
			if((fd = fopen(filename,"w")) != NULL){
				send(cfd,"OK",strlen("OK"),0);
				fseek(fd,0,SEEK_SET);
				char *filebuf;
				char file_size[SIZE_BUF];
				memset(file_size,0,sizeof(file_size));
				recv(cfd,file_size,sizeof(file_size),0);
				filebuf = (char *)malloc(atoi(file_size));
				if((len = recv(cfd,filebuf,atoi(file_size),0))>0){
					fwrite(filebuf,sizeof(char),atoi(file_size),fd);
				}
				else if(len < 0){
					log_write("IP:%s,PORT:%d Error recv!",\
							inet_ntoa(clie.sin_addr),ntohs(clie.sin_port));
					fclose(fd);
					close(cfd);
					exit(1);
				}
				else if (len == 0){
					log_write("IP:%s,PORT:%d DISCONNECT！\n",\
						inet_ntoa(clie.sin_addr),ntohs(clie.sin_port));
					fclose(fd);
					close(cfd);
					exit(1);
				}
				free(filebuf);
				fclose(fd);
			}
			else {
				log_write("IP:%s,PORT:%d OPEN FILE FAIL!",\
							inet_ntoa(clie.sin_addr),ntohs(clie.sin_port));
				send(cfd,"OPEN FILE FAIL!",SIZE_BUF,0);
				fclose(fd);
			}
		}
	}
	else if(len < 0){
		log_write("IP:%s,PORT:%d Error recv!",\
				inet_ntoa(clie.sin_addr),ntohs(clie.sin_port));
		exit(1);
	}
	else if (len == 0){
		log_write("IP:%s,PORT:%d DISCONNECT！\n",\
			inet_ntoa(clie.sin_addr),ntohs(clie.sin_port));
		exit(1);
	}
}
=======

>>>>>>> FTP
