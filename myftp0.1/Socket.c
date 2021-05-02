#include "Socket.h"

void perr_exit(const char *str){
    perror(str);
    exit(1);
}

int Socket(int family,int type,int protocol){
    int n;
again:
    if((n = socket(family,type,protocol)) == -1){
        if(errno == EINTR){
            goto again;
        }
        perr_exit("socket");
    }
    return n;
}

int Bind(int fd ,const struct sockaddr *sa,socklen_t sa_len){
    int n;
again:
    if((n = bind(fd,sa,sa_len)) == -1){
        if(errno == EINTR){
            goto again;
        }
        perr_exit("bind");
    }
    return n;
}

int Listen(int fd,int backlog){
    int n;
again:
    if((n = listen(fd,backlog)) == -1){
        if(errno == EINTR){
            goto again;
        }
        perr_exit("bind");
    }
    return n;
}

int Accept(int fd,struct sockaddr *sa,socklen_t *sa_lenptr){
    int n;
again:
    if((n = accept(fd,sa,sa_lenptr))<0){
        if(errno == ECONNABORTED|errno == EINTR){
            goto again;
        }
        perr_exit("accept");
    }
    return n;
}

int Connect(int fd,const struct sockaddr *sa,socklen_t sa_len){
    int n;
again:
    if((n = connect(fd,sa,sa_len)) == -1){
        if(errno == EINTR){
            goto again;
        }
        perr_exit("bind");
    }
    return n;
}

ssize_t Read(int fd,void *buf,size_t buf_size){
    int n;
again:
    if((n = read(fd,buf,buf_size)) == -1){
        if(errno == EINTR){
            goto again;
        }
        return -1;
    }
    return n;
}

ssize_t Write(int fd,const void *buf,size_t buf_size){
    int n;
again:
    if((n = write(fd,buf,buf_size)) == -1){
        if(errno == EINTR){
            goto again;
        }
        return -1;
    }
    return n;
}

ssize_t Read_one(int fd,char *buf){
    int n;
    n = Read(fd,buf,1);
    buf[1] = '\0';
    return n;
}

ssize_t Read_line(int fd,char *buf,size_t buf_size){
    int n;
    int i = 0;
    char ch;
    while(i < buf_size&&ch !='\n'){
        n = Read_one(fd,&ch);
        if(n > 1 && ch != '\n'){
            buf[i] = ch;
            i++;
        }
        else{
            buf[i] = '\0';
            break;
        }
    }
    return i;
}

ssize_t Recv(int sockfd, void *buf, size_t len, int flags){
    int n;
again:
    if((n = recv(sockfd,buf,len,flags)) == -1){
        if(errno == EINTR){
            goto again;
        }
        return -1;
    }
    return n;
}

ssize_t Send(int sockfd, const void *buf, size_t len, int flags){
    int n;
again:
    if((n = send(sockfd,buf,len,flags)) == -1){
        if(errno == EINTR){
            goto again;
        }
        return -1;
    }
    return n;
}

ssize_t Recv_one(int fd,char *buf,int flags){
    int n;
    n = Recv(fd,buf,1,flags);
    buf[1] = '\0';
    return n;
}

ssize_t Recv_line(int fd,char *buf,size_t len,int flags){
    int n;
    int i = 0;
    char ch;
    while(i < len&&ch !='\n'){
        n = Recv_one(fd,&ch,0);
        if(n > 1 && ch != '\n'){
            buf[i] = ch;
            i++;
        }
        else{
            buf[i] = '\0';
            break;
        }
    }
    return i;
}

int Close(int fd){
    int n;
again:
    n = close(fd);
    if(n == -1){
        if(errno == EINTR){
            goto again;
        }
        return -1;
    }
    return n;
}

int initTcpSocket(const int port,char *IP){
    int fd = Socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in serv;
    bzero(&serv,sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    if(IP == NULL){
        serv.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else{
        if(inet_pton(AF_INET,IP,&serv.sin_addr.s_addr) <= 0){
            perr_exit("inet_pton");
        }
    }
    
    int opt = 1;
    int ret ;
again:
    if(ret = setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt))!=0){
        if(errno == EINTR){
            goto again;
        }
        perr_exit("setsockopt");
    }

    Bind(fd,(struct sockaddr *)&serv,sizeof(serv));
    Listen(fd,128);
    return fd;  
}

int initTcpConn(const int port,char *IP){
    int fd = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in serv;
    bzero(&serv,sizeof(serv));
    serv.sin_port = htons(port);
    serv.sin_family = AF_INET;
    if(IP!=NULL){
        inet_pton(AF_INET,IP,&serv.sin_addr.s_addr);
    }
    else{
        inet_pton(AF_INET,"127.0.0.1",&serv.sin_addr.s_addr);
    }

    socklen_t servlen = sizeof(serv); 

    Connect(fd,(struct sockaddr *)&serv,servlen);
    return fd;
}