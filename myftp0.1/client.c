#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#define SIZE_BUF 1024
int sfd;
void Readfile();
void Chdir(char *path);
void cd_ls(int fd,struct sockaddr_in serv);
void get(int sfd,char *filename,struct sockaddr_in serv);
void put(int sfd,char *filename,struct sockaddr_in serv);

void Stop(int signo){
	send(sfd,"quit",SIZE_BUF,0);
	usleep(100);
	printf("\n");
	exit(0);
	//printf("myftp>");
}
int main(int argc,char *argv[]){
	if(argc<3){
		printf("./a.out ip_addr port \n");
		exit(1);
	}
	int port = atoi(argv[2]);
	signal(SIGINT, Stop); 
	sfd = socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in serv;
	serv.sin_port = htons(port);
	serv.sin_family = AF_INET;
	inet_pton(AF_INET,argv[1],&serv.sin_addr.s_addr);

	socklen_t servlen = sizeof(serv); 

	if(connect(sfd,(struct sockaddr *)&serv,servlen)!=0){
		perror("connect err\n");
		exit(1);
	}

	while(1){
		char buf[SIZE_BUF];
		int len;
		memset(buf,0,sizeof(buf));
		printf("myftp>");
		fgets(buf,sizeof(buf),stdin);

		if(strstr(buf,"quit")==buf){
			send(sfd,buf,sizeof(buf),0);
			usleep(100);
			break;
		}
		else if(strstr(buf,"lcd")==buf){
			char *tok;
			tok = strtok(buf," ");
			tok = strtok(NULL," \n");
			Chdir(tok);
			printf("\n");
		}
		else if(strstr(buf,"lls")==buf){
			Readfile();	
		}
		else if(strstr(buf,"lpwd")==buf){
			memset(buf,0,sizeof(buf));
			getcwd(buf,sizeof(buf));
			printf("%s\n",buf);
			
		}
		else if(strstr(buf,"pwd")==buf){
			send(sfd,buf,sizeof(buf),0);
			memset(buf,0,sizeof(buf));
			recv(sfd, buf, sizeof(buf),0);
			printf("%s\n",buf);
		}
		else{
			if(strstr(buf,"put")==buf){
				send(sfd,buf,sizeof(buf),0);
				char *tok;
				tok = strtok(buf," ");
				tok = strtok(NULL," \n");
				put(sfd,tok,serv);
			}
			else if(strstr(buf,"get")==buf){
				send(sfd,buf,sizeof(buf),0);
				char *tok;
				tok = strtok(buf," ");
				tok = strtok(NULL," \n");
				get(sfd,tok,serv);
			}
			else if((strstr(buf,"cd")==buf) ||(strstr(buf,"ls")==buf)){
				send(sfd,buf,sizeof(buf),0);
				cd_ls(sfd,serv);
			}
			else 
				printf("Error in command. Check Command\n");
		}
		printf("\n");
	}
	close(sfd);
}
void Readfile(){
	FILE *in;
	char filename[SIZE_BUF];
	if((in = popen("ls", "r"))!=NULL){
		while(fgets(filename, sizeof(filename), in)!=NULL){
			printf("%s",filename);
		}
	}
}

void Chdir(char *path){
	if(chdir(path)<0){
		printf("CHANGE CURRENT DIRECTORY FAIL!\n");
	}
	else printf("CHANGE CURRENT DIRECTORY SUCCEED!\n");
}

void cd_ls(int fd,struct sockaddr_in serv){
	char buf[SIZE_BUF];
	int len;
	while(1){
		memset(buf,0,sizeof(buf));
		len = recv(fd,buf,sizeof(buf),0);
		if(len == -1){
			printf("read err\n");
			return ;
		}
		else if (len == 0){
			printf("服务器 IP:%s,port:%d 关闭了连接！\n",\
				inet_ntoa(serv.sin_addr),ntohs(serv.sin_port));
			exit(1);
		}
		else if(len > 0){
			if(strstr(buf,"end\n") == buf){
				return ;
			}
			else {
				printf("%s",buf);
			}
		}
	}	
}

void get(int sfd,char *filename,struct sockaddr_in serv){
	int len;
	int j = 0;
	char buf[SIZE_BUF];
	
	memset(buf,0,sizeof(buf));
	if((len = recv(sfd,buf,sizeof(buf),0))>0){
		if(strstr(buf,"NOT")==buf){
			printf("SERVER:%s\n",buf);
			write(STDOUT_FILENO,buf,len);
			return ;
		}
		else{
			FILE* fd = fopen(filename,"w");
			if(fd == NULL) {
				printf("OPEN FILE FAIL!");
				send(sfd,"OPEN FILE FAIL!",\
					SIZE_BUF,0);
				fclose(fd);
				return ;
			}
			send(sfd,"OK",strlen("OK"),0);
			fseek(fd,0,SEEK_SET);
			char *filebuf;
			char file_size[SIZE_BUF];
			memset(file_size,0,sizeof(file_size));
			recv(sfd,file_size,sizeof(file_size),0);
			filebuf = (char *)malloc(atoi(file_size));
			if((len = recv(sfd,filebuf,atoi(file_size),0))>0){
				fwrite(filebuf,sizeof(char),atoi(file_size),fd);
			}
			else if(len < 0){
				perror("Error recv!");
				fclose(fd);
				close(sfd);
				exit(1);
			}
			else if (len == 0){
				printf("服务器 recv2 IP:%s,port:%d 关闭了连接！\n",\
					inet_ntoa(serv.sin_addr),ntohs(serv.sin_port));
				fclose(fd);
				close(sfd);
				exit(1);
			}
			free(filebuf);
			fclose(fd);
			printf("FILE UPLOAD DOWN!\n");
		}
	}
	else if(len < 0){
		perror("Error recv!");
		exit(1);
	}
	else if (len == 0){
		printf("服务器 recv1 IP:%s,port:%d 关闭了连接！\n",\
			inet_ntoa(serv.sin_addr),ntohs(serv.sin_port));
		exit(1);
	}
}

void put(int sfd,char *filename,struct sockaddr_in serv){
	FILE *fd;
	int len;
	char buf[SIZE_BUF];
	if((fd = fopen(filename,"r"))!=NULL){
		send(sfd,"OK",strlen("OK"),0);
		if(len = recv(sfd,buf,sizeof(buf),0)>0){
			if(strstr(buf,"FAIL") == buf){
				printf("SERVER:%s\n",buf);
				exit(0);
			}
			else{
				char *filebuf;
				
				fseek(fd,0,SEEK_END);
				int filesize = ftell(fd);
				rewind(fd);
				
				filebuf = (char *)malloc(filesize);
				memset(buf,0,sizeof(buf));
				sprintf(buf,"%d",filesize);
				send(sfd,buf,sizeof(buf),0);
				
				fread(filebuf,sizeof(char),filesize,fd);
				send(sfd,filebuf,filesize,0);
				free(filebuf);
				printf("FILE UPLOAD DOWN!\n");
			}
		}
		else if(len  == 0){
			printf("服务器 IP:%s,port:%d 关闭了连接！\n",\
				inet_ntoa(serv.sin_addr),ntohs(serv.sin_port));
			fclose(fd);
			close(sfd);
			exit(0);
		}
		else if(len < 0){
			fclose(fd);
			close(sfd);
			perror("Error recv!");
			exit(1);
		}
	}
	else {
		printf("FILE IS NOT EXIST!");
		send(sfd,"FILE IS NOT EXIST!",SIZE_BUF,0);
	}
}