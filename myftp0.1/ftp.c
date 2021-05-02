#include "ftp.h"

void Dirlist(int fd){
	FILE *in;
	char filename[SIZE_BUF];
	if((in = popen("ls", "r"))!=NULL){						//popen()会产生一个进程 必会要有子进程回收机制
		while(fgets(filename, sizeof(filename), in)!=NULL){
			send(fd,filename,sizeof(filename),0);
		}
	}
	send(fd,"end\n",sizeof(filename),0);
	
	pclose(in);
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

int get(int cfd,char *filename,struct sockaddr_in clie){
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
			return -1;				
		}
		else if(len == 0){
			
			log_write("IP:%s PORT:%d DISCONNECT！",\
					inet_ntoa(clie.sin_addr),ntohs(clie.sin_port));
			fclose(fd);
			close(cfd);
			return -1;
		}
		fclose(fd);
	}
	else{
		send(cfd,"FILE IS NOT EXIST!",\
				SIZE_BUF,0);
	}
	return 0;
}

int put(int cfd,char *filename,struct sockaddr_in clie){
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
					return -1;
				}
				else if (len == 0){
					log_write("IP:%s,PORT:%d DISCONNECT！\n",\
						inet_ntoa(clie.sin_addr),ntohs(clie.sin_port));
					fclose(fd);
					return -1;
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
		return -1;
	}
	else if (len == 0){
		log_write("IP:%s,PORT:%d DISCONNECT！\n",\
			inet_ntoa(clie.sin_addr),ntohs(clie.sin_port));
		return -1;
	}
	return 0;
}