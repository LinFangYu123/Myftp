#ifndef FTP_H
#define FTP_H
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

int get(int cfd,char *filename,struct sockaddr_in clie);

int put(int cfd,char *filename,struct sockaddr_in clie);

#endif